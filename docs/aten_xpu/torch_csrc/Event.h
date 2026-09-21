// ice: SyclEventOps.h — replaces torch/csrc/xpu/Event.h
//
// Original file: torch/csrc/xpu/Event.h
// Original purpose: Declared a CPython type object (THXPEvent / THPEvent)
//   so torch.xpu.Event could be instantiated from Python.  Embedded an
//   at::xpu::XPUEvent and exposed record/wait/query/elapsed_time/synchronize
//   as Python methods, plus get_device/get_sycl_event as properties.
//   Also supported IPC handle export/import via
//   sycl::ext::oneapi::experimental::ipc::handle_data_t.
//
// ice replacement: Pure C++ + C-ABI free functions that wrap a sycl::event
//   inside a TF_Event (plugin_data field).  IPC handle bytes are passed as
//   raw byte spans (void* + size_t).  No Python types, no PyObject*.
//   The host runtime calls these through TF_ExecutorOps vtable
//   (record_event / wait_for_event / get_event_status / block_host_for_event).
//
// No Python.h, no pybind11, no torch headers.

#pragma once

#include <sycl/sycl.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/stream.h"

namespace ice::sycl_event {

// ---------------------------------------------------------------------------
// SyclEventHandle — lives in TF_Event::plugin_data.
// ---------------------------------------------------------------------------

// ice: replaces THXPEvent.xpu_event (at::xpu::XPUEvent).
//   at::xpu::XPUEvent → heap-allocated sycl::event wrapper.
struct SyclEventHandle {
    sycl::event event;         // the underlying SYCL event
    int         device_index{-1};
    bool        enable_timing{false};
    bool        interprocess{false};
};

// ---------------------------------------------------------------------------
// C-ABI lifecycle and ops — called from TF_ExecutorOps vtable.
// ---------------------------------------------------------------------------

/// Creates a new SYCL event.  If `enable_timing` is true the underlying event
/// will carry profiling data usable by sycl_event_elapsed_ms().
/// `interprocess` requests an IPC-exportable event (requires SYCL ≥ 2026.2).
/// ice: replaces THXPEvent_pynew / at::xpu::XPUEvent(enable_timing, interprocess).
TF_CAPI_EXPORT
TF_Event sycl_event_create(int device_index, bool enable_timing,
                            bool interprocess, TF_Status* out_status);

/// Destroys the SYCL event and frees the SyclEventHandle.
/// ice: replaces THXPEvent_dealloc.
TF_CAPI_EXPORT
void sycl_event_destroy(TF_Event event);

/// Records the event on `stream`.  Subsequent waits on this event will block
/// until the queue reaches the point of recording.
/// ice: replaces THXPEvent_record / XPUEvent::record(xpu_stream).
TF_CAPI_EXPORT
void sycl_event_record(TF_Event event, TF_Stream stream, TF_Status* out_status);

/// Makes `stream` wait for `event` before executing subsequent work.
/// ice: replaces THXPEvent_wait / XPUEvent::block(xpu_stream).
TF_CAPI_EXPORT
void sycl_event_wait(TF_Event event, TF_Stream stream, TF_Status* out_status);

/// Returns true if the event has completed (non-blocking poll).
/// ice: replaces THXPEvent_query / XPUEvent::query().
TF_CAPI_EXPORT
bool sycl_event_query(TF_Event event);

/// Blocks the calling thread until the event has completed.
/// ice: replaces THXPEvent_synchronize / XPUEvent::synchronize().
TF_CAPI_EXPORT
void sycl_event_synchronize(TF_Event event, TF_Status* out_status);

/// Returns the elapsed time in milliseconds between `start` and `end`.
/// Both events must have been created with enable_timing = true.
/// ice: replaces THXPEvent_elapsed_time / XPUEvent::elapsed_time().
TF_CAPI_EXPORT
float sycl_event_elapsed_ms(TF_Event start, TF_Event end, TF_Status* out_status);

/// Returns the ordinal of the device this event was recorded on, or -1 if
/// the event has not yet been recorded.
/// ice: replaces THXPEvent_get_device.
TF_CAPI_EXPORT
int sycl_event_device_index(TF_Event event);

// ---------------------------------------------------------------------------
// IPC handle export/import (requires SYCL compiler ≥ 2026.2).
// ---------------------------------------------------------------------------

/// Serialises the event to an IPC byte handle.  Writes bytes into `out_buf`
/// (caller-owned, must be ≥ `*out_size` bytes).  Pass out_buf = nullptr on
/// the first call to query the required size.
/// ice: replaces THXPEvent_ipc_handle.
TF_CAPI_EXPORT
void sycl_event_ipc_handle(TF_Event event, void* out_buf, size_t* out_size,
                             TF_Status* out_status);

/// Reconstructs an event from a previously exported IPC handle blob.
/// ice: replaces THXPEvent_from_ipc_handle.
TF_CAPI_EXPORT
TF_Event sycl_event_from_ipc_handle(int device_index, const void* handle_bytes,
                                      size_t handle_size, TF_Status* out_status);

} // namespace ice::sycl_event

// ice: mandatory stream_executor plugin hooks.
extern "C" {
TF_CAPI_EXPORT void create_event(TF_EventOps** ops, void** plugin_context,
                                   TF_Status* out_status);
TF_CAPI_EXPORT void destroy_event(void* plugin_context);
} // extern "C"
