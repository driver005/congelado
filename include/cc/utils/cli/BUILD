load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_utils_cli",
    primary_interface = "cli.cppm",
    partitions = [
        "arguments.cppm",
        "cli.cppm",
        "command.cppm",
        "executable.cppm",
        "command.cppm",
        "recode.cppm",
        "result.cppm",
        "runner.cppm",
    ],
    deps = [
        "//cc_utils:cc_utils_pipe",
    ],
    visibility = ["//visibility:public"],
)
