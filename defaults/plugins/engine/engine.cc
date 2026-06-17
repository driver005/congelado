#include <memory>
#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>

import std;
import interfaces;
import io_shared;
import core_server;
import engine;
import core_logger;

namespace {

class EnginePlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "engine"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        return {};
    }

    [[nodiscard]] std::span<const std::string_view> get_load_before_types() const noexcept override {
        static constexpr std::string_view types[] = {"protocol"};
        return types;
    }

    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_CUSTOM; }

    void on_load(congelado::HostCallbacks const &host,
                 congelado::ConfigView const & /*cfg_view*/) override {
        auto *router_ctx =
            host.router_ctx<core::server::RouterContext<io::shared::http::Protocol>>();

        if (router_ctx == nullptr) {
            core::logger::error("engine", "no router context");
            return;
        }

        engine::register_routes(*router_ctx, m_engine_ctx);
        core::logger::important("engine", "routes registered");
    }

    void on_unload() override {}

  private:
    engine::EngineContext m_engine_ctx;
};

} // namespace

CONGELADO_PLUGIN(EnginePlugin)
