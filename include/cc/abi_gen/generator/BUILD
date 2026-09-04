load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_generator",
    primary_interface = "generator.cppm",
    partitions = [
        "known_type.cppm",
        "type_registry.cppm",
        "slot_classifier.cppm",
        "builder_emitter.cppm",
        "sonic_emitter.cppm",
    ],
    deps = ["//include/cc/abi_gen/parser:cc_abi_gen_parser"],
    visibility = ["//visibility:public"],
)
