export module io_layer_http2:plugin;

import std;
import shared;
import interfaces;
import core_config;
import :flow;

export namespace io::layer::http2 {

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
            return it != cfg->get_fields().end() ? std::string_view{it->second} : std::string_view{};
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
    [[nodiscard]] std::string_view get_bind_host()    const noexcept override { return m_host; }
    [[nodiscard]] std::uint16_t    get_bind_port()    const noexcept override { return m_port; }
    [[nodiscard]] std::uint32_t    get_bind_threads() const noexcept override { return m_threads; }
    [[nodiscard]] std::string_view get_tls_cert()     const noexcept override { return m_cert; }
    [[nodiscard]] std::string_view get_tls_key()      const noexcept override { return m_key; }

    void set_dispatch(interfaces::DispatchFn dispatch) override {
        m_dispatch = std::move(dispatch);
    }

    [[nodiscard]] ::shared::ReadCallback on_connect(::shared::SendCallback send,
                                                    ::shared::CloseCallback close) override {
        return m_flows.emplace_back(
                   std::make_unique<Flow>(std::move(send), std::move(close), m_dispatch))
            ->on_read();
    }

  private:
    std::string    m_host    = "localhost";
    std::uint16_t  m_port    = 8080;
    std::uint32_t  m_threads = 1;
    std::string    m_cert    = "server.crt";
    std::string    m_key     = "server.key";

    std::vector<std::unique_ptr<Flow>> m_flows;
    interfaces::DispatchFn             m_dispatch;
};

} // namespace io::layer::http2
