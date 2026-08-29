load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure payload-backend interface — abstract base class only. Zero dependency on any specific
# backend implementation; implementations depend downward on this target and register a factory
# function pointer into ice::sonic::Registration (cc_abi_sonic_registration) instead.
congelado_module_library(
    name = "cc_abi_builder_payload",
    primary_interface = "payload.cppm",
    partitions = [
        ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
