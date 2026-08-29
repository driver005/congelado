load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_env",
    primary_interface = "env.cppm",
    partitions = [
        "dynamic_library.cppm",
        "thread_view.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
],
    visibility = ["//visibility:public"],
)
