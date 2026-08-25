load("//bazel:build_defs.bzl", "congelado_module_library")

# async.cppm excluded: 0 bytes, unimplemented stub, never re-exported.

congelado_module_library(
    name = "io_flow_socket",
    primary_interface = "socket.cppm",
    partitions = [
        "sync.cppm",
    ],
    deps = [
        "//include/core/events:core_events",
        "//include/core/logger:core_logger",
        "//include/core/contracts:core_contract",
        "//include/io/base/socket:io_base_socket",
        "//include/utils/buffering:utils_buffering",
        "//include/io/base/leverage:io_base_leverage",
        "//include/io/flow/sender:io_flow_sender",
        "//include/io/flow/receiver:io_flow_receiver",
        "//include/interfaces:interfaces",
        "//include/shared:shared",
        "//include/utils/hashmap:hashmap",
        "//include/utils:utils_errno_translator",
    ],
    visibility = ["//visibility:public"],
)
