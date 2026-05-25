#include "core/ffi/plugin_api.hpp"

import std;
import interfaces;
import io_layer_http2;
import io_shared;
import core_config;
import core_server;

namespace {

struct RouterImpl {
    core::server::Server<io::shared::http::Protocol> server;
    explicit RouterImpl(core::server::RouterContext<io::shared::http::Protocol> ctx)
        : server{core::server::ServerBuilder<io::shared::http::Protocol>{}.build(std::move(ctx))} {}
};

class Http2Plugin final : public congelado::PluginBase {
  public:
    std::string_view name()    const noexcept override { return "http2"; }
    std::string_view version() const noexcept override { return "1.0.0"; }

    void on_load(const CongeladoHostCallbacks &, const CongeladoConfigView *cfg_view) override {
        core::config::PluginConfig cfg;
        if (cfg_view) {
            for (std::size_t i = 0; i < cfg_view->count; ++i)
                cfg.fields[std::string{cfg_view->keys[i]}] = std::string{cfg_view->values[i]};
        }
        m_protocol = std::make_unique<io::layer::http2::Http2Protocol>(cfg_view ? &cfg : nullptr);

        core::server::RouterContext<io::shared::http::Protocol> ctx;
        ctx.add_route(core::server::Route<io::shared::http::Protocol>{"/hello"}.get(
            [](interfaces::IRequest<io::shared::http::Protocol> &,
               interfaces::IResponse<io::shared::http::Protocol> &res) noexcept {
                constexpr std::string_view BODY = R"({"hello":"world"})";
                std::vector<std::byte> body;
                body.reserve(BODY.size());
                for (char c : BODY)
                    body.push_back(static_cast<std::byte>(c));
                res.set_status(interfaces::Status::OK);
                res.add_header(io::shared::http::Token::CONTENT_TYPE, "application/json");
                res.set_body(std::move(body));
            }));

        m_router.reset(new RouterImpl{std::move(ctx)});
        RouterImpl *router = m_router.get();

        m_protocol->set_router([router](interfaces::IRequest<io::shared::http::Protocol> &req,
                                        interfaces::IResponse<io::shared::http::Protocol> &res) {
            auto http_method = io::shared::http::parse_method(req.get_method());
            core::server::Method method;
            switch (http_method) {
            case io::shared::http::HttpMethod::GET:     method = core::server::Method::GET;     break;
            case io::shared::http::HttpMethod::POST:    method = core::server::Method::POST;    break;
            case io::shared::http::HttpMethod::PUT:     method = core::server::Method::PUT;     break;
            case io::shared::http::HttpMethod::DELETE:  method = core::server::Method::DELETE;  break;
            case io::shared::http::HttpMethod::PATCH:   method = core::server::Method::PATCH;   break;
            case io::shared::http::HttpMethod::HEAD:    method = core::server::Method::HEAD;    break;
            case io::shared::http::HttpMethod::OPTIONS: method = core::server::Method::OPTIONS; break;
            default: return;
            }
            try {
                router->server.match(method, req.get_target(), req, res);
            } catch (const std::runtime_error &) {}
        });
    }

    void on_unload() override {
        m_protocol.reset();
        m_router.reset();
    }

    // Protocol cap — returns interfaces::IProtocol* as void* (placeholder until CongeladoProtocolCap defined)
    CongeladoProtocolCap *protocol_cap() noexcept override {
        return reinterpret_cast<CongeladoProtocolCap *>(m_protocol.get());
    }

  private:
    std::unique_ptr<io::layer::http2::Http2Protocol> m_protocol;
    std::unique_ptr<RouterImpl>                       m_router;
};

} // namespace

CONGELADO_PLUGIN(Http2Plugin)
