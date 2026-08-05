#include <memory>
#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>

import std;
import interfaces;
import io_shared;
import core_router;
import engine;
import core_events;
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
            core::events::publish("engine.no_router_context");
            return;
        }

        // Wire a resolved storage backend into this plugin's Connector, if one was found — the
        // host resolves this before build() runs specifically so it's already here by now (see
        // sdk/heart/app.cppm's load_plugins() for why capability resolution had to move earlier
        // for this one case). No database configured is a valid state, not an error — Connector
        // just runs local-only until one shows up.
        if (auto *database = congelado::database_ctx<interfaces::IDatabase>(host)) {
            m_engine_ctx.set_db(database);
        }

        // Same early-resolution story as database_ctx above, for the Lua bridge SWITCH/DO_WHILE
        // condition evaluation needs — see sdk/heart/app.cppm's load_plugins() for where this
        // gets populated (a separate pre-build() walk filtered to runtime_name() == "lua").
        if (auto *bridge = congelado::lua_bridge_ctx<interfaces::IBridge>(host)) {
            m_engine_ctx.set_lua_bridge(bridge);
        }

        // Same early-resolution story as database_ctx/lua_bridge_ctx above, for the search
        // backend SummaryProjector pushes WorkflowSummary/TaskSummary projections into on every
        // terminal transition. No provider configured is fine — search routes just degrade to
        // empty results.
        if (auto *search = congelado::search_ctx<interfaces::ISearchProvider>(host)) {
            m_engine_ctx.set_search(search);
        }

        // Same early-resolution story as database_ctx/lua_bridge_ctx/search_ctx above, for a
        // cache-capable plugin's interfaces::ICache* — wired into this engine's own Connector via
        // set_cache(). No cache-capable plugin configured is fine — Connector falls back to its
        // own in-process LocalCache.
        if (auto *cache = congelado::cache_ctx<interfaces::ICache>(host)) {
            m_engine_ctx.set_cache(cache);
        }

        // Plain local-disk default, not a resolved plugin capability — see
        // LocalPayloadStorage's own docs on why this one doesn't need the database_ctx/
        // search_ctx-style capability resolution machinery. Directory's relative to wherever
        // this process's cwd is, same as congelado.log/openapi.json.
        m_engine_ctx.set_payload_storage(&m_payload_storage);

        // Router's live — wire this engine's routes onto it and let the host know it's done.
        engine::register_routes(*router_ctx, m_engine_ctx);
        core::logger::important("engine", "routes registered");
        core::events::publish("engine.routes.registered");

        // Background sweep: catches everything the synchronous submit_result → on_task_terminal
        // path structurally can't — a worker that polled a task and never called back, an armed
        // retry whose backoff has elapsed, a node that couldn't spawn earlier because its
        // TaskDef's RateLimitPolicy was at capacity. There's no interval/wait-until primitive
        // anywhere else in this codebase to hook a wall-clock sweep into (the ContractGroup/
        // HandlerBase cooperative scheduler is driven purely by I/O-readiness), so this is a
        // plain std::jthread on its own timer, joined via its stop_token in on_unload().
        //
        // @warning Known, unresolved concurrency gap (flagged in the Conductor-parity plan, not
        // hand-waved): Connector's local-store mode (`m_local_stores`, used whenever no database
        // backend is configured) has zero internal locking, and this plugin's http2 binding
        // already runs `threads` (congelado.toml, default 4) concurrent request-handling
        // threads against the *same* `m_engine_ctx`/Connector instance — so local-store mode
        // already has a latent data race today, independent of this sweep thread. Adding the
        // sweep as one more concurrent caller doesn't introduce a new *kind* of risk, just one
        // more caller of an already-unsynchronized path — a real gap, but a pre-existing one
        // this pass doesn't attempt to fix (that would mean either mutex-guarding Connector
        // itself or funneling every caller through one single-threaded dispatch model, both
        // bigger changes than this orchestrator pass). With a real database backend configured,
        // each request already goes through Connector's own single-consumer pending-op queue
        // (drained by whatever runs on_execute()), which is a different — and safer — story.
        m_sweep_thread = std::jthread{[this](std::stop_token stop) {
            engine::Orchestrator orchestrator{m_engine_ctx};
            while (!stop.stop_requested()) {
                orchestrator.sweep_timeouts();
                orchestrator.sweep_retries();
                orchestrator.sweep_advance();
                orchestrator.sweep_schedules();
                std::this_thread::sleep_for(std::chrono::seconds{5});
            }
        }};
    }

    /// @brief Signals the sweep thread to stop and joins it — `m_engine_ctx` itself cleans up on
    /// its own past that.
    void on_unload() noexcept override { m_sweep_thread.request_stop(); }

  private:
    engine::EngineContext m_engine_ctx;
    engine::LocalPayloadStorage m_payload_storage{std::filesystem::path{"payloads"}};
    std::jthread m_sweep_thread;
};

} // namespace

CONGELADO_PLUGIN(EnginePlugin)
