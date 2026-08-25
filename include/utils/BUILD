load("//bazel:build_defs.bzl", "congelado_module_library")

# Four independent single-file modules; subdirs (buffering/, codec/, ...) have their own BUILD.

congelado_module_library(
    name = "consts",
    primary_interface = "consts.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "utils_encode",
    primary_interface = "encode.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "utils_errno_translator",
    primary_interface = "errno_translator.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "helper",
    primary_interface = "helper.cppm",
    visibility = ["//visibility:public"],
)
