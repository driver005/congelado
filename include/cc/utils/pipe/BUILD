load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_utils_pipe",
    partitions = [
        "pipe.cppm",
        "process.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/utils/kernel:cc_utils_kernel",
    ],
)
