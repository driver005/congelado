# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_builder",
    partitions = [
        "builder/platform.cppm",
        "builder/stream_executor.cppm",
        "builder/timer.cppm",
        "builder/device.cppm",
        "builder/executor.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/stream_executor/builder:cc_ice_builder_platform",
        "//include/cc/ice/stream_executor/builder:cc_ice_builder_stream_executor",
        "//include/cc/ice/stream_executor/builder:cc_ice_builder_timer",
        "//include/cc/ice/stream_executor/builder:cc_ice_builder_device",
        "//include/cc/ice/stream_executor/builder:cc_ice_builder_executor",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_builder",
    partitions = [
        "sonic/platform.cppm",
        "sonic/stream_executor.cppm",
        "sonic/timer.cppm",
        "sonic/device.cppm",
        "sonic/executor.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/stream_executor/sonic:cc_ice_sonic_platform",
        "//include/cc/ice/stream_executor/sonic:cc_ice_sonic_stream_executor",
        "//include/cc/ice/stream_executor/sonic:cc_ice_sonic_timer",
        "//include/cc/ice/stream_executor/sonic:cc_ice_sonic_device",
        "//include/cc/ice/stream_executor/sonic:cc_ice_sonic_executor",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
