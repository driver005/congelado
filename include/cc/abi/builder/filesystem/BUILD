load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_filesystem",
    primary_interface = "filesystem.cppm",
    partitions = [
        "leaves.cppm",
        "leaves/random_access_file.cppm",
        "leaves/read_only_memory_region.cppm",
        "leaves/writable_file.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
