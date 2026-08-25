load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_client",
    primary_interface = "base.cppm",
    partitions = [
        "builder.cppm",
        "register.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
        "//include/utils/buffering:utils_buffering",
        "//include/core/otel:core_otel",
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
