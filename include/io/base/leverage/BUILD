load("//bazel:build_defs.bzl", "congelado_module_library")

# win32.cppm excluded: base.cppm only re-exports it under #ifdef _WIN32, Linux-only for now.

congelado_module_library(
    name = "io_base_leverage",
    primary_interface = "base.cppm",
    partitions = [
        "posix.cppm",
        "types.cppm",
        "uring.cppm",
    ],
    deps = [
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
