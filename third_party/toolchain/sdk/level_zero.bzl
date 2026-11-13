"""Level Zero headers as a Bazel repository: found on the host, else downloaded.

Only the headers are needed at build time; the loader (libze_loader.so) is loaded at run time.
Search order: ATEN_XPU_LEVEL_ZERO_HOME, LEVEL_ZERO_ROOT, /usr/local, /usr (the level-zero-devel
layout, include/level_zero/ze_api.h).
Download: the oneapi-src/level-zero source tarball (headers only, so it works on any architecture).
"""

load(":discover.bzl", "first_root", "search_roots", "write_status")

_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob(["include/level_zero/**/*.h"], allow_empty = True),
    includes = ["include"],
)
"""

def _impl(ctx):
    roots = search_roots(
        ctx,
        ["ATEN_XPU_LEVEL_ZERO_HOME", "LEVEL_ZERO_ROOT"],
        ["/usr/local", "/usr"],
        [],
    )
    root = first_root(ctx, roots, ["include/level_zero/ze_api.h"])
    if root:
        ctx.symlink(root + "/include/level_zero", "include/level_zero")
        ctx.file("BUILD.bazel", _BUILD)
        write_status(ctx, "level_zero", "system", root)
        return

    # The tarball keeps the headers at include/, the installed layout at include/level_zero/.
    ctx.download_and_extract(
        url = ctx.attr.url,
        sha256 = ctx.attr.sha256,
        strip_prefix = "level-zero-" + ctx.attr.version + "/include",
        output = "include/level_zero",
    )
    ctx.file("BUILD.bazel", _BUILD)
    write_status(ctx, "level_zero", "download", ctx.attr.url)

level_zero_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        "url": attr.string(),
        "sha256": attr.string(),
        "version": attr.string(),
    },
    environ = ["ATEN_XPU_LEVEL_ZERO_HOME", "LEVEL_ZERO_ROOT"],
    local = True,
)

def repo():
    level_zero_sdk(
        name = "level_zero",
        url = "https://github.com/oneapi-src/level-zero/archive/refs/tags/v1.20.2.tar.gz",
        sha256 = "b310191f1ca8fb8b72456bd8f66156348b720027fc910e539475426f4ea25639",
        version = "1.20.2",
    )
