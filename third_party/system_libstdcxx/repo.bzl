"""Loads the GNU std module source (bits/std.cc) for `import std;` — clang builds no BMI implicitly."""

load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

def repo():
    new_local_repository(
        name = "system_libstdcxx",
        path = "/usr/include/c++/16/bits",  # GCC version hardcoded, matches this host
        build_file = "//third_party/system_libstdcxx:libstdcxx_std.BUILD",
    )
