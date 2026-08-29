load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_env",
    primary_interface = "env.cppm",
    partitions = [
        "dynamic_library.cppm",
        "symbol.cppm",
        "thread.cppm",
        "time.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
