load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "io_codec_hpack",
    primary_interface = "hpack.cppm",
    partitions = [
        "types.cppm",
        "table.cppm",
        "consts.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
        "//include/io/layer/shared:io_layer_shared",
        "//include/io/codec/shared:io_codec_shared",
        "//include/utils/codec:utils_codec",
        "//include/io/error:io_error",
    ],
    visibility = ["//visibility:public"],
)
