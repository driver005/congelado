load("//third_party/toolchain/system_cc:clang_toolchain.bzl", "clang_toolchain")

# The host's clang and binutils (see repo.bzl).
clang_toolchain(
    name = "system_clang",
    ar = "@system_cc//:ar",
    clang = "@system_cc//:clang",
    clangxx = "@system_cc//:clang++",
    scanner = "clang_deps_scanner_wrapper.sh",
    strip = "@system_cc//:strip",
)
