"""NVIDIA CUPTI headers (plus the CUDA driver and crt headers they include) as a Bazel repository.

Only headers are needed at build time; libcupti.so is dlopen'ed at run time.
Search order: ATEN_XPU_CUPTI_HOME, CUDA_HOME, CUDA_PATH, /usr/local/cuda*, /opt/cuda, /usr.
Download: the pinned nvidia-cuda-{cupti,runtime,nvcc}-cu12 wheels from PyPI (x86_64 and aarch64).
"""

load(":discover.bzl", "first_root", "host_arch", "link_existing", "missing", "search_roots", "write_status")

# toolkit layout: <root>/extras/CUPTI/include; distro layout: <root>/include
_TOOLKIT_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob(["extras/CUPTI/include/**", "include/cuda*.h", "include/crt/**", "include/vector_types.h", "include/driver_types.h", "include/builtin_types.h", "include/device_types.h", "include/surface_types.h", "include/texture_types.h", "include/library_types.h", "include/cuda_stdint.h"], allow_empty = True),
    includes = ["extras/CUPTI/include", "include", "include/crt"],
)
"""

_DISTRO_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob(["include/cupti*.h", "include/cuda*.h", "include/crt/**", "include/vector_types.h", "include/driver_types.h", "include/builtin_types.h", "include/device_types.h", "include/surface_types.h", "include/texture_types.h", "include/library_types.h", "include/cuda_stdint.h", "include/Openacc/**", "include/Openmp/**", "include/generated_*.h", "include/nvperf*.h"], allow_empty = True),
    includes = ["include", "include/crt"],
)
"""

_WHEEL_BUILD = """
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob([
        "cupti/nvidia/cuda_cupti/include/**",
        "runtime/nvidia/cuda_runtime/include/**",
        "nvcc/nvidia/cuda_nvcc/include/crt/**",
    ], allow_empty = True),
    includes = [
        "cupti/nvidia/cuda_cupti/include",
        "runtime/nvidia/cuda_runtime/include",
        "nvcc/nvidia/cuda_nvcc/include",
        "nvcc/nvidia/cuda_nvcc/include/crt",
    ],
)
"""

def _impl(ctx):
    roots = search_roots(
        ctx,
        ["ATEN_XPU_CUPTI_HOME", "CUDA_HOME", "CUDA_PATH"],
        ["/usr/local/cuda", "/opt/cuda"],
        ["/usr/local", "/opt"],
    ) + ["/usr"]
    root = first_root(ctx, roots, ["extras/CUPTI/include/cupti.h", "include/cuda.h", "include/crt/host_defines.h"])
    if root:
        link_existing(ctx, root, ["extras", "include"])
        ctx.file("BUILD.bazel", _TOOLKIT_BUILD)
        write_status(ctx, "cupti", "system", root)
        return

    root = first_root(ctx, roots, ["include/cupti.h", "include/cuda.h", "include/crt/host_defines.h"])
    if root:
        link_existing(ctx, root, ["include"])
        ctx.file("BUILD.bazel", _DISTRO_BUILD)
        write_status(ctx, "cupti", "system", root)
        return

    wheels = ctx.attr.wheels.get(host_arch(ctx))
    if not wheels or ctx.os.name != "linux":
        missing(
            ctx,
            "cupti",
            'package(default_visibility = ["//visibility:public"])\ncc_library(name = "headers")\n',
            "install the CUDA toolkit and set ATEN_XPU_CUPTI_HOME (or CUDA_HOME)",
        )
        return

    for directory, spec in zip(["cupti", "runtime", "nvcc"], wheels):
        url, sha256 = spec.split(" ")
        ctx.download_and_extract(url = url, sha256 = sha256, type = "zip", output = directory)
    ctx.file("BUILD.bazel", _WHEEL_BUILD)
    write_status(ctx, "cupti", "download", "pinned PyPI wheels")

cupti_sdk = repository_rule(
    implementation = _impl,
    attrs = {
        # arch -> ["<url> <sha256>" for cupti, runtime, nvcc]
        "wheels": attr.string_list_dict(),
    },
    environ = ["ATEN_XPU_CUPTI_HOME", "CUDA_HOME", "CUDA_PATH"],
    local = True,
)

def repo():
    cupti_sdk(
        name = "cupti",
        wheels = {
            "x86_64": [
                "https://files.pythonhosted.org/packages/c1/2e/b84e32197e33f39907b455b83395a017e697c07a449a2b15fd07fc1c9981/nvidia_cuda_cupti_cu12-12.9.79-py3-none-manylinux_2_25_x86_64.whl 096bcf334f13e1984ba36685ad4c1d6347db214de03dbb6eebb237b41d9d934f",
                "https://files.pythonhosted.org/packages/bc/46/a92db19b8309581092a3add7e6fceb4c301a3fd233969856a8cbf042cd3c/nvidia_cuda_runtime_cu12-12.9.79-py3-none-manylinux2014_x86_64.manylinux_2_17_x86_64.whl 25bba2dfb01d48a9b59ca474a1ac43c6ebf7011f1b0b8cc44f54eb6ac48a96c3",
                "https://files.pythonhosted.org/packages/25/48/b54a06168a2190572a312bfe4ce443687773eb61367ced31e064953dd2f7/nvidia_cuda_nvcc_cu12-12.9.86-py3-none-manylinux2010_x86_64.manylinux_2_12_x86_64.whl 5d6a0d32fdc7ea39917c20065614ae93add6f577d840233237ff08e9a38f58f0",
            ],
            "aarch64": [
                "https://files.pythonhosted.org/packages/b4/78/351b5c8cdbd9a6b4fb0d6ee73fb176dcdc1b6b6ad47c2ffff5ae8ca4a1f7/nvidia_cuda_cupti_cu12-12.9.79-py3-none-manylinux_2_25_aarch64.whl 791853b030602c6a11d08b5578edfb957cadea06e9d3b26adbf8d036135a4afe",
                "https://files.pythonhosted.org/packages/bc/e0/0279bd94539fda525e0c8538db29b72a5a8495b0c12173113471d28bce78/nvidia_cuda_runtime_cu12-12.9.79-py3-none-manylinux2014_aarch64.manylinux_2_17_aarch64.whl 83469a846206f2a733db0c42e223589ab62fd2fabac4432d2f8802de4bded0a4",
                "https://files.pythonhosted.org/packages/d6/5c/8cc072436787104bbbcbde1f76ab4a0d89e68f7cebc758dd2ad7913a43d0/nvidia_cuda_nvcc_cu12-12.9.86-py3-none-manylinux2014_aarch64.manylinux_2_17_aarch64.whl 44e1eca4d08926193a558d2434b1bf83d57b4d5743e0c431c0c83d51da1df62b",
            ],
        },
    )
