# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/hive.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/hive/builder:cc_ice_builder_hive",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/hive.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/hive/sonic:cc_ice_sonic_hive",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
