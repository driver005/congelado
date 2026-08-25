load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_filesystem",
    primary_interface = "filesystem_module.cppm",
    partitions = [
        "filesystem_ops/filesystem_ops_builder.cppm",
        "filesystem_ops/filesystem_plugin_ops_builder.cppm",
        "filesystem_ops/filesystem_plugin_builder.cppm",
        "random_access_file/random_access_file_builder.cppm",
        "random_access_file/random_access_file_ops_builder.cppm",
        "read_only_memory_region.cppm",
        "registration.cppm",
        "writable_file/writable_file_builder.cppm",
        "writable_file/writable_file_ops_builder.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
