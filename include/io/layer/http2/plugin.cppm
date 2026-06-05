export module io_layer_http2:plugin;

import std;
import shared;
import interfaces;
import core_config;
import core_server;
import io_shared;
import :flow;

export namespace io::layer::http2 {

using Protocol = io::shared::http::Protocol;

// HTTP/2 protocol implementation.
// Handles per-connection Flow creation and request dispatch.
// Transport binding (socket, thread pool) is owned by the plugin (http2.cc).
class Http2Protocol final : public interfaces::IProtocol {
  public:
    explicit Http2Protocol(const core::config::PluginConfig *cfg = nullptr) {
        if (cfg == nullptr)
            return;

        auto field = [&](std::string_view key) -> std::string_view {
            auto it = cfg->get_fields().find(std::string{key});
            return it != cfg->get_fields().end() ? std::string_view{it->second}
                                                 : std::string_view{};
        };

        auto host = field("host");
        if (host.empty())
            throw std::runtime_error("host is required");
        auto cert = field("cert");
        if (cert.empty())
            throw std::runtime_error("cert is required");
        auto key = field("key");
        if (key.empty())
            throw std::runtime_error("key is required");
        auto port = field("port");
        if (port.empty())
            throw std::runtime_error("port is required");
        auto threads = field("threads");
        if (threads.empty())
            throw std::runtime_error("threads is required");

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

    void build(void *router_ctx) override {
        auto *router = static_cast<core::server::RouterContext<Protocol> *>(router_ctx);
        m_server.emplace(core::server::ServerBuilder<Protocol>{}.build(std::move(*router)));
        m_dispatch = [this](interfaces::IRequest<Protocol> &req,
                            interfaces::IResponse<Protocol> &res) {
            if (!m_server)
                return;

            auto http_method = io::shared::http::parse_method(req.get_method());
            core::server::Method method{};

            switch (http_method) {
            case io::shared::http::HttpMethod::GET:
                method = core::server::Method::GET;
                break;
            case io::shared::http::HttpMethod::POST:
                method = core::server::Method::POST;
                break;
            case io::shared::http::HttpMethod::PUT:
                method = core::server::Method::PUT;
                break;
            case io::shared::http::HttpMethod::DELETE:
                method = core::server::Method::DELETE;
                break;
            case io::shared::http::HttpMethod::PATCH:
                method = core::server::Method::PATCH;
                break;
            case io::shared::http::HttpMethod::HEAD:
                method = core::server::Method::HEAD;
                break;
            case io::shared::http::HttpMethod::OPTIONS:
                method = core::server::Method::OPTIONS;
                break;
            default:
                res.set_status(interfaces::Status::METHOD_NOT_ALLOWED);
                return;
            }

            try {
                m_server->match(method, req.get_target(), req, res);
            } catch (const std::runtime_error &) {
                res.set_status(interfaces::Status::NOT_FOUND);
            }
        };
    }

    [[nodiscard]] ::shared::ReadCallback on_connect(::shared::SendCallback send,
                                                    ::shared::CloseCallback close) override {
        return m_flows
            .emplace_back(std::make_unique<Flow>(std::move(send), std::move(close), m_dispatch))
            ->on_read();
    }

  private:
    std::string m_host = "localhost";
    std::uint16_t m_port = 8080;
    std::uint32_t m_threads = 1;
    std::string m_cert = "server.crt";
    std::string m_key = "server.key";

    std::vector<std::unique_ptr<Flow>> m_flows;
    interfaces::DispatchFn m_dispatch;
    std::optional<core::server::Server<Protocol>> m_server;
};

} // namespace io::layer::http2
