load("//bazel:build_defs.bzl", "congelado_module_library")

# win32.cppm excluded: only re-exported under #ifdef _WIN32, same as io/base/leverage.

congelado_module_library(
    name = "io_base_socket",
    primary_interface = "socket.cppm",
    partitions = [
        "consts.cppm",
        "posix.cppm",
    ],
    deps = [
        "//include/io/error:io_error",
        "//include/core/events:core_events",
        "//include/core/logger:core_logger",
        "//include/io/base/leverage:io_base_leverage",
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
