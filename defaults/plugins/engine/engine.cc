#include <memory>
#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>

import std;
import interfaces;
import io_shared;
import core_router;
import engine;
import core_logger;

namespace {

class EnginePlugin final : public congelado::Plugin {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"engine"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "engine"; }
    /**
     * @brief Version string for this build of the engine plugin.
     * @return `"1.0.0"`.
     */
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    /**
     * @brief This plugin has zero hard dependencies — it just needs a router context handed in
     * through `on_load`, no other plugin needs to be up first.
     * @return an empty span, no required plugin types.
     */
    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        return {};
    }

    /**
     * @brief Declares that protocol plugins (http2 etc.) must load after this one, so routes
     * are already registered onto the router by the time a protocol plugin starts serving
     * requests — ordering matters here, bet.
     * @return a span containing `"protocol"`.
     */
    [[nodiscard]] std::span<const std::string_view>
    get_load_before_types() const noexcept override {
        static constexpr std::string_view TYPES[] = {"protocol"};
        return TYPES;
    }

    /**
     * @brief Flags this as a custom-capability plugin — no logger/protocol/storage hooks, just
     * route registration on load.
     * @return `CONGELADO_CAP_CUSTOM`.
     */
    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_CUSTOM; }

    /**
     * @brief Pulls the router context out of the host callback table and registers the engine's
     * routes onto it.
     * @note No router context, no motion — this logs an error and bails instead of registering
     * anything, rather than crashing on a null dereference.
     * @param host the host callback table; used here to fetch the router context.
     * @param cfg_view unnamed/unused — this plugin doesn't read any config.
     */
    void on_load(CongeladoHostCallbacks const &host,
                 CongeladoConfigView const & /*cfg_view*/) override {
        // Pull the router context out of the host callback table first — nothing else in
        // here can happen without it.
        auto *router_ctx = congelado::router_ctx<core::router::RouterContext<>>(host);

        // No router, no motion — log it and bail instead of dereferencing a null pointer.
        if (router_ctx == nullptr) {
            core::logger::error("engine", "no router context");
            return;
        }

        // Router's live — wire this engine's routes onto it and let the host know it's done.
        engine::register_routes(*router_ctx, m_engine_ctx);
        core::logger::important("engine", "routes registered");
    }

    /// @brief No teardown needed — default no-op override, `m_engine_ctx` cleans up on its own.
    void on_unload() noexcept override {}

  private:
    engine::EngineContext m_engine_ctx;
};

} // namespace

CONGELADO_PLUGIN(EnginePlugin)
