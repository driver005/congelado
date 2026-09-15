load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_utils_cli_parser",
    partitions = [
        "parser.cppm",
        "schema.cppm",
        "option.cppm",
        "flag.cppm",
        "action.cppm",
        "helper.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/utils/cli/ast:cc_utils_cli_ast",
    ],
)
