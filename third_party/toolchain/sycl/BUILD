load("@rules_cc//cc/toolchains:args.bzl", "cc_args")
load("@rules_cc//cc/toolchains:feature.bzl", "cc_feature")
load("@rules_cc//cc/toolchains:tool.bzl", "cc_tool")
load("@rules_cc//cc/toolchains:tool_map.bzl", "cc_tool_map")
load("@rules_cc//cc/toolchains:toolchain.bzl", "cc_toolchain")

# A C++ toolchain whose compiler is the DPC++ clang++ of @sycl, so named modules and SYCL kernels
# build together. It is the system_clang toolchain with three differences: the compiler and
# archiver come from @sycl, the module dependency scanner is handed that compiler, and a `sycl`
# feature, on by default, adds the SYCL front end flags. Select it with `--config=sycl` (the platform
# below). Everything else (module arguments, include handling, the
# cpp_modules feature) is shared with //third_party/toolchain/system_cc.

constraint_setting(
    name = "sycl_compiler",
    default_constraint_value = ":host_compiler",
)

constraint_value(
    name = "host_compiler",
    constraint_setting = ":sycl_compiler",
)

constraint_value(
    name = "dpcpp",
    constraint_setting = ":sycl_compiler",
)

platform(
    name = "linux_x86_64_sycl",
    constraint_values = [
        ":dpcpp",
        "@platforms//cpu:x86_64",
        "@platforms//os:linux",
    ],
    visibility = ["//visibility:public"],
)

cc_tool(
    name = "clang",
    src = "@sycl//:bin/clang",
    # Lets Bazel build the -fPIC object variants that a linkshared target needs from its deps.
    capabilities = ["@rules_cc//cc/toolchains/capabilities:supports_pic"],
    tags = ["manual"],
)

cc_tool(
    name = "clang++",
    src = "@sycl//:bin/clang++",
    # Lets Bazel build the -fPIC object variants that a linkshared target needs from its deps.
    capabilities = ["@rules_cc//cc/toolchains/capabilities:supports_pic"],
    tags = ["manual"],
)

cc_tool(
    name = "ar",
    src = "@sycl//:bin/llvm-ar",
    tags = ["manual"],
)

cc_tool(
    name = "strip",
    src = "@system_cc//:strip",
    tags = ["manual"],
)

cc_tool(
    name = "clang_scan_deps",
    src = "sycl_deps_scanner_wrapper.sh",
    data = ["@sycl//:bin/clang++"],
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
        "@rules_cc//cc/toolchains/actions:cpp_module_deps_scanning": ":clang_scan_deps",
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile": ":clang++",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen": ":clang++",
    },
)

# The DPC++ clang finds its builtin headers (stddef.h, float.h, ...) in its own resource directory
# by absolute path, which Bazel's absolute-include check rejects. Naming the directory relative to
# the execution root, inside the @sycl repo, makes those paths ordinary tracked inputs.
cc_args(
    name = "resource_dir",
    actions = [
        "@rules_cc//cc/toolchains/actions:compile_actions",
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen",
    ],
    allowlist_include_directories = ["@sycl//:clang_resource_include"],
    args = ["-resource-dir=external/+third_party_deps+sycl/lib/clang/23"],
    tags = ["manual"],
)

# The SYCL front end flags, on every C++ compile of the toolchain, so all module interfaces
# (including std) are built with the same configuration and can be imported by each other. This is
# the host side of `-fsycl` only: the driver would also start a SPIR-V device compile of each
# translation unit, and a device compile cannot import module interfaces built for the host. The
# host pass type checks all the SYCL code, kernel lambdas included, but does not run the
# device-only checks (no recursion, no exceptions in a kernel); the kernels here are plain loops.
cc_args(
    name = "sycl_flags",
    actions = [
        "@rules_cc//cc/toolchains/actions:cpp_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen",
    ],
    args = [
        "-Xclang",
        "-fsycl-is-host",
        "-Xclang",
        "-sycl-std=2020",
        "-Xclang",
        "-fsycl-unique-prefix=aten_xpu",
    ],
    tags = ["manual"],
)

cc_feature(
    name = "sycl_feature",
    args = [":sycl_flags"],
    feature_name = "sycl",
    tags = ["manual"],
)

cc_toolchain(
    name = "sycl_clang",
    args = [
        ":resource_dir",
        "//third_party/toolchain/system_cc:absolute_system_includes",
        "//third_party/toolchain/system_cc:module_actions_user_compile_flags",
        "//third_party/toolchain/system_cc:module_actions_quote_include_paths",
        "//third_party/toolchain/system_cc:module_actions_include_paths",
        "//third_party/toolchain/system_cc:module_actions_system_include_paths",
        "//third_party/toolchain/system_cc:module_actions_external_include_paths",
        "//third_party/toolchain/system_cc:cpp20_module_compile_flags",
        "//third_party/toolchain/system_cc:module_actions_input_flags",
        "//third_party/toolchain/system_cc:module_actions_output_flags",
        "//third_party/toolchain/system_cc:module_compile_dotd_flags",
        "//third_party/toolchain/system_cc:module_compile_modmap_flags",
    ],
    compiler = "clang",
    enabled_features = [
        "@rules_cc//cc/toolchains/args:experimental_replace_legacy_action_config_features",
        ":sycl_feature",
    ],
    known_features = [
        "@rules_cc//cc/toolchains/args:experimental_replace_legacy_action_config_features",
        "//third_party/toolchain/system_cc:cpp_modules_feature",
        ":sycl_feature",
    ],
    tool_map = ":tools",
)

toolchain(
    name = "sycl_clang_toolchain",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        ":dpcpp",
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    toolchain = ":sycl_clang",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
    visibility = ["//visibility:public"],
)
