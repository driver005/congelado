load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_intern",
    primary_interface = "intern.cppm",
    partitions = [
        "allocator_attributes.cppm",
        "attrtype.cppm",
        "buffer.cppm",
        "ctx.cppm",
        "datatype.cppm",
        "file_statistics.cppm",
        "shape.cppm",
        "tensor.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/value:cc_abi_value",
    ],
    visibility = ["//visibility:public"],
)
