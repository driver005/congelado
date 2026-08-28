"""bzlmod module extension loading every third_party/<name>/repo.bzl — no WORKSPACE file to load them from directly."""

load("//bazel/toolchain/system_cc:repo.bzl", system_cc_repo = "repo")
load("//third_party/brotli:repo.bzl", brotli_repo = "repo")
load("//third_party/reflect_cpp:repo.bzl", reflect_cpp_repo = "repo")
load("//third_party/stduuid:repo.bzl", stduuid_repo = "repo")
load("//third_party/system_libstdcxx:repo.bzl", system_libstdcxx_repo = "repo")
def _third_party_impl(module_ctx):
    stduuid_repo()
    reflect_cpp_repo()
    system_libstdcxx_repo()
    brotli_repo()
    system_cc_repo()

third_party_deps = module_extension(implementation = _third_party_impl)
