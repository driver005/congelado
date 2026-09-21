# C ABI Flow — How the Congelado plugin layer works

## Architecture

The C ABI layer is a **vtable-based plugin system**. Backends (SYCL, CUDA, etc.)
implement function-pointer structs. The C++20 builder/sonic layers auto-generate
wrappers around these vtables.

```
Plugin (.so)                    C ABI (vtable)              C++20 (auto-gen)
┌─────────────┐    register     ┌──────────────┐  wrap     ┌─────────────────┐
│ SYCL impl   │───────────────→│ TF_ExecutorOps│─────────→│ ice::builder::  │
│ sycl::queue │                │ 35 fn ptrs    │          │ Executor (35 v) │
│ sycl::event │                └──────────────┘          └─────────────────┘
└─────────────┘
```

## Existing vtables and their slots

### TF_ExecutorOps (executor.h) — 35 slots

Hardware abstraction: stream/event/timer lifecycle, memory, memcpy, sync.

| Slot | Purpose | SYCL mapping |
|------|---------|--------------|
| `allocate` | device memory alloc | `sycl::malloc_device` |
| `deallocate` | device memory free | `sycl::free` |
| `host_memory_allocate` | pinned host alloc | `sycl::malloc_host` |
| `host_memory_deallocate` | pinned host free | `sycl::free` |
| `unified_memory_allocate` | shared memory alloc | `sycl::malloc_shared` |
| `unified_memory_deallocate` | shared memory free | `sycl::free` |
| `get_allocator_stats` | allocator statistics | sentinel (unsupported) |
| `device_memory_usage` | free/total memory | `sycl::info::device::global_mem_size` |
| `create_stream_internal` | create queue | `new sycl::queue(in_order)` |
| `destroy_stream_internal` | destroy queue | `delete queue` |
| `create_stream_dependency` | cross-queue dep | `ext_oneapi_submit_barrier` |
| `get_stream_status` | queue drain check | `queue.wait()` |
| `create_event_internal` | create event | `new sycl::event()` |
| `destroy_event_internal` | destroy event | `delete event` |
| `get_event_status` | event completion | `event.get_info<status>` |
| `record_event` | record on queue | `queue.ext_oneapi_submit_barrier()` |
| `wait_for_event` | queue waits on event | `depends_on(event)` |
| `create_timer_internal` | create timer pair | `new pair<event,event>` |
| `destroy_timer_internal` | destroy timer pair | `delete pair` |
| `start_timer` | record start event | `queue.ext_oneapi_submit_barrier()` |
| `stop_timer` | record stop event | `queue.ext_oneapi_submit_barrier()` |
| `memcpy_dtoh` | async D2H copy | `queue.memcpy(dst, src, n)` |
| `memcpy_htod` | async H2D copy | `queue.memcpy(dst, src, n)` |
| `memcpy_dtod` | async D2D copy | `queue.memcpy(dst, src, n)` |
| `sync_memcpy_dtoh` | sync D2H | `queue.memcpy(...).wait()` |
| `sync_memcpy_htod` | sync H2D | `queue.memcpy(...).wait()` |
| `sync_memcpy_dtod` | sync D2D | `queue.memcpy(...).wait()` |
| `block_host_for_event` | host waits on event | `event.wait()` |
| `block_host_until_done` | host waits on queue | `queue.wait_and_throw()` |
| `synchronize_all_activity` | wait all queues | `default_queue.wait_and_throw()` |
| `mem_zero` | zero-fill memory | `queue.memset(ptr, 0, n)` |
| `memset` | byte-fill memory | `queue.memset(ptr, val, n)` |
| `memset32` | 32-bit fill | `parallel_for` kernel |
| `host_callback` | host task on queue | `queue.host_task(fn)` |
| `create_stream_with_options` | stream with priority | fallback to default |

### TF_PlatformOps (platform.h) — 5 slots

Device enumeration and executor/device lifecycle.

| Slot | Purpose |
|------|---------|
| `get_device_count` | count available devices |
| `create_device_internal` | create device handle |
| `destroy_device_internal` | destroy device handle |
| `create_executor_internal` | create executor for device |
| `destroy_executor_internal` | destroy executor |

### TF_DeviceOps (device.h) — 6 slots

Device metadata.

| Slot | Purpose |
|------|---------|
| `get_numa_node` | NUMA node ID |
| `get_memory_bandwidth` | memory bandwidth (GB/s) |
| `get_gflops` | compute power (GFLOPS) |
| `get_hardware_name` | device name string |
| `get_device_vendor` | vendor string |
| `get_pci_bus_id` | PCI bus ID string |

### TF_TensorOps (tensor.h) — 18 slots

Tensor lifecycle, dtype, shape, data access.

| Slot | Purpose |
|------|---------|
| `get_name` | backend name |
| `set_dtype` / `tensor_type` | element data type |
| `set_dims` / `num_dims` / `dim` | shape |
| `set_byte_size` / `tensor_byte_size` | byte size |
| `tensor_data` | raw data pointer |
| `tensor_element_count` | total element count |
| `delete_tensor` | free tensor |
| `tensor_bitcast_from` / `tensor_bitcast_to` | reinterpret dtype |
| `tensor_copy` | deep copy |

### TF_OpKernelContextOps (context.h) — 39 slots

Kernel execution context: get inputs, allocate outputs, device info, resource
management. This is what kernel implementations (addmm, conv, attention, etc.)
use.

### Grappler vtables (grappler/) — 5 vtables

| Vtable | Slots | Purpose |
|--------|-------|---------|
| `TF_GrapplerOps` | 2 | destroy, get_name |
| `TFGrapplerOptimizerOps` | 1 | optimize (graph transform) |
| `TFGrapplerFunctionLibraryOps` | 1 | look_up_op_def |
| `TFGrapplerItemOps` | 4 | graph node metadata |
| `TFGrapplerPropertiesOps` | 5 | property inference |
| `TFGrapplerConfigsOps` | 4 | optimization configs |

### TF_ProfilerOps (profiler.h) — 6 slots

| Slot | Purpose |
|------|---------|
| `destroy` | free profiler |
| `get_name` | profiler name |
| `get_device_type` | device type string |
| `start` | begin profiling |
| `stop` | end profiling |
| `collect_data_xspace` | collect profiling data |

### TF_TimerOps (timer.h) — 1 slot

| Slot | Purpose |
|------|---------|
| `nanoseconds` | get elapsed nanoseconds |

## Flow: allocating a tensor

```
User code
  → ice::builder::Tensor::set_dims({4,32,32})
    → TF_TensorOps::set_dims (vtable call)
      → SyclTensorImpl::set_dims
        → sycl::malloc_device(nbytes, queue)
```

## Flow: executing a kernel

```
User code
  → ice::builder::OpKernelContext::get_input(0)
    → TF_OpKernelContextOps::get_input (vtable call)
      → kernel reads input tensor
  → ice::builder::OpKernelContext::allocate_output(0, ...)
    → TF_OpKernelContextOps::allocate_output (vtable call)
      → kernel allocates output
  → user kernel runs (calls SYCL directly)
  → ice::builder::OpKernelContext::set_output(0, tensor)
    → TF_OpKernelContextOps::set_output (vtable call)
```

## Flow: stream sync

```
User code
  → ice::builder::Executor::block_host_until_done(stream)
    → TF_ExecutorOps::block_host_until_done (vtable call)
      → queue.wait_and_throw()
```
