"""Loads inja 3.4.0 (github.com/pantor/inja) — not in BCR, single_include header-only."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "inja",
        url = "https://github.com/pantor/inja/archive/refs/tags/v3.4.0.tar.gz",
        sha256 = "7155f944553ca6064b26e88e6cae8b71f8be764832c9c7c6d5998e0d5fd60c55",
        strip_prefix = "inja-3.4.0",
        build_file = "//third_party/inja:inja.BUILD",
    )
