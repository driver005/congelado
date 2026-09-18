# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_builder",
    partitions = [
        "builder/span.cppm",
        "builder/tracer.cppm",
        "builder/counter.cppm",
        "builder/meter.cppm",
        "builder/histogram.cppm",
        "builder/otel.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/otel/builder:cc_ice_builder_span",
        "//include/cc/ice/otel/builder:cc_ice_builder_tracer",
        "//include/cc/ice/otel/builder:cc_ice_builder_counter",
        "//include/cc/ice/otel/builder:cc_ice_builder_meter",
        "//include/cc/ice/otel/builder:cc_ice_builder_histogram",
        "//include/cc/ice/otel/builder:cc_ice_builder_otel",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_builder",
    partitions = [
        "sonic/span.cppm",
        "sonic/tracer.cppm",
        "sonic/counter.cppm",
        "sonic/meter.cppm",
        "sonic/histogram.cppm",
        "sonic/otel.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/otel/sonic:cc_ice_sonic_span",
        "//include/cc/ice/otel/sonic:cc_ice_sonic_tracer",
        "//include/cc/ice/otel/sonic:cc_ice_sonic_counter",
        "//include/cc/ice/otel/sonic:cc_ice_sonic_meter",
        "//include/cc/ice/otel/sonic:cc_ice_sonic_histogram",
        "//include/cc/ice/otel/sonic:cc_ice_sonic_otel",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
