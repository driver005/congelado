load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_intern",
    primary_interface = "intern.cppm",
    partitions = [
        "allocator_attributes.cppm",
        "attrtype.cppm",
        "buffer.cppm",
        "datatype.cppm",
        "file_statistics.cppm",
        "runtime.cppm",
        "runtime_base.cppm",
        "shape.cppm",
        "string.cppm",
        "string_hive.cppm",
        "tensor.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/value:cc_abi_value",
    ],
    visibility = ["//visibility:public"],
)
