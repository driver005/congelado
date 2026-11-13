load("//bazel:build_defs.bzl", "congelado_module_library")

# `interfaces` spans 4 dirs (interfaces/, codec/, io/, plugin/), all partitions of one flat module.

congelado_module_library(
    name = "interfaces",
    primary_interface = "interfaces.cppm",
    partitions = [
        "cache.cppm",
        "client.cppm",
        "consts.cppm",
        "cron.cppm",
        "database.cppm",
        "events.cppm",
        "logger.cppm",
        "otel.cppm",
        "payload_storage.cppm",
        "protocol.cppm",
        "search.cppm",
        "serde.cppm",
        "status.cppm",
        "worker.cppm",
        "worker_orchestrator.cppm",
        "workflow_orchestrator.cppm",
        "codec/cache.cppm",
        "codec/db.cppm",
        "io/dispatch.cppm",
        "io/header.cppm",
        "io/io.cppm",
        "io/request.cppm",
        "io/response.cppm",
        "io/types.cppm",
        "plugin/bridge.cppm",
    ],
    hdrs = [
        "//include/core/manager:headers",
    ],
    deps = [
        "//include/shared:shared",
        "//include/io/shared:io_shared",
        "//include/utils/buffering:utils_buffering",
        "//include/utils:utils_encode",
        "@reflect_cpp//:reflect_cpp",  # interfaces/serde.cppm: #include <rfl/Generic.hpp>
    ],
    visibility = ["//visibility:public"],
)
