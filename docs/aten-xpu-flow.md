# ATen XPU Flow — How PyTorch's XPU backend works

## Architecture

ATen XPU is a **layered C++ stack** with direct SYCL calls. No plugin boundary.

```
ATen native ops (addmm, conv, attention)
  → c10/xpu primitives (XPUStream, XPUEvent, XPUCachingAllocator)
    → SYCL runtime (sycl::queue, sycl::event, sycl::malloc_device)
```

## Layers

### Layer 1: c10/xpu — low-level SYCL primitives

#### XPUFunctions (device management)

| Function | What it does |
|----------|-------------|
| `device_count()` | enumerate SYCL GPU devices (dGPU first, then iGPU) |
| `current_device()` | thread-local device index |
| `set_device(idx)` | set thread-local device |
| `exchange_device(idx)` | swap current device, return old |
| `get_raw_device(idx)` | get `sycl::device&` from global pool |
| `get_device_context()` | get shared `sycl::context&` |
| `device_synchronize(idx)` | `device.ext_oneapi_wait_and_throw()` |
| `get_device_properties(prop, idx)` | fill DeviceProp struct (60+ fields) |
| `get_device_idx_from_pointer(ptr)` | find which device owns a USM pointer |

Global state:
- `DevicePool` — vector of `sycl::device` + shared `sycl::context`
- `curDeviceIndex` — thread-local current device
- Lazily initialized on first call

#### XPUStream (queue management)

| Function | What it does |
|----------|-------------|
| `getStreamFromPool(priority, device)` | round-robin from 32-queue pool per priority |
| `getStreamFromExternal(queue*, device)` | wrap external SYCL queue |
| `getCurrentXPUStream(device)` | get thread-local current stream |
| `setCurrentXPUStream(stream)` | set thread-local current stream |
| `syncStreamsOnDevice(device)` | wait all 32×3 queues, or `device_wait` |
| `XPUStream::query()` | `queue.ext_oneapi_empty()` |
| `XPUStream::priority()` | query queue priority property |
| `XPUStream::synchronize()` | `queue.wait_and_throw()` |
| `XPUStream::is_capturing()` | check `queue_state::recording` |

Global state:
- `streams[device][priority][index]` — 3 pools × 32 queues each
- `current_streams[device]` — thread-local current stream ID
- `priority_counters[device][priority]` — round-robin index
- StreamId encodes: type (LOW/NORMAL/HIGH/EXT) + index

#### XPUEvent (event management)

| Method | What it does |
|--------|-------------|
| `XPUEvent(enable_timing, enable_ipc)` | constructor with options |
| `record(stream)` | `ext_oneapi_submit_barrier` or `enqueue_signal_event` |
| `block(stream)` | `ext_oneapi_submit_barrier(event_list)` |
| `query()` | `event.get_info<status> == complete` |
| `synchronize()` | `event.wait_and_throw()` |
| `elapsed_time(other)` | `get_profiling_info<command_end>` delta |
| `ipc_handle()` | export for inter-process sharing |

Features:
- Lazy creation (on first record)
- Reusable events (IPC, SYCL ≥ 2026.2)
- Profiling events (`submit_profiling_tag`)
- IPC support (`ipc::event::open/get`)

#### XPUCachingAllocator (memory management)

| Function | What it does |
|----------|-------------|
| `init(device_count)` | initialize allocator |
| `raw_alloc(size)` | allocate from caching pool |
| `raw_delete(ptr)` | return to caching pool |
| `emptyCache()` | release unused blocks |
| `getMemoryFraction(device)` | get memory limit fraction |
| `setMemoryFraction(frac, device)` | set memory limit |
| `getDeviceStats(device)` | allocation stats |
| `enablePeerAccess(dev, peer)` | enable P2P access |
| `recordStream(ptr, stream)` | record stream usage on block |
| `snapshot()` | capture allocator state |
| `shareIpcHandle(ptr)` | export for IPC |
| `getIpcDevPtr(handle)` | import from IPC |

Internal: block-based allocator with 512-byte alignment, expandable segments,
stream-ordered pooling.

#### PeerToPeerAccess

| Function | What it does |
|----------|-------------|
| `get_p2p_access(dev, peer)` | cached query: `ext_oneapi_can_access_peer` |
| `enablePeerAccess(dev, peer)` | enable via caching allocator |

### Layer 2: core/ — ATen-level operations

#### XPUGraph (graph capture)

| Method | What it does |
|--------|-------------|
| `capture_begin(pool)` | start recording to command graph |
| `capture_end()` | finalize command graph |
| `instantiate()` | compile to executable graph |
| `replay()` | execute captured graph |
| `reset()` | destroy executable, keep modifiable |
| `register_generator_state(state)` | track RNG state offsets |
| `enable_debug_mode()` / `debug_dump()` | debugging |

Uses: `sycl::ext::oneapi::experimental::command_graph<modifiable>`
      `sycl::ext::oneapi::experimental::command_graph<executable>`

#### XPUGeneratorImpl (RNG)

| Method | What it does |
|--------|-------------|
| `set_current_seed(seed)` | set RNG seed |
| `current_seed()` | get current seed |
| `set_offset(offset)` / `get_offset()` | Philox offset |
| `set_state(tensor)` / `get_state()` | serialize/deserialize state |
| `philox_xpu_state(inc)` | get state for kernel args |
| `register_graph(graph)` / `unregister_graph(graph)` | graph-safe RNG |

Uses Philox counter-based RNG with per-thread offsets.

#### MemPool (memory pool for graph capture)

| Method | What it does |
|--------|-------------|
| `MemPool(allocator, ...)` | create pool |
| `graph_pool_handle()` | get pool ID for graph capture |
| `beginAllocateToPool(pool)` | start allocating to pool |
| `endAllocateToPool(pool)` | stop allocating to pool |
| `markCaptureBegin/End(pool)` | mark capture boundaries |
| `releasePool(pool)` | release pool resources |

### Layer 3: mkldnn/ — oneDNN kernel implementations

All operations go through `OpKernelContext`:

| Operation | What it does |
|-----------|-------------|
| `addmm_out` | matrix multiply + bias |
| `bmm_out` | batched matrix multiply |
| `addmv_out` | matrix-vector multiply |
| `convolution` | 1D/2D/3D convolution |
| `linear` | fully connected |
| `scaled_dot_product_attention` | flash attention |
| `_scaled_mm` | scaled matrix multiply |
| `scaled_scaled_mm` | double-scaled GEMM |

These call oneDNN library functions directly (dnnl::gemm, dnnl::convolution_forward, etc.)
via SYCL queue submission.

### Layer 4: transformers/ — attention kernels

| Operation | What it does |
|-----------|-------------|
| `scaled_dot_product_attention` | XPU-optimized SDPA |
| attention backward | gradient computation |

## Flow: allocating a tensor (ATen)

```
ATen code
  → at::empty({4,32,32}, options)
    → c10::xpu::XPUCachingAllocator::raw_alloc(nbytes)
      → BlockPool search (cached)
        → if miss: sycl::malloc_device(nbytes, queue)
      → return DataPtr
```

## Flow: executing addmm (ATen)

```
at::addmm_out(result, mat1, mat2, beta, alpha)
  → at::native::xpu::addmm_out()
    → check shapes, dtypes
    → dnnl::gemm_direct(dnnl::engine, stream, ...)
      → submits SYCL kernel to queue
    → result populated
```

## Flow: stream sync (ATen)

```
XPUStream stream = getCurrentXPUStream();
stream.synchronize();
  → queue.wait_and_throw()
```

## Flow: graph capture (ATen)

```
XPUGraph graph;
graph.capture_begin(pool_id);
  → sets queue to recording state
  → all ops on capture stream are recorded

// ... run operations ...

graph.capture_end();
  → finalizes command_graph<modifiable>

graph.instantiate();
  → compiles to command_graph<executable>

graph.replay();
  → executes captured graph
```

## Key differences from C ABI flow

| Aspect | C ABI | ATen XPU |
|--------|-------|----------|
| Plugin boundary | vtable function pointers | direct C++ calls |
| Stream pool | single queue per stream | 32 queues × 3 priorities pool |
| Memory management | raw alloc/free only | caching allocator with pooling |
| Event support | basic record/wait | timing, IPC, reusable events |
| Graph capture | not implemented | full command graph lifecycle |
| RNG | handled by kernel context | dedicated GeneratorImpl |
| Device properties | 5 fields (name, vendor, bw, gflops, numa) | 60+ fields |
| P2P | not implemented | cached peer access matrix |
