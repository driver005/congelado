module;

#include <congelado/abi.h>
#include <stdio.h>

export module core_heart:app;

import std;
import interfaces;
import core_config;
import core_logger;
import core_plugin;
import worker;
import :context;
import :worker;
import :adapters;

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
    explicit App(std::filesystem::path plugin_dir = {}) : m_plugin_dirs{std::move(plugin_dir)} {}
    explicit App(std::vector<std::filesystem::path> plugin_dirs)
        : m_plugin_dirs{std::move(plugin_dirs)} {}

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
    std::vector<std::filesystem::path> m_plugin_dirs;
    core::plugin::SharedLibrary m_store;
    std::vector<core::plugin::FfiRuntime> m_runtimes;
    std::vector<std::unique_ptr<PluginWorker>> m_workers;
    std::unique_ptr<worker::WorkerContext> m_worker_ctx;

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
        CongeladoHostCallbacks cb{};
        cb.router_ctx = ctx.get_router();
        cb.controller_ctx = &ctx.get_contract_group();
        cb.leverager_ctx = &ctx.get_leverager();

        std::unordered_map<std::string, core::plugin::types::GenerationConfig> configs;
        for (auto &[name, plugin_cfg] : cfg.get_plugins()) {
            core::plugin::types::GenerationConfig gc;
            if (plugin_cfg.get_type() == "python")
                gc.set_runtimes(gc.get_runtimes() | core::plugin::types::Runtime::PYTHON);
            else if (plugin_cfg.get_type() == "lua")
                gc.set_runtimes(gc.get_runtimes() | core::plugin::types::Runtime::LUA);
            for (auto &[k, v] : plugin_cfg.get_fields()) {
                auto extra = gc.get_extra();
                extra[k] = v;
                gc.set_extra(std::move(extra));
            }
            configs[name] = std::move(gc);
        }

        for (auto &dir : m_plugin_dirs)
            m_store.scan(dir);
        auto open_res = m_store.open_all();
        if (!open_res) {
            std::println(stderr, "[heart] plugin load failed: {} — aborting",
                         open_res.error().get_message());
            std::abort();
        }
        auto runtimes = m_store.build(cb, configs);
        if (!runtimes) {
            std::println(stderr, "[heart] plugin build failed: {} — aborting",
                         runtimes.error().get_message());
            std::abort();
        }
        m_runtimes = std::move(*runtimes);

        m_worker_ctx = std::make_unique<worker::WorkerContext>("main");
        m_store.for_each([&](core::plugin::PluginRef &ref) {
            if (auto it = ref.m_data.find("congelado_worker_type"); it != ref.m_data.end()) {
                auto &type = std::any_cast<const std::string &>(it->second);
                if (type.empty())
                    return;
                auto *raw = std::any_cast<void *>(ref.m_data.at("congelado_worker_execute"));
                auto exec_fn = reinterpret_cast<core::plugin::types::WorkerExecuteFn>(raw);
                auto pw = std::make_unique<PluginWorker>(type, exec_fn);
                m_worker_ctx->add_task_worker(pw.get());
                m_workers.push_back(std::move(pw));
            }

            auto logger_adapter = core::heart::LoggerAdapter::register_from(ref);
            if (logger_adapter) {
                logger_adapter->register_logger();
            }
        });

        std::println("[heart] loaded {} plugins, {} workers", m_runtimes.size(), m_workers.size());
        return core::logger::LoggerRegistry::has_logger();
    }
};

} // namespace core::heart
