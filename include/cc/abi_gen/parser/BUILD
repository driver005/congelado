load("//bazel:build_defs.bzl", "congelado_module_library")

# The only files in this tool that #include <clang/...> — those headers live under /usr/include,
# already on the system clang toolchain's default search path
# (bazel/toolchain/system_cc/BUILD:37-42: "System clang-22 resolves headers to real absolute
# paths [...] rather than paths relative to a Bazel-tracked toolchain input"), so no extra
# -isystem flag is needed (an earlier attempt at adding one broke the deps-scanner's own,
# separately-invoked clang-scan-deps command, which resolves cstdlib's stdlib.h differently).
congelado_module_library(
    name = "cc_abi_gen_parser",
    primary_interface = "parser.cppm",
    partitions = [
        "parameter.cppm",
        "vtable_slot.cppm",
        "vtable_model.cppm",
        "vtable_naming.cppm",
        "slot_reader.cppm",
        "vtable_ast_visitor.cppm",
        "system_include_finder.cppm",
        "header_parser.cppm",
        "domain_paths.cppm",
    ],
    visibility = ["//visibility:public"],
)
