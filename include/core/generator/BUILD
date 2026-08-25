load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "statement.cppm",
        "function.cppm",
        "class_builder.cppm",
        "module_builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
