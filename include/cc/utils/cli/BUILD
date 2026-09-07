load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_utils_cli",
    primary_interface = "cli.cppm",
    partitions = [
        "arguments.cppm",
        "command.cppm",
        "executable.cppm",
        "record.cppm",
        "result.cppm",
        "runner.cppm",
    ],
    deps = [
        "//include/cc/utils/pipe:cc_utils_pipe",
    ],
    visibility = ["//visibility:public"],
)
