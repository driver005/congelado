load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_runtime_otel",
    primary_interface = "otel.cppm",
    partitions = [
        "counter.cppm",
        "histogram.cppm",
        "meter.cppm",
        "span.cppm",
        "tracer.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
    visibility = ["//visibility:public"],
)
