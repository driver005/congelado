"""DPC++ with the HIP adapter (AMD GPUs) as a Bazel repository: found on the host, else built from source.

The prebuilt Intel tarball (@sycl) ships the Level Zero and CUDA adapters only, so AMD needs this variant.
Search order: ATEN_XPU_SYCL_HIP_HOME, then the @sycl search roots. A root counts only if it also has
lib/libur_adapter_hip.so.
Build: intel/llvm source, buildbot/configure.py --hip --hip-platform AMD, then deploy-sycl-toolchain.
It needs ROCm (ROCM_PATH or /opt/rocm), cmake, ninja and python3 on the host, and takes about an hour.
"""

load(":discover.bzl", "first_root", "link_existing", "missing", "search_roots", "write_status")

_BUILD = """
package(default_visibility = ["//visibility:public"])

filegroup(
    name = "compiler",
    srcs = glob(["bin/icpx", "bin/clang++", "bin/clang"], allow_empty = True),
)

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

def _rocm_root(ctx):
    candidates = [ctx.getenv("ROCM_PATH"), ctx.getenv("ROCM_HOME"), "/opt/rocm"]
    for candidate in candidates:
        if candidate and ctx.path(candidate + "/include/hip/hip_runtime.h").exists:
            return candidate
    return None

def _build_from_source(ctx):
    rocm = _rocm_root(ctx)
    if not rocm:
        return "ROCm (include/hip/hip_runtime.h) not found; install ROCm or set ROCM_PATH"
    for tool in ["cmake", "ninja", "python3"]:
        if not ctx.which(tool):
            return "{} not found on PATH".format(tool)

    ctx.download_and_extract(
        url = ctx.attr.source_url,
        sha256 = ctx.attr.source_sha256,
        type = "tar.gz",
        output = "src",
        strip_prefix = ctx.attr.source_prefix,
    )
    source = str(ctx.path("src"))
    build = str(ctx.path("build"))
    environment = {"ROCM_PATH": rocm, "HIP_PATH": rocm}

    configure = ctx.execute(
        [
            "python3",
            source + "/buildbot/configure.py",
            "--hip",
            "--hip-platform",
            "AMD",
            "--src-dir",
            source,
            "--obj-dir",
            build,
            "--cmake-opt=-DUR_HIP_ROCM_DIR=" + rocm,
        ],
        environment = environment,
        timeout = 3600,
    )
    if configure.return_code != 0:
        return "configure failed: " + configure.stderr[-2000:]

    compile = ctx.execute(
        ["python3", source + "/buildbot/compile.py", "--obj-dir", build],
        environment = environment,
        timeout = 21600,
    )
    if compile.return_code != 0:
        return "compile failed: " + compile.stderr[-2000:]

    for name in ["bin", "include", "lib"]:
        ctx.symlink(build + "/install/" + name, name)
    return None

def _impl(ctx):
    roots = search_roots(
        ctx,
        ["ATEN_XPU_SYCL_HIP_HOME"],
        ["/opt/sycl-hip", "/opt/sycl", "/usr/local"],
        ["/opt/intel/oneapi/compiler"],
    )
    root = first_root(
        ctx,
        roots,
        ["include/sycl/sycl.hpp", "lib/libsycl.so", "lib/libur_adapter_hip.so"],
        ["bin/icpx", "bin/clang++"],
    )
    if root:
        link_existing(ctx, root, ["bin", "include", "lib"])
        ctx.file("BUILD.bazel", _BUILD)
        write_status(ctx, "sycl_hip", "system", root)
        return

    if ctx.os.name == "linux":
        error = _build_from_source(ctx)
        if not error:
            ctx.file("BUILD.bazel", _BUILD)
            write_status(ctx, "sycl_hip", "build", ctx.attr.source_url)
            return
    else:
        error = "HIP builds are Linux only"

    missing(
        ctx,
        "sycl_hip",
        'package(default_visibility = ["//visibility:public"])\ncc_library(name = "sycl")\nfilegroup(name = "compiler")\n',
        "{}; or build DPC++ with --hip yourself and set ATEN_XPU_SYCL_HIP_HOME".format(error),
    )

sycl_hip_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        "source_url": attr.string(),
        "source_sha256": attr.string(),
        "source_prefix": attr.string(),
    },
    environ = ["ATEN_XPU_SYCL_HIP_HOME", "ROCM_PATH", "ROCM_HOME", "PATH"],
    local = True,
)

def repo():
    sycl_hip_sdk(
        name = "sycl_hip",
        source_url = "https://github.com/intel/llvm/archive/refs/tags/v7.2.0.tar.gz",
        source_sha256 = "ac68497d26a975caa05d23128f392dce7bbf29f4322831fcd81949f159b11418",
        source_prefix = "llvm-7.2.0",
    )
