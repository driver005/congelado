load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing cache handle (ice::sonic::Cache) + the C-ABI adapter it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_cache for the interface type
# (ice::builder::Cache) and cc_abi_sonic_registration for the in-process factory lookup
# — never on any specific cache backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_cache",
    primary_interface = "cache.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_cache_hdrs",
        "//include/cc/abi/builder/cache:cc_abi_builder_cache",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
