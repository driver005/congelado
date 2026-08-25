load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "utils_openapi",
    primary_interface = "openapi.cppm",
    partitions = [
        "model.cppm",
        "schema.cppm",
        "registry.cppm",
        "route.cppm",
        "generator_interface.cppm",
        "generator_registry.cppm",
    ],
    deps = [
        "//include/core/router:core_router",
        "//include/serde:serde",
        "//include/interfaces:interfaces",
    ],
    visibility = ["//visibility:public"],
)
