load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing cron handle (ice::sonic::Cron) + the C-ABI adapter it uses to cross
# into a loaded plugin. Depends on cc_abi_builder_cron for the interface type
# (ice::builder::Cron) and cc_abi_sonic_registration for the in-process factory lookup —
# never on any specific cron backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_cron",
    primary_interface = "cron.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_cron_hdrs",
        "//include/cc/abi/builder/cron:cc_abi_builder_cron",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
