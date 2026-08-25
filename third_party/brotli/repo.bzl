"""Loads brotli 1.1.0 — same pin as xla's third_party_ext, patched for Bazel 9
cc_binary autoload (not autoload-eligible, unlike cc_library/cc_test) plus
xla's own layering_check disable. Swapped in via override_repo() in
MODULE.bazel since xla's own org_brotli fetch can't be patched directly."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "org_brotli",
        url = "https://github.com/google/brotli/archive/refs/tags/v1.1.0.tar.gz",
        sha256 = "e720a6ca29428b803f4ad165371771f5398faba397edf6778837a18599ea13ff",
        strip_prefix = "brotli-1.1.0",
        patches = ["//third_party/brotli:brotli-bazel9.patch"],
        patch_args = ["-p1"],
    )
