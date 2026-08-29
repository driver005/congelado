load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_intern",
    primary_interface = "intern.cppm",
    partitions = [
        "allocator_attributes.cppm",
        "buffer.cppm",
        "datatype.cppm",
        "file_statistics.cppm",
        "shape.cppm",
        "tensor.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
