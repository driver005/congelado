"""clang_toolchain: a clang C++ toolchain with C++20 module support, declared with rules_cc's rule based API.

Both toolchains (system clang, DPC++ clang) are this macro; they differ in tools, constraints and extra args.
Why hand-rolled: rules_cc's legacy default toolchain is broken under Bazel 9, and its flag backfill covers only the
{cpp,c,assembly,objc}_compile_actions, so the three module actions would run with no flags at all. The macro adds
its own flag set for exactly those actions (MODULE.bazel patches the cc_variable they need), plus the
"cpp_modules" feature that rules_cc requires before a target may use `module_interfaces`.
"""

load("@rules_cc//cc/toolchains:args.bzl", "cc_args")
load("@rules_cc//cc/toolchains:feature.bzl", "cc_feature")
load("@rules_cc//cc/toolchains:tool.bzl", "cc_tool")
load("@rules_cc//cc/toolchains:tool_map.bzl", "cc_tool_map")
load("@rules_cc//cc/toolchains:toolchain.bzl", "cc_toolchain")

_ACTIONS = "@rules_cc//cc/toolchains/actions:"
_VARIABLES = "@rules_cc//cc/toolchains/variables:"
_MODULES = [_ACTIONS + "cpp_module_deps_scanning", _ACTIONS + "cpp20_module_compile", _ACTIONS + "cpp20_module_codegen"]
_MODULE_COMPILE = [_ACTIONS + "cpp20_module_compile", _ACTIONS + "cpp20_module_codegen"]

# (suffix, actions, args, variable, iterate, required): the flags the module actions need. {x} in args is the variable.
_MODULE_ARGS = [
    ("user_flags", _MODULES, ["{v}"], "user_compile_flags", True, True),
    ("quote_includes", _MODULES, ["-iquote", "{v}"], "quote_include_paths", True, False),
    ("includes", _MODULES, ["-I{v}"], "include_paths", True, False),
    ("system_includes", _MODULES, ["-isystem", "{v}"], "system_include_paths", True, False),
    ("external_includes", _MODULES, ["-isystem", "{v}"], "external_include_paths", True, True),
    ("input", _MODULES, ["-c", "{v}"], "source_file", False, True),
    ("output", _MODULES, ["-o", "{v}"], "output_file", False, True),
    ("depfile", _MODULE_COMPILE, ["-MD", "-MF", "{v}"], "dependency_file", False, True),
    # Where the imported modules were compiled to, a response file; plain compiles that import modules need it too.
    ("modmap", [_ACTIONS + "cpp_compile"] + _MODULE_COMPILE, ["@{v}"], "cpp_module_modmap_file", False, True),
]

def clang_toolchain(
        name,
        clang,
        clangxx,
        ar,
        strip,
        scanner,
        scanner_data = [],
        args = [],
        features = [],
        target_constraints = []):
    """Declares toolchain `name` and its registrable `name_toolchain`.

    Args:
      name: the cc_toolchain target; `<name>_toolchain` is the toolchain() to register.
      clang: C compiler label. clangxx: C++ compiler and linker label. ar, strip: archiver and strip labels.
      scanner: clang-scan-deps wrapper script (the scan output goes to stdout, so it needs a wrapper).
      scanner_data: files the wrapper needs. args: extra cc_args. features: extra features, on by default.
      target_constraints: extra constraints a platform needs to select this toolchain.
    """
    cc_feature(name = name + "_cpp_modules", feature_name = "cpp_modules", tags = ["manual"])

    module_args = []
    for suffix, actions, flags, variable, iterate, required in _MODULE_ARGS:
        if suffix == "input":
            module_args.append(":" + name + "_module_kind")
        cc_args(
            name = name + "_" + suffix,
            actions = actions,
            args = [flag.replace("{v}", "{" + variable + "}") for flag in flags],
            format = {variable: _VARIABLES + variable},
            iterate_over = _VARIABLES + variable if iterate else None,
            requires_not_none = _VARIABLES + variable if required else None,
            tags = ["manual"],
        )
        module_args.append(":" + name + "_" + suffix)

    # Real host headers live at absolute paths, which Bazel's absolute inclusion check would reject.
    cc_args(
        name = name + "_absolute_includes",
        actions = [_ACTIONS + "compile_actions"] + _MODULES,
        allowlist_absolute_include_directories = ["/usr/include", "/usr/local/include", "/usr/lib/clang"],
        tags = ["manual"],
    )

    # clang cannot tell a module interface from its .cppm extension.
    cc_args(name = name + "_module_kind", actions = [_ACTIONS + "cpp20_module_compile"], args = ["-x", "c++-module"], tags = ["manual"])

    # supports_pic lets Bazel build the -fPIC objects a linkshared target needs from its deps.
    for tool, source in {"clang": clang, "clangxx": clangxx}.items():
        cc_tool(name = name + "_" + tool, src = source, capabilities = ["@rules_cc//cc/toolchains/capabilities:supports_pic"], tags = ["manual"])
    cc_tool(name = name + "_ar", src = ar, tags = ["manual"])
    cc_tool(name = name + "_strip", src = strip, tags = ["manual"])
    cc_tool(name = name + "_scanner", src = scanner, data = scanner_data, tags = ["manual"])

    cc_tool_map(
        name = name + "_tools",
        tools = {
            _ACTIONS + "assembly_actions": ":" + name + "_clang",
            _ACTIONS + "c_compile": ":" + name + "_clang",
            _ACTIONS + "cpp_compile_actions": ":" + name + "_clangxx",
            _ACTIONS + "link_actions": ":" + name + "_clangxx",
            _ACTIONS + "ar_actions": ":" + name + "_ar",
            _ACTIONS + "strip": ":" + name + "_strip",
            _ACTIONS + "cpp_module_deps_scanning": ":" + name + "_scanner",
            _ACTIONS + "cpp20_module_compile": ":" + name + "_clangxx",
            _ACTIONS + "cpp20_module_codegen": ":" + name + "_clangxx",
        },
        tags = ["manual"],
    )

    # The legacy flag backfill (archiver, link, PIC, library dirs, ...) is what makes ar and ld get any arguments.
    backfill = "@rules_cc//cc/toolchains/args:experimental_replace_legacy_action_config_features"
    cc_toolchain(
        name = name,
        args = args + [":" + name + "_absolute_includes"] + module_args,
        compiler = "clang",
        tool_map = ":" + name + "_tools",
        known_features = [backfill, ":" + name + "_cpp_modules"] + features,
        # cpp_modules stays off by default; every congelado target that uses modules requests it.
        enabled_features = [backfill] + features,
    )

    constraints = ["@platforms//os:linux", "@platforms//cpu:x86_64"]
    native.toolchain(
        name = name + "_toolchain",
        toolchain = ":" + name,
        toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
        exec_compatible_with = constraints,
        target_compatible_with = constraints + target_constraints,
        visibility = ["//visibility:public"],
    )
