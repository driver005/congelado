load("//bazel:build_defs.bzl", "congelado_module_library")

# consts.cppm excluded: 0 bytes, unimplemented stub, genuinely unused.

congelado_module_library(
    name = "io_codec_qpack",
    primary_interface = "qpack.cppm",
    partitions = [
        "types.cppm",
        "table.cppm",
    ],
    deps = [
        "//include/io/codec/shared:io_codec_shared",
        "//include/io/error:io_error",
        "//include/io/shared:io_shared",
        "//include/interfaces:interfaces",
    ],
    visibility = ["//visibility:public"],
)
