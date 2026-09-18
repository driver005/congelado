# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/node.cppm",
        "builder/module.cppm",
        "builder/attribute.cppm",
        "builder/block.cppm",
        "builder/definition.cppm",
        "builder/parameter.cppm",
        "builder/generator.cppm",
        "builder/function.cppm",
        "builder/typeinfo.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/generator/builder:cc_ice_builder_node",
        "//include/cc/ice/generator/builder:cc_ice_builder_module",
        "//include/cc/ice/generator/builder:cc_ice_builder_attribute",
        "//include/cc/ice/generator/builder:cc_ice_builder_block",
        "//include/cc/ice/generator/builder:cc_ice_builder_definition",
        "//include/cc/ice/generator/builder:cc_ice_builder_parameter",
        "//include/cc/ice/generator/builder:cc_ice_builder_generator",
        "//include/cc/ice/generator/builder:cc_ice_builder_function",
        "//include/cc/ice/generator/builder:cc_ice_builder_typeinfo",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/node.cppm",
        "sonic/module.cppm",
        "sonic/attribute.cppm",
        "sonic/block.cppm",
        "sonic/definition.cppm",
        "sonic/parameter.cppm",
        "sonic/generator.cppm",
        "sonic/function.cppm",
        "sonic/typeinfo.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/generator/sonic:cc_ice_sonic_node",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_module",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_attribute",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_block",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_definition",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_parameter",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_generator",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_function",
        "//include/cc/ice/generator/sonic:cc_ice_sonic_typeinfo",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
