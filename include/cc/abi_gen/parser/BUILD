load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_parser",
    partitions = [
        "helper/parameter.cppm",
        "helper/include_finder.cppm",
        "slot/reader.cppm",
        "slot/slot.cppm",
        "vtable/model.cppm",
        "vtable/naming.cppm",
        "vtable/ast_visitor.cppm",
        "parser.cppm",
        "registry.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//include/cc/utils/cli:cc_utils_cli"],
)
