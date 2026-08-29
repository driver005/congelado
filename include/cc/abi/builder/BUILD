load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder",
    primary_interface = "builder.cppm",
    partitions = [
        "python.cppm",
    ],
    deps = [
        # Subdirectory modules
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
        "//include/cc/abi/builder/otel:cc_abi_builder_otel",
        "//include/cc/abi/builder/io:cc_abi_builder_io",
        "//include/cc/abi/builder/filesystem:cc_abi_builder_filesystem",
        "//include/cc/abi/builder/env:cc_abi_builder_env",
        "//include/cc/abi/builder/generator:cc_abi_builder_generator",
        "//include/cc/abi/builder/cache:cc_abi_builder_cache",
        "//include/cc/abi/builder/cron:cc_abi_builder_cron",
        "//include/cc/abi/builder/database:cc_abi_builder_database",
        "//include/cc/abi/builder/events:cc_abi_builder_events",
        "//include/cc/abi/builder/logger:cc_abi_builder_logger",
        "//include/cc/abi/builder/manager:cc_abi_builder_manager",
        "//include/cc/abi/builder/orchestrator:cc_abi_builder_orchestrator",
        "//include/cc/abi/builder/payload:cc_abi_builder_payload",
        "//include/cc/abi/builder/profiler:cc_abi_builder_profiler",
        "//include/cc/abi/builder/protocol:cc_abi_builder_protocol",
        "//include/cc/abi/builder/search:cc_abi_builder_search",
        "//include/cc/abi/builder/serde:cc_abi_builder_serde",
        "//include/cc/abi/builder/worker:cc_abi_builder_worker",
        # Base deps
        "//include/c:tf_c_headers",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
