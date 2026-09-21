# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_builder",
    partitions = [
        "builder/ops.cppm",
        "builder/shape_inference_context.cppm",
        "builder/dimension_handle.cppm",
        "builder/op_definition_builder.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/ops/builder:cc_ice_builder_ops",
        "//include/cc/ice/ops/builder:cc_ice_builder_shape_inference_context",
        "//include/cc/ice/ops/builder:cc_ice_builder_dimension_handle",
        "//include/cc/ice/ops/builder:cc_ice_builder_op_definition_builder",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_builder",
    partitions = [
        "sonic/ops.cppm",
        "sonic/shape_inference_context.cppm",
        "sonic/dimension_handle.cppm",
        "sonic/op_definition_builder.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/ops/sonic:cc_ice_sonic_ops",
        "//include/cc/ice/ops/sonic:cc_ice_sonic_shape_inference_context",
        "//include/cc/ice/ops/sonic:cc_ice_sonic_dimension_handle",
        "//include/cc/ice/ops/sonic:cc_ice_sonic_op_definition_builder",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
