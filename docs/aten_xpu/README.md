# ATen XPU — SYCL ice::builder Backend

> **Status:** Reference implementation — `docs/` only, not compiled by Bazel.
> **Replaces:** Old ATen/c10-dependent async_support layer.

This directory holds a reference extraction of PyTorch's XPU/SYCL backend
**plus** a fully rewritten `async_support/` sub-layer that implements the
Congelado **ice** layer interfaces using pure [AdaptiveCpp SYCL](https://github.com/AdaptiveCpp/AdaptiveCpp).

Zero PyTorch / ATen / c10 dependencies in `async_support/`.

---

## Directory Structure

```
aten_xpu/
├── core/                    # ATen/xpu/ — reference: Core tensor context
├── mkldnn/                  # ATen/native/mkldnn/xpu/ — reference: oneDNN kernels
├── transformers/            # ATen/native/transformers/xpu/ — reference: attention
├── c10/                     # c10/xpu/ — reference: c10 XPU primitives
├── torch_csrc/              # torch/csrc/xpu/ — reference: Python bindings
├── async_support/           # ← REWRITTEN: ice::builder SYCL backend
│   ├── xpu_async_tensor.cppm   # ice::builder::TF_TensorOps via SYCL USM
│   ├── xpu_async_buffer.cppm   # Async USM buffer (alloc/copy/memset)
│   ├── xpu_async_kernels.cppm  # ice::builder::TF_ExecutorOps via sycl::queue
│   └── xpu_async.cppm          # Unified re-export facade
└── xpu_dispatch_keys.txt    # 1178 XPU dispatch entries from native_functions.yaml
```

---

## ice Layer Mapping

| SYCL primitive | ice::builder interface |
|---|---|
| `sycl::malloc_device` | `TF_ExecutorOps::allocate` |
| `sycl::malloc_host` | `TF_ExecutorOps::host_memory_allocate` |
| `sycl::malloc_shared` | `TF_ExecutorOps::unified_memory_allocate` |
| `sycl::free` | `TF_ExecutorOps::deallocate` / `host_memory_deallocate` |
| `sycl::queue` (in-order) | `TF_Stream` (`plugin_data = sycl::queue*`) |
| `sycl::queue::memcpy` | `TF_ExecutorOps::memcpy_dtoh/htod/dtod` |
| `sycl::queue::memset` | `TF_ExecutorOps::mem_zero` / `memset` |
| `sycl::queue::parallel_for` | `TF_ExecutorOps::memset32` |
| `sycl::queue::host_task` | `TF_ExecutorOps::host_callback` |
| `sycl::queue::ext_oneapi_submit_barrier` | `TF_ExecutorOps::record_event` |
| `sycl::event` | `TF_Event` (`plugin_data = sycl::event*`) |
| `sycl::device::global_mem_size` | `TF_ExecutorOps::device_memory_usage` |
| `SyclTensorImpl` (USM + shape + dtype) | `TF_TensorOps` vtable |

---

## Module Names

| Old (ATen-based) | New (ice/SYCL) |
|---|---|
| `cc_ice_builder_intern:xpu_async_tensor` | `cc_ice_sycl_backend:tensor` |
| `cc_ice_builder_intern:xpu_async_buffer` | `cc_ice_sycl_backend:buffer` |
| `cc_ice_builder_intern:xpu_async_kernels` | `cc_ice_sycl_backend:executor` |
| `cc_ice_builder_intern:xpu_async` | `cc_ice_sycl_backend` |

---

## async_support/ File Guide

### `xpu_async_tensor.cppm` — `cc_ice_sycl_backend:tensor`

Implements `ice::builder::TF_TensorOps` using:
- `SyclTensorImpl`: owns `sycl::malloc_device` USM pointer + `std::vector<int64_t>` dims + `TFDataTypeEnum` dtype.
- `XPU_AsyncTensorOps`: abstract class impl; all virtual methods delegate to `SyclTensorImpl`.
- `XPU_AsyncEvent`: SYCL event wrapper for cross-op synchronization.
- `AsyncResult`: `{ sycl::event, std::expected<void, ice::Status> }`.

```cpp
import cc_ice_sycl_backend:tensor;

sycl::queue q(sycl::gpu_selector_v);
auto* t = ice::builder::XPU_AsyncTensorOps::make(q);

int64_t dims[] = {4, 32, 32};
t->set_dtype(TF_FLOAT);
t->set_dims(dims, 3);   // allocates 4×32×32×4 bytes on GPU

auto r = t->copy_from_host_async(host_ptr, t->impl_.nbytes());
r.wait();               // blocks until DMA complete
```

### `xpu_async_buffer.cppm` — `cc_ice_sycl_backend:buffer`

Raw USM buffer with async copy/memset:
- `XPU_AsyncBuffer` — single allocation, same async API shape as old `XPU_AsyncBuffer` but backed by `sycl::malloc_device` / `sycl::free` instead of `c10::DataPtr`.
- `XPU_PooledAsyncBuffer` — same API, delegates to inner `XPU_AsyncBuffer` (SYCL has no native pool concept; pooling lives at plugin level).

### `xpu_async_kernels.cppm` — `cc_ice_sycl_backend:executor`

Implements the full `ice::builder::TF_ExecutorOps` abstract interface as `SyclExecutorImpl`:

```cpp
import cc_ice_sycl_backend:executor;

// GPU executor (falls back to CPU if no GPU)
auto* exec = ice::builder::make_gpu_executor();

// Use default queue directly
sycl::queue& q = exec->default_queue();

// Or go through ice C-ABI
TF_Stream stream;
exec->create_stream_internal(device_ops, &stream);
exec->memcpy_htod(device_ops, &stream, &mem, host_ptr, 1024);
exec->block_host_until_done(device_ops, &stream);
exec->destroy_stream_internal(device_ops, &stream);

delete exec;
```

### `xpu_async.cppm` — `cc_ice_sycl_backend`

Unified re-export facade. `import cc_ice_sycl_backend;` gives access to all
three partitions plus convenience aliases:

```cpp
using SyclTensor      = ice::builder::XPU_AsyncTensorOps;
using SyclBuffer      = ice::builder::XPU_AsyncBuffer;
using SyclPooledBuffer= ice::builder::XPU_PooledAsyncBuffer;
using SyclExecutor    = ice::builder::SyclExecutorImpl;
using SyclAsyncEvent  = ice::builder::XPU_AsyncEvent;
using SyclAsyncResult = ice::builder::AsyncResult;
```

---

## Building with Bazel (Approach A — per-plugin explicit deps)

In `include/yoshi/omah_lay/` (production plugin location):

```python
cc_binary(
    name = "sycl_plugin.so",
    srcs = [
        "sycl_executor_plugin.cc",   # your plugin entry-point
    ],
    deps = [
        "//include/yoshi/omah_lay:plugin_c_abi_base",
        "//include/yoshi/omah_lay:gpu_plugin_deps",   # @adaptive_cpp//:sycl
        "//include/cc/ice/extern/stream_executor:builder_executor",
        "//include/cc/ice/intern:builder_tensor",
        "//include/cc/ice/intern:builder_status",
    ],
    # docs/aten_xpu/async_support/*.cppm are NOT in Bazel — reference only.
    # Copy the implementation to yoshi/omah_lay/ before wiring into Bazel.
)
```

> [!NOTE]
> `docs/aten_xpu/async_support/` is **never** compiled by Bazel directly.
> It serves as the reference/design doc for the real plugin in `include/yoshi/omah_lay/`.

---

## TF_Stream / TF_Event Plugin Data Convention

```
TF_Stream::plugin_data  →  heap sycl::queue*   (owned; destroyed by destroy_stream_internal)
TF_Event::plugin_data   →  heap sycl::event*   (owned; destroyed by destroy_event_internal)
TF_Timer::plugin_data   →  heap std::pair<sycl::event,sycl::event>*  (start, stop)
TF_DeviceMemoryBase::opaque → raw USM device ptr (sycl::malloc_device output)
```

---

## SYCL Requirements

- **AdaptiveCpp** (≥ v25.10.0) — `@adaptive_cpp//:sycl` in Bazel
- CPU target: `sycl::cpu_selector_v` — always available
- GPU target: `sycl::gpu_selector_v` — requires Intel GPU + Level Zero, or ROCm/CUDA via AdaptiveCpp
- `<sycl/sycl.hpp>` must be included in the **global module fragment** (`module;` block), never inside `export module` — SYCL cannot be a C++ module interface

---

## Reference Files (ATen — not used by ice layer)

The remaining subdirectories (`core/`, `mkldnn/`, `transformers/`, `c10/`, `torch_csrc/`) are read-only reference material extracted from PyTorch for architectural study. They depend on ATen/c10 and are **not** integrated.

- Extracted from: `pytorch/pytorch@main` (shallow clone, 2026-09-18)
- `xpu_dispatch_keys.txt` — 1178 operators with `XPU:` dispatch key