load("//bazel:build_defs.bzl", "congelado_cc_binary", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_gen_lib",
    primary_interface = "cc_abi_gen.cppm",
    partitions = [
        "cli_options.cppm",
        "cli_runner.cppm",
    ],
    deps = [
        "//include/cc/abi_gen/parser:cc_abi_gen_parser",
        "//include/cc/abi_gen/generator:cc_abi_gen_generator",
        "//include/cc/abi_gen/writer:cc_abi_gen_writer",
    ],
    visibility = ["//visibility:private"],
)

congelado_cc_binary(
    name = "cc_abi_gen",
    srcs = ["bin/main.cc"],
    deps = [":cc_abi_gen_lib"],
    linkopts = ["-lclang-cpp", "-lLLVM"],
    visibility = ["//visibility:public"],
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
