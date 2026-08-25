#include <memory>
#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>

import std;
import interfaces;
import io_shared;
import core_router;
import core_contract;
import engine;
import core_events;
import core_logger;
import migration;

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
            core::events::publish("engine.no_router_context");
            return;
        }

        // The SDK owns and registers the shared Connector with the host ContractGroup. Use it
        // instead of creating an unregistered one here — otherwise DB ops queue forever. The cast
        // and set happen inside the engine module so this .cc file doesn't need to import the
        // connector module directly (doing so triggers a clang modules crash).
        if (host.connector_ctx == nullptr) {
            core::logger::error("engine", "no connector context");
            core::events::publish("engine.no_connector_context");
            return;
        }
        engine::set_shared_connector(m_engine_ctx, host.connector_ctx);

        // Same early-resolution story as the connector above, for the Lua bridge SWITCH/DO_WHILE
        // condition evaluation needs — see sdk/heart/app.cppm's load_plugins() for where this
        // gets populated (a separate pre-build() walk filtered to runtime_name() == "lua").
        if (auto *bridge = congelado::lua_bridge_ctx<interfaces::IBridge>(host)) {
            m_engine_ctx.set_lua_bridge(bridge);
        }

        // Same early-resolution story as connector_ctx/lua_bridge_ctx above, for the search
        // backend SummaryProjector pushes WorkflowSummary/TaskSummary projections into on every
        // terminal transition. No provider configured is fine — search routes just degrade to
        // empty results.
        if (auto *search = congelado::search_ctx<interfaces::ISearchProvider>(host)) {
            m_engine_ctx.set_search(search);
        }

        // Cron timing lives in a cron plugin (interfaces::ICron) now, not in the engine. Resolved
        // before build() like search/cache. Require-plugin policy: with no cron backend, schedules
        // simply never fire — log it loudly rather than degrade silently.
        if (host.cron_ctx == nullptr) {
            core::logger::error("engine", "no cron backend — schedules will not fire");
            core::events::publish("engine.no_cron_backend");
        } else {
            engine::set_cron(m_engine_ctx, host.cron_ctx);
            // Install the fire callback (fired job name → started workflow via the workflow backend)
            // and seed the backend with every currently active schedule, since the cron plugin
            // starts empty.
            engine::install_cron_scheduling(
                m_engine_ctx,
                static_cast<interfaces::IWorkflowOrchestrator *>(host.workflow_orchestrator_ctx));
        }

        // Externalized payload storage is now a resolved capability (the payload_local plugin, or
        // an S3-backed one), same as database/search/cache. No backend resolved just leaves payload
        // storage unset — the engine only uses it on the (not-yet-wired) large-blob path.
        if (host.payload_storage_ctx == nullptr) {
            core::logger::warning("engine", "no payload_storage backend resolved");
        } else {
            m_engine_ctx.set_payload_storage(
                static_cast<interfaces::IExternalPayloadStorage *>(host.payload_storage_ctx));
        }

        // Route dispatch through the resolved worker_orchestrator backend (e.g. the local
        // orchestrator plugin) — the engine no longer carries its own queue logic. With no backend
        // resolved the queue endpoints (poll/claim) degrade to an error; log it loudly.
        if (host.worker_orchestrator_ctx == nullptr) {
            core::logger::error("engine",
                                "no worker_orchestrator backend — task dispatch will not work");
            core::events::publish("engine.no_worker_orchestrator");
        } else {
            m_engine_ctx.set_orchestrator(
                static_cast<interfaces::IWorkerOrchestrator *>(host.worker_orchestrator_ctx));
        }

        // The workflow-lifecycle backend (DAG side), resolved the same way. The engine's built-in
        // Orchestrator still owns the actual DAG advance; this is the pluggable lifecycle surface.
        if (host.workflow_orchestrator_ctx == nullptr) {
            core::logger::error("engine", "no workflow_orchestrator backend");
            core::events::publish("engine.no_workflow_orchestrator");
        } else {
            m_engine_ctx.set_workflow_orchestrator(
                static_cast<interfaces::IWorkflowOrchestrator *>(host.workflow_orchestrator_ctx));
        }

        // Router's live — wire this engine's routes onto it and let the host know it's done.
        engine::register_routes(*router_ctx, m_engine_ctx);
        core::logger::important("engine", "routes registered");
        core::events::publish("engine.routes.registered");

        // Register engine model baselines with the shared migration registry — registration
        // only, no run. The host runs the one global migration pass itself, once, after every
        // plugin has loaded and gone ready (see congelado::heart::App::load_plugins) — running
        // migrations here, per-plugin, would mean any plugin loaded after `engine` never got its
        // own baseline picked up before the run happened.
        engine::register_migrations();

        // The DAG orchestration (advance + background sweep) now lives entirely in the
        // workflow_orchestrator plugin (resolved into m_engine_ctx above). The engine keeps no
        // orchestrator and registers no sweep — the plugin owns both.
    }

  private:
    engine::EngineContext m_engine_ctx;
};

} // namespace

CONGELADO_PLUGIN(EnginePlugin)
