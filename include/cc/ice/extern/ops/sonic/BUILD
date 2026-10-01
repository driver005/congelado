# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_extern_ops_sonic",
    partitions = [
        "ops.cppm",
        "shape_inference_context.cppm",
        "dimension_handle.cppm",
        "op_definition_builder.cppm",
        "shape_handle.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/ice/intern/sonic:cc_ice_intern_sonic",
    ],
)
