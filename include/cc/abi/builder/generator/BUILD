load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure generator-builder interface — abstract base classes + registry. Zero dependency on any
# specific generator implementation (e.g. cc_stable_hlo); implementations depend downward on
# this target and register themselves into GeneratorBuilderRegistry.
congelado_module_library(
    name = "cc_abi_builder_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "base_attribute.cppm",
        "base_controller.cppm",
        "base_definition.cppm",
        "base_parameter.cppm",
        "base_sourcecode.cppm",
        "base_typeinfo.cppm",
        "registry.cppm",
    ],
    deps = [
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
