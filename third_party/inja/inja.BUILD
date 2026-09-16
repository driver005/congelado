load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "inja",
    hdrs = ["single_include/inja/inja.hpp"],
    includes = ["single_include"],
    visibility = ["//visibility:public"],
    deps = ["@nlohmann_json"],
)
