# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/random_access_file.cppm",
        "builder/writable_file.cppm",
        "builder/read_only_memory_region.cppm",
        "builder/filesystem.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/filesystem/builder:cc_ice_builder_random_access_file",
        "//include/cc/ice/filesystem/builder:cc_ice_builder_writable_file",
        "//include/cc/ice/filesystem/builder:cc_ice_builder_read_only_memory_region",
        "//include/cc/ice/filesystem/builder:cc_ice_builder_filesystem",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/random_access_file.cppm",
        "sonic/writable_file.cppm",
        "sonic/read_only_memory_region.cppm",
        "sonic/filesystem.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/filesystem/sonic:cc_ice_sonic_random_access_file",
        "//include/cc/ice/filesystem/sonic:cc_ice_sonic_writable_file",
        "//include/cc/ice/filesystem/sonic:cc_ice_sonic_read_only_memory_region",
        "//include/cc/ice/filesystem/sonic:cc_ice_sonic_filesystem",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
