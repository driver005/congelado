load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_writer",
    primary_interface = "writer.cppm",
    partitions = [
        "diff_result.cppm",
        "clang_format_runner.cppm",
        "diff_reporter.cppm",
        "generated_file_writer.cppm",
    ],
    visibility = ["//visibility:public"],
)
