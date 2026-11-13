# aten_xpu: SYCL backend on the ice layer

Reference SYCL/oneDNN plugin that implements the generated `ice::builder::*` interfaces.
It lives under `docs/` and Bazel does not build it.

## Layout

The tree mirrors `include/cc/ice`. Each directory is one C++ module, and each file is one partition holding one class.

```
docs/aten_xpu/
├── base.cppm                 aten_xpu (re-exports intern + extern)
├── plugin.cc                 C entry points only (create_plugin, create_*/destroy_*)
├── intern/                   aten_xpu_intern
│   ops_table, handle, status, tensor, tensor_factory, and the plain C++ builders
│   (no SYCL): string, status_store, buffer, registration, complex, duration, time_point,
│   data_type, attr_type, shape, hash, file_statistics, text_tensor, array, vector, deque,
│   bit_set, span, list, forward_list, hive, map, set, failure
├── extern/                   aten_xpu_extern
│   ├── stream_executor/      platform, device, device_guard, peer_access, level_zero,
│   │                         executor, stream, stream_pool, event, timer, memory, mem_pool,
│   │                         stream_executor, allocator/{allocator, block, block_pool,
│   │                         expandable_segment, stats, trace, snapshot, ipc_memory,
│   │                         host_cache, pluggable_allocator}
│   ├── random_generator/     random_generator, philox_state, philox_codec
│   ├── grappler/             grappler, device_graph, capture_status, optimizer
│   ├── filesystem/           filesystem, random_access_file, writable_file,
│   │                         read_only_memory_region, tree (std::filesystem + POSIX)
│   ├── kernel/               kernel (SyclKernel<Derived> : ice::builder::Kernel), registrar,
│   │                         context/, onednn/, matmul/, linear/, convolution/,
│   │                         quantized/, scaled/, attention/, recurrent/
│   └── registration/         SyclPluginRegistry (all vtables, host ops, kernel registration)
└── test/                     gtest suites on the ice::sonic wrappers (fixture.cppm)
```

## Lifecycle

- The host calls a vtable's `create` slot. The plugin allocates the concrete `Sycl*` object and `create` stores `plugin_data` from the new object in `plugin_data`. `destroy` deletes the object.
- `create_*_internal(parent, handle)` binds an object that already exists (for example `SyclStream::bind`). Generated wrappers copy the `{plugin_data}` handle, so `T::from_handle` finds the same object.
- Pool and current-stream slots (`get_stream_from_pool`, `get_current_stream`, `get_default_random_generator`) bind the caller's handle to shared state, such as a shared queue or a shared Philox state. They never swap `plugin_data`.
- `SyclOpsTable` holds two kinds of ops tables: host tables (status, string, buffer, kernel context/construction, grappler item) and the plugin's own vtables. `SyclPluginRegistry::initialize` fills it.

## Generated API conventions

- Class names drop `TF_` and `Ops`: `ice::builder::Device`, `ice::sonic::Registration`.
- Plain C structs and enums have value classes in `ice::sonic` (`PoolId`, `DeviceProperties`, `MemorySpace`, `Code`, ...). Struct parameters are `ice::sonic::T*` (NULL stays possible); enums go by value or as `X&` for `out_*`.
- Scalar `out_*` parameters are references (`std::int32_t& out_x`).
- Arrays: the implementer (`ice::builder`) gets `std::span` over the host's array, the caller (`ice::sonic`) passes a `std::vector`.
- Nothing in the plugin is `static` except the two `getInstance()` entry points; `create` slots are lambdas in `SyclPluginRegistry`.

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
| `torch_csrc/profiler/*` (XPU activity trace) | `extern/profiler`: SYCL event profiling, XSpace output |
| `c10/test/**` | `test/` |

The Python bindings and CMake glue were dropped. Every behavior they exposed is now reachable through the ice slots.

## Profilers

Four independent `ice::builder::Profiler` implementations, each its own plugin library with its own `create_profiler` (one per library is what the C ABI allows). The three vendor ones do not use SYCL.

| Profiler | Library | Reports | Needs |
|---|---|---|---|
| SYCL (`extern/profiler/sycl`) | the main `aten_xpu` plugin | what the plugin submits: copies, fills, oneDNN primitives, host callbacks, allocator actions | `aspect::queue_profiling`; `ATEN_XPU_PROFILING=1` at load for device timestamps |
| PTI (`extern/profiler/pti`) | `libaten_xpu_profiler_pti.so` | every kernel and copy on an Intel GPU, real kernel names | Intel PTI (`libpti_view.so`) |
| CUPTI (`extern/profiler/cupti`) | `libaten_xpu_profiler_cupti.so` | every kernel, copy and fill on an NVIDIA GPU | CUPTI (`libcupti.so`) |
| rocprofiler (`extern/profiler/rocprofiler`) | `libaten_xpu_profiler_rocprofiler.so` | every kernel dispatch and copy on an AMD GPU | rocprofiler-sdk |

All of them return a TensorFlow `XSpace` (protobuf, `extern/profiler/proto/xplane.proto`, vendored from openxla/xla) as a `uint8` tensor.

- **SYCL:** works on every device with `aspect::queue_profiling` (Level Zero, OpenCL, CUDA, HIP). Without `ATEN_XPU_PROFILING=1` queues have no profiling, so `start` still works but activities show at submission time with zero duration and the XSpace carries a warning. One XPlane per device, one XLine per queue and kind, plus an `allocator` line. Not recorded: copies on throwaway queues (`blocking_copy`, tensor copies, host reads), the Philox graph-capture fills and barriers.
- **Vendor profilers:** `common/` holds what they share (tracer interface, `VendorProfiler`, a host-memory tensor, the plugin owner). Each library also exports `create_tensor`, because the host needs the tensor ops to read the returned `XSpace`. One XPlane per device the tracer reported (`/device:<tracer>:<device>`), one XLine per queue and kind. `start` fails with a status error if the tracer is missing.
- Each tracer is optional twice over: its header (`pti/pti_view.h`, `cupti.h`, `rocprofiler-sdk/rocprofiler.h`) at build time, its library at run time.
- PTI and CUPTI generally have to start before the GPU runtime initialises. rocprofiler-sdk only reports if it loads its library before HSA starts (it finds the exported `rocprofiler_configure`).
- The CUPTI record structs follow the header (`CUpti_ActivityKernel9`, `Memcpy6`, `Memset4`, CUPTI 12.9); another CUPTI version may need other struct names.
- Build the vendor libraries with `--force_pic`: `bazel build --force_pic //docs/aten_xpu/extern/profiler/pti:libaten_xpu_profiler_pti.so` (protobuf is linked statically).

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

## Plain C++ builders

Every `intern/` builder and the filesystem family have a plain C++ implementation (`String`, `Status`, `Vector`, `FilesystemTree`, ...). None of them needs SYCL.

- `SyclPluginRegistry` owns one prototype of each class, so every C vtable has a stable address, and getters such as `getVectorOps()` return it. There is no second registry.
- `plugin.cc` exports `create_<name>` for all of them. `create_string`, `create_status` and `create_buffer` are weak, so a real host definition overrides them; the other types bind to whichever status and string ops are in effect.
- Values the C ABI cannot carry come from the constructor: `Map` and `Set` take key and value sizes, `BitSet` its size, `Span::bind` its memory, `Duration` and `TimePoint` their ticks, and the files and memory region their path. Objects made through a `create` slot use defaults (8 byte keys and values, 64 bits, no path).
- Elements of the containers are byte erased: call `set_element_size` first. Pointers handed out by `get`, `find` and `data` are valid until the container changes.
- `FilesystemTree` returns string tensors as `TextTensor`: one dimension, every string NUL terminated back to back. The caller frees them with the tensor's `delete_tensor`.
- Tests: `test/{foundation,value,sequence,associative,filesystem}_test.cc` use `RegistryFixture` and need no XPU device.

## Known gaps

- ABI slots the plain C++ builders had to guess, to be fixed in the C headers: `Map` and `Set` have no key or value size, `Span` and `BitSet` have no bind or size slot, `Duration` has no setter (`TimePoint` fills only a `Duration`), `FilesystemTree::paths_exist` receives one String for an array (only the first path is checked), `free_options` has no context, and the filesystem has no open slot.

- LSTM runs forward inference only; no training workspace.
- SDPA has no dropout. The fused path covers non-causal attention; causal attention uses the math path.
- The math path supports grouped-query attention, except with an explicit mask or in backward.
- `create_kernel` is not exported; kernels register through `SyclKernelRegistrar`.
- Not compiled anywhere yet. This needs a SYCL compiler (icpx/AdaptiveCpp), oneDNN, and Level Zero headers.
