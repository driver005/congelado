module;

#include <congelado/abi.h>
#include <csignal>
#include <cstdio>

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
import connector;
import migration;
import core_contract;
import core_router;
import :context;
import :adapters;

std::filesystem::path expand_tilde(const std::filesystem::path &path) {
    std::string path_str = path.string();
    // Only a leading '~' gets expanded — and only if HOME is actually set, otherwise the
    // path is left untouched rather than half-rewritten.
    if (!path_str.empty() &&
        path_str[0] == '~') { // FIXME(clang-tidy): unchecked operator[], consider .at()
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

        AppContext ctx(cfg->get_threads().value_or(std::thread::hardware_concurrency()));
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
        // Signal every plugin to begin graceful shutdown first while the contract pool is still
        // running. Protocol plugins need the pool to drain open connections (send GOAWAY, flush
        // in-flight responses, close idle sockets). Once plugins have finished draining, stop the
        // contract thread pool and release the connector contract. Only after the pool has joined
        // do we unload plugins via close_all(), so no worker thread touches plugin state after
        // on_unload() runs.
        m_store.shutdown_plugins();
        ctx.stop();
        std::println("[heart] signaled protocol layer to close connections");
        std::println("[heart] shutdown requested — tearing down");
        // Detach every OTel log-record provider before any plugin gets unloaded — a plugin's
        // own on_unload()/Shutdown() can itself log (OTel's SDK does), which would otherwise
        // fan straight back out through OtelLogBridge into the very provider mid-teardown.
        // Confirmed live as a segfault without this.
        ctx.get_log_record_registry().clear();
        // Explicit, not left to fall out of scope: close_all() must run here, before `ctx`
        // (below) goes out of scope at the end of this function. `ctx` owns the ambient
        // logger/event/otel registries every plugin's on_unload() (and any background thread
        // it stops along the way, e.g. OTel's batch span/log processors) may still touch —
        // relying on m_store's own destructor for this (as before) fires too late: that only
        // runs once the ServerRunner temporary in main.cc is destroyed, well after `ctx` here
        // has already been torn down, leaving a live window for a plugin's background thread
        // to dereference an already-dead registry. Safe to also let m_store's destructor run
        // close_all() again later — it's idempotent once m_runtimes/m_order are empty.
        m_store.close_all();
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
     * the logger capability. Delegates each phase to a private helper below, in this fixed
     * order — the ordering constraints (capability resolution before `build()`, the post-build
     * walk before `write_document()`, migrations after both) are documented on the helpers that
     * actually need them.
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
        CongeladoHostCallbacks cb = make_host_callbacks(ctx);
        auto configs = make_generation_configs(cfg);

        scan_and_open_plugins();
        discard_unlisted_provider_gated_plugins(cfg);

        cb.database_ctx = resolve_database_capability(cfg);
        auto generator_registry = resolve_openapi_generators();
        cb.lua_bridge_ctx = resolve_lua_bridge_capability();
        cb.search_ctx = resolve_search_capability(cfg);
        cb.cache_ctx = resolve_cache_capability(cfg);
        cb.cron_ctx = resolve_cron_capability(cfg);

        wire_shared_connector(ctx, cb);

        // The OpenAPI generator is mandatory at startup, same as the logger — no plugin
        // exporting the OPENAPI capability means there's no way to serve/write the API
        // document at all, so this aborts boot rather than silently degrading.
        if (!generator_registry.has_generator()) {
            std::println(stderr, "[heart] no OpenAPI generator plugin found — aborting");
            std::abort();
        }
        auto *openapi_generator = generator_registry.get_generators().front().get();

        // Register the global content-negotiation middleware BEFORE build_plugins() — the http2
        // plugin's build() consumes (moves out) the RouterContext, so anything added afterward
        // is lost. The middleware reads the serde format registry lazily at request time, so it
        // doesn't matter that formats aren't wired up until wire_post_build_capabilities() below;
        // no request is served until load_plugins() returns.
        ctx.get_router()->add_global_middleware(serde::content_negotiation_middleware);

        build_plugins(cb, configs);
        wire_post_build_capabilities(ctx);
        run_migrations(cfg, ctx, cb);

        // Construct+register the OTel log bridge unconditionally, regardless of whether any
        // log-capable OTel provider was actually found above — OtelLogBridge::emit() is a
        // graceful no-op when LogRecordRegistry ends up empty, same "safe with nothing
        // registered" contract every other registry here already has.
        congelado::heart::OtelLogBridge::install(ctx.get_logger_registry());

        write_openapi_document(openapi_generator);

        std::println("[heart] loaded plugins");
        return ctx.get_logger_registry().has_logger();
    }

    /**
     * @brief Top-level `[providers]` table lookup — e.g. `database = "postgres"`, `search =
     * "elasticsearch"`, `logger = ["file_logger", "otel_otlp_plugin"]` — names which plugin(s)
     * get picked for a given capability, by stem (the `[plugins.<stem>]` section key, not the
     * plugin's own get_name()).
     * @param cfg the loaded config.
     * @param capability the capability name to look up (e.g. `"database"`).
     * @return the first configured provider stem for `capability`, or `std::nullopt` if absent
     * or empty — falls back to that capability's own pre-existing default resolution (first one
     * found for single-active capabilities, every one found for logger).
     */
    static std::optional<std::string> preferred_provider(const core::config::Config &cfg,
                                                          std::string_view capability) {
        auto it = cfg.get_providers().find(std::string{capability});
        if (it == cfg.get_providers().end() || it->second.empty()) {
            return std::nullopt;
        }
        return it->second.front();
    }

    /**
     * @brief Checks whether `stem` is listed under `capability` in the top-level `[providers]`
     * table.
     * @param cfg the loaded config.
     * @param capability the capability name to look up (e.g. `"events"`).
     * @param stem the plugin stem to check for.
     * @return `true` if listed, or if `[providers]` has no entry at all for `capability` (every
     * plugin's in by default); `false` if the entry exists but doesn't name `stem`.
     */
    static bool is_provider_listed(const core::config::Config &cfg, std::string_view capability,
                                   std::string_view stem) {
        auto it = cfg.get_providers().find(std::string{capability});
        if (it == cfg.get_providers().end()) {
            return true; // no [providers] entry at all — every plugin's in by default
        }
        return std::ranges::find(it->second, stem) != it->second.end();
    }

    /**
     * @brief Derives a plugin's stem (its `.so` filename minus the `lib` prefix and extension)
     * from its own `PluginRef` metadata — the same identity `[providers]` entries key off.
     * @param plugin the plugin to derive a stem for.
     * @return the derived stem, or an empty string if the plugin has no `"path"` metadata.
     */
    static std::string plugin_stem(core::plugin::types::PluginRef &plugin) {
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
    }

    /**
     * @brief Builds the host callback table skeleton every plugin gets handed at load time —
     * router, contract group, contract registry, and leverager all point back into `ctx`.
     * @param ctx the app context supplying each pointer.
     * @return the populated skeleton; `load_plugins()` fills in the remaining capability-derived
     * fields (`database_ctx`, `connector_ctx`, etc.) afterward.
     */
    static CongeladoHostCallbacks make_host_callbacks(AppContext &ctx) {
        CongeladoHostCallbacks cb{};
        cb.router_ctx = ctx.get_router();
        cb.controller_ctx = &ctx.get_contract_group();
        cb.registry_ctx = &ctx.get_contract_registry();
        cb.leverager_ctx = &ctx.get_leverager();
        return cb;
    }

    /**
     * @brief Turns each config-file plugin entry into a `GenerationConfig`: the plugin's
     * declared type is added as a wanted runtime name unconditionally — an open set, not a
     * fixed python/lua pair, so a bridge for any user-registered runtime name works with no
     * code change here. A plugin with no matching bridge just gets a lookup miss later, same as
     * an unregistered serde format.
     * @param cfg the loaded config.
     * @return one `GenerationConfig` per configured plugin, keyed by plugin name.
     */
    static std::unordered_map<std::string, core::plugin::types::GenerationConfig>
    make_generation_configs(const core::config::Config &cfg) {
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
        return configs;
    }

    /**
     * @brief Scans the plugin directory (or directories) and opens every discovered `.so` —
     * boot-time hard-fail, no partial-load recovery. External scans first, if given, then the
     * internal (build-output) directory, all before the single `open_all()`/`for_each()` pass
     * below.
     */
    void scan_and_open_plugins() {
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
    }

    /**
     * @brief Provider-gated capabilities (events, logger) get filtered here, right after
     * open(), before build() ever calls congelado_init on them — resolve_event_sink()/
     * LoggerAdapter::register_from() work off open()-populated symbols alone, same established
     * precedent as resolve_storage()/resolve_bridge() being called pre-build() in the resolve_*
     * helpers below. A plugin like kafka_events_plugin sitting in the plugins directory but
     * absent from `[providers] events = [...]` must never actually connect to a broker —
     * discarding it here (before congelado_init) is the difference between "never loaded" and
     * "loaded, just not registered", which a post-build-only check couldn't give.
     * @param cfg the loaded config, supplying the `[providers]` table.
     */
    void discard_unlisted_provider_gated_plugins(const core::config::Config &cfg) {
        std::vector<std::string> to_discard;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto stem = plugin_stem(*plugin);
            bool reject = (congelado::heart::resolve_event_sink(*plugin) &&
                          !is_provider_listed(cfg, "events", stem)) ||
                         (congelado::heart::LoggerAdapter::register_from(*plugin) &&
                          !is_provider_listed(cfg, "logger", stem));
            if (reject) {
                to_discard.push_back(std::any_cast<const std::string &>(plugin->m_data.at("name")));
            }
        });
        for (const auto &name : to_discard) {
            m_store.discard(name);
        }
    }

    /**
     * @brief Resolves a storage-capable plugin's IDatabase* BEFORE build() runs (not after,
     * like the post-build walk in wire_post_build_capabilities()) — capability dlsym resolution
     * doesn't depend on on_load/congelado_init having run (those symbols are already populated
     * by open()), but the post-build walk happens after every plugin's on_load (including the
     * engine plugin's) has already finished — too late for EngineContext::set_db() to ever
     * receive it through that path. `[providers] database = "..."` picks a specific one by
     * stem; with none set (or the named plugin not actually loaded), first IDatabase found
     * wins, matching Connector's existing single-backend-at-a-time design.
     * @param cfg the loaded config, supplying the `[providers] database` preference.
     * @return the resolved (or first-found fallback) `IDatabase*`, or `nullptr` if none found.
     */
    [[nodiscard]] void *resolve_database_capability(const core::config::Config &cfg) {
        auto preferred_database = preferred_provider(cfg, "database");
        void *resolved_database = nullptr;
        void *fallback_database = nullptr;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            if (resolved_database != nullptr) {
                return;
            }
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto database = congelado::heart::resolve_storage(*plugin);
            if (!database) {
                return;
            }
            if (fallback_database == nullptr) {
                fallback_database = database.get();
            }
            if (preferred_database && plugin_stem(*plugin) == *preferred_database) {
                resolved_database = database.get();
            }
        });
        return resolved_database != nullptr ? resolved_database : fallback_database;
    }

    /**
     * @brief Resolves every OpenAPI-generator-capable plugin BEFORE build() runs, same "resolve
     * before build()" reasoning as resolve_database_capability() — the live serve route has to
     * land in RouterContext before build() runs any plugin's on_load (protocol plugins like
     * http2 compile the route trie eagerly in on_load), so this can't wait for the post-build
     * walk in wire_post_build_capabilities().
     * @return the registry of every resolved generator (checked via has_generator() by the
     * caller, which also extracts the one actually used for write_document()).
     */
    [[nodiscard]] utils::openapi::OpenApiGeneratorRegistry resolve_openapi_generators() {
        utils::openapi::OpenApiGeneratorRegistry generator_registry;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            if (auto generator = congelado::heart::resolve_openapi_generator(*plugin)) {
                generator_registry.add_generator(std::move(generator));
            }
        });
        return generator_registry;
    }

    /**
     * @brief Same "resolve before build()" reasoning, for the one bridge runtime a host-side
     * on_load hook currently needs early: the engine plugin's Lua condition evaluator
     * (SWITCH/DO_WHILE/EventHandler conditions). Kept as its own walk rather than folded into
     * resolve_database_capability() since it's filtering on a different predicate
     * (runtime_name() rather than "resolves at all") — unlike database_ctx's "first IDatabase
     * wins" (Connector only ever has one backend at a time anyway), a deployment can have both
     * python_bridge and lua_bridge loaded at once, and "first bridge found" would be a coin
     * flip on load order, so this specifically waits for one whose runtime_name() is "lua".
     * @return the resolved lua `IBridge*`, or `nullptr` if none found.
     */
    [[nodiscard]] void *resolve_lua_bridge_capability() {
        void *resolved_lua_bridge = nullptr;
        m_store.for_each(
            [&resolved_lua_bridge](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
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
        return resolved_lua_bridge;
    }

    /**
     * @brief Same "resolve before build()" reasoning, for a search-capable plugin's
     * ISearchProvider* — the engine plugin's on_load wires this into EngineContext for its
     * terminal-transition SummaryProjector. More than one search-capable plugin can be loaded
     * at once here (postgres_plugin doubles as one; a separate elasticsearch_plugin is a whole
     * other one) — `[providers] search = "..."` picks a specific one by stem, same pattern as
     * resolve_database_capability() above; with none set, first one found wins.
     * @param cfg the loaded config, supplying the `[providers] search` preference.
     * @return the resolved (or first-found fallback) `ISearchProvider*`, or `nullptr` if none
     * found.
     */
    [[nodiscard]] void *resolve_search_capability(const core::config::Config &cfg) {
        auto preferred_search = preferred_provider(cfg, "search");
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
        return resolved_search != nullptr ? resolved_search : fallback_search;
    }

    /**
     * @brief Same "resolve before build()" reasoning, for a cache-capable plugin's
     * interfaces::ICache* — the engine plugin's on_load wires this into its own Connector via
     * set_cache(). More than one cache-capable plugin can be loaded at once — `[providers]
     * cache = "..."` picks a specific one by stem, same pattern as
     * resolve_database_capability()/resolve_search_capability() above; with none set, first one
     * found wins. No cache-capable plugin found at all just leaves Connector on its own
     * in-process LocalCache fallback, same graceful-degradation story as database/search.
     * @param cfg the loaded config, supplying the `[providers] cache` preference.
     * @return the resolved (or first-found fallback) `ICache*`, or `nullptr` if none found.
     */
    [[nodiscard]] void *resolve_cache_capability(const core::config::Config &cfg) {
        auto preferred_cache = preferred_provider(cfg, "cache");
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
        return resolved_cache != nullptr ? resolved_cache : fallback_cache;
    }

    /**
     * @brief Same "resolve before build()" reasoning, for a cron-capable plugin's
     * interfaces::ICron* — the engine plugin's on_load installs its fire callback and seeds its
     * existing WorkflowSchedules into the backend. `[providers] cron = "..."` picks a specific one
     * by stem, same pattern as resolve_search_capability()/resolve_cache_capability() above; with
     * none set, first one found wins. No cron-capable plugin found at all leaves cron_ctx null, so
     * the engine logs the misconfiguration and schedules never fire.
     * @param cfg the loaded config, supplying the `[providers] cron` preference.
     * @return the resolved (or first-found fallback) `ICron*`, or `nullptr` if none found.
     */
    [[nodiscard]] void *resolve_cron_capability(const core::config::Config &cfg) {
        auto preferred_cron = preferred_provider(cfg, "cron");
        void *resolved_cron = nullptr;
        void *fallback_cron = nullptr;
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            if (resolved_cron != nullptr) {
                return;
            }
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto cron = congelado::heart::resolve_cron_provider(*plugin);
            if (!cron) {
                return;
            }
            if (fallback_cron == nullptr) {
                fallback_cron = cron.get();
            }
            if (preferred_cron && plugin_stem(*plugin) == *preferred_cron) {
                resolved_cron = cron.get();
            }
        });
        return resolved_cron != nullptr ? resolved_cron : fallback_cron;
    }

    /**
     * @brief Wires the resolved DB/cache backends (already populated on `cb` by the caller)
     * into the shared SDK connector (owned by `ctx`), registers that connector as a contract
     * worker with a wake callback so queued DB operations schedule it when idle, and stashes
     * the pointer into `cb.connector_ctx` for plugins that need it.
     * @note Starts the contract IDLE, not SCHEDULED — the contract is only scheduled when
     * enqueue() transitions the pending queue from empty to non-empty; starting SCHEDULED would
     * make the very first enqueue() try to schedule an already-scheduled contract and abort.
     * The contract handle is stored in the registry so `stop()` can release it before the
     * thread pool joins.
     * @param ctx the app context owning the shared connector/contract group/contract registry.
     * @param cb the host callback table; `database_ctx`/`cache_ctx` must already be populated,
     * `connector_ctx` gets written here.
     */
    void wire_shared_connector(AppContext &ctx, CongeladoHostCallbacks &cb) {
        auto *shared_connector = ctx.get_connector();
        shared_connector->set_database(static_cast<interfaces::IDatabase *>(cb.database_ctx));
        shared_connector->set_cache(static_cast<interfaces::ICache *>(cb.cache_ctx));

        auto connector_contract =
            shared_connector->create(ctx.get_contract_group(), core::contract::ContractState::IDLE);
        shared_connector->set_wake([c = connector_contract]() mutable { c.schedule(); });
        ctx.get_contract_registry().add(std::move(connector_contract));

        cb.connector_ctx = shared_connector;
    }

    /**
     * @brief Builds/inits every opened plugin against the host callback table and per-plugin
     * generation configs — boot-time hard-fail, no partial-build recovery.
     * @note The live /openapi route itself is self-registered by the openapi_generator plugin's
     * own on_load (declares get_load_before_types() == {"protocol"}, same ordering constraint
     * this used to enforce manually here) — nothing to do on this side beyond calling build().
     * @param cb the fully-populated host callback table.
     * @param configs per-plugin generation configs, keyed by plugin name.
     */
    void build_plugins(
        const CongeladoHostCallbacks &cb,
        const std::unordered_map<std::string, core::plugin::types::GenerationConfig> &configs) {
        auto build_res = m_store.build(cb, configs);
        if (!build_res) {
            std::println(stderr, "[heart] plugin build failed: {} — aborting",
                         build_res.error().get_message());
            std::abort();
        }
    }

    /**
     * @brief Walks every loaded plugin and wires up a LoggerAdapter/format/bridge/OTel provider
     * for whichever one(s) actually export that capability — plugins without it just get
     * skipped. Logger and events are the two capabilities where more than one provider is
     * genuinely meant to be active at once (LoggerRegistry fans out to every registered sink,
     * same for event sinks) — `[providers]` filtering for both already happened in
     * discard_unlisted_provider_gated_plugins(), before build(), so every plugin still in
     * m_store here has already earned its spot; no re-check needed on this pass.
     * @warning Must run BEFORE write_openapi_document() — that call dispatches through
     * serde::Ser (runtime format lookup against ctx.get_serde_format_registry()), which is
     * still empty until this walk's add_format() calls run; doing it the other way around
     * always failed with "no format plugin loaded for 'application/json'" even with json_plugin
     * correctly built and loaded, since nothing had registered it into the registry yet.
     * @param ctx the app context whose logger/serde/event/otel registries get populated.
     */
    void wire_post_build_capabilities(AppContext &ctx) {
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
                    ctx.get_tracer_registry().add_provider(
                        std::shared_ptr<interfaces::ITracerProvider>(
                            tracer, [](interfaces::ITracerProvider *) {}));
                }
                if (auto *meter = otel_provider->get_meter_provider()) {
                    ctx.get_meter_registry().add_provider(
                        std::shared_ptr<interfaces::IMeterProvider>(
                            meter, [](interfaces::IMeterProvider *) {}));
                }
                if (auto *log_provider = otel_provider->get_log_provider()) {
                    ctx.get_log_record_registry().add_provider(
                        std::shared_ptr<interfaces::ILogRecordProvider>(
                            log_provider, [](interfaces::ILogRecordProvider *) {}));
                }
            }
        });
    }

    /**
     * @brief Global migration run — exactly once, after every plugin has both loaded (on_load)
     * and gone ready (on_ready), so every plugin's migration::Registry baseline is registered,
     * and after wire_post_build_capabilities() has wired the SQL dialect serde format in (the
     * create_table<T>() DDL a baseline emits dispatches through serde::Ser, which needs that
     * format loaded first). Running this per-plugin — the old approach, inside the engine
     * plugin's own on_load — meant any plugin loaded after engine never got its baseline picked
     * up before the run happened; migrations are a host-owned, cross-plugin concern.
     * @param cfg the loaded config, supplying the migrations directory.
     * @param ctx the app context whose shared connector runs the migration queries.
     * @param cb the host callback table, supplying the resolved `database_ctx`.
     */
    void run_migrations(const core::config::Config &cfg, AppContext &ctx,
                       const CongeladoHostCallbacks &cb) {
        auto *migration_db = static_cast<interfaces::IDatabase *>(cb.database_ctx);
        if (migration_db != nullptr && migration_db->is_connected()) {
            core::logger::info("heart", "running migrations from {}", cfg.get_migrations_dir());
            if (!migration::Runner::run_all_blocking(migration_db, ctx.get_connector(),
                                                     cfg.get_migrations_dir())) {
                core::logger::fatal("heart", "migrations failed — aborting startup");
                std::abort();
            }
            core::logger::important("heart", "migrations complete");
        } else if (migration_db != nullptr) {
            core::logger::warning("heart",
                                  "database resolved but not connected — skipping migrations");
        }
        // Success, skip-no-DB, or skip-not-connected all count as "the global pass is as done as
        // it's going to get" — signal it either way so anything waiting on migration::Status
        // (e.g. a plugin's background sweep thread that can't safely start until its own tables
        // are guaranteed to exist) doesn't block forever.
        migration::Status::mark_ready();
    }

    /**
     * @brief Writes the OpenAPI document out — best-effort, a failure here shouldn't take down
     * boot. Must run after wire_post_build_capabilities() — see that function's own docs for
     * why.
     * @param generator the resolved OpenAPI generator to write the document with.
     */
    static void write_openapi_document(interfaces::IOpenApiGenerator *generator) {
        if (auto write_res = generator->write_document("Congelado API", "1.0.0", "openapi.json");
            !write_res) {
            std::println(stderr, "[heart] failed to write openapi document: {}", write_res.error());
        } else {
            std::println("[heart] generated openapi document");
        }
    }
};

} // namespace congelado::heart
