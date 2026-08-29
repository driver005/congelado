load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi",
    primary_interface = "abi.cppm",
    deps = [
        "//include/cc/abi/builder:cc_abi_builder",
        "//include/cc/abi/sonic:cc_abi_sonic",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
