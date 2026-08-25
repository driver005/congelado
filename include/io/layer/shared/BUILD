load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "io_layer_shared",
    primary_interface = "shared.cppm",
    partitions = [
        "types.cppm",
        "codec.cppm",
        "ping.cppm",
    ],
    deps = [
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
