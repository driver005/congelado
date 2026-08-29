load("//bazel:build_defs.bzl", "congelado_module_library")

# Plugin loading primitives — the host side of the fixed-entry-point (.so with init_plugin)
# plugin boundary: DynamicLibrary (dlopen/dlclose) and Symbol (dlsym + call). Moved here from
# env because the plugin loader owns this boundary. One class per file:
#   dynamic_library.cppm               — DynamicLibrary (RAII dlopen handle, move-only)
#   symbol.cppm                        — Symbol (resolved dlsym pointer, callable)
congelado_module_library(
    name = "cc_abi_sonic_plugin",
    primary_interface = "plugin.cppm",
    partitions = [
        "dynamic_library.cppm",
        "symbol.cppm",
    ],
    deps = [
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
