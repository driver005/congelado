load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "connector",
    primary_interface = "connector.cppm",
    deps = [
        "//include/core/contracts:core_contract",
        "//include/core/events:core_events",
        "//include/core/logger:core_logger",
        "//include/core/otel:core_otel",
        "//include/interfaces:interfaces",
        "//include/shared:shared",
        "//include/serde:serde",
        "@stduuid//:stduuid",  # connector.cppm: #include <uuid.h>
    ],
    visibility = ["//visibility:public"],
)
