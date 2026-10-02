load("//bazel:build_defs.bzl", "congelado_cc_test", "congelado_module_library")

congelado_module_library(
    name = "utils_hash",
    primary_interface = "sha256.cppm",
    visibility = ["//visibility:public"],
)

congelado_cc_test(
    name = "sha256",
    srcs = ["sha256_test.cc"],
    linkopts = ["-lcrypto"],
    deps = [":utils_hash"],
)
