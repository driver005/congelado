load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi",
    primary_interface = "abi.cppm",
    deps = [
        "//include/cc/abi/builder:cc_abi_builder",
        "//include/cc/abi/runtime:cc_abi_runtime",
        "//include/cc/abi/value:cc_abi_value",
    ],
    visibility = ["//visibility:public"],
)
