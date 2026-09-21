# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/kernel.cppm",
        "builder/builder.cppm",
        "builder/context.cppm",
        "builder/construction.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/kernel/builder:cc_ice_builder_kernel",
        "//include/cc/ice/kernel/builder:cc_ice_builder_builder",
        "//include/cc/ice/kernel/builder:cc_ice_builder_context",
        "//include/cc/ice/kernel/builder:cc_ice_builder_construction",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/kernel.cppm",
        "sonic/builder.cppm",
        "sonic/context.cppm",
        "sonic/construction.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/kernel/sonic:cc_ice_sonic_kernel",
        "//include/cc/ice/kernel/sonic:cc_ice_sonic_builder",
        "//include/cc/ice/kernel/sonic:cc_ice_sonic_context",
        "//include/cc/ice/kernel/sonic:cc_ice_sonic_construction",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
