load("//bazel:build_defs.bzl", "congelado_module_library")

# yoshi's plugin loader — loads a plugin .so and calls its fixed-entry-point init_plugin
# symbol, filling in a TF_PluginInfo (c/extern/plugin/registration.h — the generic entry
# point, unrelated to cc_abi_sonic_filesystem's own type="filesystem" registration) — same
# host-allocates/plugin-fills-in-place, no-callback-into-host contract TensorFlow's real
# filesystem plugins originally used. Its own target, not bundled into a broader yoshi/lucki
# aggregate — one class per file:
#   plugin_loader.cppm                 — PluginLoader (dlopen's the .so via
#                                         ice::sonic::DynamicLibrary, resolves and calls
#                                         init_plugin)
congelado_module_library(
    name = "yoshi_lucki_plugin",
    primary_interface = "plugin.cppm",
    partitions = [
        "plugin_loader.cppm",
    ],
    deps = [
        "//include/c:extern_plugin_hdrs",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/plugin:cc_abi_sonic_plugin",
    ],
    visibility = ["//visibility:public"],
)
