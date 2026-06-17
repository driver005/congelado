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
        if (home)
            path_str.replace(0, 1, home);
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
        bool plugin_logger = load_plugins(*cfg, ctx);

        if (!plugin_logger) {
            std::println(stderr, "[heart] no logger plugin found — aborting");
            std::abort();
        }

        std::println("[heart] finished initialization");
        std::promise<void>().get_future().wait();
        return 0;
    }

  private:
    std::filesystem::path m_plugin_dir;
    core::plugin::PluginManager m_manager;

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

    bool load_plugins(const core::config::Config &cfg, AppContext &ctx) {
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
            m_manager.add_plugin(so, &plugin_cfg);
        }

        m_manager.activate(ctx.get_router(), &ctx.get_contract_group(), &ctx.get_leverager());

        std::println("[heart] loaded {} plugins", m_manager.get_all().size());
        return core::logger::LoggerRegistry::has_logger();
    }
};

} // namespace core::heart
