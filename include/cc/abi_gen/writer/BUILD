load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_writer",
    partitions = [
        "helper/diff.cppm",
        "runner/diff.cppm",
        "runner/formatter.cppm",
        "writer.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//include/cc/utils/cli:cc_utils_cli"],
)
