module core.manager.loader;
@nogc nothrow:

import core.ffi.bridge   : FfiBridge, LoadError;
import core.config.types : PluginConfig;
import core.manager.handle : PluginHandle;

// Probe phase: opens the .so and reads metadata. Does NOT call on_load.
// Call bridge->activate() after dependency sorting.
// PORT-NOTE: std::expected<PluginHandle, LoadError> → two-out-param style:
//   returns PluginHandle (null on failure) and sets out LoadError.
PluginHandle open(const(char)[] path,
                  const(PluginConfig) plugin_cfg,
                  out LoadError err) {
    return FfiBridge.open_bridge(path, plugin_cfg, err);
}

// Convenience overload — no config
PluginHandle open(const(char)[] path, out LoadError err) {
    return FfiBridge.open_bridge(path, null, err);
}
