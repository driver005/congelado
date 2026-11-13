load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_otel",
    primary_interface = "otel.cppm",
    partitions = [
        "registry.cppm",
        "span.cppm",
        "metrics.cppm",
        "http.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
    ],
    visibility = ["//visibility:public"],
)
