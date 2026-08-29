load("//bazel:build_defs.bzl", "congelado_module_library")

# Real, native implementations of things include/c/ only ever declares as a C ABI (dormant,
# no backend in this repo) — dynamic_library.cppm is the POSIX dlopen/dlsym-backed
# DynamicLibrary class, matching include/core/manager/shared_lib.cppm's own real plugin
# loader rather than the unimplemented TF_LoadSharedLibrary/TF_GetSymbolFromLibrary C ABI in
# c/extern/env/dynamic_library.h. Its own target, not bundled into a broader yoshi/omah_lay
# aggregate — one class per file:
#   dynamic_library.cppm                — DynamicLibrary (owns a dlopen handle, RAII dlclose,
#                                          errors as ice::sonic::Status)
congelado_module_library(
    name = "yoshi_omah_lay_env",
    primary_interface = "env.cppm",
    partitions = [
        "dynamic_library.cppm",
    ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
