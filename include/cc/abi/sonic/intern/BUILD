load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_intern",
    primary_interface = "intern.cppm",
    partitions = [
        "buffer.cppm",
        "datatype.cppm",
        "runtime.cppm",
        "shape.cppm",
        "tensor.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
