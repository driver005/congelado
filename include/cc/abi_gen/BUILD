load("//bazel:build_defs.bzl", "congelado_cc_binary", "congelado_module_library")

# Top-level module: just the CLI layer (its own partitions) plus the three separate,
# separately-targeted modules below (parser/generator/writer), imported the same way
# include/cc/abi/{builder,sonic,primitives}.cppm import their own per-domain modules.
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
    # libclang-cpp.so (Tooling/AST/ASTMatchers/Frontend/Basic/Lex) depends on libLLVM.so at
    # runtime but doesn't re-export its llvm:: symbols itself, so both are needed at link time.
    linkopts = ["-lclang-cpp", "-lLLVM"],
    visibility = ["//visibility:public"],
)

# One process, one genrule, covering every pilot domain — not one genrule per domain package.
# This is what makes the TypeRegistry completeness check (see cli_runner.cppm's
# fail_if_types_pending) actually mean something at build time: a genrule per domain would each
# run cc_abi_gen as its own fresh process with no registry shared between them. --out-dir points
# this at a directory under this package (genrule outputs must live in the rule's own package);
# GeneratedFileWriter creates the builder/<domain>/ and sonic/<domain>/ subdirectories itself,
# since this tree starts out empty on every build. Each domain's own BUILD file (e.g.
# include/cc/abi/builder/cache/BUILD) then points primary_interface at this genrule's output
# directly, as a cross-package label — Bazel allows referencing another package's declared
# outputs, it only requires that a rule's own outputs live within its own package.
genrule(
    name = "generate_pilot",
    srcs = [
        "//include/c:extern/cache/cache.h",
        "//include/c:extern/logger/logger.h",
    ],
    tools = [":cc_abi_gen"],
    outs = [
        "generated/builder/cache/cache.cppm",
        "generated/sonic/cache/cache.cppm",
        "generated/builder/logger/logger.cppm",
        "generated/sonic/logger/logger.cppm",
    ],
    cmd = "$(location :cc_abi_gen) generate --pilot --out-dir $(RULEDIR)/generated",
    visibility = ["//visibility:public"],
)
