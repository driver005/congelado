load("//bazel:build_defs.bzl", "congelado_cc_binary", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_lib",
    partitions = [
        "cli_options.cppm",
        "cli_runner.cppm",
    ],
    primary_interface = "cc_abi_gen.cppm",
    visibility = ["//visibility:private"],
    deps = [
        "//include/cc/abi_gen/generator:cc_abi_gen_generator",
        "//include/cc/abi_gen/parser:cc_abi_gen_parser",
        "//include/cc/abi_gen/writer:cc_abi_gen_writer",
        "//include/cc/utils/cli:cc_utils_cli",
        "//include/cc/utils/cli/ast:cc_utils_cli_ast",
        "//include/cc/utils/cli/parser:cc_utils_cli_parser",
    ],
)

congelado_cc_binary(
    name = "cc_abi_gen",
    srcs = ["bin/main.cc"],
    linkopts = [
        "-lclang-cpp",
        "-lLLVM",
    ],
    visibility = ["//visibility:public"],
    deps = [":cc_abi_gen_lib"],
)

# genrule(
#     name = "generate_pilot",
#     srcs = [
#         "//include/c:extern/cache/cache.h",
#         "//include/c:extern/logger/logger.h",
#     ],
#     tools = [":cc_abi_gen"],
#     outs = [
#         "generated/builder/cache/cache.cppm",
#         "generated/sonic/cache/cache.cppm",
#         "generated/builder/logger/logger.cppm",
#         "generated/sonic/logger/logger.cppm",
#     ],
#     cmd = "$(location :cc_abi_gen) generate --pilot --out-dir $(RULEDIR)/generated",
#     visibility = ["//visibility:public"],
# )
