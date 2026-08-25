"""Exposes /usr/bin (system clang-22 + binutils) as a Bazel repo.

Mirrors third_party/system_libstdcxx's new_local_repository pattern — same
host, same reasoning: rules_cc's auto-detected default toolchain
(cc_configure_extension) is broken under Bazel 9 (calls the fully-removed
native cc_toolchain_suite with no Starlark fallback anywhere in rules_cc), so
we hand-declare a minimal one via rules_cc's modern cc_toolchain API instead.
"""

load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

def repo():
    new_local_repository(
        name = "system_cc",
        path = "/usr/bin",
        build_file = "//bazel/toolchain/system_cc:system_cc.BUILD",
    )
