module;

#include <stdio.h>

export module core_heart:app;

import std;
import interfaces;
import core_config;
import core_logger;
import core_plugin;
import congelado;
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
            std::println(stderr, "[heart] no protocol plugin found — load a plugin with Cap::Protocol");
            std::abort();
        }

        app::Server server{*proto};

        // plugins must outlive server — both live until process termination.
        std::promise<void>().get_future().wait();
        return 0;
    }

  private:
    std::filesystem::path m_plugin_dir;

    std::optional<core::config::Config> load_config(const std::filesystem::path &raw_path) {
        std::filesystem::path path = expand_tilde(raw_path);

        if (path.empty() || !std::filesystem::exists(path)) {
            std::println("[heart] no config file at '{}', using defaults", path.empty() ? "<none>" : path.string());
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

    // Loads all plugins from config. Registers all logger plugins.
    // Sets proto to the first protocol plugin found.
    // Passes ctx.router_ptr() to every plugin so all can add routes to the
    // same global RouterContext during their on_load().
    // Returns true if any logger plugin was registered.
    bool load_plugins(const core::config::Config &cfg, AppContext &ctx,
                      std::vector<core::plugin::PluginHandle> &handles,
                      std::shared_ptr<interfaces::IProtocol> &proto) {
        void *router_ctx = ctx.router_ptr();

        for (auto &[name, plugin_cfg] : cfg.plugins) {
#if defined(_WIN32)
            auto so = m_plugin_dir / (name + ".dll");
#else
            auto so = m_plugin_dir / ("lib" + name + ".so");
#endif
            if (!std::filesystem::exists(so)) {
                std::println(stderr, "[heart] plugin '{}' not found at '{}'", name, so.string());
                continue;
            }

            auto result = core::plugin::load(so, &plugin_cfg, router_ctx);
            if (!result) {
                std::println(stderr, "[heart] plugin '{}' failed to load: {}", name, result.error().detail);
                continue;
            }

            std::println("[heart] loaded plugin '{}'", name);
            handles.push_back(*result);

            if (auto logger = core::plugin::make_logger(*result)) {
                core::logger::LoggerRegistry::register_logger(std::move(logger));
            }

            if (!proto) {
                proto = core::plugin::make_protocol(*result);
            }
        }

        return core::logger::LoggerRegistry::has_logger();
    }
};

} // namespace core::heart
