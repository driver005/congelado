load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure serde-backend interface — abstract base class only. Zero dependency on any specific
# backend implementation; implementations depend downward on this target and register a factory
# function pointer into ice::sonic::RegistrationRuntime (cc_abi_sonic_registration) instead.
congelado_module_library(
    name = "cc_abi_builder_serde",
    primary_interface = "serde.cppm",
    partitions = [
        ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
