"""oneDNN from source, one repository per GPU vendor (@onednn_intel, @onednn_nvidia, @onednn_amd, @onednn_generic).

oneDNN fixes the GPU vendor at build time, so "all cards" means one repository per vendor. rules_foreign_cc runs CMake
with ninja and the DPC++ compiler of @sycl (@sycl_hip for AMD), as one action. The static libdnnl.a links into the
plugins; ninja runs 6 jobs, as the C++ compiles do (.bazelrc local_resources). NVIDIA and AMD find CUDA (cuDNN, cuBLAS) or ROCm (HIP, rocBLAS, MIOpen) through `--action_env=CUDA_HOME` or
`--action_env=ROCM_PATH`; AMD also takes `-DDNNL_AMD_SYCL_KERNELS_TARGET_ARCH=gfx90a` in `cache_entries` for the generic kernels.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

_BUILD = """
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(name = "src", srcs = glob(["**"], exclude = ["tests/**", "doc/**", "examples/**"]))

cmake(
    name = "onednn",
    lib_source = ":src",
    generate_args = ["-GNinja"],
    build_args = ["-j", "6"],
    generate_crosstool_file = False,
    cache_entries = {entries},
    env = {{
        "LD_LIBRARY_PATH": "$$EXT_BUILD_ROOT$$/external/{compiler}/lib",
        # The C++ toolchain's flags (-fsycl-is-host, ...) clash with oneDNN's own -fsycl; it sets its flags itself.
        "CFLAGS": "",
        "CXXFLAGS": "",
        "ASMFLAGS": "",
    }},
    build_data = ["@sycl//:toolchain_files"],
    out_static_libs = ["libdnnl.a"],
    linkopts = ["-ldl", "-lpthread", "-ltbb"],
    deps = ["@sycl//:sycl"],
    visibility = ["//visibility:public"],
)
"""

# vendor: (compiler repository, extra cache entries). PyTorch's XPU set plus the CPU engine on SYCL for tests.
_VENDORS = {
    "INTEL": ("sycl", {}),
    "NVIDIA": ("sycl", {}),
    "AMD": ("sycl_hip", {"ONEDNN_BUILD_GRAPH": "OFF"}),
    "GENERIC": ("sycl", {"ONEDNN_BUILD_GRAPH": "OFF"}),
}

def repo(launcher = ""):
    """launcher: path of sccache (or ccache); like PyTorch, it caches every oneDNN object so a flag change does not recompile all."""
    for vendor, (compiler, extra) in _VENDORS.items():
        root = "$$EXT_BUILD_ROOT$$/external/" + Label("@" + compiler + "//:toolchain_files").repo_name
        entries = {
            "CMAKE_BUILD_TYPE": "Release",
            "CMAKE_C_COMPILER": root + "/bin/clang",
            "CMAKE_CXX_COMPILER": root + "/bin/clang++",
            "CMAKE_POSITION_INDEPENDENT_CODE": "ON",
            "DNNL_CPU_RUNTIME": "SYCL",
            "DNNL_GPU_RUNTIME": "SYCL",
            "DNNL_GPU_VENDOR": vendor,
            "DNNL_LIBRARY_TYPE": "STATIC",
            "DNNL_DPCPP_HOST_COMPILER": "g++",
            "DNNL_BUILD_TESTS": "OFF",
            "DNNL_BUILD_EXAMPLES": "OFF",
            "DNNL_BUILD_DOC": "OFF",
        }
        entries.update(extra)
        if launcher:
            entries["CMAKE_C_COMPILER_LAUNCHER"] = launcher
            entries["CMAKE_CXX_COMPILER_LAUNCHER"] = launcher
        http_archive(
            name = "onednn_" + vendor.lower(),
            url = "https://github.com/uxlfoundation/oneDNN/archive/refs/tags/v3.14.2.tar.gz",
            sha256 = "ed509c84b9d73bacabff00e6aaedbde570568e26dedb129030fee9c8c196f168",
            strip_prefix = "oneDNN-3.14.2",
            build_file_content = _BUILD.format(entries = entries, compiler = Label("@" + compiler + "//:toolchain_files").repo_name),
        )
