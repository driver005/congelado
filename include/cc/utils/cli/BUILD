load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_utils_cli",
    partitions = [
        "command/arguments.cppm",
        "command/command.cppm",
        "executable.cppm",
        "record.cppm",
        "result.cppm",
        "runner.cppm",
    ],
    primary_interface = "cli.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/utils/pipe:cc_utils_pipe",
    ],
)
