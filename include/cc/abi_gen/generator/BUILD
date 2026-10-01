load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_generator",
    partitions = [
        "emitter/runtime.cppm",
        "emitter/runtime_spec.cppm",
        "emitter/vtable.cppm",
        "helper/dependency_info.cppm",
        "helper/formatter.cppm",
        "helper/module_naming.cppm",
        "helper/types.cppm",
        "runtime.cppm",
    ],
    primary_interface = "generator.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//include/cc/abi_gen/parser:cc_abi_gen_parser",
        "//include/cc/abi_gen/writer:cc_abi_gen_writer",
        "//include/cc/templating:cc_templating",
    ],
)
