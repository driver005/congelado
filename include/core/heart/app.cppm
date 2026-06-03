module;

#include <stdio.h>

export module core_heart:app;

import std;
import interfaces;
import core_config;
import core_logger;
import core_plugin;
import :context;

std::filesystem::path expand_tilde(std::filesystem::path path) {
    std::string path_str = path.string();
    if (!path_str.empty() && path_str[0] == '~') {
        const char *home = std::getenv("HOME");
        if (home) {
            path_str.replace(0, 1, home);
        }
    }
    return std::filesystem::path(path_str);
}

export namespace core::heart {

class App {
  public:
    explicit App(std::filesystem::path plugin_dir = {}) : m_plugin_dir{std::move(plugin_dir)} {}

    int run(const std::filesystem::path &config_path = "congelado.toml") {
        auto cfg = load_config(config_path);
        if (!cfg) {
            std::println(stderr, "[heart] config load failed — aborting");
            std::abort();
        }

        AppContext ctx;
        std::vector<core::plugin::PluginHandle> plugins;
        std::shared_ptr<interfaces::IProtocol> proto;
        bool plugin_logger = load_plugins(*cfg, ctx, plugins, proto);

        if (!plugin_logger) {
            std::println(stderr, "[heart] no logger plugin found — aborting");
            std::abort();
        }

        if (!proto) {
            std::println(stderr,
                         "[heart] no protocol plugin found — load a plugin with Cap::Protocol");
            std::abort();
        }

        proto->build(ctx.get_router());

        // Block forever until process termination.
        std::promise<void>().get_future().wait();
        return 0;
    }

  private:
    std::filesystem::path m_plugin_dir;

    std::optional<core::config::Config> load_config(const std::filesystem::path &raw_path) {
        std::filesystem::path path = expand_tilde(raw_path);

        if (path.empty() || !std::filesystem::exists(path)) {
            std::println("[heart] no config file at '{}', using defaults",
                         path.empty() ? "<none>" : path.string());
            return core::config::Config{};
        }

        auto result = core::config::load(path);
        if (!result) {
            std::println(stderr, "[heart] config error: {}", result.error());
            return std::nullopt;
        }

        std::println("[heart] loaded config from '{}'", path.string());
        return std::move(*result);
    }

    // Probes, filters, sorts, and activates all plugins from config.
    // Phase 1: probe (open .so, read metadata).
    // Phase 2: uniqueness filter — skip duplicates by type tag.
    // Phase 3: dependency sort (Kahn's algorithm) — abort on missing dep or cycle.
    // Phase 4: activate in sorted order — register loggers, find protocol.
    // Returns true if any logger plugin registered.
    bool load_plugins(const core::config::Config &cfg, AppContext &ctx,
                      std::vector<core::plugin::PluginHandle> &handles,
                      std::shared_ptr<interfaces::IProtocol> &proto) {
        // ── Phase 1: probe ────────────────────────────────────────────────────
        std::vector<core::plugin::PluginHandle> probed;

        for (auto &[name, plugin_cfg] : cfg.get_plugins()) {
#if defined(_WIN32)
            auto so = m_plugin_dir / (name + ".dll");
#else
            auto so = m_plugin_dir / ("lib" + name + ".so");
#endif
            if (!std::filesystem::exists(so)) {
                std::println(stderr, "[heart] plugin '{}' not found at '{}'", name, so.string());
                continue;
            }

            auto result = core::plugin::open(so, &plugin_cfg);
            if (!result) {
                std::println(stderr, "[heart] plugin '{}' failed to open: {}", name,
                             result.error().get_detail());
                continue;
            }

            probed.push_back(std::move(*result));
        }

        // ── Phase 2: uniqueness filter ────────────────────────────────────────
        std::unordered_map<std::string, std::string> seen_types; // type_tag → first plugin name
        std::vector<core::plugin::PluginHandle> surviving;

        for (auto &bridge : probed) {
            auto unique_type = std::string{bridge->get_unique_type()};
            if (unique_type.empty()) {
                surviving.push_back(bridge);
                continue;
            }
            if (!seen_types.contains(unique_type)) {
                seen_types[unique_type] = std::string{bridge->get_name()};
                surviving.push_back(bridge);
            } else {
                std::println(stderr,
                             "[heart] plugin '{}' skipped — unique type '{}' already claimed by '{}'",
                             bridge->get_name(), unique_type, seen_types[unique_type]);
            }
        }

        // ── Phase 3: dependency sort (Kahn's algorithm) ───────────────────────
        std::unordered_map<std::string, core::plugin::PluginHandle> name_map;
        for (auto &bridge : surviving)
            name_map[std::string{bridge->get_name()}] = bridge;

        // Verify all declared requirements are present.
        for (auto &bridge : surviving) {
            for (auto &req : bridge->get_requires()) {
                if (!name_map.contains(req)) {
                    std::println(stderr,
                                 "[heart] plugin '{}' requires '{}' which is not loaded — aborting",
                                 bridge->get_name(), req);
                    std::abort();
                }
            }
        }

        // Build in-degree and adjacency list.
        std::unordered_map<std::string, int>                      in_degree;
        std::unordered_map<std::string, std::vector<std::string>> dependents;

        for (auto &bridge : surviving) {
            auto name = std::string{bridge->get_name()};
            in_degree.try_emplace(name, 0);
            for (auto &req : bridge->get_requires()) {
                dependents[req].push_back(name);
                ++in_degree[name];
            }
        }

        std::queue<std::string> ready;
        for (auto &[name, deg] : in_degree) {
            if (deg == 0)
                ready.push(name);
        }

        std::vector<core::plugin::PluginHandle> sorted;
        sorted.reserve(surviving.size());
        while (!ready.empty()) {
            auto name = ready.front();
            ready.pop();
            sorted.push_back(name_map[name]);
            for (auto &dependent : dependents[name]) {
                if (--in_degree[dependent] == 0)
                    ready.push(dependent);
            }
        }

        if (sorted.size() != surviving.size()) {
            std::println(stderr, "[heart] plugin dependency cycle detected — aborting");
            std::abort();
        }

        // ── Phase 4: activate ─────────────────────────────────────────────────
        auto *router_ctx     = ctx.get_router();
        auto *controller_ctx = &ctx.get_contract_group();
        auto *leverager_ctx  = &ctx.get_leverager();

        for (auto &bridge : sorted) {
            bridge->activate(router_ctx, controller_ctx, leverager_ctx);
            handles.push_back(bridge);
            std::println("[heart] loaded plugin '{}'", bridge->get_name());

            if (auto logger = core::plugin::make_logger(bridge)) {
                core::logger::LoggerRegistry::register_logger(std::move(logger));
            }
            if (!proto) {
                proto = core::plugin::make_protocol(bridge);
            }
        }

        return core::logger::LoggerRegistry::has_logger();
    }
};

} // namespace core::heart
