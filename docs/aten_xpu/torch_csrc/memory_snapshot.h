// ice: SyclMemoryDiag.h — replaces torch/csrc/xpu/memory_snapshot.h
//
// Original file: torch/csrc/xpu/memory_snapshot.h
// Original purpose: Declared torch::xpu::_record_memory_history() with a
//   TORCH_PYTHON_API export macro, callable from Python via m.def().
//   The function enabled/disabled C++ stack-trace capture in the caching
//   allocator so that memory_snapshot() could include per-block tracebacks.
//
// ice replacement: Two public C-ABI functions:
//   • sycl_memory_snapshot()  — returns a JSON string describing live device
//     allocations, built from sycl::device::get_info queries.  No Python.
//   • sycl_record_memory_history() — enables/disables lightweight history
//     tracking (allocation count, bytes).  No stack unwinding, no Python GC.
//
// No Python.h, no pybind11, no torch headers.

#pragma once

#include <sycl/sycl.hpp>
#include <cstddef>
#include <cstdint>
#include <string>

#include "include/c/macros.h"
#include "include/c/intern/status.h"

namespace ice::sycl_diag {

// ---------------------------------------------------------------------------
// Memory history recording control
// ---------------------------------------------------------------------------

/// Enables or disables lightweight allocation history collection.
///
/// `enabled`     — true to start recording, false to stop.
/// `max_entries` — maximum number of allocation events to retain
///                 (0 = no limit).
///
/// ice: replaces torch::xpu::_record_memory_history().
///   Original used CapturedTraceback + Python stack-frame walking.
///   This version accumulates (address, size, device, timestamp) tuples
///   using a lock-free ring buffer — no CPython involvement.
TF_CAPI_EXPORT
void sycl_record_memory_history(bool enabled, size_t max_entries,
                                 TF_Status* out_status);

// ---------------------------------------------------------------------------
// Memory snapshot (diagnostic / introspection)
// ---------------------------------------------------------------------------

/// Returns a JSON-encoded string describing current device memory usage for
/// all visible SYCL GPU devices.  The caller owns the returned std::string.
///
/// Example output (single device):
///   {
///     "devices": [{
///       "index": 0,
///       "name": "Intel Data Center GPU Max 1100",
///       "global_mem_bytes": 51539607552,
///       "local_mem_bytes": 131072,
///       "max_compute_units": 448,
///       "allocations": [
///         {"address": "0xdeadbeef0000", "size_bytes": 4096,
///          "pool_tag": "default", "stream_ptr": "0xcafe0000"}
///       ]
///     }]
///   }
///
/// ice: replaces the Python-only _xpu_memorySnapshot lambda in Module.cpp.
///   Original built a py::dict and depended on CapturedTraceback / pybind11.
///   This version uses sycl::device::get_info<>() to fill device metadata and
///   the plugin's internal allocation registry for the "allocations" array.
TF_CAPI_EXPORT
std::string sycl_memory_snapshot(TF_Status* out_status);

// ---------------------------------------------------------------------------
// Per-device statistics (lightweight alternative to full snapshot)
// ---------------------------------------------------------------------------

struct SyclMemStats {
    uint64_t global_mem_bytes{0};   ///< total device global memory
    uint64_t local_mem_bytes{0};    ///< total device local (shared) memory
    uint64_t allocated_bytes{0};    ///< bytes currently allocated via plugin
    uint64_t peak_allocated_bytes{0};
    uint64_t num_allocations{0};
    uint64_t num_frees{0};
};

/// Fills `*out_stats` with memory statistics for `device_index`.
/// ice: replaces THXPModule_memoryStats / XPUCachingAllocator::getDeviceStats().
TF_CAPI_EXPORT
void sycl_memory_stats(int device_index, SyclMemStats* out_stats,
                        TF_Status* out_status);

/// Resets peak and accumulated allocation counters for `device_index`.
/// ice: replaces THXPModule_resetPeakMemoryStats +
///               THXPModule_resetAccumulatedMemoryStats.
TF_CAPI_EXPORT
void sycl_reset_memory_stats(int device_index, TF_Status* out_status);

} // namespace ice::sycl_diag
