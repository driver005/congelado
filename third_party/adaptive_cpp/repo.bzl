"""Loads AdaptiveCpp (formerly hipSYCL/OpenSYCL) v25.10.0 — multi-target,
vendor-agnostic SYCL implementation for modern heterogeneous computing.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "adaptive_cpp",
        url = "https://github.com/AdaptiveCpp/AdaptiveCpp/archive/refs/tags/v25.10.0.tar.gz",
        sha256 = "334b16ebff373bd2841f83332c2ae9a45ec192f2cf964d5fdfe94e1140776059",
        strip_prefix = "AdaptiveCpp-25.10.0",
        build_file = "//third_party/adaptive_cpp:adaptive_cpp.BUILD",
    )
