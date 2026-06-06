#include <memory>
import congelado_plugin;
#include <congelado/plugin.h>

import std;
import shared;
import interfaces;
import io_layer_http2;
import io_shared;
import core_config;
import core_server;
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
        static constexpr std::string_view reqs[] = {"FileLogger"};
        return reqs;
    }

    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_PROTOCOL; }

    void on_load(congelado::HostCallbacks const &host,
                 congelado::ConfigView const &cfg_view) override {
        core::config::PluginConfig cfg;
        cfg_view.for_each([&](std::string_view key, std::string_view val) {
            cfg.add_field(std::string{key}, std::string{val});
        });

        m_protocol =
            std::make_unique<io::layer::http2::Http2Protocol>(cfg_view.empty() ? nullptr : &cfg);

        auto *router_ctx = host.router_ctx<core::server::RouterContext<io::shared::http::Protocol>>();

        if (router_ctx != nullptr) {
            router_ctx->add_route(core::server::Route<io::shared::http::Protocol>{"/hello"}.get(
                [](interfaces::IRequest<io::shared::http::Protocol> &,
                   interfaces::IResponse<io::shared::http::Protocol> &res) noexcept {
                    constexpr std::string_view BODY = R"({"hello":"world time for bed !!!!"})";
                    std::vector<std::byte> body;
                    body.reserve(BODY.size());
                    for (char ch : BODY)
                        body.push_back(static_cast<std::byte>(ch));
                    res.set_status(interfaces::Status::OK);
                    res.add_header(io::shared::http::Token::CONTENT_TYPE, "application/json");
                    res.set_body(std::move(body));
                }));
        }

        auto *contract_group = host.controller_ctx<core::contract::ContractGroup<>>();
        auto *leverager     = host.leverager_ctx<io::base::leverage::Leverager<io::base::leverage::Context>>();

        if (contract_group == nullptr || leverager == nullptr) {
            core::logger::error("http2", "no contract group or leverager");
            return;
        }

        core::logger::important("http2", "listening on {}:{}", m_protocol->get_bind_host(),
                                m_protocol->get_bind_port());

        m_socket_flow.emplace(io::base::socket::Endpoint{std::string{m_protocol->get_bind_host()},
                                                         m_protocol->get_bind_port()},
                              *leverager, *contract_group);
        m_socket_flow->add_on_accept(
            [this](shared::SendCallback send, shared::CloseCallback close) -> shared::ReadCallback {
                return m_protocol->on_connect(std::move(send), std::move(close));
            });
        m_socket_flow->build();
    }

    void on_unload() override {
        m_socket_flow.reset();
        m_protocol.reset();
    }

    void *protocol_get() noexcept override { return static_cast<void *>(m_protocol.get()); }

  private:
    std::unique_ptr<io::layer::http2::Http2Protocol> m_protocol;
    std::optional<io::base::flow::sync::ServerFlowSocket<core::contract::ContractGroup<>,
                                                         io::base::socket::Protocol::TLS>>
        m_socket_flow;
};

} // namespace

CONGELADO_PLUGIN(Http2Plugin)
