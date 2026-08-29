load("//bazel:build_defs.bzl", "congelado_module_library")

# Wraps c/extern/plugin/registration.h's generic TF_PluginInfo/TF_InitPlugin — the fixed
# entry-point symbol yoshi::core::PluginLoader dlsym's and calls, unrelated to
# cc_abi_sonic_filesystem's own opaque-handle registration (type="filesystem" via
# ice::sonic::RegistrationRuntime). One class per file:
#   plugin_runtime.cppm                — PluginRuntime (non-owning TF_PluginInfo* wrapper)
congelado_module_library(
    name = "cc_abi_sonic_plugin",
    primary_interface = "plugin.cppm",
    partitions = [
        "plugin_runtime.cppm",
    ],
    deps = [
        "//include/c:extern_plugin_registration_hdrs",
    ],
    visibility = ["//visibility:public"],
)
