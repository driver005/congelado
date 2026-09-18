# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_extern",
    partitions = [
        "builder/io.cppm",
        "builder/store.cppm",
        "builder/pubsub.cppm",
        "builder/jobber.cppm",
        "builder/filesystem.cppm",
        "builder/otel.cppm",
        "builder/generator.cppm",
        "builder/registration.cppm",
        "builder/profiler.cppm",
        "builder/serde.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/ice/builder:cc_ice_builder_io",
        "//include/cc/ice/ice/builder:cc_ice_builder_store",
        "//include/cc/ice/ice/builder:cc_ice_builder_pubsub",
        "//include/cc/ice/ice/builder:cc_ice_builder_jobber",
        "//include/cc/ice/ice/builder:cc_ice_builder_filesystem",
        "//include/cc/ice/ice/builder:cc_ice_builder_otel",
        "//include/cc/ice/ice/builder:cc_ice_builder_generator",
        "//include/cc/ice/ice/builder:cc_ice_builder_registration",
        "//include/cc/ice/ice/builder:cc_ice_builder_profiler",
        "//include/cc/ice/ice/builder:cc_ice_builder_serde",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_extern",
    partitions = [
        "sonic/io.cppm",
        "sonic/store.cppm",
        "sonic/pubsub.cppm",
        "sonic/jobber.cppm",
        "sonic/filesystem.cppm",
        "sonic/otel.cppm",
        "sonic/generator.cppm",
        "sonic/registration.cppm",
        "sonic/profiler.cppm",
        "sonic/serde.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/ice/sonic:cc_ice_sonic_io",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_store",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_pubsub",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_jobber",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_filesystem",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_otel",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_generator",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_registration",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_profiler",
        "//include/cc/ice/ice/sonic:cc_ice_sonic_serde",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
