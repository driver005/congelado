module;

#include <congelado/abi.h>
#include <cstdio>
#include <csignal>

export module congelado_heart:app;

import std;
import interfaces;
import core_config;
import core_logger;
import core_events;
import core_otel;
import core_plugin;
import utils_openapi;
import serde;
import :context;
import :adapters;

std::filesystem::path expand_tilde(const std::filesystem::path &path) {
    std::string path_str = path.string();
    // Only a leading '~' gets expanded — and only if HOME is actually set, otherwise the
    // path is left untouched rather than half-rewritten.
    if (!path_str.empty() && path_str[0] == '~') {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        // getenv has no portable thread-safe alternative in std C++; only called here at
        // single-threaded boot time, before load_plugins() spins up anything concurrent.
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        const char *home = std::getenv("HOME");
        if (home != nullptr) {
            path_str.replace(0, 1, home);
        }
    }
    return {path_str};
}

// Set only from inside the SIGINT/SIGTERM handler below — a plain atomic<bool>, not a
// std::promise, since std::signal handlers may only touch lock-free atomics/sig_atomic_t per the
// standard's async-signal-safety rules.
std::atomic<bool> g_shutdown_requested{false};

/// @brief SIGINT/SIGTERM handler — signals the wait loop in `ServerRunner::run()` to fall through
/// so the (otherwise infinite) `run()` call returns and its plugin store's destructor actually
/// runs `congelado_on_unload()` on every loaded plugin (flushing OTel's batched exporters, among
/// other things) instead of the process just getting killed mid-flight.
void request_shutdown(int /*signal*/) noexcept {
    g_shutdown_requested.store(true, std::memory_order_relaxed);
}

export namespace congelado::heart {

class ServerRunner {
  public:
    /**
     * @brief Builds a runner pointed at the directory (or directories) it'll scan for plugin
     * shared libraries.
     * @param external_plugin_dir optional user-chosen directory for custom, non-built-in
     * plugins. This is the only argument a normal caller should ever pass.
     * @param internal_plugin_dir the built-in plugins directory (defaults to `"plugins"`, i.e.
     * `$(builddir)/plugins` in the shipped layout). Do NOT change how this is populated or
     * defaulted under normal circumstances — this is the SDK-managed build-output location,
     * not a user-facing knob.
     */
    explicit ServerRunner(std::optional<std::filesystem::path> external_plugin_dir = std::nullopt,
                          std::filesystem::path internal_plugin_dir = "plugins")
        : m_external_plugin_dir{std::move(external_plugin_dir)},
          m_internal_plugin_dir{std::move(internal_plugin_dir)} {}

    /**
     * @brief The whole heart entrypoint — loads config, boots up the app context, loads every
     * plugin off disk, wires the logger, and then blocks forever. This is the W that keeps the
     * process alive.
     * @warning Config-load failure, no logger plugin found, or no OpenAPI generator plugin found
     * all call `std::abort()` — this is a hard boot-time bail, not a recoverable error path.
     * Don't expect a return value on the L.
     * @param config_path path to the `congelado.toml` config file; missing file falls back to
     * defaults rather than failing.
     * @return `0` on a clean SIGINT/SIGTERM-triggered shutdown; otherwise never returns (boot
     * failure paths `std::abort()` instead).
     */
    int run(const std::filesystem::path &config_path = "congelado.toml") {
        // Config has to load clean or there's nothing sane to boot with.
        auto cfg = load_config(config_path);
        if (!cfg) {
            std::println(stderr, "[heart] config load failed — aborting");
            std::abort();
        }

        AppContext ctx;
        // Point the ambient core::logger::*/serde::Ser facades at this process's one set of
        // registries — has to happen before load_plugins() below, since plugins register into
        // them as they load.
        core::logger::LoggerRegistry::set_active(&ctx.get_logger_registry());
        // Events are optional, same posture as OTel below — no plugin exporting the EVENTS
        // capability just means core::events::publish(...) degrades to a debug log line.
        core::events::EventBusRegistry::set_active(&ctx.get_event_bus_registry());
        serde::SerdeFormatRegistry::set_active(&ctx.get_serde_format_registry());
        // OTel is optional, unlike the logger — no plugin exporting the OTEL capability just
        // means every core::otel::* facade call degrades to a silent no-op, no boot-time abort.
        core::otel::TracerRegistry::set_active(&ctx.get_tracer_registry());
        core::otel::MeterRegistry::set_active(&ctx.get_meter_registry());
        core::otel::LogRecordRegistry::set_active(&ctx.get_log_record_registry());

        bool plugin_logger = load_plugins(*cfg, ctx);

        // Need at least one logger plugin loaded, otherwise we're flying blind — not a W.
        if (!plugin_logger) {
            std::println(stderr, "[heart] no logger plugin found — aborting");
            std::abort();
        }

        // Everything's wired up — park here until asked to stop. A plain infinite
        // future.wait() (the previous approach) never returns on SIGTERM/SIGINT — the default
        // disposition just kills the process outright, skipping every destructor including
        // m_store's, which is what actually calls congelado_on_unload() (flushing OTel's batched
        // exporters, among other plugin teardown). Polling the flag instead of blocking on it
        // keeps this a plain function, matching what std::signal needs as a handler.
        std::signal(SIGINT, request_shutdown);
        std::signal(SIGTERM, request_shutdown);
        std::println("[heart] finished initialization");
        while (!g_shutdown_requested.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::println("[heart] shutdown requested — tearing down");
        return 0;
    }

  private:
    std::optional<std::filesystem::path> m_external_plugin_dir;
    std::filesystem::path m_internal_plugin_dir;
    core::plugin::SharedLibrary m_store{"plugin"};

    /**
     * @brief Loads `congelado.toml`, tilde-expanding the path first — an empty or
     * nonexistent path is not an error, it just means "use defaults", lowkey the friendlier
     * failure mode for a config loader.
     * @param raw_path path to the config file, possibly `~`-prefixed and possibly missing.
     * @return a populated `Config` on success (defaulted if no file was found), or
     * `std::nullopt` if the file exists but fails to parse.
     */
    static std::optional<core::config::Config> load_config(const std::filesystem::path &raw_path) {
        std::filesystem::path path = expand_tilde(raw_path);

        // No path, or the path doesn't exist — that's fine, just run on defaults.
        if (path.empty() || !std::filesystem::exists(path)) {
            std::println("[heart] no config file at '{}', using defaults",
                         path.empty() ? "<none>" : path.string());
            return core::config::Config{};
        }

        // File's there, so it needs to actually parse — a bad file is a real error, unlike
        // a missing one.
        auto result = core::config::load(path);
        if (!result) {
            std::println(stderr, "[heart] config error: {}", result.error());
            return std::nullopt;
        }

        std::println("[heart] loaded config from '{}'", path.string());
        return std::move(*result);
    }

    /**
     * @brief Does basically everything: builds the host callback table, resolves per-plugin
     * generation configs, scans/opens every plugin `.so` in `m_external_plugin_dir`/
     * `m_internal_plugin_dir`, resolves the mandatory OpenAPI-generator capability and registers
     * its live serve route ahead of `build()`, builds/inits every opened plugin, writes out the
     * generated OpenAPI document, and wires up a logger adapter for whichever plugin(s) export
     * the logger capability. Big function, does a lot of heavy lifting — that's the motion.
     * @warning Aborts the process (`std::abort()`) if plugin open fails, plugin build fails, or
     * no loaded plugin exports the OPENAPI capability — this is boot-time, not a recoverable path.
     * @note The OpenAPI serve route is registered on `ctx.get_router()` *before* `build()` runs
     * any plugin's `on_load`, on purpose: protocol plugins (e.g. http2) compile the route trie
     * eagerly inside their own `on_load`, so the route has to already be there or it just won't
     * show up. The generator plugin only needs to be `open()`ed (not `build()`t) for its
     * capability pointer to resolve, same as the storage capability just below it.
     * @param cfg the loaded config, supplying per-plugin type/fields used to build
     * `GenerationConfig`s.
     * @param ctx the app context whose router/contract-group/leverager get handed to plugins
     * through the host callback table.
     * @return `true` if at least one loaded plugin registered a logger (checked via
     * `LoggerRegistry::has_logger()`), `false` otherwise.
     */
    bool load_plugins(const core::config::Config &cfg, AppContext &ctx) {
        // Host callback table every plugin gets handed at load time — router, contract
        // group, and leverager all point back into this one AppContext.
        CongeladoHostCallbacks cb{};
        cb.router_ctx = ctx.get_router();
        cb.controller_ctx = &ctx.get_contract_group();
        cb.leverager_ctx = &ctx.get_leverager();

        // Top-level `[providers]` table — e.g. `database = "postgres"`, `search =
        // "elasticsearch"`, `logger = ["file_logger", "otel_otlp_plugin"]` — names which
        // plugin(s) get picked for a given capability, by stem (the `[plugins.<stem>]` section
        // key, not the plugin's own get_name()). Absent entirely, or an absent/empty entry for
        // one capability, falls back to that capability's own pre-existing default resolution
        // (first one found for single-active capabilities, every one found for logger).
        auto preferred = [&cfg](std::string_view capability) -> std::optional<std::string> {
            auto it = cfg.get_providers().find(std::string{capability});
            if (it == cfg.get_providers().end() || it->second.empty()) {
                return std::nullopt;
            }
            return it->second.front();
        };
        auto is_listed = [&cfg](std::string_view capability, std::string_view stem) -> bool {
            auto it = cfg.get_providers().find(std::string{capability});
            if (it == cfg.get_providers().end()) {
                return true; // no [providers] entry at all — every plugin's in by default
            }
            return std::ranges::find(it->second, stem) != it->second.end();
        };
        auto plugin_stem = [](core::plugin::types::PluginRef &plugin) -> std::string {
            auto path_it = plugin.m_data.find("path");
            if (path_it == plugin.m_data.end()) {
                return {};
            }
            auto stem = std::filesystem::path{std::any_cast<const std::string &>(path_it->second)}
                            .stem()
                            .string();
            if (stem.starts_with("lib")) {
                stem = stem.substr(3);
            }
            return stem;
        };

        // Turn each config-file plugin entry into a GenerationConfig: the plugin's declared
        // type is just added as a wanted runtime name unconditionally — an open set, not a
        // fixed python/lua pair, so a bridge for any user-registered runtime name works with no
        // code change here. A plugin with no matching bridge just gets a lookup miss later,
        // same as an unregistered serde format.
        std::unordered_map<std::string, core::plugin::types::GenerationConfig> configs;
        for (const auto &[name, plugin_cfg] : cfg.get_plugins()) {
            core::plugin::types::GenerationConfig gc;
            gc.add_runtime(plugin_cfg.get_type());
            for (const auto &[key, value] : plugin_cfg.get_fields()) {
                auto extra = gc.get_extra();
                extra[key] = value;
                gc.set_extra(std::move(extra));
            }
            configs[name] = std::move(gc);
        }

        // Scan the plugin directory (or directories), open every discovered .so, then
        // build/init each one against the host callback table and per-plugin generation
        // configs — both steps are boot-time hard-fails, no partial-load recovery. External
        // scans first, if given, then the internal (build-output) directory, all before the
        // single open_all()/for_each() pass below.
        if (m_external_plugin_dir && !m_external_plugin_dir->empty()) {
            m_store.scan(*m_external_plugin_dir);
        }
        m_store.scan(m_internal_plugin_dir);
        auto open_res = m_store.open_all();
        if (!open_res) {
            std::println(stderr, "[heart] plugin load failed: {} — aborting",
                         open_res.error().get_message());
            std::abort();
        }

        // Resolve a storage-capable plugin's IDatabase* BEFORE build() runs (not after, like
        // the logger/serde/bridge walk below) — capability dlsym resolution doesn't depend on
        // on_load/congelado_init having run (those symbols are already populated by open()), but
        // the other walk happens after build(), i.e. after every plugin's on_load (including
        // the engine plugin's) has already finished — too late for EngineContext::set_db() to
        // ever receive it through that path. Stash the raw pointer into cb.database_ctx so
        // whichever plugin needs it can read it back during its own on_load. `[providers]
        // database = "..."` picks a specific one by stem; with none set (or the named plugin
        // not actually loaded), first IDatabase found wins, matching Connector's existing
        // single-backend-at-a-time design.
        // Provider-gated capabilities (events, logger) get filtered here, right after open(),
        // before build() ever calls congelado_init on them — resolve_event_sink()/
        // LoggerAdapter::register_from() work off open()-populated symbols alone, same
        // established precedent as resolve_storage()/resolve_bridge() being called pre-build()
        // below. A plugin like kafka_events_plugin sitting in the plugins directory but absent
        // from `[providers] events = [...]` must never actually connect to a broker — discarding
        // it here (before congelado_init) is the difference between "never loaded" and "loaded,
        // just not registered", which the old post-build-only is_listed() check couldn't give.
        std::vector<std::string> to_discard;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto stem = plugin_stem(*plugin);
            bool reject = (congelado::heart::resolve_event_sink(*plugin) && !is_listed("events", stem)) ||
                          (congelado::heart::LoggerAdapter::register_from(*plugin) &&
                           !is_listed("logger", stem));
            if (reject) {
                to_discard.push_back(std::any_cast<const std::string &>(plugin->m_data.at("name")));
            }
        });
        for (const auto &name : to_discard) {
            m_store.discard(name);
        }

        auto preferred_database = preferred("database");
        void *resolved_database = nullptr;
        void *fallback_database = nullptr;
        // Same "resolve before build()" reasoning applies to the OpenAPI-generator capability:
        // the live serve route has to land in RouterContext before build() runs any plugin's
        // on_load (protocol plugins like http2 compile the route trie eagerly in on_load), so
        // the generator plugin's IOpenApiGenerator* must be resolved here too, not in the
        // logger/serde/bridge walk further below which only runs after build().
        utils::openapi::OpenApiGeneratorRegistry generator_registry;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            if (resolved_database == nullptr) {
                if (auto database = congelado::heart::resolve_storage(*plugin)) {
                    if (fallback_database == nullptr) {
                        fallback_database = database.get();
                    }
                    if (preferred_database && plugin_stem(*plugin) == *preferred_database) {
                        resolved_database = database.get();
                    }
                }
            }
            if (auto generator = congelado::heart::resolve_openapi_generator(*plugin)) {
                generator_registry.add_generator(std::move(generator));
            }
        });
        cb.database_ctx = resolved_database != nullptr ? resolved_database : fallback_database;

        // Same "resolve before build()" reasoning, for the one bridge runtime a host-side
        // on_load hook currently needs early: the engine plugin's Lua condition evaluator
        // (SWITCH/DO_WHILE/EventHandler conditions). Kept as its own walk rather than folded
        // into the one above since it's filtering on a different predicate (runtime_name()
        // rather than "resolves at all") — unlike database_ctx's "first IDatabase wins"
        // (Connector only ever has one backend at a time anyway), a deployment can have both
        // python_bridge and lua_bridge loaded at once, and "first bridge found" would be a coin
        // flip on load order, so this specifically waits for one whose runtime_name() is "lua".
        void *resolved_lua_bridge = nullptr;
        m_store.for_each([&resolved_lua_bridge](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            if (resolved_lua_bridge != nullptr) {
                return;
            }
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto bridge = congelado::heart::resolve_bridge(*plugin);
            if (!bridge) {
                return;
            }
            if (bridge->runtime_name() == std::string_view{"lua"}) {
                resolved_lua_bridge = bridge.get();
            }
        });
        cb.lua_bridge_ctx = resolved_lua_bridge;

        // Same "resolve before build()" reasoning, for a search-capable plugin's
        // ISearchProvider* — the engine plugin's on_load wires this into EngineContext for its
        // terminal-transition SummaryProjector. More than one search-capable plugin can be
        // loaded at once here (postgres_plugin doubles as one; a separate elasticsearch_plugin
        // is a whole other one) — `[providers] search = "..."` picks a specific one by stem,
        // same pattern as database_ctx above; with none set, first one found wins.
        auto preferred_search = preferred("search");
        void *resolved_search = nullptr;
        void *fallback_search = nullptr;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            if (resolved_search != nullptr) {
                return;
            }
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto search = congelado::heart::resolve_search_provider(*plugin);
            if (!search) {
                return;
            }
            if (fallback_search == nullptr) {
                fallback_search = search.get();
            }
            if (preferred_search && plugin_stem(*plugin) == *preferred_search) {
                resolved_search = search.get();
            }
        });
        cb.search_ctx = resolved_search != nullptr ? resolved_search : fallback_search;

        // Same "resolve before build()" reasoning, for a cache-capable plugin's interfaces::ICache*
        // — the engine plugin's on_load wires this into its own Connector via set_cache(). More
        // than one cache-capable plugin can be loaded at once — `[providers] cache = "..."` picks
        // a specific one by stem, same pattern as database_ctx/search_ctx above; with none set,
        // first one found wins. No cache-capable plugin found at all just leaves Connector on its
        // own in-process LocalCache fallback, same graceful-degradation story as database/search.
        auto preferred_cache = preferred("cache");
        void *resolved_cache = nullptr;
        void *fallback_cache = nullptr;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            if (resolved_cache != nullptr) {
                return;
            }
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto cache = congelado::heart::resolve_cache(*plugin);
            if (!cache) {
                return;
            }
            if (fallback_cache == nullptr) {
                fallback_cache = cache.get();
            }
            if (preferred_cache && plugin_stem(*plugin) == *preferred_cache) {
                resolved_cache = cache.get();
            }
        });
        cb.cache_ctx = resolved_cache != nullptr ? resolved_cache : fallback_cache;

        // The OpenAPI generator is mandatory at startup, same as the logger — no plugin
        // exporting the OPENAPI capability means there's no way to serve/write the API
        // document at all, so this aborts boot rather than silently degrading.
        if (!generator_registry.has_generator()) {
            std::println(stderr, "[heart] no OpenAPI generator plugin found — aborting");
            std::abort();
        }
        auto *openapi_generator = generator_registry.get_generators().front().get();

        // The live /openapi route itself is now self-registered by the openapi_generator
        // plugin's own on_load (declares get_load_before_types() == {"protocol"}, same
        // ordering constraint this used to enforce manually here) — nothing to do on this side
        // anymore besides resolving openapi_generator for the write_document() call below.
        auto build_res = m_store.build(cb, configs);
        if (!build_res) {
            std::println(stderr, "[heart] plugin build failed: {} — aborting",
                         build_res.error().get_message());
            std::abort();
        }

        // Walk every loaded plugin and wire up a LoggerAdapter/format/bridge for whichever
        // one(s) actually export that capability — plugins without it just get skipped. Logger
        // and events are the two capabilities where more than one provider is genuinely meant to
        // be active at once (LoggerRegistry fans out to every registered sink, same for event
        // sinks) — `[providers]` filtering for both already happened above, before build(), so
        // every plugin still in m_store here has already earned its spot; no is_listed() re-check
        // needed on this pass. Has to run BEFORE write_document() below — that call dispatches
        // through serde::Ser (runtime format lookup against ctx.get_serde_format_registry()),
        // which is still empty until this loop's add_format() calls run; doing it the other way
        // around always failed with "no format plugin loaded for 'application/json'" even with
        // json_plugin correctly built and loaded, since nothing had registered it into the
        // registry yet at the point write_document() ran.
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            if (auto logger_adapter = congelado::heart::LoggerAdapter::register_from(*plugin)) {
                logger_adapter->register_logger(ctx.get_logger_registry());
            }
            if (auto format = congelado::heart::resolve_serde_format(*plugin)) {
                ctx.get_serde_format_registry().add_format(std::move(format));
            }
            if (auto sink = congelado::heart::resolve_event_sink(*plugin)) {
                ctx.get_event_bus_registry().add_sink(std::move(sink));
            }
            if (auto bridge = congelado::heart::resolve_bridge(*plugin)) {
                // The bridge self-identifies via runtime_name() — nothing here needs to guess
                // from the plugin's own display name. broadcast_bridge() pushes it into every
                // already-opened plugin's FfiRuntime and seeds any opened afterward.
                m_store.broadcast_bridge(std::move(bridge));
            }
            if (auto otel_provider = congelado::heart::resolve_otel_provider(*plugin)) {
                // A provider may support any subset of the three signals — each accessor
                // defaults to nullptr, register only whichever ones this plugin actually has.
                if (auto *tracer = otel_provider->get_tracer_provider()) {
                    ctx.get_tracer_registry().add_provider(std::shared_ptr<interfaces::ITracerProvider>(
                        tracer, [](interfaces::ITracerProvider *) {}));
                }
                if (auto *meter = otel_provider->get_meter_provider()) {
                    ctx.get_meter_registry().add_provider(std::shared_ptr<interfaces::IMeterProvider>(
                        meter, [](interfaces::IMeterProvider *) {}));
                }
                if (auto *log_provider = otel_provider->get_log_provider()) {
                    ctx.get_log_record_registry().add_provider(
                        std::shared_ptr<interfaces::ILogRecordProvider>(
                            log_provider, [](interfaces::ILogRecordProvider *) {}));
                }
            }
        });

        // Construct+register the OTel log bridge unconditionally, regardless of whether any
        // log-capable OTel provider was actually found above — OtelLogBridge::emit() is a
        // graceful no-op when LogRecordRegistry ends up empty, same "safe with nothing
        // registered" contract every other registry here already has.
        congelado::heart::OtelLogBridge::install(ctx.get_logger_registry());

        // Writing the doc out is best-effort — a failure here shouldn't take down boot. Must run
        // after the for_each loop above — see that loop's own comment for why.
        if (auto write_res = openapi_generator->write_document("Congelado API", "1.0.0", "openapi.json");
            !write_res) {
            std::println(stderr, "[heart] failed to write openapi document: {}", write_res.error());
        } else {
            std::println("[heart] generated openapi document");
        }

        std::println("[heart] loaded plugins");
        return ctx.get_logger_registry().has_logger();
    }
};

} // namespace congelado::heart
