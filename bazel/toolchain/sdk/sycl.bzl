"""SYCL (Intel DPC++) as a Bazel repository: found on the host, else downloaded.

Search order: ATEN_XPU_SYCL_HOME, CMPLR_ROOT, ONEAPI_ROOT (its compiler/latest), /opt/intel/oneapi/compiler/*,
~/intel/oneapi/compiler/*, /opt/sycl, /usr/local. A root counts only if it has the SYCL headers, libsycl
and a compiler (icpx or clang++); a runtime-only oneAPI install is skipped.
Download: the intel/llvm release tarball (x86_64 Linux only; Intel ships no aarch64 build).
"""

load(":discover.bzl", "first_root", "host_arch", "link_existing", "missing", "search_roots", "write_status")

_BUILD = """
package(default_visibility = ["//visibility:public"])

filegroup(
    name = "compiler",
    srcs = glob(["bin/icpx", "bin/clang++", "bin/clang"], allow_empty = True),
)

# Everything a compile with -fsycl needs from the toolchain besides the compiler binary.
filegroup(
    name = "toolchain_files",
    srcs = glob(["bin/**", "include/**", "lib/**"], allow_empty = True),
)

cc_library(
    name = "sycl",
    hdrs = glob(["include/**"], allow_empty = True),
    includes = ["include"],
    srcs = glob(["lib/libsycl.so"], allow_empty = True),
)
"""

def _impl(ctx):
    roots = search_roots(
        ctx,
        ["ATEN_XPU_SYCL_HOME", "CMPLR_ROOT"],
        ["/opt/sycl", "/usr/local"],
        ["/opt/intel/oneapi/compiler", ctx.getenv("HOME", "") + "/intel/oneapi/compiler"],
    )
    oneapi = ctx.getenv("ONEAPI_ROOT")
    if oneapi:
        roots = [oneapi + "/compiler/latest"] + roots

    root = first_root(
        ctx,
        roots,
        ["include/sycl/sycl.hpp", "lib/libsycl.so"],
        ["bin/icpx", "bin/clang++"],
    )
    if root:
        link_existing(ctx, root, ["bin", "include", "lib"])
        ctx.file("BUILD.bazel", _BUILD)
        write_status(ctx, "sycl", "system", root)
        return

    if host_arch(ctx) == "x86_64" and ctx.os.name == "linux":
        ctx.download_and_extract(
            url = ctx.attr.url,
            sha256 = ctx.attr.sha256,
            type = "tar.gz",
        )
        ctx.file("BUILD.bazel", _BUILD)
        write_status(ctx, "sycl", "download", ctx.attr.url)
        return

    missing(
        ctx,
        "sycl",
        'package(default_visibility = ["//visibility:public"])\ncc_library(name = "sycl")\nfilegroup(name = "compiler")\n',
        "build DPC++ from source (github.com/intel/llvm) and set ATEN_XPU_SYCL_HOME",
    )

sycl_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        "url": attr.string(),
        "sha256": attr.string(),
    },
    environ = ["ATEN_XPU_SYCL_HOME", "CMPLR_ROOT", "ONEAPI_ROOT", "HOME"],
    local = True,
)

def repo():
    sycl_sdk(
        name = "sycl",
        url = "https://github.com/intel/llvm/releases/download/v7.2.0/sycl_linux.tar.gz",
        sha256 = "385586a25e75ac727883f0be5e442dec0ee81b3c20f9a29037a79921ad2f1728",
    )
