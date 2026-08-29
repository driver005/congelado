load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing serde handle (ice::sonic::Serde) + the C-ABI adapter it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_serde for the interface type
# (ice::builder::Serde) and cc_abi_sonic_registration for the in-process factory lookup —
# never on any specific serde backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_serde",
    primary_interface = "serde.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_serde_hdrs",
        "//include/cc/abi/builder/serde:cc_abi_builder_serde",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
