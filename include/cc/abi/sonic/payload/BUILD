load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing payload handle (ice::sonic::Payload) + the C-ABI adapter it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_payload for the interface type
# (ice::builder::Payload) and cc_abi_sonic_registration for the in-process factory
# lookup — never on any specific payload backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_payload",
    primary_interface = "payload.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_payload_hdrs",
        "//include/cc/abi/builder/payload:cc_abi_builder_payload",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
