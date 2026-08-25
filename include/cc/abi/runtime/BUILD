load("//bazel:build_defs.bzl", "congelado_module_library")

# Top-level standalone targets (one per .cppm)
congelado_module_library(
    name = "cache",
    primary_interface = "cache.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "cron",
    primary_interface = "cron.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "database",
    primary_interface = "database.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "events",
    primary_interface = "events.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "logger",
    primary_interface = "logger.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "manager",
    primary_interface = "manager.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "ops",
    primary_interface = "ops.cppm",
    deps = [
        "//include/c:tf_c_headers",
    ],
)

congelado_module_library(
    name = "orchestrator",
    primary_interface = "orchestrator.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "payload",
    primary_interface = "payload.cppm",
    deps = [
        "//include/c:tf_c_headers",
    ],
)

congelado_module_library(
    name = "profiler",
    primary_interface = "profiler.cppm",
    deps = [
        "//include/c:tf_c_headers",
    ],
)

congelado_module_library(
    name = "python",
    primary_interface = "python.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "serde",
    primary_interface = "serde.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

congelado_module_library(
    name = "worker",
    primary_interface = "worker.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
    ],
)

# Root aggregator - standalone partitions + subdirectory module deps
congelado_module_library(
    name = "cc_abi_runtime",
    primary_interface = "runtime.cppm",
    partitions = [
        "cache.cppm",
        "cron.cppm",
        "database.cppm",
        "events.cppm",
        "logger.cppm",
        "manager.cppm",
        "ops.cppm",
        "orchestrator.cppm",
        "payload.cppm",
        "profiler.cppm",
        "protocol/protocol_server_runtime.cppm",
        "protocol/protocol_runtime.cppm",
        "python.cppm",
        "search/search_query_runtime.cppm",
        "search/search_runtime.cppm",
        "serde.cppm",
        "worker.cppm",
    ],
    deps = [
        # Subdirectory modules (each is a complete module with its own partitions)
        "//include/cc/abi/runtime/intern:cc_abi_runtime_intern",
        "//include/cc/abi/runtime/otel:cc_abi_runtime_otel",
        "//include/cc/abi/runtime/io:cc_abi_runtime_io",
        "//include/cc/abi/runtime/filesystem:cc_abi_runtime_filesystem",
        "//include/cc/abi/runtime/env:cc_abi_runtime_env",
        "//include/cc/abi/runtime/generator:cc_abi_runtime_generator",
        # Base deps
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
