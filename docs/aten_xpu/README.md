# aten_xpu: SYCL backend on the ice layer

Reference SYCL/oneDNN plugin that implements the generated `ice::builder::TF_*Ops` interfaces.
It lives under `docs/` and Bazel does not build it.

## Layout

The tree mirrors `include/cc/ice`. Each directory is one C++ module, and each file is one partition holding one class.

```
docs/aten_xpu/
├── base.cppm                 aten_xpu (re-exports intern + extern)
├── plugin.cc                 C entry points only (create_plugin, create_*/destroy_*)
├── intern/                   aten_xpu_intern
│   ops_table, handle, status, tensor, tensor_factory
├── extern/                   aten_xpu_extern
│   ├── stream_executor/      platform, device, device_guard, peer_access, level_zero,
│   │                         executor, stream, stream_pool, event, timer, memory, mem_pool,
│   │                         stream_executor, allocator/{allocator, block, block_pool,
│   │                         expandable_segment, stats, trace, snapshot, ipc_memory,
│   │                         host_cache, pluggable_allocator}
│   ├── random_generator/     random_generator, philox_state, philox_codec
│   ├── grappler/             grappler, device_graph, capture_status, optimizer
│   ├── kernel/               kernel (SyclKernel<Derived> : TF_KernelOps), registrar,
│   │                         context/, onednn/, matmul/, linear/, convolution/,
│   │                         quantized/, scaled/, attention/, recurrent/
│   └── registration/         SyclPluginRegistry (vtables, host ops, kernel registration)
└── test/                     gtest suites on the ice::sonic wrappers (fixture.cppm)
```

## Lifecycle

- The host calls a vtable's `create` slot. The plugin allocates the concrete `Sycl*` object and `SyclHandle::attach` stores it in `plugin_data`. `destroy` deletes the object.
- `create_*_internal(parent, handle)` binds an object that already exists (for example `SyclStream::bind`). Generated wrappers copy the `{plugin_data}` handle, so `SyclHandle::resolve<T>` finds the same object.
- Pool and current-stream slots (`get_stream_from_pool`, `get_current_stream`, `get_default_random_generator`) bind the caller's handle to shared state, such as a shared queue or a shared Philox state. They never swap `plugin_data`.
- `SyclOpsTable` holds two kinds of ops tables: host tables (status, string, buffer, kernel context/construction, grappler item) and the plugin's own vtables. `SyclPluginRegistry::initialize` fills it.

## Legacy mapping

| Legacy | Now |
|---|---|
| `c10/XPUStream.*`, `torch_csrc/Stream.*` | `stream`, `stream_pool`, executor current stream |
| `c10/XPUEvent.h`, `torch_csrc/Event.*` | `event` (timing and IPC) |
| `c10/XPUFunctions.*`, `c10/PeerToPeerAccess.*`, `core/XPUDevice.h` | `platform`, `peer_access` |
| `c10/XPUDeviceProp.h`, `core/XPUContext.*` | `device` |
| `c10/impl/XPUGuardImpl.*` | `device_guard` |
| `core/detail/LazyLevelZero.*` | `level_zero` |
| `c10/XPUCachingAllocator.*`, `torch_csrc/memory_snapshot.*` | `allocator/` |
| `core/CachingHostAllocator.*` | `allocator/host_cache` (`TF_MEMORY_SPACE_HOST_PINNED`) |
| `torch_csrc/XPUPluggableAllocator.*` | `allocator/pluggable_allocator` |
| `core/MemPool.*` | `mem_pool` |
| `core/XPUGeneratorImpl.*`, `PhiloxXpuState.h` | `random_generator/` |
| `core/XPUGraph.*`, graph utils | `grappler/device_graph`, `capture_status` |
| `mkldnn/**`, `transformers/**`, `core/XPUScaledBlas.*` | `kernel/**` |
| `core/detail/XPUHooks.*`, `torch_csrc/Module.*` | `registration`, `plugin.cc` |
| `c10/test/**` | `test/` |

The Python bindings and CMake glue were dropped. Every behavior they exposed is now reachable through the ice slots.

## Kernels

| Op | Class |
|---|---|
| Addmm, BatchMatMul, Baddbmm, Addmv | `matmul/*` |
| Linear (unary/binary fusion) | `linear/linear` |
| Conv2D (N-d, groups), ConvolutionBackward, transposed conv | `convolution/*` |
| QuantizedMatMul and QuantizedLinear (`weight_transposed`), QuantizedConv2D | `quantized/int8_*` |
| WeightOnlyQuantizedMatMul (u4, grouped) | `quantized/woq_matmul` |
| ScaledMatMul (tensor-wise, row-wise, block-wise recipes) | `scaled/*` |
| ScaledDotProductAttention, its backward | `attention/*` (oneDNN graph fused path plus math path) |
| LSTM (multi-layer, bidirectional, inference) | `recurrent/lstm` |

## Known gaps

- LSTM runs forward inference only; no training workspace.
- SDPA has no dropout. The fused path covers non-causal attention; causal attention uses the math path.
- The math path supports grouped-query attention, except with an explicit mask or in backward.
- `create_kernel` is not exported; kernels register through `SyclKernelRegistrar`.
- Not compiled anywhere yet. This needs a SYCL compiler (icpx/AdaptiveCpp), oneDNN, and Level Zero headers.
