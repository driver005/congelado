export module io_layer_http2:plugin;

import std;
import shared;
import interfaces;
import core_config;
import :flow;

export namespace io::layer::http2 {

// Built-in protocol plugin for HTTP/2.
// Reads transport settings (host, port, tls, threads) from [plugins.http2] config.
class Http2Protocol final : public interfaces::IProtocol {
  public:
    explicit Http2Protocol(const core::config::PluginConfig *cfg = nullptr) {
        if (cfg == nullptr) {
            return;
        }

        auto field = [&](std::string_view key) -> std::string_view {
            auto it = cfg->get_fields().find(std::string{key});
            return it != cfg->get_fields().end() ? std::string_view{it->second} : std::string_view{};
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

        host_ = std::string{host};
        cert_ = std::string{cert};
        key_ = std::string{key};
        std::from_chars(port.data(), port.data() + port.size(), port_);
        std::from_chars(threads.data(), threads.data() + threads.size(), threads_);
    }

    [[nodiscard]] std::string_view get_protocol_name() const noexcept override { return "http/2"; }
    [[nodiscard]] std::string_view get_bind_host() const noexcept override { return host_; }
    [[nodiscard]] std::uint16_t get_bind_port() const noexcept override { return port_; }
    [[nodiscard]] std::uint32_t get_bind_threads() const noexcept override { return threads_; }
    [[nodiscard]] std::string_view get_tls_cert() const noexcept override { return cert_; }
    [[nodiscard]] std::string_view get_tls_key() const noexcept override { return key_; }

    void set_router(DispatchFn dispatch) { m_dispatch = std::move(dispatch); }

    [[nodiscard]] ::shared::ReadCallback on_connect(::shared::SendCallback send,
                                                    ::shared::CloseCallback close) override {
        return flows_.emplace_back(std::make_unique<Flow>(std::move(send), std::move(close), m_dispatch))->on_read();
    }

  private:
    std::string host_ = "localhost";
    std::uint16_t port_ = 8080;
    std::uint32_t threads_ = 1;
    std::string cert_ = "server.crt";
    std::string key_ = "server.key";

    std::vector<std::unique_ptr<Flow>> flows_;
    DispatchFn m_dispatch;
};

} // namespace io::layer::http2
