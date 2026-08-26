export module core_client:builder;

import std;
import interfaces;
import utils_buffering;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::client {

// Fluent HTTP-verb request builder. Protocol-agnostic by construction: it never names a
// concrete request type itself (interfaces::io::IRequest's base-class virtuals all abort —
// it's a CRTP base, not directly usable). Building the real concrete request is the
// runtime IClient's own job — see IClient::create_request() — so send() asks the client
// wired in via with_runtime() to build one rather than needing to know its type.
class Client
{
public:
    /**
     * @brief Deleted — no bare Client allowed, every instance has to come from one of the
     * static factories (get()/post()/etc) below so method+path are always set together.
     */
    Client() = delete;
    /**
     * @brief Defaulted dtor, nothing extra to tear down here.
     */
    ~Client() = default;

    /**
     * @brief Deleted — Client holds a `reference_wrapper` onto the runtime IClient, copying
     * that reference around is straight up the wrong move, so copying's off the table.
     */
    Client(const Client&) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor right above.
     */
    Client& operator=(const Client&) = delete;

    /**
     * @brief Defaulted move ctor — moving a Client around is all good, it's only copying
     * that's banned.
     */
    Client(Client&&) = default;
    /**
     * @brief Defaulted move assign, same deal as the move ctor.
     */
    Client& operator=(Client&&) = default;

    /**
     * @brief Spins up a fresh GET Client pointed at `path`. Bet, straightforward one.
     * @param path the request path/target.
     * @return a Client pre-loaded with the GET method and `path`.
     */
    static Client get(std::string_view path)
    {
        return {"GET", path};
    }

    /**
     * @brief Spins up a fresh HEAD Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the HEAD method and `path`.
     */
    static Client head(std::string_view path)
    {
        return {"HEAD", path};
    }

    /**
     * @brief Spins up a fresh POST Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the POST method and `path`.
     */
    static Client post(std::string_view path)
    {
        return {"POST", path};
    }

    /**
     * @brief Spins up a fresh PUT Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the PUT method and `path`.
     */
    static Client put(std::string_view path)
    {
        return {"PUT", path};
    }

    /**
     * @brief Spins up a fresh DELETE Client pointed at `path`. That resource's cooked once
     * this actually gets sent.
     * @param path the request path/target.
     * @return a Client pre-loaded with the DELETE method and `path`.
     */
    static Client del(std::string_view path)
    {
        return {"DELETE", path};
    }

    /**
     * @brief Spins up a fresh PATCH Client pointed at `path`.
     * @param path the request path/target.
     * @return a Client pre-loaded with the PATCH method and `path`.
     */
    static Client patch(std::string_view path)
    {
        return {"PATCH", path};
    }

    /**
     * @brief Spins up a fresh OPTIONS Client pointed at `path`. Last one of the crew, same
     * pattern all the way down.
     * @param path the request path/target.
     * @return a Client pre-loaded with the OPTIONS method and `path`.
     */
    static Client options(std::string_view path)
    {
        return {"OPTIONS", path};
    }

    /**
     * @brief Spins up a fresh Client for an arbitrary method string — the escape hatch for
     * runtime-determined verbs the fixed get()/post()/etc. factories above don't cover.
     * @param method the HTTP method string, verbatim.
     * @param path the request path/target.
     * @return a Client pre-loaded with `method` and `path`.
     */
    static Client custom(std::string_view method, std::string_view path)
    {
        return {method, path};
    }

    /**
     * @brief Builder chain — wires this Client up to the runtime IClient that'll actually ship
     * the request out.
     * @param client the runtime client to bind, kept as a reference so no ownership motion
     * happens here.
     * @return `*this`, moved, so the chain keeps going.
     */
    Client&& with_runtime(interfaces::IClient& client) &&
    {
        m_client = client;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the stream id the eventually-built request gets tagged
     * with, and that response correlation on the caller's side keys off of.
     * @param stream_id the stream id to use.
     * @return `*this`, moved, so the chain keeps going.
     */
    Client&& with_stream_id(std::uint32_t stream_id) && noexcept
    {
        m_stream_id = stream_id;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the callback that fires whenever data comes back in on this
     * request.
     * @param func the receive-dispatch callback to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    Client&& on_receive(interfaces::io::ReceiveDispatchFn&& func) &&
    {
        m_receive_dispatch_fn = std::move(func);
        return std::move(*this);
    }

    /**
     * @brief In-place version of with_runtime() — mutates `*this` directly instead of chaining
     * through a moved rvalue.
     * @param client the runtime client to bind.
     */
    void set_runtime(interfaces::IClient& client)
    {
        m_client = client;
    }

    /**
     * @brief In-place version of with_stream_id().
     * @param stream_id the stream id to use.
     */
    void set_stream_id(std::uint32_t stream_id) noexcept
    {
        m_stream_id = stream_id;
    }

    /**
     * @brief In-place version of on_receive().
     * @param func the receive-dispatch callback to install.
     */
    void set_on_receive(interfaces::io::ReceiveDispatchFn&& func)
    {
        m_receive_dispatch_fn = std::move(func);
    }

    /**
     * @brief Buffers a header by string name — applied onto the real request once send()
     * builds it, since there's no concrete request to stamp it onto yet.
     * @param key the header name.
     * @param value the header value.
     */
    void add_header(std::string_view key, std::string_view value)
    {
        m_headers.emplace_back(std::string{key}, std::string{value});
    }

    /**
     * @brief Buffers a header by interned `Token` — same deal as the string-name overload,
     * just the faster-lookup token path.
     * @param token the header's token.
     * @param value the header value.
     */
    void add_header(interfaces::io::types::Token token, std::string_view value)
    {
        m_headers.emplace_back(token, std::string{value});
    }

    /**
     * @brief Buffers bytes to append onto the request body once send() builds the real
     * request — same reasoning as add_header(), nothing to write onto yet.
     * @param body the bytes to append.
     */
    void add_body(std::string_view body)
    {
        m_body.append(body);
    }

    /**
     * @brief Asks `client` to build the real concrete request (via IClient::create_request(),
     * so this class never has to name the concrete type itself) and stamps
     * method/path/headers/body onto it. Does NOT send — hand the result to `Register::send()`
     * for correlated dispatch, or call `send()` for fire-and-forget.
     * @param client the runtime client whose concrete request type gets built.
     * @return the fully-stamped request, ready to send.
     */
    [[nodiscard]] std::unique_ptr<interfaces::io::IRequest> build(interfaces::IClient& client) const
    {
        auto request = client.create_request(m_stream_id);
        request->set_header(interfaces::io::types::Token::METHOD, m_method);
        request->set_header(interfaces::io::types::Token::PATH, m_path);
        for (const auto& [name_or_token, value]: m_headers) {
            std::visit(
                [&](const auto& name) {
                    request->set_header(name, value);
                },
                name_or_token
            );
        }

        if (!m_body.empty()) {
            // Matches the buffering pattern already used at every other call_engine()-style
            // call site in this codebase (see the worker runtime's WorkerContext).
            // FIXME(clang-tidy): cppcoreguidelines-owning-memory — would need
            // gsl::owner<BufferNode *>, but this codebase has no GSL dependency; the buffering
            // subsystem's push_back()/acquire() are all noexcept, raw-pointer, terminate-on-OOM
            // by convention.
            auto* node = new utils::buffering::BufferNode(
                m_body.size()
            ); // NOLINT(cppcoreguidelines-owning-memory)
            for (char character: m_body) {
                node->push_back(static_cast<std::byte>(character));
            }
            request->get_body().push_back(node, 0, m_body.size());
        }
        return request;
    }

    /**
     * @brief Fire-and-forget send — builds the real request and ships it, with no response
     * correlation. Use `Register::send()` when the response needs to be routed back.
     * @throws std::runtime_error if no runtime client was ever set.
     */
    void send()
    {
        if (!m_client.has_value()) {
            throw std::runtime_error("Please set runtime first");
        }
        auto& client = m_client.value().get();
        auto request = build(client);
        client.send(*request);
    }

private:
    /**
     * @brief Real ctor behind every static factory — stashes method and path for send() to
     * stamp onto the real request once it's actually built.
     * @param method the HTTP method string (GET/POST/etc).
     * @param path the request path/target.
     */
    Client(std::string_view method, std::string_view path) :
        m_method{method},
        m_path{path}
    {
    }

    std::string m_method;
    std::string m_path;
    std::vector<std::pair<std::variant<std::string, interfaces::io::types::Token>, std::string>>
        m_headers;
    std::string m_body;
    interfaces::io::ReceiveDispatchFn m_receive_dispatch_fn;
    std::optional<std::reference_wrapper<interfaces::IClient>> m_client;
    std::uint32_t m_stream_id{0};
};

} // namespace core::client

// build()/send() need a real interfaces::IClient whose create_request() hands back a fully
// working concrete IRequest — IRequest's own header/body virtuals abort by default (see
// interfaces/io/request.cppm) unless overridden by a protocol implementation like http2's, which
// needs a live socket/session. Client has no public getters either, so the only thing observable
// here without one is send()'s own "no runtime bound" guard.
#ifdef CONGELADO_TEST
namespace core::client::tests {
using namespace boost::ut;

suite<"Client"> client_suite = [] {
    "send() throws when no runtime has been bound"_test = [] {
        Client client = Client::get("/foo");
        expect(throws<std::runtime_error>([&] {
            client.send();
        }));
    };

    "every verb factory produces a usable Client that still needs a runtime to send"_test = [] {
        expect(throws<std::runtime_error>([] {
            Client::post("/x").send();
        }));
        expect(throws<std::runtime_error>([] {
            Client::put("/x").send();
        }));
        expect(throws<std::runtime_error>([] {
            Client::del("/x").send();
        }));
        expect(throws<std::runtime_error>([] {
            Client::patch("/x").send();
        }));
        expect(throws<std::runtime_error>([] {
            Client::head("/x").send();
        }));
        expect(throws<std::runtime_error>([] {
            Client::options("/x").send();
        }));
        expect(throws<std::runtime_error>([] {
            Client::custom("TRACE", "/x").send();
        }));
    };
};

} // namespace core::client::tests
#endif
