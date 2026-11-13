"""AMD rocprofiler-sdk headers (plus the HSA and HIP headers they include) as a Bazel repository.

Only headers are needed at build time; the SDK's own library loads this plugin's rocprofiler_configure.
Search order: ATEN_XPU_ROCPROFILER_HOME, ROCM_PATH, ROCM_HOME, /opt/rocm, /opt/rocm-*.
Download: the pinned ROCm apt packages from repo.radeon.com (x86_64 only; AMD ships no aarch64 packages).
"""

load(":discover.bzl", "first_root", "host_arch", "link_existing", "missing", "search_roots", "write_status")

_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob(["include/**"], allow_empty = True),
    includes = ["include"],
)
"""

def _impl(ctx):
    roots = search_roots(
        ctx,
        ["ATEN_XPU_ROCPROFILER_HOME", "ROCM_PATH", "ROCM_HOME"],
        ["/opt/rocm"],
        [],
    )
    opt = ctx.path("/opt")
    if opt.exists:
        roots.extend(sorted([str(child) for child in opt.readdir() if child.basename.startswith("rocm-")], reverse = True))

    root = first_root(
        ctx,
        roots,
        ["include/rocprofiler-sdk/rocprofiler.h", "include/hsa/hsa.h", "include/hip/hip_runtime.h"],
    )
    if root:
        link_existing(ctx, root, ["include"])
        ctx.file("BUILD.bazel", _BUILD)
        write_status(ctx, "rocprofiler", "system", root)
        return

    if host_arch(ctx) != "x86_64" or ctx.os.name != "linux" or not ctx.which("ar"):
        missing(
            ctx,
            "rocprofiler",
            'package(default_visibility = ["//visibility:public"])\ncc_library(name = "headers")\n',
            "install ROCm (rocprofiler-sdk, hsa-rocr-dev, hip-dev) and set ATEN_XPU_ROCPROFILER_HOME",
        )
        return

    # Each .deb is an ar archive holding data.tar.gz with /opt/rocm-<version>/include/...; the
    # include children of the three packages (rocprofiler-sdk, hsa, hsakmt, hip) do not overlap.
    for index, spec in enumerate(ctx.attr.debs):
        url, sha256 = spec.split(" ")
        directory = "_debs/{}".format(index)
        ctx.download(url = url, sha256 = sha256, output = directory + "/package.deb")
        for command in (["ar", "x", "package.deb", "data.tar.gz"], ["tar", "xzf", "data.tar.gz"]):
            result = ctx.execute(command, working_directory = directory)
            if result.return_code != 0:
                fail("rocprofiler: {} failed: {}".format(" ".join(command), result.stderr))
        for rocm in ctx.path(directory + "/opt").readdir():
            for child in ctx.path(str(rocm) + "/include").readdir():
                ctx.symlink(child, "include/" + child.basename)
    ctx.file("BUILD.bazel", _BUILD)
    write_status(ctx, "rocprofiler", "download", "pinned ROCm 7.2.4 packages")

rocprofiler_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        "debs": attr.string_list(),
    },
    environ = ["ATEN_XPU_ROCPROFILER_HOME", "ROCM_PATH", "ROCM_HOME"],
    local = True,
)

def repo():
    rocprofiler_sdk(
        name = "rocprofiler",
        debs = [
            "https://repo.radeon.com/rocm/apt/7.2.4/pool/main/r/rocprofiler-sdk/rocprofiler-sdk_1.1.0-93~24.04_amd64.deb 39270239e68660cd025d3c9e84a04478696d9737f69012cd350c380aad911c30",
            "https://repo.radeon.com/rocm/apt/7.2.4/pool/main/r/rocprofiler-sdk-roctx/rocprofiler-sdk-roctx_1.1.0-93~24.04_amd64.deb 4c45f467341b14fc1e1db3c9dc2475d7e880650df8ec71db46b461d7b12ffb91",
            "https://repo.radeon.com/rocm/apt/7.2.4/pool/main/h/hsa-rocr-dev/hsa-rocr-dev_1.18.0.70204-93~24.04_amd64.deb dec6c34a06b888fe2f1717647817a00bc7d09e00089f5b97358697032362a7db",
            "https://repo.radeon.com/rocm/apt/7.2.4/pool/main/h/hip-dev/hip-dev_7.2.53211.70204-93~24.04_amd64.deb 1fd68b7e90c17d201563f9cd4eae536bd2cf089fad026181770be2750d552376",
        ],
    )
