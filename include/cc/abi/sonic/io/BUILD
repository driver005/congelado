load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_io",
    primary_interface = "io.cppm",
    partitions = [
        "enums.cppm",
        "leaves.cppm",
        "registration.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
