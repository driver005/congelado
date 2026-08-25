load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "io_flow_sender",
    primary_interface = "sender.cppm",
    partitions = [
        "async.cppm",
        "sync.cppm",
    ],
    deps = [
        "//include/core/events:core_events",
        "//include/core/logger:core_logger",
        "//include/interfaces:interfaces",
        "//include/utils/buffering:utils_buffering",
        "//include/io/base/socket:io_base_socket",
        "//include/shared:shared",
        "//include/io/base/leverage:io_base_leverage",
        "//include/utils:utils_errno_translator",
    ],
    visibility = ["//visibility:public"],
)
