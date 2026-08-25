load("//bazel:build_defs.bzl", "congelado_module_library")

# Zero cross-module imports — the Phase 1 proof that module_interfaces + partitions work at all.

congelado_module_library(
    name = "core_ffi",
    primary_interface = "ffi.cppm",
    partitions = [
        "bridge.cppm",
    ],
    visibility = ["//visibility:public"],
)
