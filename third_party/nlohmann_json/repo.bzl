"""Loads nlohmann/json 3.11.3 — not in BCR pinned form used here, single_include header-only."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "nlohmann_json",
        url = "https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz",
        sha256 = "0d8ef5af7f9794e3263480193c491549b2ba6cc74bb018906202ada498a79406",
        strip_prefix = "json-3.11.3",
        build_file = "//third_party/nlohmann_json:nlohmann_json.BUILD",
    )
