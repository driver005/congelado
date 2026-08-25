load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "io_codec_shared",
    primary_interface = "shared.cppm",
    partitions = [
        "consts.cppm",
        "types.cppm",
        "table.cppm",
        "atom.cppm",
        "lowlevel.cppm",
        "huffman.cppm",
    ],
    deps = [
        "//include/io/error:io_error",
        "//include/utils/codec:utils_codec",
        "//include/utils/hashmap:hashmap",
        "//include/interfaces:interfaces",
    ],
    visibility = ["//visibility:public"],
)
