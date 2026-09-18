# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_intern",
    partitions = [
        "builder/builder.cppm",
        "builder/sonic.cppm",
        "builder/list.cppm",
        "builder/forward_list.cppm",
        "builder/hive.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/ice/builder:cc_ice_builder_builder",
        "//include/cc/ice/ice/builder:cc_ice_builder_sonic",
        "//include/cc/ice/ice/builder:cc_ice_builder_list",
        "//include/cc/ice/ice/builder:cc_ice_builder_forward_list",
        "//include/cc/ice/ice/builder:cc_ice_builder_hive",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_intern",
    partitions = [
        "sonic/builder.cppm",
        "sonic/sonic.cppm",
        "sonic/list.cppm",
        "sonic/forward_list.cppm",
        "sonic/hive.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/ice/sonic:cc_ice_sonic_builder",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_sonic",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_list",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_forward_list",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_hive",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
