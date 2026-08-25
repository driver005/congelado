load("//bazel:build_defs.bzl", "congelado_module_library")

# types.cppm's real partition is `:utils`, not `:types` — filename doesn't match.

congelado_module_library(
    name = "core_router",
    primary_interface = "base.cppm",
    partitions = [
        "router.cppm",
        "builder.cppm",
        "handler.cppm",
        "middleware.cppm",
        "types.cppm",
        "executor.cppm",
        "consts.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
        "//include/core/otel:core_otel",
        "//include/core/events:core_events",
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)
