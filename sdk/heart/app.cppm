module;

#include <congelado/abi.h>
#include <cstdio>

export module congelado_heart:app;

import std;
import interfaces;
import core_config;
import core_logger;
import core_plugin;
import utils_openapi;
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

export namespace congelado::heart {

class ServerRunner {
  public:
    /**
     * @brief Builds a runner pointed at the directory it'll scan for plugin shared libraries.
     * @param plugin_dir filesystem directory to scan for plugin `.so`s on `run()`.
     */
    explicit ServerRunner(std::filesystem::path plugin_dir)
        : m_plugin_dir{std::move(plugin_dir)} {}

    /**
     * @brief The whole heart entrypoint — loads config, boots up the app context, loads every
     * plugin off disk, wires the logger, and then blocks forever. This is the W that keeps the
     * process alive.
     * @warning Config-load failure or no logger plugin found both call `std::abort()` — this is
     * a hard boot-time bail, not a recoverable error path. Don't expect a return value on the L.
     * @param config_path path to the `congelado.toml` config file; missing file falls back to
     * defaults rather than failing.
     * @return `0` on success. In practice this never returns, since the happy path parks on a
     * `std::promise<void>` future that's never fulfilled.
     */
    int run(const std::filesystem::path &config_path = "congelado.toml") {
        // Config has to load clean or there's nothing sane to boot with.
        auto cfg = load_config(config_path);
        if (!cfg) {
            std::println(stderr, "[heart] config load failed — aborting");
            std::abort();
        }

        AppContext ctx;
        bool plugin_logger = load_plugins(*cfg, ctx);

        // Need at least one logger plugin loaded, otherwise we're flying blind — not a W.
        if (!plugin_logger) {
            std::println(stderr, "[heart] no logger plugin found — aborting");
            std::abort();
        }

        // Everything's wired up — park here forever, the process stays alive off this future.
        std::println("[heart] finished initialization");
        std::promise<void>().get_future().wait();
        return 0;
    }

  private:
    std::filesystem::path m_plugin_dir;
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
     * generation configs, registers the OpenAPI serve route ahead of time, scans/opens/builds
     * every plugin `.so` in `m_plugin_dir`, writes out the generated OpenAPI document, and wires
     * up a logger adapter for whichever plugin(s) export the logger capability. Big function,
     * does a lot of heavy lifting — that's the motion.
     * @warning Aborts the process (`std::abort()`) if plugin open or plugin build fails — this
     * is boot-time, not a recoverable path.
     * @note The OpenAPI serve route is registered on `ctx.get_router()` *before* any plugin
     * loads, on purpose: protocol plugins (e.g. http2) compile the route trie eagerly inside
     * their own `on_load`, so the route has to already be there or it just won't show up.
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

        // Turn each config-file plugin entry into a GenerationConfig: pick the runtime bit
        // for python/lua types, then fold every extra TOML field into the config's extras map.
        std::unordered_map<std::string, core::plugin::types::GenerationConfig> configs;
        for (const auto &[name, plugin_cfg] : cfg.get_plugins()) {
            core::plugin::types::GenerationConfig gc;
            if (plugin_cfg.get_type() == "python") {
                gc.set_runtimes(gc.get_runtimes() | core::plugin::types::Runtime::PYTHON);
            } else if (plugin_cfg.get_type() == "lua") {
                gc.set_runtimes(gc.get_runtimes() | core::plugin::types::Runtime::LUA);
            }
            for (const auto &[key, value] : plugin_cfg.get_fields()) {
                auto extra = gc.get_extra();
                extra[key] = value;
                gc.set_extra(std::move(extra));
            }
            configs[name] = std::move(gc);
        }

        auto generator =
            utils::openapi::Generator{}.title("Congelado API").version("1.0.0");
        // Registered before plugins load: protocol plugins (e.g. http2) compile the route
        // trie eagerly inside their own on_load, so this must already be in RouterContext
        // by then. The handler builds its body lazily from Registry at request time.
        ctx.get_router()->add_route(generator.serve());

        // Scan the plugin directory, open every discovered .so, then build/init each one
        // against the host callback table and per-plugin generation configs — both steps
        // are boot-time hard-fails, no partial-load recovery.
        m_store.scan(m_plugin_dir);
        auto open_res = m_store.open_all();
        if (!open_res) {
            std::println(stderr, "[heart] plugin load failed: {} — aborting",
                         open_res.error().get_message());
            std::abort();
        }
        auto build_res = m_store.build(cb, configs);
        if (!build_res) {
            std::println(stderr, "[heart] plugin build failed: {} — aborting",
                         build_res.error().get_message());
            std::abort();
        }

        // Writing the doc out is best-effort — a failure here shouldn't take down boot.
        if (auto write_res = generator.write(generator.generate()); !write_res) {
            std::println(stderr, "[heart] failed to write openapi document: {}", write_res.error());
        } else {
            std::println("[heart] generated openapi document");
        }

        // Walk every loaded plugin and wire up a LoggerAdapter for whichever one(s)
        // actually export the logger capability — plugins with no logger just get skipped.
        m_store.for_each([&](const std::shared_ptr<core::plugin::FfiRuntime> &runtime) {
            auto plugin = runtime->get_plugin();
            if (!plugin) {
                return;
            }
            auto logger_adapter = congelado::heart::LoggerAdapter::register_from(*plugin);
            if (logger_adapter) {
                logger_adapter->register_logger();
            }
        });

        std::println("[heart] loaded plugins");
        return core::logger::LoggerRegistry::has_logger();
    }
};

} // namespace congelado::heart
