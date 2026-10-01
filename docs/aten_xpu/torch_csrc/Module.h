// ice: SyclPlugin.h — replaces torch/csrc/xpu/Module.h
//
// Original file: torch/csrc/xpu/Module.h
// Original purpose: Exposed a PyMethodDef* table and initModule(PyObject*)
//   so that CPython could register XPU management functions (set_device,
//   get_device_count, synchronize, memory_stats, etc.) into torch._C.
//
// ice replacement: declares create_plugin() — the single C-ABI entry point
//   the host (congelado runtime) calls after dlopen'ing the SYCL plugin .so.
//   All former Python-exposed helpers become internal C++ functions or are
//   accessible through the TF_StreamExecutor / TF_ExecutorOps vtable.
//
// No Python.h, no pybind11, no torch headers.

#pragma once

#include "include/c/extern/plugin/registration.h"
#include "include/c/extern/stream_executor/stream_executor.h"
#include "include/c/intern/status.h"
#include "include/c/macros.h"

#include <sycl/sycl.hpp>

// ---------------------------------------------------------------------------
// Plugin entry point (mandatory C-ABI symbol; host dlsym's "create_plugin")
// ---------------------------------------------------------------------------

// ice: replaces PyMethodDef* THXPModule_methods() + torch::xpu::initModule().
//   The host discovers this symbol via dlsym and calls it to fill in the
//   TF_PluginInfo struct (name, version).  All capability registration
//   (executor vtable, allocator hooks) happens in SyclPlugin.cpp via
//   init_stream_executor().
extern "C" TF_CAPI_EXPORT void create_plugin(TF_PluginInfo* plugin_info);

// ---------------------------------------------------------------------------
// Lifecycle helpers used internally by the plugin (C++ linkage is fine here)
// ---------------------------------------------------------------------------

namespace ice::sycl_plugin {

/// Returns the number of SYCL GPU devices visible on this host.
/// ice: replaces THXPModule_getDeviceCount_wrap / at::xpu::device_count().
int sycl_device_count();

/// Returns the ordinal of the currently active SYCL device for this thread.
/// ice: replaces THXPModule_getDevice_wrap / c10::xpu::current_device().
int sycl_current_device();

/// Sets the currently active SYCL device for this thread.
/// ice: replaces THXPModule_setDevice_wrap / c10::xpu::set_device().
void sycl_set_device(int device_index);

/// Exchanges the current device, returning the previous one.
/// ice: replaces THXPModule_exchangeDevice_wrap.
int sycl_exchange_device(int device_index);

/// Synchronises all queues on the given device.
/// ice: replaces THXPModule_xpuSynchronize / c10::xpu::syncStreamsOnDevice().
void sycl_synchronize_device(int device_index);

/// Returns the current SYCL queue for the given device.
/// ice: replaces THXPModule_getCurrentStream_raw; plugin_data is sycl::queue*.
TF_Stream sycl_get_current_stream(int device_index);

/// Installs a SYCL queue (from external source) as the current stream.
/// ice: replaces THXPModule_setStream_wrap / at::xpu::setCurrentXPUStream().
void sycl_set_current_stream(TF_Stream stream);

} // namespace ice::sycl_plugin
