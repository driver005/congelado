load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_io",
    primary_interface = "io_module.cppm",
    partitions = [
        "io.cppm",
        "registration.cppm",
        "request.cppm",
        "response.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
