// ice: SyclStreamOps.h — replaces torch/csrc/xpu/Stream.h
//
// Original file: torch/csrc/xpu/Stream.h
// Original purpose: Declared a CPython type object (THXPStream / THPStream)
//   so torch.xpu.Stream could be instantiated from Python.  Embedded an
//   at::xpu::XPUStream and exposed sycl_queue / priority / query / synchronize
//   as Python attributes/methods via PyTypeObject machinery.
//
// ice replacement: Pure C++ + C-ABI free functions that wrap a sycl::queue*
//   inside a TF_Stream (plugin_data field).  No Python types, no PyObject*.
//   The host runtime calls these functions through TF_ExecutorOps or directly
//   via dlsym for diagnostics.
//
// No Python.h, no pybind11, no torch headers.

#pragma once

#include <sycl/sycl.hpp>
#include <cstdint>

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/executor.h"

namespace ice::sycl_stream {

// ---------------------------------------------------------------------------
// SyclStreamHandle — thin wrapper that lives in TF_Stream::plugin_data.
// ---------------------------------------------------------------------------

// ice: replaces THXPStream.xpu_stream (at::xpu::XPUStream).
//   c10::xpu::XPUStream → sycl::queue* (ordinal tracked alongside).
struct SyclStreamHandle {
    sycl::queue* queue{nullptr};  // never null after create_sycl_stream()
    int          device_index{-1};
    int          priority{0};

    // ice: replaces stream.id() — use the queue's pointer address as opaque id.
    uint64_t stream_id() const noexcept {
        return reinterpret_cast<uint64_t>(queue);
    }
};

// ---------------------------------------------------------------------------
// C-ABI compatible stream lifecycle — called from TF_ExecutorOps vtable.
// ---------------------------------------------------------------------------

/// Allocates a new SYCL in-order queue on `device_index` with `priority`.
/// Returns a heap-allocated SyclStreamHandle whose address becomes
/// TF_Stream::plugin_data.
/// ice: replaces THXPStream_pynew / at::xpu::getStreamFromPool().
TF_CAPI_EXPORT
TF_Stream sycl_stream_create(int device_index, int priority, TF_Status* out_status);

/// Destroys the SYCL queue and frees the SyclStreamHandle.
/// ice: replaces THXPStream_dealloc.
TF_CAPI_EXPORT
void sycl_stream_destroy(TF_Stream stream);

/// Returns true if all previously-submitted work on the stream has completed.
/// ice: replaces THXPStream_query / XPUStream::query().
TF_CAPI_EXPORT
bool sycl_stream_query(TF_Stream stream);

/// Blocks the calling thread until all work on the stream has completed.
/// ice: replaces THXPStream_synchronize / XPUStream::synchronize().
TF_CAPI_EXPORT
void sycl_stream_synchronize(TF_Stream stream, TF_Status* out_status);

/// Returns true if lhs and rhs wrap the same sycl::queue.
/// ice: replaces THXPStream_eq.
TF_CAPI_EXPORT
bool sycl_stream_equal(TF_Stream lhs, TF_Stream rhs);

/// Returns the raw sycl::queue* stored in the stream handle.
/// ice: replaces THXPStream_get_sycl_queue (Python exposed the ptr as an int).
TF_CAPI_EXPORT
sycl::queue* sycl_stream_get_queue(TF_Stream stream);

/// Returns the [least, greatest] valid priority range for streams.
/// ice: replaces THXPStream_priority_range / XPUStream::priority_range().
TF_CAPI_EXPORT
void sycl_stream_priority_range(int* out_least, int* out_greatest);

// ---------------------------------------------------------------------------
// Required create_stream / destroy_stream symbols (called by stream_executor.h
// init_stream_executor helper).
// ---------------------------------------------------------------------------

} // namespace ice::sycl_stream

// ice: these two C symbols are the mandatory stream_executor plugin hooks.
extern "C" {
TF_CAPI_EXPORT void create_stream(TF_StreamOps** ops, void** plugin_context,
                                   TF_Status* out_status);
TF_CAPI_EXPORT void destroy_stream(void* plugin_context);
} // extern "C"
