load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_filesystem",
    primary_interface = "filesystem_module.cppm",
    partitions = [
        "filesystem_ops/filesystem_ops_runtime.cppm",
        "filesystem_ops/filesystem_plugin_ops_runtime.cppm",
        "filesystem_ops/filesystem_plugin_runtime.cppm",
        "random_access_file/random_access_file_runtime.cppm",
        "random_access_file/random_access_file_ops_runtime.cppm",
        "read_only_memory_region.cppm",
        "writable_file/writable_file_runtime.cppm",
        "writable_file/writable_file_ops_runtime.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
