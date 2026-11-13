"""oneDNN as a Bazel repository, one repository per GPU vendor: found on the host, else built from source.

oneDNN fixes the GPU vendor at build time (DNNL_GPU_VENDOR is INTEL, NVIDIA, AMD or GENERIC), so "all cards"
means one repository per vendor: @onednn_intel, @onednn_nvidia, @onednn_amd, @onednn_generic.
Search order: ATEN_XPU_ONEDNN_<VENDOR>_HOME (for example ATEN_XPU_ONEDNN_AMD_HOME), /opt/onednn-<vendor>.
A root counts only if its dnnl_config.h was configured for the same vendor and libdnnl exists.
Build: the pinned release tarball with the matching DPC++ repository (@sycl, or @sycl_hip for AMD) as compiler.
The repository only downloads the source; onednn_build.bzl compiles it as a Bazel action (cached, shown as "Compiling
oneDNN", ninja status in /tmp/aten_xpu_onednn_<vendor>.log).
 - INTEL: needs nothing beyond the compiler.
 - NVIDIA: needs a CUDA toolkit with cuDNN and cuBLAS (CUDA_HOME).
 - AMD and GENERIC build without the Graph API (ONEDNN_BUILD_GRAPH=OFF): no fused-attention path there.
 - GENERIC: plain SYCL kernels for any SYCL device (slower, fewer ops); needs nothing beyond the compiler.
 - AMD: needs ROCm with HIP, rocBLAS and MIOpen (ROCM_PATH or /opt/rocm); ATEN_XPU_AMD_ARCH (for example gfx90a)
   additionally enables oneDNN's generic SYCL kernels on AMD.
"""

load(":discover.bzl", "missing", "write_status")

_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "onednn",
    hdrs = glob(["include/**"], allow_empty = True),
    includes = ["include"],
    srcs = glob(["lib/libdnnl.so"], allow_empty = True),
)
"""

_EMPTY_BUILD = 'package(default_visibility = ["//visibility:public"])\ncc_library(name = "onednn")\n'

def _env_name(vendor):
    return "ATEN_XPU_ONEDNN_{}_HOME".format(vendor)

def _vendor_matches(ctx, root, vendor):
    config = ctx.path(root + "/include/oneapi/dnnl/dnnl_config.h")
    if not config.exists or not ctx.path(root + "/lib/libdnnl.so").exists:
        return False
    return "#define DNNL_GPU_VENDOR DNNL_VENDOR_" + vendor in ctx.read(config)

def _vendor_library_root(ctx, vendor):
    """The vendor math-library install the build links against, or an error string."""
    if vendor == "AMD":
        for candidate in [ctx.getenv("ROCM_PATH"), ctx.getenv("ROCM_HOME"), "/opt/rocm"]:
            if candidate and ctx.path(candidate + "/lib/libMIOpen.so").exists and ctx.path(candidate + "/lib/librocblas.so").exists:
                return candidate, None
        return None, "ROCm with HIP, rocBLAS and MIOpen not found; install them or set ROCM_PATH"
    if vendor == "NVIDIA":
        for candidate in [ctx.getenv("CUDA_HOME"), ctx.getenv("CUDA_PATH"), "/usr/local/cuda", "/opt/cuda"]:
            if candidate and ctx.path(candidate + "/include/cudnn.h").exists and ctx.path(candidate + "/include/cublas_v2.h").exists:
                return candidate, None
        return None, "CUDA toolkit with cuDNN and cuBLAS not found; install them or set CUDA_HOME"
    return "", None

def _source_build_file(ctx, vendor, arguments):
    """BUILD file whose onednn target compiles the downloaded source as a Bazel action."""
    return """
load("{rule}", "onednn_cmake")

package(default_visibility = ["//visibility:public"])

onednn_cmake(
    name = "onednn",
    vendor = "{vendor}",
    cmakelists = "src/CMakeLists.txt",
    sources = glob(["src/**"], exclude = ["src/tests/**", "src/doc/**", "src/examples/**"]),
    compiler = "{compiler}",
    cmake_args = {arguments},
)
""".format(
        rule = str(Label("//third_party/toolchain/sdk:onednn_build.bzl")),
        vendor = vendor,
        compiler = "@@{}//:bin/clang++".format(ctx.attr.compiler.repo_name),
        arguments = repr(arguments),
    )

def _prepare_source_build(ctx):
    """Downloads the source and returns the BUILD file text, or an error string."""
    vendor = ctx.attr.vendor
    compiler_root = str(ctx.path(ctx.attr.compiler).dirname)
    if not ctx.path(compiler_root + "/bin/clang++").exists:
        return None, "no DPC++ compiler in " + compiler_root + " (the matching SYCL repository is empty)"
    for tool in ["cmake", "ninja"]:
        if not ctx.which(tool):
            return None, "{} not found on PATH".format(tool)

    library_root, error = _vendor_library_root(ctx, vendor)
    if error:
        return None, error

    ctx.report_progress("downloading oneDNN source")
    ctx.download_and_extract(
        url = ctx.attr.url,
        sha256 = ctx.attr.sha256,
        type = "tar.gz",
        output = "src",
        strip_prefix = ctx.attr.strip_prefix,
    )

    # The compiler, source directory and install prefix are added by the build action.
    arguments = [
        "-DDNNL_CPU_RUNTIME=SYCL",
        "-DDNNL_GPU_RUNTIME=SYCL",
        "-DDNNL_GPU_VENDOR=" + vendor,
        "-DDNNL_BUILD_TESTS=OFF",
        "-DDNNL_BUILD_EXAMPLES=OFF",
        "-DDNNL_BUILD_DOC=OFF",
    ]
    if library_root:
        arguments.append("-DCMAKE_PREFIX_PATH=" + library_root)
    if vendor in ("AMD", "GENERIC"):
        # The Graph API (fused attention) supports only the INTEL and NVIDIA vendors.
        arguments.append("-DONEDNN_BUILD_GRAPH=OFF")
    arch = ctx.getenv("ATEN_XPU_AMD_ARCH")
    if vendor == "AMD" and arch:
        arguments.append("-DDNNL_AMD_SYCL_KERNELS_TARGET_ARCH=" + arch)
    return _source_build_file(ctx, vendor, arguments), None

def _impl(ctx):
    vendor = ctx.attr.vendor
    for root in [ctx.getenv(_env_name(vendor)), "/opt/onednn-" + vendor.lower()]:
        if root and _vendor_matches(ctx, root, vendor):
            for name in ["include", "lib"]:
                ctx.symlink(root + "/" + name, name)
            ctx.file("BUILD.bazel", _BUILD)
            write_status(ctx, "onednn_" + vendor.lower(), "system", root)
            return

    build_file, error = (None, "oneDNN SYCL builds are Linux only here")
    if ctx.os.name == "linux":
        build_file, error = _prepare_source_build(ctx)
    if not error:
        ctx.file("BUILD.bazel", build_file)
        write_status(ctx, "onednn_" + vendor.lower(), "build", ctx.attr.url)
        return

    missing(
        ctx,
        "onednn_" + vendor.lower(),
        _EMPTY_BUILD,
        "{}; or build oneDNN with DNNL_GPU_VENDOR={} and set {}".format(error, vendor, _env_name(vendor)),
    )

onednn_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        "vendor": attr.string(mandatory = True, values = ["INTEL", "NVIDIA", "AMD", "GENERIC"]),
        "compiler": attr.label(mandatory = True, allow_single_file = True),
        "url": attr.string(),
        "sha256": attr.string(),
        "strip_prefix": attr.string(),
    },
    environ = [
        "ATEN_XPU_ONEDNN_INTEL_HOME",
        "ATEN_XPU_ONEDNN_NVIDIA_HOME",
        "ATEN_XPU_ONEDNN_AMD_HOME",
        "ATEN_XPU_ONEDNN_GENERIC_HOME",
        "ATEN_XPU_AMD_ARCH",
        "ROCM_PATH",
        "ROCM_HOME",
        "CUDA_HOME",
        "CUDA_PATH",
        "PATH",
    ],
    local = True,
)

def repo():
    for vendor, compiler in [("INTEL", "@sycl//:BUILD.bazel"), ("NVIDIA", "@sycl//:BUILD.bazel"), ("AMD", "@sycl_hip//:BUILD.bazel"), ("GENERIC", "@sycl//:BUILD.bazel")]:
        onednn_sdk(
            name = "onednn_" + vendor.lower(),
            vendor = vendor,
            compiler = compiler,
            url = "https://github.com/uxlfoundation/oneDNN/archive/refs/tags/v3.14.2.tar.gz",
            sha256 = "ed509c84b9d73bacabff00e6aaedbde570568e26dedb129030fee9c8c196f168",
            strip_prefix = "oneDNN-3.14.2",
        )
