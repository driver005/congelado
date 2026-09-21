// ice: SyclPluggableAllocator.h — replaces torch/csrc/xpu/XPUPluggableAllocator.h
//
// Original file: torch/csrc/xpu/XPUPluggableAllocator.h
// Original purpose: Declared XPUPluggableAllocator — a C++ class inheriting
//   c10::xpu::XPUCachingAllocator::XPUAllocator — that allowed callers to
//   supply custom malloc/free function pointers at runtime (e.g., from Python
//   via _xpu_customAllocator).  Python could swap the global allocator via
//   changeCurrentAllocator().
//
// ice replacement: A custom allocator integrates through the TF_ExecutorOps
//   vtable — specifically the `allocate` and `deallocate` function pointers.
//   A plugin author fills in a SyclPluggableAllocatorVtable and registers it
//   by calling sycl_pluggable_allocator_install().
//
// Key type mapping:
//   c10::DeviceIndex           → int
//   sycl::queue*               → sycl::queue* (unchanged — SYCL is kept)
//   c10::xpu::MempoolId_t      → struct PoolId { uint64_t hi, lo; }
//   TORCH_PYTHON_API / TORCH_XPU_API → (removed; TF_CAPI_EXPORT used instead)
//   C10_DISABLE_COPY_AND_ASSIGN → deleted copy/assign declared manually
//
// No Python.h, no pybind11, no torch headers.

#pragma once

#include <sycl/sycl.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/executor.h"

namespace ice::sycl_alloc {

// ---------------------------------------------------------------------------
// PoolId — replaces c10::xpu::MempoolId_t (std::pair<uint64_t,uint64_t>)
// ice: MempoolId_t → struct PoolId { uint64_t hi, lo; }
// ---------------------------------------------------------------------------
struct PoolId {
    uint64_t hi{0};
    uint64_t lo{0};

    bool operator==(const PoolId& o) const noexcept {
        return hi == o.hi && lo == o.lo;
    }
};

// ---------------------------------------------------------------------------
// AllocationMetadata — bookkeeping for custom-allocator owned blocks.
// ice: replaces torch::xpu::XPUPluggableAllocator::_AllocationMetadata.
//   c10::DeviceIndex → int
// ---------------------------------------------------------------------------
struct AllocationMetadata {
    size_t       size{0};
    int          device_index{-1};
    sycl::queue* queue{nullptr};   // queue active at time of allocation
};

// ---------------------------------------------------------------------------
// SyclPluggableAllocatorVtable — caller-supplied function pointers.
//
// This is the ice analogue of the std::function<> members in the original
// XPUPluggableAllocator.  The vtable is embedded into TF_ExecutorOps so the
// host runtime never calls any C++ virtual methods.
//
// ice: replaces alloc_fn_ / free_fn_ / init_fn_ / record_stream_fn_.
// ---------------------------------------------------------------------------
struct SyclPluggableAllocatorVtable {
    /// Allocate `size` bytes on `device_index` using the given SYCL queue.
    /// Must return a non-null pointer on success; on failure set *out_status.
    /// ice: replaces std::function<void*(size_t, int, sycl::queue*)> alloc_fn_.
    void* (*alloc)(size_t size, int device_index, sycl::queue* queue,
                   TF_Status* out_status){nullptr};

    /// Free a block previously returned by alloc.
    /// ice: replaces std::function<void(void*, size_t, int, sycl::queue*)> free_fn_.
    void (*free)(void* ptr, size_t size, int device_index,
                 sycl::queue* queue){nullptr};

    /// Optional: called once when the allocator is first used.
    /// ice: replaces std::function<void(int)> init_fn_.
    void (*init)(int device_count){nullptr};

    /// Optional: hint that `ptr` will be used on `queue` beyond its lifetime
    /// as the current stream.
    /// ice: replaces std::function<void(void*, sycl::queue*)> record_stream_fn_.
    void (*record_stream)(void* ptr, sycl::queue* queue){nullptr};
};

// ---------------------------------------------------------------------------
// SyclPluggableAllocator — C++ object that wraps the vtable and keeps
// AllocationMetadata for every live block (needed by free()).
//
// ice: replaces class XPUPluggableAllocator : XPUAllocator.
//   Instead of inheriting a virtual-method base class, the allocator exposes
//   the TF_ExecutorOps-compatible free functions below.
// ---------------------------------------------------------------------------
class SyclPluggableAllocator {
public:
    explicit SyclPluggableAllocator(SyclPluggableAllocatorVtable vtable)
        : vtable_(vtable) {}

    // Non-copyable, non-movable (matches C10_DISABLE_COPY_AND_ASSIGN intent).
    SyclPluggableAllocator(const SyclPluggableAllocator&)            = delete;
    SyclPluggableAllocator& operator=(const SyclPluggableAllocator&) = delete;

    /// Allocate, recording metadata for the returned block.
    /// ice: replaces XPUPluggableAllocator::malloc() + allocate().
    void* alloc(size_t size, int device_index, sycl::queue* queue,
                TF_Status* out_status);

    /// Free a block.  Looks up metadata to supply size/queue to vtable_.free.
    /// ice: replaces XPUPluggableAllocator::raw_delete().
    void free(void* ptr, TF_Status* out_status);

    /// Called once by the plugin init path; delegates to vtable_.init.
    /// ice: replaces XPUPluggableAllocator::init(device_count).
    void init(int device_count);

    bool initialized() const noexcept { return initialized_; }

    /// Copies `count` bytes between two device pointers using the current queue.
    /// ice: replaces XPUPluggableAllocator::copy_data().
    void copy_data(void* dest, const void* src, size_t count,
                   sycl::queue* queue, TF_Status* out_status) const;

    /// Hint: `ptr` will be used on `queue` after the current stream retires.
    /// ice: replaces XPUPluggableAllocator::recordStream().
    void record_stream(void* ptr, sycl::queue* queue);

private:
    SyclPluggableAllocatorVtable vtable_;
    mutable std::mutex           mutex_;
    std::unordered_map<void*, AllocationMetadata> metadata_;
    bool initialized_{false};
    int  device_count_{0};
};

// ---------------------------------------------------------------------------
// Global registry — one pluggable allocator per plugin .so at a time.
// ---------------------------------------------------------------------------

/// Returns the currently installed custom allocator, or nullptr if none.
/// ice: replaces torch::xpu::XPUPluggableAllocator::getCurrentAllocator().
TF_CAPI_EXPORT
SyclPluggableAllocator* sycl_pluggable_allocator_current();

/// Creates and installs a custom allocator from function pointers.
/// The returned object is owned by the plugin; call
/// sycl_pluggable_allocator_uninstall() to remove it.
///
/// ice: replaces createCustomAllocator() + changeCurrentAllocator().
///   Original validated "not yet initialized" via XPUCachingAllocator::get().
///   Here the precondition is: sycl_pluggable_allocator_current() == nullptr.
TF_CAPI_EXPORT
SyclPluggableAllocator* sycl_pluggable_allocator_install(
    SyclPluggableAllocatorVtable vtable,
    int device_count,
    TF_Status* out_status);

/// Removes the currently installed custom allocator.
/// Must be called before the plugin .so is dlclose'd.
TF_CAPI_EXPORT
void sycl_pluggable_allocator_uninstall();

// ---------------------------------------------------------------------------
// TF_ExecutorOps shim helpers (called from SyclPlugin.cpp)
// ---------------------------------------------------------------------------
//
// These adapters bridge between the ice C-ABI (TF_Executor*, TF_Device*, ...)
// and SyclPluggableAllocator so the executor vtable pointers can delegate:
//
//   executor_ops.allocate   → sycl_exec_alloc()
//   executor_ops.deallocate → sycl_exec_free()
//
// ice: these replace the indirect calls through c10::SetAllocator / c10::kXPU.

void sycl_exec_alloc(TF_Executor* executor, TF_Device* device,
                      uint64_t size, int64_t memory_space,
                      TF_DeviceMemoryBase* mem);

void sycl_exec_free(TF_Executor* executor, TF_Device* device,
                     TF_DeviceMemoryBase* memory);

} // namespace ice::sycl_alloc
