load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_config",
    primary_interface = "config.cppm",
    partitions = [
        "loader.cppm",
        "types.cppm",
    ],
    deps = [
        "//include/core/events:core_events",
        "@simdjson//:simdjson",  # loader.cppm: #include <simdjson.h>
        "//third_party/tomlplusplus_shim:toml++",  # loader.cppm: #include <toml++/toml.hpp> — see that target's comment
    ],
    visibility = ["//visibility:public"],
)
