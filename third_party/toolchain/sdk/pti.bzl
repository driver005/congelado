"""Intel PTI headers as a Bazel repository: found on the host, else downloaded.

Only the headers are needed at build time; libpti_view.so is dlopen'ed at run time.
Search order: ATEN_XPU_PTI_HOME, PTI_ROOT, /opt/intel/oneapi/pti/*, /usr/local, /usr.
Download: the intel/pti-gpu source tarball (headers only, so it works on any architecture).
"""

load(":discover.bzl", "first_root", "link_existing", "write_status", "search_roots")

_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob(["include/pti/*.h"], allow_empty = True),
    includes = ["include"],
)
"""

def _impl(ctx):
    roots = search_roots(
        ctx,
        ["ATEN_XPU_PTI_HOME", "PTI_ROOT"],
        ["/usr/local", "/usr"],
        ["/opt/intel/oneapi/pti"],
    )
    root = first_root(ctx, roots, ["include/pti/pti_view.h", "include/pti/pti.h"])
    if root:
        ctx.symlink(root + "/include/pti", "include/pti")
        ctx.file("BUILD.bazel", _BUILD)
        write_status(ctx, "pti", "system", root)
        return

    ctx.download_and_extract(
        url = ctx.attr.url,
        sha256 = ctx.attr.sha256,
        strip_prefix = "pti-gpu-pti-1.2.0/sdk/include",
        output = "include",
    )

    # The tarball is source: the two headers CMake generates at configure time are written here.
    major, minor, patch = ctx.attr.version.split(".")
    template = ctx.read("include/pti/pti_version.h.in")
    for key, value in {
        "@PTI_VERSION@": ctx.attr.version,
        "@PROJECT_VERSION_MAJOR@": major,
        "@PROJECT_VERSION_MINOR@": minor,
        "@PROJECT_VERSION_PATCH@": patch,
        "@PTI_COMMIT_HASH@": "release-" + ctx.attr.version,
    }.items():
        template = template.replace(key, value)
    ctx.file("include/pti/pti_version.h", template)
    ctx.file(
        "include/pti/pti_export.h",
        "#pragma once\n#define PTI_EXPORT __attribute__((visibility(\"default\")))\n" +
        "#define PTI_NO_EXPORT __attribute__((visibility(\"hidden\")))\n",
    )
    ctx.file("BUILD.bazel", _BUILD)
    write_status(ctx, "pti", "download", ctx.attr.url)

pti_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        "url": attr.string(),
        "sha256": attr.string(),
        "version": attr.string(),
    },
    environ = ["ATEN_XPU_PTI_HOME", "PTI_ROOT"],
    local = True,
)

def repo():
    pti_sdk(
        name = "pti",
        url = "https://github.com/intel/pti-gpu/archive/refs/tags/pti-1.2.0.tar.gz",
        sha256 = "b2fe5853d2f68efbc2da9a402a72e2d3ae53dd350e4604f0e4e904ff60b68394",
        version = "1.2.0",
    )
