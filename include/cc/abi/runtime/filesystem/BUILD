load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_runtime_filesystem",
    primary_interface = "filesystem_module.cppm",
    partitions = [
        "filesystem_ops.cppm",
        "random_access_file.cppm",
        "read_only_memory_region.cppm",
        "writable_file.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
