load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure generator-builder interface — abstract base classes + the flat TF_Generator
# vtable. Zero dependency on any specific generator implementation (e.g. cc_stable_hlo);
# implementations depend downward on this target and export themselves through
# TF_InitGenerator (the factory that fills the vtable produced by
# get_generic_vtable()).
congelado_module_library(
    name = "cc_abi_builder_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "attribute.cppm",
        "definition.cppm",
        "function.cppm",
        "parameter.cppm",
        "typeinfo.cppm",
    ],
    deps = [
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
