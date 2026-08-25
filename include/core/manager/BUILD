load("//bazel:build_defs.bzl", "congelado_module_library")

# --- Headers ---

filegroup(
    name = "headers",
    srcs = ["abi.h"],
    visibility = ["//visibility:public"],
)

# --- Module ---

congelado_module_library(
    name = "core_plugin",
    primary_interface = "plugin.cppm",
    partitions = [
        "bridge.cppm",
        "ffi.cppm",
        "plugin_ref.cppm",
        "shared_lib.cppm",
        "types.cppm",
        "value.cppm",
    ],
    hdrs = [
        ":headers",
        "//include/congelado:headers",
    ],
    deps = [
        "//include/interfaces:interfaces",
        "//include/core/events:core_events",
        "//include/core/ffi:core_ffi",
    ],
    visibility = ["//visibility:public"],
)
