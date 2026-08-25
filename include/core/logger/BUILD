load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_logger",
    primary_interface = "logger.cppm",
    partitions = [
        "registry.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
    ],
    visibility = ["//visibility:public"],
)
