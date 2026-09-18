# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/schedule.cppm",
        "builder/observe.cppm",
        "builder/durable.cppm",
        "builder/job.cppm",
        "builder/jobber.cppm",
        "builder/options.cppm",
        "builder/task.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/jobber/builder:cc_ice_builder_schedule",
        "//include/cc/ice/jobber/builder:cc_ice_builder_observe",
        "//include/cc/ice/jobber/builder:cc_ice_builder_durable",
        "//include/cc/ice/jobber/builder:cc_ice_builder_job",
        "//include/cc/ice/jobber/builder:cc_ice_builder_jobber",
        "//include/cc/ice/jobber/builder:cc_ice_builder_options",
        "//include/cc/ice/jobber/builder:cc_ice_builder_task",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/schedule.cppm",
        "sonic/observe.cppm",
        "sonic/durable.cppm",
        "sonic/job.cppm",
        "sonic/jobber.cppm",
        "sonic/options.cppm",
        "sonic/task.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_schedule",
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_observe",
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_durable",
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_job",
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_jobber",
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_options",
        "//include/cc/ice/jobber/sonic:cc_ice_sonic_task",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
