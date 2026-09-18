# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_sonic",
    partitions = [
        "builder/server.cppm",
        "builder/socket.cppm",
        "builder/client.cppm",
        "builder/request.cppm",
        "builder/response.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/io/builder:cc_ice_builder_server",
        "//include/cc/ice/io/builder:cc_ice_builder_socket",
        "//include/cc/ice/io/builder:cc_ice_builder_client",
        "//include/cc/ice/io/builder:cc_ice_builder_request",
        "//include/cc/ice/io/builder:cc_ice_builder_response",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_sonic",
    partitions = [
        "sonic/server.cppm",
        "sonic/socket.cppm",
        "sonic/client.cppm",
        "sonic/request.cppm",
        "sonic/response.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/io/sonic:cc_ice_sonic_server",
        "//include/cc/ice/io/sonic:cc_ice_sonic_socket",
        "//include/cc/ice/io/sonic:cc_ice_sonic_client",
        "//include/cc/ice/io/sonic:cc_ice_sonic_request",
        "//include/cc/ice/io/sonic:cc_ice_sonic_response",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
