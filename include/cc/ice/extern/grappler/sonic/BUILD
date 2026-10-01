# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_extern_grappler_sonic",
    partitions = [
        "device_graph.cppm",
        "function_library.cppm",
        "item.cppm",
        "optimizer.cppm",
        "configs.cppm",
        "grappler.cppm",
        "properties.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/ice/extern/random_generator/sonic:cc_ice_extern_random_generator_sonic",
        "//include/cc/ice/extern/stream_executor/sonic:cc_ice_extern_stream_executor_sonic",
        "//include/cc/ice/intern/sonic:cc_ice_intern_sonic",
    ],
)
