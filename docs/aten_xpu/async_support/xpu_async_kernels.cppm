// SYCL Executor Backend — ice::builder::TF_ExecutorOps implementation
// Maps the full StreamExecutor vtable to AdaptiveCpp SYCL primitives.
// NO ATen / c10 / PyTorch dependencies.

module;

#include <sycl/sycl.hpp>
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/device.h"

export module cc_ice_sycl_backend:executor;

import std;
import cc_ice_builder_stream_executor:executor;
import cc_ice_builder_intern:status;
import cc_ice_sycl_backend:tensor;   // AsyncResult, XPU_AsyncEvent

export namespace ice::builder {

// ---------------------------------------------------------------------------
// SyclStreamHandle — wraps a sycl::queue* stored in TF_Stream::plugin_data
// ---------------------------------------------------------------------------
// Convention (matching StreamExecutor plugin contract):
//   TF_Stream::plugin_data → sycl::queue* (owned by SyclExecutorImpl's pool)
//   TF_Event::plugin_data  → sycl::event* (heap-allocated)
//   TF_Timer::plugin_data  → std::pair<sycl::event,sycl::event>* (start/stop)

namespace detail {

[[nodiscard]] inline sycl::queue* stream_queue(TF_Stream* s) noexcept {
    return static_cast<sycl::queue*>(s->plugin_data);
}

[[nodiscard]] inline sycl::event* event_native(TF_Event* e) noexcept {
    return static_cast<sycl::event*>(e->plugin_data);
}

[[nodiscard]] inline void* usm_ptr(const TF_DeviceMemoryBase* m) noexcept {
    return m->opaque;
}

[[nodiscard]] inline void* usm_ptr(TF_DeviceMemoryBase* m) noexcept {
    return m->opaque;
}

} // namespace detail

// ---------------------------------------------------------------------------
// SyclExecutorImpl — owns a sycl::device and queue pool; implements the
//                    full TF_ExecutorOps abstract interface.
// ---------------------------------------------------------------------------
class SyclExecutorImpl : public TF_ExecutorOps {
public:
    // Construct from a chosen SYCL device (e.g. gpu_selector_v or cpu_selector_v).
    explicit SyclExecutorImpl(const sycl::device& dev)
        : device_(dev),
          default_context_(dev),
          default_queue_(default_context_, dev,
                         sycl::property::queue::in_order{}) {}

    ~SyclExecutorImpl() override = default;

    // ---- Device memory (USM device_alloc) ---------------------------------

    [[nodiscard]] std::expected<void, ice::Status> allocate(
        const ice::sonic::TF_DeviceOps& /*device*/,
        uint64_t size,
        int64_t  /*memory_space*/,
        TF_DeviceMemoryBase* mem) noexcept override {
        try {
            mem->opaque = sycl::malloc_device(size, default_queue_);
            mem->size   = size;
            if (!mem->opaque)
                return std::unexpected(ice::Status::from_message("sycl::malloc_device OOM"));
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> deallocate(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_DeviceMemoryBase* memory) noexcept override {
        try {
            sycl::free(memory->opaque, default_queue_);
            memory->opaque = nullptr;
            memory->size   = 0;
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Host-pinned memory (USM host_alloc) ------------------------------

    [[nodiscard]] std::expected<void, ice::Status> host_memory_allocate(
        const ice::sonic::TF_DeviceOps& /*device*/,
        uint64_t size,
        void**   out_mem) noexcept override {
        try {
            *out_mem = sycl::malloc_host(size, default_queue_);
            if (!*out_mem)
                return std::unexpected(ice::Status::from_message("sycl::malloc_host OOM"));
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> host_memory_deallocate(
        const ice::sonic::TF_DeviceOps& /*device*/,
        void* mem) noexcept override {
        try {
            sycl::free(mem, default_queue_);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Shared (USM shared_alloc) ----------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> unified_memory_allocate(
        const ice::sonic::TF_DeviceOps& /*device*/,
        uint64_t bytes,
        void**   out_location) noexcept override {
        try {
            *out_location = sycl::malloc_shared(bytes, default_queue_);
            if (!*out_location)
                return std::unexpected(ice::Status::from_message("sycl::malloc_shared OOM"));
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> unified_memory_deallocate(
        const ice::sonic::TF_DeviceOps& /*device*/,
        void* location) noexcept override {
        try {
            sycl::free(location, default_queue_);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Allocator stats -------------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> get_allocator_stats(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_AllocatorStats* stats,
        _Bool* out_success) noexcept override {
        // SYCL USM does not expose a caching allocator with stats natively.
        // Populate with sentinel values so callers can detect "unsupported".
        if (stats) {
            stats->bytes_in_use       = -1;
            stats->peak_bytes_in_use  = -1;
            stats->largest_alloc_size = -1;
            stats->bytes_limit        = -1;
        }
        if (out_success) *out_success = false;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> device_memory_usage(
        const ice::sonic::TF_DeviceOps& /*device*/,
        int64_t* out_free,
        int64_t* out_total,
        _Bool* out_success) noexcept override {
        try {
            // query via SYCL device info (not universally available)
            auto total = static_cast<int64_t>(
                device_.get_info<sycl::info::device::global_mem_size>());
            if (out_total)   *out_total   = total;
            if (out_free)    *out_free    = -1;  // free not directly queryable
            if (out_success) *out_success = true;
            return {};
        } catch (const sycl::exception& e) {
            if (out_success) *out_success = false;
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Stream management -----------------------------------------------
    //
    // Each TF_Stream wraps a heap-allocated sycl::queue (in-order by default).

    [[nodiscard]] std::expected<void, ice::Status> create_stream_internal(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream) noexcept override {
        try {
            auto* q = new sycl::queue(
                default_context_, device_,
                sycl::property::queue::in_order{});
            stream->plugin_data = q;
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_stream_internal(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream) noexcept override {
        delete detail::stream_queue(stream);
        stream->plugin_data = nullptr;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> create_stream_dependency(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* dependent,
        TF_Stream* other) noexcept override {
        // Implement a cross-queue dependency using a barrier event.
        try {
            auto* dep_q   = detail::stream_queue(dependent);
            auto* other_q = detail::stream_queue(other);
            auto  barrier = other_q->ext_oneapi_submit_barrier();
            dep_q->submit([&](sycl::handler& h) {
                h.depends_on(barrier);
                h.single_task([]{});
            });
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> get_stream_status(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream) noexcept override {
        try {
            // An in-order queue is "done" if no pending work — submit a no-op
            // and wait to confirm the queue is drained.
            detail::stream_queue(stream)->wait();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> create_stream_with_options(
        const ice::sonic::TF_DeviceOps& device,
        const TF_StreamOptions* /*options*/,
        TF_Stream* stream) noexcept override {
        // Options (priority, etc.) are not portable across SYCL implementations;
        // fall back to the default in-order stream.
        return create_stream_internal(device, stream);
    }

    // ---- Event management ------------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> create_event_internal(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Event* event) noexcept override {
        try {
            event->plugin_data = new sycl::event();
            return {};
        } catch (...) {
            return std::unexpected(ice::Status::from_message("failed to allocate sycl::event"));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_event_internal(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Event* event) noexcept override {
        delete detail::event_native(event);
        event->plugin_data = nullptr;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_event_status(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Event* event,
        TF_EventStatus* out_status) noexcept override {
        auto* ev = detail::event_native(event);
        auto  cs = ev->get_info<sycl::info::event::command_execution_status>();
        *out_status = (cs == sycl::info::event_command_status::complete)
                      ? TF_EventStatus::TF_EVENT_COMPLETE
                      : TF_EventStatus::TF_EVENT_PENDING;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> record_event(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_Event* event) noexcept override {
        try {
            auto* q  = detail::stream_queue(stream);
            auto* ev = detail::event_native(event);
            *ev = q->ext_oneapi_submit_barrier();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> wait_for_event(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_Event* event) noexcept override {
        try {
            auto* q  = detail::stream_queue(stream);
            auto* ev = detail::event_native(event);
            q->submit([&](sycl::handler& h) {
                h.depends_on(*ev);
                h.single_task([]{});
            });
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Timer management ------------------------------------------------
    //
    // SYCL timers are modelled as a pair<sycl::event, sycl::event> (start, stop).

    [[nodiscard]] std::expected<void, ice::Status> create_timer_internal(
        const ice::sonic::TF_DeviceOps& /*device*/,
        const ice::sonic::TF_TimerOps& timer) noexcept override {
        using Pair = std::pair<sycl::event, sycl::event>;
        timer.handle()->plugin_data = new Pair();
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_timer_internal(
        const ice::sonic::TF_DeviceOps& /*device*/,
        const ice::sonic::TF_TimerOps& timer) noexcept override {
        using Pair = std::pair<sycl::event, sycl::event>;
        delete static_cast<Pair*>(timer.handle()->plugin_data);
        timer.handle()->plugin_data = nullptr;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> start_timer(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        const ice::sonic::TF_TimerOps& timer) noexcept override {
        using Pair = std::pair<sycl::event, sycl::event>;
        try {
            auto* pair = static_cast<Pair*>(timer.handle()->plugin_data);
            pair->first = detail::stream_queue(stream)->ext_oneapi_submit_barrier();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> stop_timer(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        const ice::sonic::TF_TimerOps& timer) noexcept override {
        using Pair = std::pair<sycl::event, sycl::event>;
        try {
            auto* pair = static_cast<Pair*>(timer.handle()->plugin_data);
            pair->second = detail::stream_queue(stream)->ext_oneapi_submit_barrier();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Memcpy: async (stream-ordered) ----------------------------------

    [[nodiscard]] std::expected<void, ice::Status> memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size) noexcept override {
        try {
            detail::stream_queue(stream)->memcpy(host_dst, detail::usm_ptr(device_src), size);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> memcpy_htod(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size) noexcept override {
        try {
            detail::stream_queue(stream)->memcpy(detail::usm_ptr(device_dst), host_src, size);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> memcpy_dtod(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size) noexcept override {
        try {
            detail::stream_queue(stream)->memcpy(
                detail::usm_ptr(device_dst),
                detail::usm_ptr(device_src),
                size);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Memcpy: synchronous --------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> sync_memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& /*device*/,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size) noexcept override {
        try {
            default_queue_.memcpy(host_dst, detail::usm_ptr(device_src), size).wait();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> sync_memcpy_htod(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size) noexcept override {
        try {
            default_queue_.memcpy(detail::usm_ptr(device_dst), host_src, size).wait();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> sync_memcpy_dtod(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size) noexcept override {
        try {
            default_queue_.memcpy(
                detail::usm_ptr(device_dst),
                detail::usm_ptr(device_src),
                size).wait();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Host synchronization -------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> block_host_for_event(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Event* event) noexcept override {
        try {
            detail::event_native(event)->wait();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> block_host_until_done(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream) noexcept override {
        try {
            detail::stream_queue(stream)->wait_and_throw();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> synchronize_all_activity(
        const ice::sonic::TF_DeviceOps& /*device*/) noexcept override {
        try {
            default_queue_.wait_and_throw();
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Memset ---------------------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> mem_zero(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_DeviceMemoryBase* location,
        uint64_t size) noexcept override {
        try {
            detail::stream_queue(stream)->memset(detail::usm_ptr(location), 0, size);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> memset(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size) noexcept override {
        try {
            detail::stream_queue(stream)->memset(
                detail::usm_ptr(location), static_cast<int>(pattern), size);
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    [[nodiscard]] std::expected<void, ice::Status> memset32(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size) noexcept override {
        // SYCL memset only supports byte patterns; fill 32-bit pattern manually.
        try {
            auto* q   = detail::stream_queue(stream);
            auto* ptr = static_cast<uint32_t*>(detail::usm_ptr(location));
            size_t count = size / sizeof(uint32_t);
            q->submit([&](sycl::handler& h) {
                h.parallel_for(sycl::range<1>(count), [=](sycl::id<1> idx) {
                    ptr[idx] = pattern;
                });
            });
            return {};
        } catch (const sycl::exception& e) {
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Host callback ---------------------------------------------------

    [[nodiscard]] std::expected<void, ice::Status> host_callback(
        const ice::sonic::TF_DeviceOps& /*device*/,
        TF_Stream* stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        _Bool* out_success) noexcept override {
        try {
            // Submit a host task after current in-order queue drains.
            detail::stream_queue(stream)->submit([&](sycl::handler& h) {
                h.host_task([=]() {
                    callback_fn(callback_arg, nullptr);  // nullptr = OK status
                });
            });
            if (out_success) *out_success = true;
            return {};
        } catch (const sycl::exception& e) {
            if (out_success) *out_success = false;
            return std::unexpected(ice::Status::from_message(e.what()));
        }
    }

    // ---- Accessors -------------------------------------------------------

    [[nodiscard]] sycl::queue&   default_queue()   noexcept { return default_queue_; }
    [[nodiscard]] sycl::device&  sycl_device()     noexcept { return device_; }
    [[nodiscard]] sycl::context& sycl_context()    noexcept { return default_context_; }

private:
    sycl::device  device_;
    sycl::context default_context_;
    sycl::queue   default_queue_;
};

// ---------------------------------------------------------------------------
// Factory helpers
// ---------------------------------------------------------------------------

// Create a SYCL executor targeting the first GPU found (falls back to CPU).
[[nodiscard]] inline SyclExecutorImpl* make_gpu_executor() {
    try {
        return new SyclExecutorImpl(sycl::device(sycl::gpu_selector_v));
    } catch (const sycl::exception&) {
        return new SyclExecutorImpl(sycl::device(sycl::cpu_selector_v));
    }
}

// Create a SYCL executor explicitly targeting the CPU.
[[nodiscard]] inline SyclExecutorImpl* make_cpu_executor() {
    return new SyclExecutorImpl(sycl::device(sycl::cpu_selector_v));
}

// Create a SYCL executor for a specific platform-local device ordinal.
[[nodiscard]] inline SyclExecutorImpl* make_executor_for_device(const sycl::device& dev) {
    return new SyclExecutorImpl(dev);
}

} // namespace ice::builder