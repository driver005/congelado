# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_builder",
    partitions = [
        "builder/function_library.cppm",
        "builder/item.cppm",
        "builder/optimizer.cppm",
        "builder/configs.cppm",
        "builder/grappler.cppm",
        "builder/properties.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/grappler/builder:cc_ice_builder_function_library",
        "//include/cc/ice/grappler/builder:cc_ice_builder_item",
        "//include/cc/ice/grappler/builder:cc_ice_builder_optimizer",
        "//include/cc/ice/grappler/builder:cc_ice_builder_configs",
        "//include/cc/ice/grappler/builder:cc_ice_builder_grappler",
        "//include/cc/ice/grappler/builder:cc_ice_builder_properties",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_builder",
    partitions = [
        "sonic/function_library.cppm",
        "sonic/item.cppm",
        "sonic/optimizer.cppm",
        "sonic/configs.cppm",
        "sonic/grappler.cppm",
        "sonic/properties.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/grappler/sonic:cc_ice_sonic_function_library",
        "//include/cc/ice/grappler/sonic:cc_ice_sonic_item",
        "//include/cc/ice/grappler/sonic:cc_ice_sonic_optimizer",
        "//include/cc/ice/grappler/sonic:cc_ice_sonic_configs",
        "//include/cc/ice/grappler/sonic:cc_ice_sonic_grappler",
        "//include/cc/ice/grappler/sonic:cc_ice_sonic_properties",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
