#include "core/ffi/plugin_api.hpp"

#include <memory>

import std;
import interfaces;
import io_layer_http2;
import io_shared;
import core_config;
import core_server;

namespace {

class Http2Plugin final : public congelado::PluginBase {
  public:
    [[nodiscard]] std::string_view name() const noexcept override { return "http2"; }
    [[nodiscard]] std::string_view version() const noexcept override { return "1.0.0"; }

    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_PROTOCOL; }

    void on_load(const CongeladoHostCallbacks &host, const CongeladoConfigView *cfg_view) override {
        core::config::PluginConfig cfg;
        if (cfg_view != nullptr) {
            for (std::size_t i = 0; i < cfg_view->count; ++i)
                cfg.add_field(std::string{cfg_view->keys[i]}, std::string{cfg_view->values[i]});
        }

        m_protocol = std::make_unique<io::layer::http2::Http2Protocol>(cfg_view != nullptr ? &cfg : nullptr);

        m_active_ctx = static_cast<core::server::RouterContext<io::shared::http::Protocol> *>(host.router_ctx);

        if (m_active_ctx != nullptr) {
            m_active_ctx->add_route(core::server::Route<io::shared::http::Protocol>{"/hello"}.get(
                [](interfaces::IRequest<io::shared::http::Protocol> &,
                   interfaces::IResponse<io::shared::http::Protocol> &res) noexcept {
                    constexpr std::string_view BODY = R"({"hello":"world time for bed !!!!"})";
                    std::vector<std::byte> body;
                    body.reserve(BODY.size());
                    for (char ch : BODY) {
                        body.push_back(static_cast<std::byte>(ch));
                    }
                    res.set_status(interfaces::Status::OK);
                    res.add_header(io::shared::http::Token::CONTENT_TYPE, "application/json");
                    res.set_body(std::move(body));
                }));

            m_built_server = std::unique_ptr<core::server::Server<io::shared::http::Protocol>>(
                new core::server::Server<io::shared::http::Protocol>(
                    core::server::ServerBuilder<io::shared::http::Protocol>{}.build(*m_active_ctx)));
        }

        m_protocol->set_router([this](interfaces::IRequest<io::shared::http::Protocol> &req,
                                      interfaces::IResponse<io::shared::http::Protocol> &res) {
            if (m_built_server == nullptr) {
                return;
            }

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
                return;
            }

            try {
                m_built_server->match(method, req.get_target(), req, res);
            } catch (const std::runtime_error &) {
                res.set_status(interfaces::Status::NOT_FOUND);
            }
        });
    }

    void on_unload() override {
        m_protocol.reset();
        m_built_server.reset();
        m_active_ctx = nullptr;
    }

    void *protocol_get() noexcept override { return static_cast<void *>(m_protocol.get()); }

  private:
    core::server::RouterContext<io::shared::http::Protocol> *m_active_ctx{nullptr};
    std::unique_ptr<core::server::Server<io::shared::http::Protocol>> m_built_server;
    std::once_flag m_build_flag;
    std::unique_ptr<io::layer::http2::Http2Protocol> m_protocol;
};

} // namespace

CONGELADO_PLUGIN(Http2Plugin)
