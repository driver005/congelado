load("//bazel:build_defs.bzl", "congelado_module_library")

# Top-level standalone targets (one per .cppm)
congelado_module_library(
    name = "cache",
    primary_interface = "cache.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "cron",
    primary_interface = "cron.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "database",
    primary_interface = "database.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "events",
    primary_interface = "events.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "logger",
    primary_interface = "logger.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "manager",
    primary_interface = "manager.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "ops",
    primary_interface = "ops.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
        "//include/cc/abi/value:cc_abi_value",
    ],
)

congelado_module_library(
    name = "orchestrator",
    primary_interface = "orchestrator.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "payload",
    primary_interface = "payload.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "profiler",
    primary_interface = "profiler.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "protocol",
    primary_interface = "protocol.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "python",
    primary_interface = "python.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "search",
    primary_interface = "search.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "serde",
    primary_interface = "serde.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

congelado_module_library(
    name = "worker",
    primary_interface = "worker.cppm",
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
)

# Root aggregator - standalone partitions + subdirectory module deps
congelado_module_library(
    name = "cc_abi_builder",
    primary_interface = "builder.cppm",
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
        "protocol.cppm",
        "python.cppm",
        "search.cppm",
        "serde.cppm",
        "worker.cppm",
    ],
    deps = [
        # Subdirectory modules (each is a complete module with its own partitions)
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
        "//include/cc/abi/builder/otel:cc_abi_builder_otel",
        "//include/cc/abi/builder/io:cc_abi_builder_io",
        "//include/cc/abi/builder/filesystem:cc_abi_builder_filesystem",
        "//include/cc/abi/builder/env:cc_abi_builder_env",
        "//include/cc/abi/builder/generator:cc_abi_builder_generator",
        # Base deps
        "//include/c:tf_c_headers",
        "//include/cc/abi/value:cc_abi_value",
    ],
    visibility = ["//visibility:public"],
)
