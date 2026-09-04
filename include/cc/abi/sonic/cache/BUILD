load("//bazel:build_defs.bzl", "congelado_module_library")

# See include/cc/abi/builder/cache/BUILD's header comment for the pattern this mirrors.

# Mainframe-facing cache handle (ice::sonic::Cache) + the C-ABI adapter it uses to
# cross into a loaded plugin. The adapter is standalone (no dependency on the builder
# tier); cc_abi_sonic_registration provides the in-process factory lookup — never on
# any specific cache backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_cache",
    primary_interface = "//include/cc/abi_gen:generated/sonic/cache/cache.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_cache_hdrs",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
        "//include/cc/abi/primitives:cc_abi_primitives",
    ],
    visibility = ["//visibility:public"],
)
