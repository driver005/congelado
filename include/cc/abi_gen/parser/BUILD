load("//bazel:build_defs.bzl", "congelado_cc_test", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_parser",
    primary_interface = "base.cppm",
    partitions = [
        "helper/parameter.cppm",
        "helper/include_finder.cppm",
        "helper/paths.cppm",
        "slot/reader.cppm",
        "slot/slot.cppm",
        "vtable/model.cppm",
        "vtable/naming.cppm",
        "vtable/ast_visitor.cppm",
        "base.cppm"
        "parser.cppm",
        "registry.cppm"
    ],
    deps = ["//include/cc/abi_gen/utils:cc_utils_cli"],
    visibility = ["//visibility:public"],
)

congelado_cc_test(
    name = "parser",
    srcs = ["parser_test.cc"],
    data = ["//include/c:extern_client_hdrs"],
    deps = [":cc_abi_gen_parser"],
)
