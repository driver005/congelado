export module core_plugin:loader;

import std;
import core_ffi;
import core_config;
import :handle;

export namespace core::plugin {

[[nodiscard]]
inline std::expected<PluginHandle, LoadError>
load(const std::filesystem::path &path, const core::config::PluginConfig *plugin_cfg = nullptr,
     void *router_ctx = nullptr, void *controller_ctx = nullptr, void *leverager_ctx = nullptr) {
    auto result = core::ffi::FfiBridge::load(path, plugin_cfg, router_ctx, controller_ctx, leverager_ctx);
    if (!result) {
        return std::unexpected(std::move(result.error()));
    }
    return std::move(*result);
}

} // namespace core::plugin
