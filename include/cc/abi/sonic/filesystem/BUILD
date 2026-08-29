load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_filesystem",
    primary_interface = "filesystem.cppm",
    partitions = [
        "leaves.cppm",
        "leaves/random_access_file.cppm",
        "leaves/random_access_file_runtime.cppm",
        "leaves/read_only_memory_region.cppm",
        "leaves/read_only_memory_region_runtime.cppm",
        "leaves/writable_file.cppm",
        "leaves/writable_file_runtime.cppm",
        "registration.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
