load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing generator handle (ice::sonic::generator::Runtime) + the C-ABI adapter classes it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_generator for the interface types
# (ice::builder::generator::Builder et al.) and cc_abi_sonic_intern for StringRuntime (the
# interface's return type) — never on any specific generator implementation like cc_stable_hlo.
congelado_module_library(
    name = "cc_abi_sonic_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "attribute.cppm",
        "runtime.cppm",
        "definition.cppm",
        "function.cppm",
        "parameter.cppm",
        "typeinfo.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/generator:cc_abi_builder_generator",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
