load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_runtime_io",
    primary_interface = "io_module.cppm",
    partitions = [
        "io.cppm",
        "request.cppm",
        "response.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
