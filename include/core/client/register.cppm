// Partition named `registry` (not `register`) — `register` is a reserved C++ keyword, and the
// module-partition scanner parses `:register` to an empty partition name, breaking the import.
export module core_client:registry;

import std;
import interfaces;
import core_otel;
#ifdef CONGELADO_TEST
import boost.ut;
import shared;
#endif

export namespace core::client {

/**
 * @brief Client-side request→response correlator with built-in tracing. Owns the
 * `stream_id → pending-callback` map: `send()` ships a request and remembers which callback the
 * response belongs to (keyed on the transport-assigned stream id), `dispatch()` matches an incoming
 * response back to it. Every send starts a CLIENT span (with `traceparent` propagation) that's ended
 * when the matching response lands.
 */
class Register {
  public:
    /**
     * @brief Binds the runtime client that actually ships requests and assigns their stream ids.
     * @param client the runtime client, kept by reference (must outlive this Register).
     */
    void set_runtime(interfaces::IClient &client) { m_runtime = client; }

    /// @brief Whether a runtime client has been bound yet. @return true once set_runtime ran.
    [[nodiscard]] bool has_runtime() const noexcept { return m_runtime.has_value(); }

    /// @brief The bound runtime client — for callers that need it to build a request off the same
    /// client this Register sends through (e.g. core::client::Client::build).
    /// @return the runtime IClient. @warning UB if has_runtime() is false.
    [[nodiscard]] interfaces::IClient &runtime() const { return m_runtime.value().get(); }

    /**
     * @brief Ships `request` and registers `on_response` under the transport-assigned stream id.
     * Starts a CLIENT span named `"<method> <path>"`, injects `traceparent`, and records the
     * response status on that span when it lands.
     * @param request the request to send (already built, e.g. via `Client::build()`).
     * @param on_response fired with the response once `dispatch()` matches it back.
     * @return the transport-assigned stream id the response will be correlated on.
     * @throws std::runtime_error if no runtime was set.
     */
    std::uint32_t send(std::unique_ptr<interfaces::io::IRequest> request,
                       std::move_only_function<void(interfaces::io::IResponse &)> on_response) {
        if (!m_runtime.has_value()) {
            throw std::runtime_error("core::client::Register: no runtime set");
        }

        const auto METHOD = request->find_header(interfaces::io::types::Token::METHOD);
        const auto PATH = request->find_header(interfaces::io::types::Token::PATH);
        auto span = core::otel::start_detached_span(std::format("{} {}", METHOD, PATH),
                                                    interfaces::SpanKind::CLIENT);
        span.set_attribute("http.request.method", METHOD);
        span.set_attribute("url.path", PATH);
        request->set_header("traceparent", core::otel::format_traceparent(span.context()));

        // Send first — the transport assigns the real stream id and hands it back; that's the key
        // dispatch() correlates on (the pre-send id gets overwritten by the session).
        const auto ASSIGNED = m_runtime->get().send(*request);

        m_pending[ASSIGNED] = [span = std::move(span), on_response = std::move(on_response)](
                                  interfaces::io::IResponse &response) mutable {
            const auto CODE = interfaces::io::types::status_code(response.get_status());
            span.set_attribute("http.response.status_code", static_cast<std::int64_t>(CODE));
            span.set_status(response.is_success() ? interfaces::SpanStatus::OK
                                                  : interfaces::SpanStatus::ERROR,
                            "");
            span.end();
            on_response(response);
        };
        return ASSIGNED;
    }

    /**
     * @brief Matches an incoming response back to the `send()` waiting on its stream id and fires
     * the stored callback. A response with no matching pending callback is dropped, no drama.
     * @param request the request the response arrived for — read for its stream id.
     * @param response the response to hand to the stored callback.
     */
    void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response) {
        auto it = m_pending.find(request.get_stream_id());
        if (it == m_pending.end()) {
            return;
        }
        // Move + erase before invoking so a re-entrant send() in the callback can't collide.
        auto callback = std::move(it->second);
        m_pending.erase(it);
        callback(response);
    }

    /**
     * @brief Builds the `ReceiveDispatchFn` to wire into the transport — every response it delivers
     * routes into `dispatch()`.
     * @return a receive-dispatch callback bound to this Register.
     */
    [[nodiscard]] interfaces::io::ReceiveDispatchFn make_dispatch() {
        return [this](interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                      std::function<void()> /*send*/) { dispatch(req, res); };
    }

  private:
    std::optional<std::reference_wrapper<interfaces::IClient>> m_runtime;
    std::unordered_map<std::uint32_t,
                       std::move_only_function<void(interfaces::io::IResponse &)>>
        m_pending;
};

} // namespace core::client

// send()'s success path needs a live interfaces::IClient whose create_request() hands back a
// fully working concrete IRequest (find_header()/set_header() abort on the base IRequest unless
// overridden by a protocol implementation like http2's, which needs a live socket/session) — only
// the runtime-binding and no-match dispatch bookkeeping are covered below. The fake client only
// implements the bare minimum to be constructible; none of its methods are actually invoked.
#ifdef CONGELADO_TEST
namespace core::client::tests {
using namespace boost::ut;

class RegisterFakeClient : public interfaces::IClient {
  public:
    shared::ReadCallback on_connect(shared::SendCallback, shared::CloseCallback) override {
        return {};
    }
    std::uint32_t send(interfaces::io::IRequest &) override { return 0; }
    [[nodiscard]] std::unique_ptr<interfaces::io::IRequest>
    create_request(std::uint32_t stream_id) override {
        return std::make_unique<interfaces::io::IRequest>(stream_id);
    }
};

suite<"Register"> register_suite = [] {
    "starts with no runtime bound"_test = [] {
        Register registry;
        expect(not registry.has_runtime());
    };

    "set_runtime binds a runtime client"_test = [] {
        Register registry;
        RegisterFakeClient client;
        registry.set_runtime(client);
        expect(registry.has_runtime());
        expect(std::addressof(registry.runtime()) == std::addressof(client));
    };

    "send throws when no runtime has been bound"_test = [] {
        Register registry;
        auto request = std::make_unique<interfaces::io::IRequest>(1);
        expect(throws<std::runtime_error>([&] {
            registry.send(std::move(request), [](interfaces::io::IResponse &) {});
        }));
    };

    "dispatch drops a response with no matching pending callback"_test = [] {
        Register registry;
        interfaces::io::IRequest request{42};
        interfaces::io::IResponse response{42};
        registry.dispatch(request, response);
    };

    "make_dispatch returns a callback wired into dispatch"_test = [] {
        Register registry;
        auto dispatch_fn = registry.make_dispatch();
        interfaces::io::IRequest request{7};
        interfaces::io::IResponse response{7};
        dispatch_fn(request, response, [] {});
    };
};

} // namespace core::client::tests
#endif
