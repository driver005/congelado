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
    Client(interfaces::io::ReceiveDispatchFn &&dispatch)
        : m_flow{nullptr}, m_dispatch{std::move(dispatch)} {}

    ~Client() override = default;

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(Client &&) = delete;
    Client &operator=(Client &&) = delete;

    [[nodiscard]] ::shared::ReadCallback on_connect(::shared::SendCallback send,
                                                    ::shared::CloseCallback close) override {
        m_flow = std::make_unique<ClientFlow>(std::move(send), std::move(close), m_dispatch);

        auto connector = m_flow->on_connect();

        return connector();
    }

    // Please pass in a HttpRequest object. Else this function will throw a std::bad_cast exception.
    void send(interfaces::io::IRequest &req) override {
        try {
            auto &http_request = dynamic_cast<HttpRequest &>(req);

            m_flow->sender(http_request);
            return;
        } catch (const std::bad_cast &e) {
            core::logger::error("http2", "Failed to cast IRequest to HttpRequest: {}", e.what());
        }
    }

  private:
    std::unique_ptr<ClientFlow> m_flow;
    interfaces::io::ReceiveDispatchFn m_dispatch;
};

class Server {
  public:
    Server() = default;
    ~Server() = default;

    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;

    void build(void *router_ctx) {
        auto *router = static_cast<core::router::RouterContext<> *>(router_ctx);
        m_server.emplace(core::router::RouteBuilder{}.build(std::move(*router)));
        m_dispatch = [this](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
            if (!m_server) {
                return;
            }

            auto method = interfaces::io::types::parse_method(req.get_method());

            try {
                m_server->match(method, req.get_path(), req, res);
            } catch (const std::runtime_error &) {
                res.set_status(interfaces::io::types::Status::NOT_FOUND);
            }
        };
    }

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
    explicit Http2Protocol(const core::config::PluginConfig *cfg = nullptr) {
        if (cfg == nullptr) {
            throw std::runtime_error("config is required");
        }

        auto field = [&](std::string_view key) -> std::string_view {
            auto it = cfg->get_fields().find(std::string{key});
            return it != cfg->get_fields().end() ? std::string_view{it->second}
                                                 : std::string_view{};
        };

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

        m_host = std::string{host};
        m_cert = std::string{cert};
        m_key = std::string{key};
        std::from_chars(port.data(), port.data() + port.size(), m_port);
        std::from_chars(threads.data(), threads.data() + threads.size(), m_threads);
    }

    [[nodiscard]] std::string_view get_protocol_name() const noexcept override { return "http/2"; }
    [[nodiscard]] std::string_view get_bind_host() const noexcept override { return m_host; }
    [[nodiscard]] std::uint16_t get_bind_port() const noexcept override { return m_port; }
    [[nodiscard]] std::uint32_t get_bind_threads() const noexcept override { return m_threads; }
    [[nodiscard]] std::string_view get_tls_cert() const noexcept override { return m_cert; }
    [[nodiscard]] std::string_view get_tls_key() const noexcept override { return m_key; }

    [[nodiscard]] std::unique_ptr<Server> get_server() override {
        return std::make_unique<Server>();
    }
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
