export module io_layer_http2:plugin;

import std;
import shared;
import interfaces;
import core_config;
import core_router;
import core_events;
import core_logger;
import core_contract;
import io_shared;
import io_base_socket;
import io_base_leverage;
import io_flow_socket;
import :extension;
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
    explicit Client(interfaces::io::ReceiveDispatchFn &&dispatch)
        : m_flow{nullptr}, m_dispatch{std::move(dispatch)} {}

    /**
     * @brief Registers an HTTP/2 extension into this client's own registry. Register before
     * `on_connect()` builds the flow so the extension sees the connection's hooks.
     * @note No-op on null (see `HttpExtensionRegistry::add_extension`).
     * @param extension the extension to register.
     */
    void register_extension(std::shared_ptr<IHttpExtension> extension) {
        m_extension_registry.add_extension(std::move(extension));
    }

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
        m_flow = std::make_unique<ClientFlow>(std::move(send), std::move(close),
                                              m_extension_registry, m_dispatch);

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
     * @note No-ops (logged) if called before `on_connect()` has fired — `m_flow` doesn't exist
     * yet at that point, so there's nothing to send through.
     */
    // Please pass in a HttpRequest object. Else this function will throw a std::bad_cast exception.
    void send(interfaces::io::IRequest &req) override {
        if (!m_flow) {
            core::logger::error("http2", "send() called before the connection was established");
            core::events::publish("http2.send.not_connected");
            return;
        }
        try {
            // Downcast to the concrete request type this protocol actually knows how to frame.
            auto &http_request = dynamic_cast<HttpRequest &>(req);

            m_flow->sender(http_request);
            return;
        } catch (const std::bad_cast &e) {
            // Wrong IRequest type got routed here, no cap — log it and drop the request instead
            // of propagating the exception up to the caller.
            core::logger::error("http2", "Failed to cast IRequest to HttpRequest: {}", e.what());
            core::events::publish("http2.request.cast_failed", {{"error", e.what()}});
        }
    }

    /**
     * @brief Builds a fresh `HttpRequest` tagged with `stream_id` — the concrete-type escape
     * hatch generic callers (holding just an `IClient&`) reach through instead of naming
     * `HttpRequest` themselves.
     * @param stream_id the stream id to tag the new request with.
     * @return a heap-allocated `HttpRequest`.
     */
    [[nodiscard]] std::unique_ptr<interfaces::io::IRequest>
    create_request(std::uint32_t stream_id) override {
        return std::make_unique<HttpRequest>(stream_id);
    }

    /**
     * @brief Opens the outbound TCP/TLS connection to `endpoint` and drives this client's own
     * on_connect() once it lands — the socket-transport counterpart to
     * `Http2Protocol::get_client()`, so callers work through this `Client` abstraction instead
     * of wiring a `ClientFlowSocket` themselves.
     * @param endpoint the peer endpoint to connect to.
     * @param leverager the io_uring leverager backing the connection.
     * @param contract_group the contract group the connector/worker get registered against.
     * @param verify_peer forwarded straight to the underlying socket — `false` to skip
     * verifying a self-signed/dev peer cert not in the system trust store.
     * @param on_connected fires once the TCP/TLS connect and HTTP/2 handshake actually land —
     * this is the earliest point `send()` is safe to call (before that, `m_flow` is still null
     * and `send()` would dereference it). Callers that hand this client off to something that
     * calls `send()` concurrently (e.g. a poll loop on another thread) must wait for this
     * callback rather than assuming `connect()` returning means the connection is ready — the
     * connect/handshake sequence is async, `connect()` only kicks it off.
     * @return success, or an error if a flow is already up (call retry() instead), or if the
     * underlying connect fails (propagated from `ClientFlowSocket::build()`).
     */
    [[nodiscard]] std::expected<void, std::string>
    connect(io::base::socket::Endpoint endpoint,
            io::base::leverage::Leverager<io::base::leverage::Context> &leverager,
            core::contract::ContractGroup<> &contract_group, bool verify_peer = true,
            std::function<void()> on_connected = {}) {
        if (m_socket_flow.has_value()) {
            return std::unexpected("client already running");
        }
        m_socket_flow.emplace(std::move(endpoint), leverager, contract_group, verify_peer);
        m_socket_flow->add_on_accept([this, on_connected = std::move(on_connected)](
                                         ::shared::SendCallback send,
                                         ::shared::CloseCallback close) -> ::shared::ReadCallback {
            auto read_callback = on_connect(std::move(send), std::move(close));
            if (on_connected) {
                on_connected();
            }
            return read_callback;
        });
        return m_socket_flow->build();
    }

    /**
     * @brief Retries the connect/handshake sequence on the flow built by connect() — see
     * `ClientFlowSocket::retry()` for the actual act-now-or-defer logic.
     * @return success, or an error if connect() hasn't been called yet, if already connected, or
     * if the synchronous portion of a fresh attempt fails.
     */
    std::expected<void, std::string> retry() {
        if (!m_socket_flow.has_value()) {
            return std::unexpected("client not connecting");
        }
        return m_socket_flow->retry();
    }

  private:
    std::unique_ptr<ClientFlow> m_flow;
    interfaces::io::ReceiveDispatchFn m_dispatch;
    HttpExtensionRegistry m_extension_registry;
    std::optional<io::base::flow::sync::ClientFlowSocket<core::contract::ContractGroup<>,
                                                         io::base::socket::Protocol::TLS>>
        m_socket_flow;
};

class Server {
  public:
    /**
     * @brief Builds an empty server — no route table, no flows, and an empty extension
     * registry. Route table gets wired in via build(); flows spin up per-connection in
     * on_connect(); extensions register via register_extension() before serving starts.
     */
    Server() = default;
    /**
     * @brief Defaulted dtor — `m_flows` are all `unique_ptr`s, clean up on their own.
     */
    ~Server() = default;

    /**
     * @brief Registers an HTTP/2 extension into this server's own registry — the seam separate
     * extension plugins call (via the published `Server*`, see `Http2Plugin`) during their
     * `on_load`, before this server starts accepting connections in the protocol plugin's
     * `on_ready`.
     * @note No-op on null (see `HttpExtensionRegistry::add_extension`).
     * @param extension the extension to register.
     */
    void register_extension(std::shared_ptr<IHttpExtension> extension) {
        m_extension_registry.add_extension(std::move(extension));
    }

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
        auto *router = static_cast<core::router::RouterContext<> *>(router_ctx);
        m_server.emplace(core::router::RouteBuilder{}.build(std::move(*router)));
        m_executor.emplace(&*m_server);
        m_dispatch = [this](interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                            std::function<void()> send) {
            // Guard — no executor (never built) means nothing to dispatch to, return a 503.
            if (!m_executor) {
                constexpr std::string_view BODY = R"({"error":"Service Unavailable"})";
                std::vector<std::byte> body;
                body.reserve(BODY.size());
                for (char ch : BODY) {
                    body.push_back(static_cast<std::byte>(ch));
                }
                res.set_status(interfaces::io::types::Status::SERVICE_UNAVAILABLE);
                res.add_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
                res.set_body(std::move(body));
                send();
                return;
            }
            m_executor->enqueue(req, res, std::move(send));
        };
    }

    /**
     * @brief Access to the async request executor bound to this server's route table — the http2
     * plugin registers it as a contract worker (wake + contract handle) after build().
     * @return reference to the executor; only valid after build() has run.
     */
    [[nodiscard]] core::router::RouterExecutor &executor() noexcept { return *m_executor; }

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
            .emplace_back(std::make_unique<ServerFlow>(std::move(send), std::move(close),
                                                       m_extension_registry, m_dispatch))
            ->on_read();
    }

    /**
     * @brief Gracefully closes every in-progress session — sends each one a GOAWAY (clean
     * shutdown code) and lets its close callback tear down the underlying socket — then blocks
     * until every connection has finished flushing and the executor has no pending work.
     * @note Doesn't clear `m_flows` — the now-closed `ServerFlow`s stay put and get destroyed
     * normally whenever this `Server` itself does.
     */
    void close() noexcept {
        mark_closed();

        // No timeout — block until the server is naturally drained.
        while (!is_idle()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    /**
     * @brief Sends GOAWAY on every in-progress session and tears each transport down. Best
     * effort — one bad session's teardown never stops the rest.
     */
    void mark_closed() noexcept {
        for (auto &flow : m_flows) {
            try {
                flow->close();
            } catch (...) { // NOLINT(bugprone-empty-catch) — one bad session's teardown must
                            // not stop the rest from closing, or crash the shutdown path.
                core::logger::error("http2/server", "session close failed during shutdown");
                core::events::publish("http2.server.session_close_failed");
            }
        }
    }

    /**
     * @brief Checks whether every connection has finished flushing and has no active streams.
     * @return true if all flows are finished.
     */
    [[nodiscard]] bool is_idle() noexcept {
        if (m_executor.has_value() && !m_executor->is_idle()) {
            return false;
        }
        for (auto &flow : m_flows) {
            if (!flow->is_idle()) {
                return false;
            }
        }
        return true;
    }

  private:
    HttpExtensionRegistry m_extension_registry;
    std::vector<std::unique_ptr<ServerFlow>> m_flows;
    interfaces::io::ReceiveDispatchFn m_dispatch;
    std::optional<core::router::RouteHandler<>> m_server;
    std::optional<core::router::RouterExecutor> m_executor;
};

// HTTP/2 protocol implementation.
// Handles per-connection ServerFlow creation and request dispatch.
// Transport binding (socket, thread pool) is owned by the plugin (http2.cc).
class Http2Protocol final : public interfaces::IProtocol<Server> {
  public:
    /**
     * @brief Pulls host/cert/key/port straight out of `cfg`'s field map — every one of
     * them is mandatory, this bails the moment any single field's missing.
     * @warning No default fallback for any field despite `cfg` itself defaulting to `nullptr`
     * in the signature — pass `nullptr` (or a config missing even one field) and this throws
     * immediately. That optional-looking default parameter is a bit of a trap, don't read it as
     * "config's optional."
     * @param cfg the plugin config to read `host`/`cert`/`key`/`port` from.
     * @throws std::runtime_error if `cfg` is `nullptr`, or if any required field
     * (host/cert/key/port) is missing/empty.
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

        // All fields validated non-empty — safe to store/parse into the real member types now.
        m_host = std::string{host};
        m_cert = std::string{cert};
        m_key = std::string{key};
        std::from_chars(port.data(), port.data() + port.size(), m_port);
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
    std::string m_cert = "server.crt";
    std::string m_key = "server.key";
};

} // namespace io::layer::http2
