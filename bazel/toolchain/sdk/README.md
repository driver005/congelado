# Vendor SDK repositories

Each repository tries two routes in order: search for an existing install, else download a pinned archive. `@<name>//:STATUS.md` records which route ran.

| Repo | Search env vars | Fixed search paths | Download (pinned sha256) |
| --- | --- | --- | --- |
| `@sycl` | `ATEN_XPU_SYCL_HOME`, `CMPLR_ROOT`, `ONEAPI_ROOT` | `/opt/intel/oneapi/compiler/*` | DPC++ release `sycl_linux.tar.gz`, x86_64 only |
| `@pti` | `ATEN_XPU_PTI_HOME`, `PTI_ROOT` | `/opt/intel/oneapi/pti` | pti-gpu `pti-1.2.0` headers, `pti_export.h` and `pti_version.h` generated |
| `@cupti` | `ATEN_XPU_CUPTI_HOME`, `CUDA_HOME`, `CUDA_PATH` | `/usr/local/cuda*`, `/opt/cuda`, `/usr` | PyPI wheels `cupti`, `runtime`, `nvcc` (x86_64 and aarch64) |
| `@rocprofiler` | `ATEN_XPU_ROCPROFILER_HOME`, `ROCM_PATH`, `ROCM_HOME` | `/opt/rocm`, `/opt/rocm-*` | ROCm 7.2.4 debs: rocprofiler-sdk, roctx, hsa-rocr-dev, hip-dev (x86_64 only) |

## Install first

The downloads cover headers and the DPC++ compiler. Drivers, vendor math libraries and build tools come from your package manager. Install only the vendors you target.

Arch, copy and paste (pick the lines for your vendors):

```bash
# build tools, always
sudo pacman -S --needed cmake ninja python

# Intel GPU (libpti comes from the oneAPI repo, not pacman)
sudo pacman -S --needed intel-compute-runtime level-zero-loader level-zero-headers

# NVIDIA GPU (the driver itself: nvidia or nvidia-dkms)
sudo pacman -S --needed nvidia-utils cuda cudnn

# AMD GPU
sudo pacman -S --needed rocm-hip-sdk rocblas miopen-hip rocprofiler
```

Always (any build-from-source fallback):

| Need | Arch | Debian / Ubuntu |
| --- | --- | --- |
| cmake, ninja, python3 | `cmake ninja python` | `cmake ninja-build python3` |

Intel GPU (`@sycl`, `@pti`, `@onednn_intel`):

| Need | Arch | Debian / Ubuntu |
| --- | --- | --- |
| GPU compute runtime | `intel-compute-runtime` | `intel-opencl-icd libze-intel-gpu1` |
| Level Zero loader and headers | `level-zero-loader level-zero-headers` | `libze1 libze-dev` |
| PTI runtime (`libpti`, profiler only) | oneAPI `intel-pti` | oneAPI apt repo `intel-pti` |

NVIDIA GPU (`@cupti`, `@onednn_nvidia`):

| Need | Arch | Debian / Ubuntu |
| --- | --- | --- |
| Driver with CUDA support | `nvidia-utils` (plus `nvidia` or `nvidia-dkms`) | `nvidia-driver-<version>` |
| CUDA toolkit (`cuBLAS`, `cupti`) | `cuda` | `cuda-toolkit-<version>` (NVIDIA repo) |
| cuDNN (headers and `libcudnn`) | `cudnn` | `libcudnn9-dev-cuda-<version>` (NVIDIA repo) |

AMD GPU (`@sycl_hip`, `@rocprofiler`, `@onednn_amd`):

| Need | Arch | Debian / Ubuntu |
| --- | --- | --- |
| ROCm with HIP | `rocm-hip-sdk` (pulls in `rocblas`, `miopen-hip`, `hip-runtime-amd`) | `rocm-hip-sdk` (AMD ROCm apt repo) |
| rocBLAS | `rocblas` | `rocblas-dev` |
| MIOpen | `miopen-hip` | `miopen-hip-dev` |
| rocprofiler-sdk (profiler only) | `rocprofiler` | `rocprofiler-sdk` |

- Arch names were checked against the sync database. Debian and Ubuntu names come from the vendor repositories and were not checked; confirm them with `apt search`.
- ROCm and CUDA install under `/opt/rocm` and `/opt/cuda` or `/usr/local/cuda`, which the search route already looks in. Anything elsewhere needs `ROCM_PATH` or `CUDA_HOME`.
- A machine that only runs the plugin needs the driver and vendor runtime libraries, not cmake or the dev packages.

## SYCL with HIP and oneDNN per vendor

| Repo | Search env vars | Fallback |
| --- | --- | --- |
| `@sycl_hip` | `ATEN_XPU_SYCL_HIP_HOME` (needs `lib/libur_adapter_hip.so`) | build intel/llvm v7.2.0 with `--hip` (about 1 hour, needs ROCm, cmake, ninja) |
| `@onednn_intel` | `ATEN_XPU_ONEDNN_INTEL_HOME` | build oneDNN v3.14.2 with `@sycl` |
| `@onednn_nvidia` | `ATEN_XPU_ONEDNN_NVIDIA_HOME` | build with `@sycl`; needs `CUDA_HOME` with cuDNN and cuBLAS |
| `@onednn_generic` | `ATEN_XPU_ONEDNN_GENERIC_HOME` | build with `@sycl`; plain SYCL kernels for any SYCL device, slower and fewer ops, no vendor libraries needed |
| `@onednn_amd` | `ATEN_XPU_ONEDNN_AMD_HOME` | build with `@sycl_hip`; needs ROCm with HIP, rocBLAS, MIOpen; `ATEN_XPU_AMD_ARCH=gfx90a` also enables the generic SYCL kernels |

- oneDNN fixes the GPU vendor at build time, so each vendor is a separate library.
- The prebuilt Intel DPC++ tarball has the Level Zero and CUDA adapters only; AMD needs `@sycl_hip`.
- AdaptiveCpp cannot compile oneDNN, so this route keeps DPC++ as the compiler.

Notes:
- Repos are lazy. A failed lookup only breaks targets that reference the repo.
- No install and no download for the host arch yields an empty repo plus a DEBUG message, not a build-wide failure.
- Vendor profiler libraries are `dlopen`ed at runtime, so only headers are needed to build.
- `@rocprofiler` consumers must pass `-D__HIP_PLATFORM_AMD__`; the custom toolchain drops `cc_library.defines`.
- Plugin `.so` targets need `--force_pic`.
- Force the download route by pointing the env var at a missing directory, or force a specific install with `--repo_env=ATEN_XPU_<NAME>_HOME=/path`.
