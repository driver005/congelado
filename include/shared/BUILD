load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "shared",
    primary_interface = "shared.cppm",
    partitions = [
        "handler.cppm",
        "task_queue.cppm",
        "transport.cppm",
        "types.cppm",
        "flow.cppm",
        "logger.cppm",
        "leverage.cppm",
        "socket.cppm",
    ],
    deps = [
        "//include/utils/buffering:utils_buffering",
    ],
    visibility = ["//visibility:public"],
)
