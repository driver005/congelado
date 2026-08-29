load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure events-backend interface — abstract base class only. Zero dependency on any specific
# backend implementation; implementations depend downward on this target and register a factory
# function pointer into ice::sonic::RegistrationRuntime (cc_abi_sonic_registration) instead.
congelado_module_library(
    name = "cc_abi_builder_events",
    primary_interface = "events.cppm",
    partitions = [
        ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
