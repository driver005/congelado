load("//bazel:build_defs.bzl", "congelado_module_library")

# flow.cppm re-exports three separate whole modules (sibling subdirs), not partitions of its own.

congelado_module_library(
    name = "io_base_flow",
    primary_interface = "flow.cppm",
    deps = [
        "//include/shared:shared",
        "//include/io/base/leverage:io_base_leverage",
        "//include/utils/buffering:utils_buffering",
        "//include/io/flow/sender:io_flow_sender",
        "//include/io/flow/receiver:io_flow_receiver",
        "//include/io/flow/socket:io_flow_socket",
    ],
    visibility = ["//visibility:public"],
)
