load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "io_flow_receiver",
    primary_interface = "reveiver.cppm",
    partitions = [
        "async.cppm",
        "sync.cppm",
    ],
    deps = [
        "//include/core/events:core_events",
        "//include/core/logger:core_logger",
        "//include/shared:shared",
        "//include/interfaces:interfaces",
        "//include/io/base/socket:io_base_socket",
        "//include/utils/buffering:utils_buffering",
        "//include/utils:utils_errno_translator",
    ],
    visibility = ["//visibility:public"],
)
