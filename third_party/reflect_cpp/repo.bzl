"""Loads reflect-cpp 0.23.0, matches Conan's pin — BCR only has 0.25.0, which crashes clang 22."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "reflect_cpp",
        url = "https://github.com/getml/reflect-cpp/archive/refs/tags/v0.23.0.tar.gz",
        sha256 = "9c5650d7ef0ab2b0ff617095280c641e6d770d9efc62dc04d86bb6ea56b55130",
        strip_prefix = "reflect-cpp-0.23.0",
        build_file = "//third_party/reflect_cpp:reflect_cpp.BUILD",
    )
