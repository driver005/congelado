"""Loads stduuid — not in BCR, matches Conan's pinned 1.2.3."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "stduuid",
        url = "https://github.com/mariusbancila/stduuid/archive/refs/tags/v1.2.3.tar.gz",
        sha256 = "b1176597e789531c38481acbbed2a6894ad419aab0979c10410d59eb0ebf40d3",
        strip_prefix = "stduuid-1.2.3",
        build_file = "//third_party/stduuid:stduuid.BUILD",
    )
