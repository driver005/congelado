load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing events handle (ice::sonic::Events) + the C-ABI adapter it uses to
# cross into a loaded plugin. The adapter is standalone (no dependency on the builder
# tier); cc_abi_sonic_registration provides the in-process factory lookup — never on
# any specific events backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_events",
    primary_interface = "events.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_events_hdrs",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
        "//include/cc/abi/primitives:cc_abi_primitives",
    ],
    visibility = ["//visibility:public"],
)
