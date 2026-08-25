"""Loads OpenXLA (XLA) from source via http_archive."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    XLA_COMMIT = "e5d008bb03d9b133b0881daf3d108adea8e6625b"
    XLA_SHA256 = "c25a0c11ee8c593fb0b25903735e2b006d5d3e9fbeb2c36344cd0b0dcf97d768"

    http_archive(
        name = "xla",
        url = "https://github.com/openxla/xla/archive/{commit}.tar.gz".format(commit = XLA_COMMIT),
        sha256 = XLA_SHA256,
        strip_prefix = "xla-{commit}".format(commit = XLA_COMMIT),
        build_file = "//third_party/xla:BUILD.bazel",
        patch_cmds = [
            # Comment out the missing ROCm definition load in xla.default.bzl
            "sed -i 's|load(\"@local_config_rocm//rocm:build_defs.bzl\"|# load(\"@local_config_rocm//rocm:build_defs.bzl\"|g' xla/xla.default.bzl",
            # Inject a stub function if rocm_default_copts or build defs are invoked
            "echo 'def rocm_default_copts(): return []' >> xla/xla.default.bzl",
        ],
    )
