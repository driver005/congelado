load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing generator handle (ice::sonic::Generator) + the standalone C-ABI
# adapter classes it uses to cross into a loaded plugin through the flat TF_Generator
# vtable. No dependency on the builder tier; cc_abi_sonic_intern provides the generic
# Runtime base — never on any specific generator implementation like cc_stable_hlo.
congelado_module_library(
    name = "cc_abi_sonic_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "attribute.cppm",
        "runtime.cppm",
        "function.cppm",
        "parameter.cppm",
        "typeinfo.cppm",
    ],
    deps = [
        "//include/c:extern_generator_hdrs",
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
