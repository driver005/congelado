load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//bazel:build_defs.bzl", "congelado_cc_binary", "congelado_module_library")
load("//bazel:copts.bzl", "congelado_copts")

# Build-time code generator: emits ONLY table.cppm — the one file that actually varies
# with generator/ops.json's content. Everything else yoshi_omah_lay_stable_hlo needs is a normal
# hand-written file in this package — one class per file, base/ subfolder for core value
# types, skipper/ subfolder for the IR tree, everything else flat at the top level.
#
# Plain cc_binary, not congelado_cc_binary: build.cc doesn't `import std;` or use C++ modules
# itself (it only emits table.cppm's module *source text* as raw strings) — this
# toolchain's C++-modules deps-scanner can't resolve @reflect_cpp's headers from inside a
# scanned module partition, so staying off that path entirely avoids the problem instead of
# working around it.
cc_binary(
    name = "stable_hlo_build",
    srcs = ["build.cc"],
    copts = congelado_copts(),
    deps = [
        "@reflect_cpp//:reflect_cpp",  # for rfl::json — this tool only, never yoshi_omah_lay_stable_hlo itself
    ],
)

# Runs build.cc at build time, so `bazel build //include/yoshi/omah_lay/stable_hlo:yoshi_omah_lay_stable_hlo`
# regenerates table.cppm whenever generator/ops.json changes.
genrule(
    name = "stable_hlo_generated",
    srcs = ["generator/ops.json"],
    tools = [":stable_hlo_build"],
    outs = [
        "table.cppm",
    ],
    cmd = "$(location :stable_hlo_build) $(location generator/ops.json) $(RULEDIR)",
)

# Pure StableHLO MLIR-text builder — zero MLIR/LLVM linkage, and zero @reflect_cpp linkage
# either: the manifest is compiled-in data (table.cppm), not parsed at runtime, so
# nothing in this module ever depends on it. One module, one target (Bazel requires all
# partitions of a C++20 module in a single cc_library). Every class implements one of
# cc/abi/builder/generator's real abstract base classes directly, owning its own fields rather
# than wrapping a separate plain-data struct. Bound and unbound (schema) forms of the same
# concept are one class each — Parameter (real Shape, schema side uses a rank-only placeholder)
# and Operation (real attr type metadata always, bound values optional) — not two, so this
# module doesn't carry a hand-written-only-for-C++-ergonomics split beside the one thing the
# generic ice::builder::generator interface actually needs. One class per file:
#   base/                              — dtype.cppm/shape.cppm (core value types;
#                                         dtype/shape also implement
#                                         ice::builder::TypeInfo/generator::Attribute
#                                         directly), parameter.cppm/attribute.cppm (Parameter —
#                                         bound-or-schema SSA reference — and Attribute —
#                                         schema-side attr KIND: name + real cpp_type/optional/
#                                         list).
#   skipper/                           — operation.cppm (Operation — bound op call OR unbound
#                                         schema entry, both a ice::builder::
#                                         Definition), function.cppm (Function), module.cppm
#                                         (Module). Each returns its own rendering directly as
#                                         ice::sonic::StringRuntime (render()) — no shared
#                                         external sink object.
#   (top level, cont'd)                — table.cppm (the ONE generated file — the
#                                         compiled-in op table, built as vector<Operation> via
#                                         the same append_parameter/append_attr/append_result way
#                                         a normal caller builds a real Operation); builder.cppm
#                                         (Builder, implements
#                                         ice::builder::Generator's typed-API tier —
#                                         doesn't opt into the interface's second, generic/
#                                         C-ABI-crossable tier; stable_hlo.cc registers its
#                                         factory function pointer into
#                                         ice::sonic::Registration instead)
congelado_module_library(
    name = "yoshi_omah_lay_stable_hlo",
    primary_interface = "stable_hlo.cppm",
    partitions = [
        "base/dtype.cppm",
        "base/shape.cppm",
        "base/parameter.cppm",
        "base/attribute.cppm",
        "skipper/operation.cppm",
        "skipper/function.cppm",
        "skipper/module.cppm",
        "table.cppm",
        "builder.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",  # dtype.cppm's get_data_type() maps onto TF_DataType
        "//include/cc/abi/builder/generator:cc_abi_builder_generator",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",  # DataTypeEnum for handle tensors
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)

# Shared-library entry point for yoshi::core::PluginLoader (include/yoshi/lucki/plugin) — a
# init_plugin export (c/extern/plugin/registration.h) whose only job is existing as a
# dlsym-able symbol. Its body registers cc::stable_hlo::Builder's factory function pointer into
# ice::sonic::Registration (cc_abi_sonic_registration) under type="generator" — the
# process-wide shared storage (backed by //include/c:registration_shared, linkshared) that
# makes the registration visible to the host process once this .so is dlopen'd.
congelado_cc_binary(
    name = "stable_hlo_plugin",
    srcs = ["stable_hlo.cc"],
    linkshared = True,
    deps = [
        ":yoshi_omah_lay_stable_hlo",
        "//include/c:extern_generator_hdrs",
        "//include/c:extern_plugin_hdrs",
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
)
