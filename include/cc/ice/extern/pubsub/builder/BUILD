# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_builder",
    partitions = [
        "builder/channel.cppm",
        "builder/subscription.cppm",
        "builder/publish.cppm",
        "builder/pubsub.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/pubsub/builder:cc_ice_builder_channel",
        "//include/cc/ice/pubsub/builder:cc_ice_builder_subscription",
        "//include/cc/ice/pubsub/builder:cc_ice_builder_publish",
        "//include/cc/ice/pubsub/builder:cc_ice_builder_pubsub",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_builder",
    partitions = [
        "sonic/channel.cppm",
        "sonic/subscription.cppm",
        "sonic/publish.cppm",
        "sonic/pubsub.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/pubsub/sonic:cc_ice_sonic_channel",
        "//include/cc/ice/pubsub/sonic:cc_ice_sonic_subscription",
        "//include/cc/ice/pubsub/sonic:cc_ice_sonic_publish",
        "//include/cc/ice/pubsub/sonic:cc_ice_sonic_pubsub",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
