# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/forward_list.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/forward_list/builder:cc_ice_builder_forward_list",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/forward_list.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/forward_list/sonic:cc_ice_sonic_forward_list",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
