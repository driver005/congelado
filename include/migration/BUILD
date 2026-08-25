load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "migration",
    primary_interface = "runner.cppm",
    deps = [
        "//include/interfaces:interfaces",
        "//include/connector:connector",
        "//include/core/logger:core_logger",
        "//include/core/events:core_events",
        "//include/utils/hash:utils_hash",
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
