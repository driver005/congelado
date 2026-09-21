# C ABI vs ATen XPU — Gap Analysis

## What the C ABI already covers

These ATen operations map cleanly to existing C ABI vtables.

| ATen operation | C ABI vtable | Coverage |
|----------------|--------------|----------|
| Queue create/destroy/sync | TF_ExecutorOps | Full (9 slots) |
| Event create/destroy/record/wait/query | TF_ExecutorOps | Full (6 slots) |
| Timer create/destroy/start/stop | TF_ExecutorOps | Full (4 slots) |
| Device/Unified memory alloc/free | TF_ExecutorOps | Full (6 slots) |
| Memcpy D2H/H2D/D2D async+sync | TF_ExecutorOps | Full (6 slots) |
| Memset/Memzero | TF_ExecutorOps | Full (4 slots) |
| Device count | TF_PlatformOps | Full |
| Device name/vendor/NUMA/BW/GFLOPS/PCI | TF_DeviceOps | Full (6 slots) |
| Tensor lifecycle (dtype/shape/data/copy) | TF_TensorOps | Full (18 slots) |
| Kernel context (inputs/outputs/device info) | TF_OpKernelContextOps | Full (39 slots) |
| Profiling start/stop/collect | TF_ProfilerOps | Full (6 slots) |

## What ATen has that C ABI lacks

### Genuinely missing (no ABI slot at all)

| ATen capability | What it does | Where it belongs |
|-----------------|-------------|-----------------|
| **Stream pool** | 32 queues × 3 priorities, round-robin, thread-local current stream per device | ExecutorOps (add stream_pool ops) |
| **Caching allocator** | block-based pooling, 512-byte alignment, expandable segments, stream-ordered, memory fraction limits, snapshot | ExecutorOps (add allocator ops) |
| **P2P access** | cached peer-access query, enable peer access via allocator | PlatformOps or ExecutorOps |
| **Graph capture** | begin/end/instantiate/replay/reset command graphs, pool-based allocation | ExecutorOps (add graph ops) |
| **RNG generator** | Philox counter, seed/offset/state, graph-safe RNG state tracking | Kernel context (add RNG ops) |
| **Device properties (extended)** | 60+ fields (compute capability, EU count, max work group, sub-group sizes, etc.) | DeviceOps (extend) |
| **Priority queues** | create queues with specific priority levels | ExecutorOps (add priority stream ops) |

### Different flow but covered by existing slots

| ATen flow | C ABI equivalent | Adaptation needed |
|-----------|-----------------|-------------------|
| `record_event` on queue | `TF_ExecutorOps::record_event` | Direct map |
| `block` queue on event | `TF_ExecutorOps::wait_for_event` | Direct map |
| `sycl::malloc_device` | `TF_ExecutorOps::allocate` | Direct map |
| `queue.wait_and_throw()` | `TF_ExecutorOps::block_host_until_done` | Direct map |
| `queue.memcpy(dst, src, n)` | `TF_ExecutorOps::memcpy_*` | Direct map |
| `queue.memset(ptr, val, n)` | `TF_ExecutorOps::memset` | Direct map |

### Kernel ops that need new ABI slots

These are operations kernel implementations need but the kernel context
vtable doesn't expose.

| ATen kernel requirement | What it does | ABI gap |
|------------------------|-------------|---------|
| `get_raw_device(idx)` | get `sycl::device&` from pool | PlatformOps needs `get_device_handle` |
| `get_device_context()` | get shared `sycl::context&` | PlatformOps needs `get_device_context` |
| `device_synchronize(idx)` | wait all work on device | ExecutorOps already has `synchronize_all_activity` |
| `get_device_properties(prop, idx)` | 60+ field DeviceProp | DeviceOps needs property accessors |
| `get_device_idx_from_pointer(ptr)` | find which device owns USM ptr | ExecutorOps needs `device_for_ptr` |

## Summary

| Category | Count | Status |
|----------|-------|--------|
| Fully covered by existing ABI | 15+ operations | No change needed |
| Missing: stream pool | 3-5 slots needed | Add to ExecutorOps |
| Missing: caching allocator | 4-6 slots needed | Add to ExecutorOps |
| Missing: P2P access | 2 slots needed | Add to PlatformOps |
| Missing: graph capture | 5-6 slots needed | Add to ExecutorOps |
| Missing: RNG generator | 3-4 slots needed | Add to OpKernelContextOps |
| Missing: device properties | 5-10 slots needed | Extend DeviceOps |
| Missing: priority queues | 1-2 slots needed | Add to ExecutorOps |

**Total new slots needed: ~25-35** across 4 vtables.

## Recommendation

The C ABI can cover everything ATen XPU needs. The approach:

1. **Extend existing vtables** (don't create new ones) — ExecutorOps,
   PlatformOps, DeviceOps, OpKernelContextOps
2. **ATen XPU needs a caching allocator on top of raw alloc** — implement
   in C++ builder/sonic layer, not in the ABI (the ABI provides raw alloc,
   builder adds pooling)
3. **Stream pool is a builder concern** — ABI provides create/destroy stream,
   builder adds pool management and thread-local current stream
4. **Graph capture is a builder concern** — ABI provides the primitives,
   builder adds the state machine (begin/end/instantiate/replay)
