"""Wraps a system-installed AdaptiveCpp (github.com/AdaptiveCpp/AdaptiveCpp) SYCL
toolchain as a Bazel repo. Mirrors bazel/toolchain/system_cc's new_local_repository
pattern: AdaptiveCpp is its own LLVM-based compiler stack (the `acpp` compiler driver
plus its runtime libs), not something worth rebuilding from source inside Bazel — install
it once on the host via its own CMake build (see AdaptiveCpp's installing.md), this just
points at the install prefix.

Hardcodes /usr/local (AdaptiveCpp's own CMake default -DCMAKE_INSTALL_PREFIX). If it's
installed elsewhere on a given host, change `path` below to match.
"""

load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

def repo():
    new_local_repository(
        name = "adaptivecpp",
        path = "/usr/local",
        build_file = "//bazel/toolchain/adaptivecpp:adaptivecpp.BUILD",
    )
