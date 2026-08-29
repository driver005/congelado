load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing logger handle (ice::sonic::Logger) + the C-ABI adapter it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_logger for the interface type
# (ice::builder::Logger) and cc_abi_sonic_registration for the in-process factory lookup
# — never on any specific logger backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_logger",
    primary_interface = "logger.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_logger_hdrs",
        "//include/cc/abi/builder/logger:cc_abi_builder_logger",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
