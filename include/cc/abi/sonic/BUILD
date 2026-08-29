load("//bazel:build_defs.bzl", "congelado_module_library")

# Root aggregator - subdirectory module deps only
congelado_module_library(
    name = "cc_abi_sonic",
    primary_interface = "sonic.cppm",
    partitions = [
        "python.cppm",
    ],
    deps = [
        # Subdirectory modules
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/otel:cc_abi_sonic_otel",
        "//include/cc/abi/sonic/io:cc_abi_sonic_io",
        "//include/cc/abi/sonic/filesystem:cc_abi_sonic_filesystem",
        "//include/cc/abi/sonic/env:cc_abi_sonic_env",
        "//include/cc/abi/sonic/plugin:cc_abi_sonic_plugin",
        "//include/cc/abi/sonic/generator:cc_abi_sonic_generator",
        "//include/cc/abi/sonic/cache:cc_abi_sonic_cache",
        "//include/cc/abi/sonic/cron:cc_abi_sonic_cron",
        "//include/cc/abi/sonic/database:cc_abi_sonic_database",
        "//include/cc/abi/sonic/events:cc_abi_sonic_events",
        "//include/cc/abi/sonic/logger:cc_abi_sonic_logger",
        "//include/cc/abi/sonic/manager:cc_abi_sonic_manager",
        "//include/cc/abi/sonic/orchestrator:cc_abi_sonic_orchestrator",
        "//include/cc/abi/sonic/payload:cc_abi_sonic_payload",
        "//include/cc/abi/sonic/profiler:cc_abi_sonic_profiler",
        "//include/cc/abi/sonic/protocol:cc_abi_sonic_protocol",
        "//include/cc/abi/sonic/search:cc_abi_sonic_search",
        "//include/cc/abi/sonic/serde:cc_abi_sonic_serde",
        "//include/cc/abi/sonic/worker:cc_abi_sonic_worker",
        # Base deps
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
