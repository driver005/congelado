#include <memory>
#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>

import std;
import shared;
import interfaces;
import io_layer_http2;
import io_shared;
import core_config;
import core_router;
import core_contract;
import io_base_flow;
import io_base_socket;
import io_base_leverage;
import core_logger;

namespace {

class Http2Plugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "http2"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "protocol"; }

    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        static constexpr std::string_view reqs[] = {"file_logger"};
        return reqs;
    }

    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_PROTOCOL; }

    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg_view) override {
        core::config::PluginConfig cfg;
        congelado::config_for_each(cfg_view, [&](std::string_view key, std::string_view val) {
            cfg.add_field(std::string{key}, std::string{val});
        });

        auto protocol = io::layer::http2::Http2Protocol{cfg_view.count == 0 ? nullptr : &cfg};

        m_server = protocol.get_server();

        auto *router_ctx = congelado::router_ctx<core::router::RouterContext<>>(host);

        if (router_ctx != nullptr) {
            router_ctx->add_route(core::router::Route<>{"/hello"}.get(
                [](interfaces::io::IRequest &, interfaces::io::IResponse &res) noexcept {
                    constexpr std::string_view BODY =
                        R"({"message":"Hello from the HTTP/2 server plugin :D"})";
                    std::vector<std::byte> body;
                    body.reserve(BODY.size());
                    for (char ch : BODY) {
                        body.push_back(static_cast<std::byte>(ch));
                    }
                    res.set_status(interfaces::io::types::Status::OK);
                    res.add_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
                    res.set_body(std::move(body));
                }));
            m_router_ctx = router_ctx;
        }

        m_contract_group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
        m_leverager =
            congelado::leverager_ctx<io::base::leverage::Leverager<io::base::leverage::Context>>(
                host);

        if (!m_contract_group || !m_leverager) {
            core::logger::error("http2", "no contract group or leverager");
            return;
        }

        m_bind_host = std::string{protocol.get_bind_host()};
        m_bind_port = protocol.get_bind_port();
    }

    void on_ready() noexcept override {
        if (!m_server || !m_router_ctx)
            return;

        m_server->build(static_cast<void *>(m_router_ctx));

        core::logger::important("http2", "listening on {}:{}", m_bind_host, m_bind_port);

        try {
            m_socket_flow.emplace(io::base::socket::Endpoint{m_bind_host, m_bind_port},
                                  *m_leverager, *m_contract_group);
            m_socket_flow->add_on_accept(
                [this](shared::SendCallback send,
                       shared::CloseCallback close) -> shared::ReadCallback {
                    return m_server->on_connect(std::move(send), std::move(close));
                });
            m_socket_flow->build();
        } catch (...) {
            core::logger::error("http2", "failed to start socket flow");
        }
    }

    void on_unload() noexcept override {
        m_socket_flow.reset();
        m_server.reset();
    }

    void *protocol_get() noexcept { return static_cast<void *>(m_server.get()); }

  private:
    std::unique_ptr<io::layer::http2::Server> m_server;
    std::optional<io::base::flow::sync::ServerFlowSocket<core::contract::ContractGroup<>,
                                                         io::base::socket::Protocol::TLS>>
        m_socket_flow;
    core::router::RouterContext<> *m_router_ctx{nullptr};
    core::contract::ContractGroup<> *m_contract_group{nullptr};
    io::base::leverage::Leverager<io::base::leverage::Context> *m_leverager{nullptr};
    std::string m_bind_host;
    std::uint16_t m_bind_port{0};
};

} // namespace

CONGELADO_PLUGIN(Http2Plugin)
