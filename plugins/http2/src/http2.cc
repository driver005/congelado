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
import core_events;

namespace {

class Http2Plugin final : public congelado::Plugin {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"http2"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "http2"; }
    /**
     * @brief Version string for this build of the http2 plugin.
     * @return `"1.0.0"`.
     */
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    /**
     * @brief Unique type tag identifying this as the (or a) protocol plugin.
     * @return `"protocol"`.
     */
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "protocol"; }

    /**
     * @brief Won't load before `file_logger` is up — so protocol errors have somewhere to land
     * from the jump instead of getting dropped on the floor.
     * @return a span containing `"file_logger"`.
     */
    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        static constexpr std::string_view REQS[] = {"file_logger"};
        return REQS;
    }

    /**
     * @brief Flags this as a protocol-capable plugin, so the host wires `protocol_get` into the
     * `_cap_dispatch` routing.
     * @return `CONGELADO_CAP_PROTOCOL`.
     */
    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_PROTOCOL; }

    /**
     * @brief Builds the HTTP/2 server off the parsed config, registers a demo `/hello` route on
     * the router (if one's available), and resolves the contract group + leverager it'll need
     * later in `on_ready`.
     * @warning If `router_ctx` resolves but the contract group or leverager don't, this logs an
     * error and returns early — but `m_server`/`m_router_ctx` are already set by that point
     * (they're assigned before the contract-group/leverager check). `on_ready()`'s guard only
     * checks `m_server`/`m_router_ctx`, so it'll sail past and dereference the still-null
     * `m_leverager`/`m_contract_group` — a real null-deref footgun, not something the `try`
     * block in `on_ready` catches either, straight cooked. Worth fixing the guard, but not
     * touching that here.
     * @param host the host callback table; used to resolve the router context, contract group,
     * and leverager.
     * @param cfg_view this plugin's config view, copied field-by-field into a
     * `core::config::PluginConfig` before being handed to `Http2Protocol`.
     */
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg_view) override {
        // Copy every host-supplied config field into a PluginConfig — Http2Protocol wants its
        // own owned copy, not a view into the host's storage.
        core::config::PluginConfig cfg;
        congelado::config_for_each(cfg_view, [&](std::string_view key, std::string_view val) {
            cfg.add_field(std::string{key}, std::string{val});
        });

        // A host that never gave this plugin its own config section (e.g. some other host
        // bootstrapping just its serde format plugins before real per-plugin config exists)
        // still hands over a view with the generic runtimes/python_module/lua_table/
        // lua_safe_mode fields — never truly empty — but none of THIS plugin's actually
        // required fields. Http2Protocol's ctor throws unconditionally when any of
        // host/cert/key/port/threads is missing (no "use defaults" mode), so check for the one
        // this host is least likely to invent by accident before ever constructing it. Leave
        // m_server unset otherwise; on_ready already bails quiet when it's null.
        if (!cfg.get_fields().contains("host")) {
            return;
        }

        auto protocol = io::layer::http2::Http2Protocol{&cfg};

        m_server = protocol.get_server();

        // Only register the demo route if a router context actually resolved — no router,
        // no route, no crash either.
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

        // Resolve the contract group and leverager `on_ready` will need to stand up the
        // socket flow — see the class-level @warning, this guard doesn't actually protect
        // `on_ready` since m_server/m_router_ctx are already set above.
        m_contract_group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
        m_leverager =
            congelado::leverager_ctx<io::base::leverage::Leverager<io::base::leverage::Context>>(
                host);

        if (m_contract_group == nullptr || m_leverager == nullptr) {
            core::logger::error("http2", "no contract group or leverager");
            core::events::publish("http2.load.dependency_missing",
                                 {{"contract_group", m_contract_group == nullptr ? "missing" : "ok"},
                                  {"leverager", m_leverager == nullptr ? "missing" : "ok"}});
            return;
        }

        // Everything resolved — stash the bind address for on_ready to actually listen on.
        m_bind_host = std::string{protocol.get_bind_host()};
        m_bind_port = protocol.get_bind_port();
    }

    /**
     * @brief Wires the router into the built server and stands up the listening socket flow.
     * @note Only guards on `m_server`/`m_router_ctx` being set — see the `@warning` on `on_load`
     * for the case where `m_leverager`/`m_contract_group` can still be null here despite that
     * guard passing.
     */
    void on_ready() noexcept override {
        // Nothing to serve without both the server and a router wired in — bail quiet.
        if (m_server == nullptr || m_router_ctx == nullptr) {
            return;
        }

        // Everything from build() through the socket flow setup below is wrapped in try/catch
        // since build()/logger formatting/bind/listen can all throw — one failed plugin
        // shouldn't take the whole host down, and this whole path crosses a noexcept boundary.
        try {
            m_server->build(static_cast<void *>(m_router_ctx));

            core::logger::important("http2", "listening on {}:{}", m_bind_host, m_bind_port);
            core::events::publish("http2.server.listening",
                                 {{"host", m_bind_host}, {"port", std::format("{}", m_bind_port)}});

            // Stand up the actual listening socket flow, routing accepted connections through
            // the built server's on_connect.
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
            core::events::publish("http2.server.start_failed",
                                 {{"host", m_bind_host}, {"port", std::format("{}", m_bind_port)}});
        }
    }

    /// @brief Tears down the socket flow before the server, in that order — no in-flight
    /// connections getting served by a server that's already gone, that's an easy L to avoid.
    void on_unload() noexcept override {
        m_socket_flow.reset();
        m_server.reset();
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's raw server pointer.
     * @return `m_server.get()` type-erased to `void*`, or `nullptr` if the server was never
     * built.
     */
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
