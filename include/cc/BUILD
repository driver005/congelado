# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_",
    partitions = [
        "builder/ice.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/cc/builder:cc_ice_builder_ice",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_",
    partitions = [
        "sonic/ice.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/cc/sonic:cc_ice_sonic_ice",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
