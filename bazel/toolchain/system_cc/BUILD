load("@rules_cc//cc/toolchains:args.bzl", "cc_args")
load("@rules_cc//cc/toolchains:feature.bzl", "cc_feature")
load("@rules_cc//cc/toolchains:tool.bzl", "cc_tool")
load("@rules_cc//cc/toolchains:tool_map.bzl", "cc_tool_map")
load("@rules_cc//cc/toolchains:toolchain.bzl", "cc_toolchain")

# rules_cc's built-in arg backfill (experimental_replace_legacy_action_config_features)
# scopes every flag set (source-file-in, output-file-out, user copts, ...) to
# @rules_cc//cc/toolchains/actions:compile_actions, which is only
# {cpp,c,assembly,objc}_compile_actions — it does not include the three C++20-modules
# action names at all, so those three otherwise get invoked with zero flags. Nothing short
# of forking rules_cc's own vendored args/BUILD targets could "extend" their fixed scope,
# so this toolchain supplies its own parallel set of cc_args below, scoped to exactly these
# three, replicating just the parts of compile_input_flags/compile_output_flags/
# compile_flags that module actions actually need.
MODULE_ACTIONS = [
    "@rules_cc//cc/toolchains/actions:cpp_module_deps_scanning",
    "@rules_cc//cc/toolchains/actions:cpp20_module_compile",
    "@rules_cc//cc/toolchains/actions:cpp20_module_codegen",
]

# rules_cc's cc_helper.check_cpp_modules() hard-requires a toolchain-known feature literally
# named "cpp_modules" before it'll let any cc_library/cc_binary use `module_interfaces=` —
# every congelado_module_library-based target (this whole include/cc/... C++ modules tree)
# requests it via `features = ["cpp_modules"]`, but nothing declared it toolchain-side, so
# the request silently resolved to nothing and every such target failed analysis with
# "to use C++ modules, the feature cpp_modules must be enabled". Legacy rules_cc toolchains
# get this for free from unix_cc_toolchain_config.bzl's built-in cpp_modules_feature; the
# hand-rolled modern-API toolchain below (see repo.bzl for why it's hand-rolled) needs its
# own declaration.
cc_feature(
    name = "cpp_modules_feature",
    feature_name = "cpp_modules",
    tags = ["manual"],
)

# System clang-22 resolves headers to real absolute paths (/usr/include,
# /usr/include/c++/16, /usr/lib/clang/22/include, ...) rather than paths
# relative to a Bazel-tracked toolchain input, since it's the real host
# install, not a relocated hermetic one. Allowlist them so Bazel's absolute-
# path-inclusion check doesn't reject every external target that includes
# libstdc++/libc headers.
cc_args(
    name = "absolute_system_includes",
    actions = ["@rules_cc//cc/toolchains/actions:compile_actions"] + MODULE_ACTIONS,
    allowlist_absolute_include_directories = [
        "/usr/include",
        "/usr/local/include",
        "/usr/lib/clang/22/include",
    ],
    tags = ["manual"],
)

# Module actions' own replacement for compiler_input_flags/compiler_output_flags/
# compile_flags (see MODULE_ACTIONS comment above) — source file in, output file out,
# per-target user copts (congelado_copts()'s -Iinclude, -std=c++26, etc.) carried through.
cc_args(
    name = "module_actions_user_compile_flags",
    actions = MODULE_ACTIONS,
    args = ["{user_compile_flags}"],
    format = {"user_compile_flags": "@rules_cc//cc/toolchains/variables:user_compile_flags"},
    iterate_over = "@rules_cc//cc/toolchains/variables:user_compile_flags",
    requires_not_none = "@rules_cc//cc/toolchains/variables:user_compile_flags",
    tags = ["manual"],
)

cc_args(
    name = "module_actions_input_flags",
    actions = MODULE_ACTIONS,
    args = ["-c", "{source_file}"],
    format = {"source_file": "@rules_cc//cc/toolchains/variables:source_file"},
    requires_not_none = "@rules_cc//cc/toolchains/variables:source_file",
    tags = ["manual"],
)

cc_args(
    name = "module_actions_output_flags",
    actions = MODULE_ACTIONS,
    args = ["-o", "{output_file}"],
    format = {"output_file": "@rules_cc//cc/toolchains/variables:output_file"},
    requires_not_none = "@rules_cc//cc/toolchains/variables:output_file",
    tags = ["manual"],
)

# The two module-compile actions invoke clang++ directly (not through a wrapper, unlike
# cpp_module_deps_scanning), so unlike that action's stub .ddi.d (see
# clang_deps_scanner_wrapper.sh), this asks clang++ to generate the real "<output>.d"
# Bazel expects natively via its own -MD/-MF support, rather than faking one.
cc_args(
    name = "module_compile_dotd_flags",
    actions = [
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen",
    ],
    args = ["-MD", "-MF", "{dependency_file}"],
    format = {"dependency_file": "@rules_cc//cc/toolchains/variables:dependency_file"},
    requires_not_none = "@rules_cc//cc/toolchains/variables:dependency_file",
    tags = ["manual"],
)

# Tells clang++ where the other modules this TU depends on (e.g. "import std;") were
# compiled to — without this, module_compile sees "module 'std' not found" even though std
# itself compiled fine. "@%{cpp_module_modmap_file}" is a response file clang reads more
# flags from (matches unix_cc_toolchain_config.bzl's clang-specific
# `"@%{cpp_module_modmap_file}"`); the variable's *value* is already computed by 0.2.22's
# own compile.bzl (additional_build_variables) regardless of the label — see MODULE.bazel's
# patch_cmds comment for why the cc_variable() declaration itself had to be patched in.
# cpp_compile (plain non-module .cc/.cppm TUs that still `import` modules, e.g. any
# congelado_cc_binary's main.cc) needs this exactly as much as the two module actions do —
# matches upstream's own cpp_module_modmap_file scope (cpp_compile + both module actions).
cc_args(
    name = "module_compile_modmap_flags",
    actions = [
        "@rules_cc//cc/toolchains/actions:cpp_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen",
    ],
    args = ["@{cpp_module_modmap_file}"],
    format = {"cpp_module_modmap_file": "@rules_cc//cc/toolchains/variables:cpp_module_modmap_file"},
    requires_not_none = "@rules_cc//cc/toolchains/variables:cpp_module_modmap_file",
    tags = ["manual"],
)

# System clang-22 (see repo.bzl) needs no extra args: invoked with no flags it
# already finds the right GCC-16 libstdc++/resource-dir search paths and
# successfully compiles+links+runs a trivial program (verified by hand before
# writing this). Per-target copts (congelado_copts()/STRICT_C_OPTIONS/etc.)
# supply everything else.
cc_tool(
    name = "clang",
    src = "@system_cc//:clang",
    tags = ["manual"],
)

cc_tool(
    name = "clang++",
    src = "@system_cc//:clang++",
    tags = ["manual"],
)

cc_tool(
    name = "ar",
    src = "@system_cc//:ar",
    tags = ["manual"],
)

cc_tool(
    name = "strip",
    src = "@system_cc//:strip",
    tags = ["manual"],
)

# C++20 modules dependency scanning needs clang-scan-deps invoked through a wrapper (see
# clang_deps_scanner_wrapper.sh) — it isn't a drop-in Bazel tool on its own (needs
# `-format=p1689 --` plus the real compiler prepended, and its actual output goes to
# stdout, not a normal `-o` argument). Legacy rules_cc toolchains generate an equivalent
# wrapper from clang_deps_scanner_wrapper.sh.tpl; the modern declarative API has no
# built-in shortcut for this, so it's wired by hand here.
cc_tool(
    name = "clang_scan_deps",
    src = "clang_deps_scanner_wrapper.sh",
    tags = ["manual"],
)

cc_tool_map(
    name = "tools",
    tags = ["manual"],
    tools = {
        "@rules_cc//cc/toolchains/actions:assembly_actions": ":clang",
        "@rules_cc//cc/toolchains/actions:c_compile": ":clang",
        "@rules_cc//cc/toolchains/actions:cpp_compile_actions": ":clang++",
        "@rules_cc//cc/toolchains/actions:link_actions": ":clang++",
        "@rules_cc//cc/toolchains/actions:ar_actions": ":ar",
        "@rules_cc//cc/toolchains/actions:strip": ":strip",
        # C++20 modules pipeline (see cc_helper.check_cpp_modules()/
        # cpp_module_deps_scanning callers in rules_cc's compile.bzl): dependency
        # scanning gets its own tool, the two module-compile phases reuse clang++
        # (matches unix_cc_toolchain_config.bzl's legacy config exactly — same
        # underlying clang++ binary drives -x c++-module compile and codegen alike).
        "@rules_cc//cc/toolchains/actions:cpp_module_deps_scanning": ":clang_scan_deps",
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile": ":clang++",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen": ":clang++",
    },
)

# clang needs to be told a module-interface source is a module (it can't infer this from
# the .cppm extension alone) — matches unix_cc_toolchain_config.bzl's own
# `"-x", "c++-module" if ctx.attr.compiler == "clang" else "c++"` for this exact action.
cc_args(
    name = "cpp20_module_compile_flags",
    actions = ["@rules_cc//cc/toolchains/actions:cpp20_module_compile"],
    args = ["-x", "c++-module"],
    tags = ["manual"],
)

cc_toolchain(
    name = "system_clang",
    args = [
        ":absolute_system_includes",
        ":module_actions_user_compile_flags",
        ":cpp20_module_compile_flags",
        ":module_actions_input_flags",
        ":module_actions_output_flags",
        ":module_compile_dotd_flags",
        ":module_compile_modmap_flags",
    ],
    compiler = "clang",
    tool_map = ":tools",
    # Backfills all the legacy-default arg sets (archiver flags, link flags,
    # compile input/output flags, PIC, library search dirs, ...) that Bazel's
    # built-in C++ actions expect — see rules_cc's own args/BUILD comment.
    # Without this the toolchain "works" for compiling but ar/ld get invoked
    # with no arguments at all.
    known_features = [
        "@rules_cc//cc/toolchains/args:experimental_replace_legacy_action_config_features",
        ":cpp_modules_feature",
    ],
    # Not in enabled_features: matches upstream's own cpp_modules_feature default
    # (enabled = False in unix_cc_toolchain_config.bzl) — opt-in per target via
    # `features = ["cpp_modules"]`, which every congelado_module_library/
    # congelado_cc_library/congelado_cc_binary target already requests.
    enabled_features = ["@rules_cc//cc/toolchains/args:experimental_replace_legacy_action_config_features"],
)

toolchain(
    name = "system_clang_toolchain",
    toolchain = ":system_clang",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    visibility = ["//visibility:public"],
)
