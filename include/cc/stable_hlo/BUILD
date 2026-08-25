load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//bazel:build_defs.bzl", "congelado_module_library")
load("//bazel:copts.bzl", "congelado_copts")

# Build-time code generator: emits ONLY schema/table.cppm — the one file that actually varies
# with generator/ops.json's content. Everything else cc_stable_hlo needs is a normal
# hand-written file in this package — one class per file, subdirectories (op/, schema/,
# builder/) wherever a concern needed more than one.
#
# Plain cc_binary, not congelado_cc_binary: build.cc doesn't `import std;` or use C++ modules
# itself (it only emits schema/table.cppm's module *source text* as raw strings) — this
# toolchain's C++-modules deps-scanner can't resolve @reflect_cpp's headers from inside a
# scanned module partition, so staying off that path entirely avoids the problem instead of
# working around it.
cc_binary(
    name = "stable_hlo_build",
    srcs = ["build.cc"],
    copts = congelado_copts(),
    deps = [
        "@reflect_cpp//:reflect_cpp",  # for rfl::json — this tool only, never cc_stable_hlo itself
    ],
)

# Runs build.cc at build time, so `bazel build //include/cc/stable_hlo:cc_stable_hlo`
# regenerates schema/table.cppm whenever generator/ops.json changes.
genrule(
    name = "stable_hlo_generated",
    srcs = ["generator/ops.json"],
    tools = [":stable_hlo_build"],
    outs = [
        "schema/table.cppm",
    ],
    cmd = "$(location :stable_hlo_build) $(location generator/ops.json) $(RULEDIR)",
)

# Pure StableHLO MLIR-text builder — zero MLIR/LLVM linkage, and zero @reflect_cpp linkage
# either: the manifest is compiled-in data (schema/table.cppm), not parsed at runtime, so
# nothing in this module ever depends on it. One module, one target (Bazel requires all
# partitions of a C++20 module in a single cc_library). Every class implements one of
# cc/abi/builder/generator's real abstract base classes directly. One class per file:
#   dtype/shape/error/value/source_code.cppm  — core value types + the GeneratorSourceCodeBase sink
#   op/                                        — the IR tree, each node a GeneratorDefinitionViewBase:
#                                                 type_info_view, value_parameter_view, attribute_view
#                                                 (StableHloOpAttributeView), op (StableHloOp),
#                                                 function (StableHloFunction), mlir_module (StableHloModule)
#   schema/                                     — the unbound schema side, sibling of op/:
#                                                 types (the plain schema data structs — grouped in one
#                                                 file, not one-per-struct: no behavior beyond fields),
#                                                 parameter_view, attribute_view (StableHloAttributeView),
#                                                 definition_view, table (the ONE generated file — the
#                                                 compiled-in op table)
#   builder/                                    — builder (StableHloBuilder, implements
#                                                 GeneratorBuilderBase), registrar (registers "stablehlo"
#                                                 into GeneratorBuilderRegistry — the only place this
#                                                 module touches cc/abi, and only downward: cc/abi never
#                                                 imports cc_stable_hlo)
congelado_module_library(
    name = "cc_stable_hlo",
    primary_interface = "stable_hlo.cppm",
    partitions = [
        "dtype.cppm",
        "shape.cppm",
        "error.cppm",
        "value.cppm",
        "source_code.cppm",
        "op/type_info_view.cppm",
        "op/value_parameter_view.cppm",
        "op/attribute_view.cppm",
        "op/op.cppm",
        "op/function.cppm",
        "op/mlir_module.cppm",
        "schema/types.cppm",
        "schema/parameter_view.cppm",
        "schema/attribute_view.cppm",
        "schema/definition_view.cppm",
        "schema/table.cppm",
        "builder/builder.cppm",
        "builder/registrar.cppm",
    ],
    deps = [
        "//include/cc/abi/builder/generator:cc_abi_builder_generator",
    ],
    visibility = ["//visibility:public"],
)
