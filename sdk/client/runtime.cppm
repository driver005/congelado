export module congelado_client:runtime;

import std;
import interfaces;
import serde;

export namespace congelado::client {

class ClientRuntime {
  public:
    /** @brief Deleted — this is a static-only utility class, no instances allowed, no cap. */
    ClientRuntime() = delete;

    // FIXME(clang-tidy): readability-identifier-naming — is_vector_v intentionally follows the
    // standard library's is_*_v type-trait naming convention (lower_case + trailing _v), not
    // UPPER_CASE; renaming it would be inconsistent with every std:: trait it mirrors.
    template <typename T>
    static constexpr bool is_vector_v = false;  // NOLINT(readability-identifier-naming) — intentionally mirrors std::is_*_v trait naming convention

    /**
     * @brief Points the runtime at the transport that actually sends requests — gotta call this
     * once before any generated route function runs, or nothing's going anywhere.
     * @param value the client to use; the runtime keeps a non-owning pointer to it.
     */
    static void setClient(interfaces::IClient &value) noexcept { m_client = &value; }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception

    /**
     * @brief Gets the currently configured transport client, whatever's wired up.
     * @warning Aborts the process if setClient() was never called — no client means every send
     * is a guaranteed L, so this fails loud and fast instead of null-deref-ing later.
     * @return reference to the configured client.
     */
    [[nodiscard]] static interfaces::IClient &getClient() noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        // Fail loud and fast — no client means every send is a guaranteed L anyway, better to
        // abort here than null-deref somewhere downstream.
        if (m_client == nullptr) {
            std::abort();
        }
        return *m_client;
    }

    using RequestFactory =
        std::function<std::unique_ptr<interfaces::io::IRequest>(std::uint32_t)>;

    /**
     * @brief Registers the factory used to construct new protocol-specific requests — keeps the
     * runtime protocol-agnostic, big W, while callers plug in whatever IRequest impl they need.
     * @param factory callable that builds an IRequest given a stream id.
     */
    static void setRequestFactory(RequestFactory factory) noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        m_request_factory = std::move(factory);
    }

    /**
     * @brief Spins up a fresh request, auto-assigning it the next stream id — clean motion every
     * time.
     * @warning Aborts the process if setRequestFactory() was never called — same fail-fast deal
     * as getClient().
     * @return newly constructed request, ready to have method/path/body filled in.
     */
    [[nodiscard]] static std::unique_ptr<interfaces::io::IRequest> new_request() {
        // Same fail-fast deal as getClient() — no factory means there's nothing to build.
        if (!m_request_factory) {
            std::abort();
        }
        return m_request_factory(m_next_stream_id++);
    }

    // No-response overload (a bare `std::function<void(void)>` is not a well-formed
    // instantiation of the templated overload below on every standard library — kept as a
    // separate, non-template overload so operations with no response body never try to
    // instantiate send<void>). There's no body to deserialize, so success/failure is decided
    // by the response's status instead.
    /**
     * @brief Sends a request that's got no response body to deserialize — success or failure
     * comes down purely to the response's status code, then the right callback fires once
     * dispatch() matches the response back to this request's stream id.
     * @param request request to send; ownership moves into the pending-callback map until the
     * matching response arrives.
     * @param onResponse fired with no args if the response is a W.
     * @param onError fired with the response's status text if the response is an L.
     */
    static void send(std::unique_ptr<interfaces::io::IRequest> request,
                     std::function<void()> onResponse,
                     std::function<void(std::string)> onError = [](const std::string &) {}) {
        auto stream_id = request->get_stream_id();
        // Stash the callback keyed by stream id, bet — dispatch() looks it up once the matching
        // response actually shows up.
        m_pending[stream_id] = [on_response = std::move(onResponse),
                                on_error = std::move(onError)](interfaces::io::IResponse &response) {
            if (response.is_success()) {
                on_response();
            } else {
                on_error(std::string{response.get_status_text()});
            }
        };
        getClient().send(*request);
    }

    /**
     * @brief Sends a request and deserializes the response body into `Res` once it lands —
     * vector-typed responses run through Json::decode_array (since a bare std::vector<T> isn't
     * itself ISerializable, only T is), everything else takes the generic serde::Ser::deserialize
     * path. No cap, this overload does the heavy lifting.
     * @tparam Res type to deserialize the response body into.
     * @param request request to send; ownership moves into the pending-callback map until the
     * matching response arrives.
     * @param onResponse fired with the deserialized value if the response's a W and decodes clean.
     * @param onError fired with an error message if the response's an L or fails to decode.
     */
    template <typename Res>
    static void send(std::unique_ptr<interfaces::io::IRequest> request,
                     std::function<void(Res)> onResponse,
                     std::function<void(std::string)> onError = [](const std::string &) {}) {
        auto stream_id = request->get_stream_id();
        // Same pending-callback stash as the non-template overload, but this callback actually
        // decodes the body before firing onResponse.
        m_pending[stream_id] = [on_response = std::move(onResponse),
                                on_error = std::move(onError)](interfaces::io::IResponse &response) {
            auto body_bytes = response.get_body();
            std::string body(reinterpret_cast<const char *>(body_bytes.data()), body_bytes.size());  // FIXME(clang-tidy): reinterpret_cast usage
            // serde::Ser::deserialize<T> requires T itself to be ISerializable — a bare
            // std::vector<T> never is (only its element type is), so array-typed responses
            // (e.g. a "list" endpoint) go through Json::decode_array<T> instead.
            if constexpr (is_vector_v<Res>) {
                auto result = serde::Json::decode_array<typename Res::value_type>(body);
                if (result) {
                    on_response(std::move(*result));
                } else {
                    on_error(result.error());
                }
            } else {
                auto result = serde::Ser::deserialize<Res>(response.get_content_type(), body);
                if (result) {
                    on_response(std::move(*result));
                } else {
                    on_error(result.error());
                }
            }
        };
        getClient().send(*request);
    }

    /**
     * @brief Matches an incoming response back to the send() call that's waiting on its stream
     * id and fires the stored callback — this is what the transport layer calls the second a
     * response shows up. A response with no matching pending callback (already dispatched, or
     * never registered) just gets dropped, no drama.
     * @param request the original request, used only to read its stream id.
     * @param response the response that arrived; handed straight to the stored callback.
     */
    static void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response) {
        // No matching pending callback — already dispatched, or never registered — just drop
        // it, no drama.
        auto it = m_pending.find(request.get_stream_id());
        if (it == m_pending.end()) {
            return;
        }
        // Pull the callback out and erase before invoking it, so a re-entrant send() during the
        // callback can't collide with this same slot.
        auto callback = std::move(it->second);
        m_pending.erase(it);
        callback(response);
    }

  private:
    static inline interfaces::IClient *m_client = nullptr;
    static inline RequestFactory m_request_factory;
    static inline std::unordered_map<std::uint32_t,
                                     std::function<void(interfaces::io::IResponse &)>> m_pending;
    static inline std::uint32_t m_next_stream_id{1};
};

// FIXME(clang-tidy): readability-identifier-naming — same is_vector_v naming call as above;
// this is the specialization of that same std::is_*_v-style trait, kept consistent with it.
template <typename T>
constexpr bool ClientRuntime::is_vector_v<std::vector<T>> = true;  // NOLINT(readability-identifier-naming) — intentionally mirrors std::is_*_v trait naming convention

} // namespace congelado::client
