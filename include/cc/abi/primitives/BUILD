load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_primitives",
    primary_interface = "primitives.cppm",
    partitions = [
        "status.cppm",
        "string.cppm",
        "string_hive.cppm",
        "tensor_handle.cppm",
    ],
    deps = [
        "//include/c:tf_status_hdrs",
        "//include/c:tf_tstring_hdrs",
        "//include/c:tf_tensor_hdrs",
    ],
    visibility = ["//visibility:public"],
)
