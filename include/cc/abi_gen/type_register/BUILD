load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_type_register",
    primary_interface = "type_register.cppm",
    partitions = [
        "type_rule.cppm",
        "registry.cppm",
        "slot_shape.cppm",
    ],
    deps = [
        "//include/cc/abi_gen/parser:cc_abi_gen_parser",
    ],
    visibility = ["//visibility:public"],
)
