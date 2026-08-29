load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing events handle (ice::sonic::Events) + the C-ABI adapter it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_events for the interface type
# (ice::builder::Events) and cc_abi_sonic_registration for the in-process factory lookup
# — never on any specific events backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_events",
    primary_interface = "events.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_events_hdrs",
        "//include/cc/abi/builder/events:cc_abi_builder_events",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
