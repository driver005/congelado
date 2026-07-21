export module io_layer_http2:plugin;

import std;
import shared;
import interfaces;
import core_config;
import core_router;
import core_logger;
import io_shared;
import :flow;
import :request;

export namespace io::layer::http2 {

class Client final : public interfaces::IClient {
  public:
    /**
     * @brief Builds an HTTP/2 client, no `ClientFlow` yet — that only spins up once
     * on_connect() actually fires.
     * @param dispatch the dispatch fn forwarded into `ClientFlow` on connect, hands received
     * request/response pairs off to whoever's listening.
     */
    Client(interfaces::io::ReceiveDispatchFn &&dispatch)
        : m_flow{nullptr}, m_dispatch{std::move(dispatch)} {}

    /**
     * @brief Defaulted override, nothing extra to tear down here — `m_flow`'s a `unique_ptr`,
     * cleans itself up fine.
     */
    ~Client() override = default;

    /**
     * @brief Deleted — copying a client would double-own the underlying flow/dispatch state,
     * not happening.
     */
    Client(const Client &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor right above.
     */
    Client &operator=(const Client &) = delete;

    /**
     * @brief Deleted too — even moving isn't wired up here despite `IClient` allowing it, this
     * concrete type opts out of both.
     */
    Client(Client &&) = delete;
    /**
     * @brief Deleted, matches the move ctor right above.
     */
    Client &operator=(Client &&) = delete;

    /**
     * @brief Spins up the `ClientFlow` for this connection and kicks off its handshake.
     * @note Matches the `IClient` contract exactly — wires `send`/`close` into a fresh
     * `ClientFlow`, then calls the flow's on-connect callback and returns whatever read
     * callback comes back out of it.
     * @param send callback the flow uses to push bytes out.
     * @param close callback the flow uses to tear the connection down.
     * @return the read callback to invoke for every subsequent chunk of incoming bytes.
     */
    [[nodiscard]] ::shared::ReadCallback on_connect(::shared::SendCallback send,
                                                    ::shared::CloseCallback close) override {
        // Flow only gets built once we actually have send/close callbacks to wire it up with.
        m_flow = std::make_unique<ClientFlow>(std::move(send), std::move(close), m_dispatch);

        // on_connect() hands back a callback that itself runs the handshake and returns the
        // steady-state read callback — invoke it right away, no lazy deferral here.
        auto connector = m_flow->on_connect();

        return connector();
    }

    /**
     * @brief Forwards `req` into the flow's sender, casting it down to the concrete
     * `HttpRequest` type this protocol actually deals in.
     * @warning Pass in anything that isn't a real `HttpRequest` under the hood and
     * `dynamic_cast` throws `std::bad_cast` — caught right here and just logged, not
     * re-thrown, so the request silently never gets sent instead of blowing up the caller. Easy
     * footgun if some other `IRequest` implementation ever gets routed to this client by
     * mistake.
     * @param req the request to send — must actually be an `HttpRequest` at runtime.
     */
    // Please pass in a HttpRequest object. Else this function will throw a std::bad_cast exception.
    void send(interfaces::io::IRequest &req) override {
        try {
            // Downcast to the concrete request type this protocol actually knows how to frame.
            auto &http_request = dynamic_cast<HttpRequest &>(req);

            m_flow->sender(http_request);
            return;
        } catch (const std::bad_cast &e) {
            // Wrong IRequest type got routed here, no cap — log it and drop the request instead
            // of propagating the exception up to the caller.
            core::logger::error("http2", "Failed to cast IRequest to HttpRequest: {}", e.what());
        }
    }

  private:
    std::unique_ptr<ClientFlow> m_flow;
    interfaces::io::ReceiveDispatchFn m_dispatch;
};

class Server {
  public:
    /**
     * @brief Defaulted ctor — server starts with no route table and no flows yet, both get
     * wired in later via build().
     */
    Server() = default;
    /**
     * @brief Defaulted dtor — `m_flows` are all `unique_ptr`s, clean up on their own.
     */
    ~Server() = default;

    /**
     * @brief Deleted — copying a server would duplicate every live connection's flow state,
     * not a sane operation.
     */
    Server(const Server &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor right above.
     */
    Server &operator=(const Server &) = delete;

    /**
     * @brief Deleted too — moving isn't wired up for this type either.
     */
    Server(Server &&) = delete;
    /**
     * @brief Deleted, matches the move ctor right above.
     */
    Server &operator=(Server &&) = delete;

    /**
     * @brief Builds the route table from `router_ctx` and wires up the request/response
     * dispatch lambda every future `ServerFlow` gets handed.
     * @warning `router_ctx` gets `static_cast` from `void *` to `core::router::RouterContext<>
     * *` with zero runtime type checking — pass the wrong thing here and it's straight UB, no
     * safety net, no cap. This satisfies the `ServerConcept` requirement from
     * `interfaces::protocol.cppm` (`build(router_ctx)` returning void), that's the whole reason
     * the param's untyped `void *` in the first place.
     * @param router_ctx must actually point at a `core::router::RouterContext<>` — no way to
     * verify that from in here.
     */
    void build(void *router_ctx) {
        // Untyped void* recovered back to the real router context type — no runtime check possible.
        auto *router = static_cast<core::router::RouterContext<> *>(router_ctx);
        // Build the actual route table from the router context, once, up front.
        m_server.emplace(core::router::RouteBuilder{}.build(std::move(*router)));
        // Every ServerFlow spun up later shares this same dispatch closure.
        m_dispatch = [this](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
            // Guard — no route table means nothing to match against, just no-op.
            if (!m_server) {
                return;
            }

            auto method = interfaces::io::types::parse_method(req.get_method());

            // Try to match the request against the route table; an unmatched route surfaces as
            // a runtime_error, which we translate into a plain 404 instead of letting it escape.
            try {
                m_server->match(method, req.get_path(), req, res);
            } catch (const std::runtime_error &) {
                res.set_status(interfaces::io::types::Status::NOT_FOUND);
            }
        };
    }

    /**
     * @brief Spins up a fresh `ServerFlow` for a newly-accepted connection, stores it so it
     * outlives this call, and hands back its read callback.
     * @note This is the `ServerConcept`-required `on_connect` (see `interfaces::protocol.cppm`),
     * not an `override` — `Server` satisfies the concept structurally, it doesn't inherit from
     * anything.
     * @param send callback the flow uses to push bytes out for this connection.
     * @param close callback the flow uses to tear this connection down.
     * @return the read callback to invoke for every subsequent chunk of incoming bytes on this
     * connection.
     */
    [[nodiscard]] ::shared::ReadCallback on_connect(::shared::SendCallback send,
                                                    ::shared::CloseCallback close) {
        return m_flows
            .emplace_back(
                std::make_unique<ServerFlow>(std::move(send), std::move(close), m_dispatch))
            ->on_read();
    }

  private:
    std::vector<std::unique_ptr<ServerFlow>> m_flows;
    interfaces::io::ReceiveDispatchFn m_dispatch;
    std::optional<core::router::RouteHandler<>> m_server;
};

// HTTP/2 protocol implementation.
// Handles per-connection ServerFlow creation and request dispatch.
// Transport binding (socket, thread pool) is owned by the plugin (http2.cc).
class Http2Protocol final : public interfaces::IProtocol<Server> {
  public:
    /**
     * @brief Pulls host/cert/key/port/threads straight out of `cfg`'s field map — every one of
     * them is mandatory, this bails the moment any single field's missing.
     * @warning No default fallback for any field despite `cfg` itself defaulting to `nullptr`
     * in the signature — pass `nullptr` (or a config missing even one field) and this throws
     * immediately. That optional-looking default parameter is a bit of a trap, don't read it as
     * "config's optional."
     * @param cfg the plugin config to read `host`/`cert`/`key`/`port`/`threads` from.
     * @throws std::runtime_error if `cfg` is `nullptr`, or if any required field
     * (host/cert/key/port/threads) is missing/empty.
     */
    explicit Http2Protocol(const core::config::PluginConfig *cfg = nullptr) {
        // Guard — the "optional-looking" default param is a trap, nullptr always throws.
        if (cfg == nullptr) {
            throw std::runtime_error("config is required");
        }

        // Small local lookup helper — empty string_view stands in for "field not present".
        auto field = [&](std::string_view key) -> std::string_view {
            auto it = cfg->get_fields().find(std::string{key});
            return it != cfg->get_fields().end() ? std::string_view{it->second}
                                                 : std::string_view{};
        };

        // Every one of these five fields is mandatory — bail the moment one's missing, no
        // fallback defaults.
        auto host = field("host");
        if (host.empty()) {
            throw std::runtime_error("host is required");
        }
        auto cert = field("cert");
        if (cert.empty()) {
            throw std::runtime_error("cert is required");
        }
        auto key = field("key");
        if (key.empty()) {
            throw std::runtime_error("key is required");
        }
        auto port = field("port");
        if (port.empty()) {
            throw std::runtime_error("port is required");
        }
        auto threads = field("threads");
        if (threads.empty()) {
            throw std::runtime_error("threads is required");
        }

        // All fields validated non-empty — safe to store/parse into the real member types now.
        m_host = std::string{host};
        m_cert = std::string{cert};
        m_key = std::string{key};
        std::from_chars(port.data(), port.data() + port.size(), m_port);
        std::from_chars(threads.data(), threads.data() + threads.size(), m_threads);
    }

    /**
     * @brief Grabs this protocol's name.
     * @return always `"http/2"`.
     */
    [[nodiscard]] std::string_view get_protocol_name() const noexcept override { return "http/2"; }
    /**
     * @brief Grabs the configured bind host.
     * @return the bind host, parsed from config in the ctor.
     */
    [[nodiscard]] std::string_view get_bind_host() const noexcept override { return m_host; }
    /**
     * @brief Grabs the configured bind port.
     * @return the bind port, parsed from config in the ctor.
     */
    [[nodiscard]] std::uint16_t get_bind_port() const noexcept override { return m_port; }
    /**
     * @brief Grabs the configured connection-handling thread count.
     * @return the thread count, parsed from config in the ctor.
     */
    [[nodiscard]] std::uint32_t get_bind_threads() const noexcept override { return m_threads; }
    /**
     * @brief Grabs the configured TLS cert path.
     * @return the TLS cert path, read from config in the ctor.
     */
    [[nodiscard]] std::string_view get_tls_cert() const noexcept override { return m_cert; }
    /**
     * @brief Grabs the configured TLS key path.
     * @return the TLS key path, read from config in the ctor.
     */
    [[nodiscard]] std::string_view get_tls_key() const noexcept override { return m_key; }

    /**
     * @brief Overrides the base's throw-by-default with a real implementation — hands back a
     * fresh, empty `Server` ready for `build()`.
     * @return a heap-allocated, not-yet-built `Server`.
     */
    [[nodiscard]] std::unique_ptr<Server> get_server() override {
        return std::make_unique<Server>();
    }
    /**
     * @brief Overrides the base's throw-by-default with a real implementation — hands back a
     * fresh `Client` wired to `dispatch`.
     * @warning The `// TODO: implement client` comment right above is stale — a working
     * `Client` genuinely does get constructed and returned here, this override isn't a stub.
     * @param dispatch forwarded straight into the new `Client`'s ctor.
     * @return a heap-allocated `Client`, not yet connected.
     */
    // TODO: implement client
    [[nodiscard]] std::unique_ptr<interfaces::IClient>
    get_client(interfaces::io::ReceiveDispatchFn &&dispatch) override {
        return std::make_unique<Client>(std::move(dispatch));
    }

  private:
    std::string m_host = "localhost";
    std::uint16_t m_port = 8080;
    std::uint32_t m_threads = 1;
    std::string m_cert = "server.crt";
    std::string m_key = "server.key";
};

} // namespace io::layer::http2
