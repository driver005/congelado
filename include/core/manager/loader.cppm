export module core_plugin:loader;

import std;
import core_ffi;
import core_config;
import :handle;

export namespace core::plugin {

// Probe phase: opens the .so and reads metadata. Does NOT call on_load.
// Call bridge->activate() after dependency sorting.
[[nodiscard]]
inline std::expected<PluginHandle, LoadError>
open(const std::filesystem::path &path,
     const core::config::PluginConfig *plugin_cfg = nullptr) {
    return core::ffi::FfiBridge::open(path, plugin_cfg);
}

} // namespace core::plugin
