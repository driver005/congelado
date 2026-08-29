load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_otel",
    primary_interface = "otel.cppm",
    partitions = [
        "enums.cppm",
        "leaves.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
        "//include/cc/abi/value:cc_abi_value",
    ],
    visibility = ["//visibility:public"],
)
