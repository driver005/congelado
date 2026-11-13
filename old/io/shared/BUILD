load("//bazel:build_defs.bzl", "congelado_module_library")

# io_shared spans this dir plus its http/ subdir, all partitions of one flat module.

congelado_module_library(
    name = "io_shared",
    primary_interface = "shared.cppm",
    partitions = [
        "types.cppm",
        "consts.cppm",
        "http/header.cppm",
        "http/http.cppm",
        "http/types.cppm",
    ],
    deps = [
        "//include/io/error:io_error",
    ],
    visibility = ["//visibility:public"],
)
