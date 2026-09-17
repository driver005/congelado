load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "emitter/emitter.cppm",
        "helper/formater.cppm",
        "runtime.cppm",
    ],
    deps = [
        "//include/cc/abi_gen/parser:cc_abi_gen_parser",
        "//include/cc/abi_gen/writer:cc_abi_gen_writer",
        "//include/cc/templating:cc_templating",
    ],
    visibility = ["//visibility:public"],
)
