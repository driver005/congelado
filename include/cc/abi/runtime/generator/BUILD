load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing generator handle (GeneratorRuntime) + the C-ABI adapter classes it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_generator for the interface types
# (GeneratorBuilderBase et al.) and cc_abi_builder_intern for StringBuilder (the interface's
# return type) — never on any specific generator implementation like cc_stable_hlo.
congelado_module_library(
    name = "cc_abi_runtime_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "c_attribute.cppm",
        "c_controller.cppm",
        "c_definition.cppm",
        "c_parameter.cppm",
        "c_sourcecode.cppm",
        "c_typeinfo.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/generator:cc_abi_builder_generator",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
