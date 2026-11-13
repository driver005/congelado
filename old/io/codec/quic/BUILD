load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "io_quic",
    primary_interface = "quic.cppm",
    partitions = [
        "connection.cppm",
        "crypto.cppm",
        "tls.cppm",
        "types.cppm",
    ],
    visibility = ["//visibility:public"],
)
