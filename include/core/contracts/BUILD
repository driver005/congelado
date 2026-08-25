load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_contract",
    primary_interface = "contract.cppm",
    partitions = [
        "consts.cppm",
        "signal_tree.cppm",
        "types.cppm",
    ],
    deps = [
        "//include/core/events:core_events",
        "//include/core/logger:core_logger",
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
