load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_writer",
    primary_interface = "base.cppm",
    partitions = [
        "helper/diff.cppm",
        "runner/diff.cppm",
        "runner/formater.cppm",
        "writer.cppm",
    ],
    deps = ["//include/cc/utils/cli:cc_utils_cli"],
    visibility = ["//visibility:public"],
)
