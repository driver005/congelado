"""Loads UCX (Unified Communication X, github.com/openucx/ucx) 1.20.1 — not header-only,
not in BCR. Built from source via rules_foreign_cc's configure_make (the release tarball
ships a pre-generated `configure`, no autoreconf needed) rather than wrapping a system
install, so the vendor-agnostic network leg (UCX picks GPUDirect RDMA on NVIDIA or
ROCmRDMA on AMD at runtime, whichever the host's transport supports) doesn't depend on
what happens to already be installed on the build host.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "ucx",
        url = "https://github.com/openucx/ucx/releases/download/v1.20.1/ucx-1.20.1.tar.gz",
        sha256 = "545c419a7b5e04643cb8bff5a19b3b5071a8f8f0605f1e8efb36f8f3d7bfb9d3",
        strip_prefix = "ucx-1.20.1",
        build_file = "//third_party/ucx:ucx.BUILD",
    )
