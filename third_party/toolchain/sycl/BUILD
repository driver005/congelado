load("@rules_cc//cc/toolchains:args.bzl", "cc_args")
load("@rules_cc//cc/toolchains:feature.bzl", "cc_feature")
load("//third_party/toolchain/system_cc:clang_toolchain.bzl", "clang_toolchain")

# The default C++ toolchain: the system_clang one with the DPC++ clang of @sycl (modules and SYCL kernels build together),
# a scanner handed that compiler, and the `sycl` feature. Registered before system_cc, which stays the fallback.

# DPC++ finds its builtin headers by absolute path in its resource directory, which Bazel's inclusion check rejects;
# naming the directory relative to the execution root makes them tracked inputs.
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

# SYCL front end flags on every C++ compile, so all module interfaces (std included) share one configuration. This is
# the host side of `-fsycl` only: a device compile cannot import host module interfaces, so kernels are type checked
# on the host but the device-only checks (no recursion, no exceptions in a kernel) do not run.
cc_args(
    name = "sycl_flags",
    actions = [
        "@rules_cc//cc/toolchains/actions:cpp_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_compile",
        "@rules_cc//cc/toolchains/actions:cpp20_module_codegen",
    ],
    args = ["-Xclang", "-fsycl-is-host", "-Xclang", "-sycl-std=2020", "-Xclang", "-fsycl-unique-prefix=aten_xpu"],
    tags = ["manual"],
)

cc_feature(name = "sycl_feature", args = [":sycl_flags"], feature_name = "sycl", tags = ["manual"])

clang_toolchain(
    name = "sycl_clang",
    ar = "@sycl//:bin/llvm-ar",
    args = [":resource_dir"],
    clang = "@sycl//:bin/clang",
    clangxx = "@sycl//:bin/clang++",
    features = [":sycl_feature"],
    scanner = "sycl_deps_scanner_wrapper.sh",
    scanner_data = ["@sycl//:bin/clang++"],
    strip = "@system_cc//:strip",
)
