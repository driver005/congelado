load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_filesystem",
    primary_interface = "filesystem_module.cppm",
    partitions = [
        "filesystem_ops.cppm",
        "random_access_file.cppm",
        "read_only_memory_region.cppm",
        "registration.cppm",
        "writable_file.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
