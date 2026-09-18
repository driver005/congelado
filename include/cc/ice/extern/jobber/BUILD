# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_jobber",
    partitions = [
        "builder/builder.cppm",
        "builder/sonic.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/extern/builder:cc_ice_builder_builder",
        "//include/cc/ice/extern/builder:cc_ice_builder_sonic",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_jobber",
    partitions = [
        "sonic/builder.cppm",
        "sonic/sonic.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/extern/sonic:cc_ice_sonic_builder",
        "//include/cc/ice/extern/sonic:cc_ice_sonic_sonic",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
