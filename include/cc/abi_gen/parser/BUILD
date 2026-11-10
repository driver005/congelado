load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_parser",
    partitions = [
        "utils.cppm",
        "visitor.cppm",
        "parser.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/abi_gen/classify:cc_abi_gen_classify",
        "//include/cc/utils/cli:cc_utils_cli",
    ],
)
