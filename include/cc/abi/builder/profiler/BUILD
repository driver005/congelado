load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure profiler-backend interface — abstract base class only. Zero dependency on any specific
# backend implementation; a real backend is only ever reachable through the C ABI (see
# cc_abi_sonic_profiler) — no in-process registration path, deliberately, so a profiler plugin
# built independently by a different toolchain stays safe to load.
congelado_module_library(
    name = "cc_abi_builder_profiler",
    primary_interface = "profiler.cppm",
    partitions = [
        ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
