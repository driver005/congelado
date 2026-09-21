# GENERATED FILE — DO NOT EDIT BY HAND.
# Produced by cc_abi_gen from cc/ice. Re-run
# `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
# regenerate; edits made directly to this file will be overwritten.

load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_ice_builder_builder",
    partitions = [
        "builder/collection.cppm",
        "builder/store.cppm",
        "builder/watch.cppm",
        "builder/admin.cppm",
        "builder/transaction.cppm",
        "builder/index.cppm",
        "builder/query.cppm",
    ],
    primary_interface = "builder/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/store/builder:cc_ice_builder_collection",
        "//include/cc/ice/store/builder:cc_ice_builder_store",
        "//include/cc/ice/store/builder:cc_ice_builder_watch",
        "//include/cc/ice/store/builder:cc_ice_builder_admin",
        "//include/cc/ice/store/builder:cc_ice_builder_transaction",
        "//include/cc/ice/store/builder:cc_ice_builder_index",
        "//include/cc/ice/store/builder:cc_ice_builder_query",
    ],
)

congelado_module_library(
    name = "cc_ice_sonic_builder",
    partitions = [
        "sonic/collection.cppm",
        "sonic/store.cppm",
        "sonic/watch.cppm",
        "sonic/admin.cppm",
        "sonic/transaction.cppm",
        "sonic/index.cppm",
        "sonic/query.cppm",
    ],
    primary_interface = "sonic/base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/ice/store/sonic:cc_ice_sonic_collection",
        "//include/cc/ice/store/sonic:cc_ice_sonic_store",
        "//include/cc/ice/store/sonic:cc_ice_sonic_watch",
        "//include/cc/ice/store/sonic:cc_ice_sonic_admin",
        "//include/cc/ice/store/sonic:cc_ice_sonic_transaction",
        "//include/cc/ice/store/sonic:cc_ice_sonic_index",
        "//include/cc/ice/store/sonic:cc_ice_sonic_query",
        "//include/cc/ice/sonic_registration:cc_ice_sonic_registration",
    ],
)
