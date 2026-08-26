load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure generator-builder interface — abstract base classes + registry. Zero dependency on any
# specific generator implementation (e.g. cc_stable_hlo); implementations depend downward on
# this target and register themselves into ice::builder::generator::BuilderRegistry.
congelado_module_library(
    name = "cc_abi_builder_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "attribute.cppm",
        "controller/node_handle.cppm",
        "controller/function.cppm",
        "controller/builder.cppm",
        "definition.cppm",
        "parameter.cppm",
        "typeinfo.cppm",
        "builder_registry.cppm",
    ],
    deps = [
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
