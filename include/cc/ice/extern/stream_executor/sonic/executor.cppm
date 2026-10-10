// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/executor.h"

export module cc_ice_extern_stream_executor_sonic:executor;

import std;
import :allocator;
import :device;
import :event;
import :stream;
import :timer;
import cc_ice_extern_random_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ExecutorOps : public ice::sonic::Runtime<::TF_ExecutorOps, ::TF_Executor>
{
public:
    TF_ExecutorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_ExecutorOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Executor* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_ExecutorOps(const ::TF_ExecutorOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ExecutorOps(const ::TF_ExecutorOps* ops, ::TF_Executor* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void device_memory_usage(
        const ice::sonic::TF_DeviceOps& device,
        int64_t* out_free,
        int64_t* out_total,
        _Bool* out_success
    ) const noexcept
    {
        m_ops->device_memory_usage(
            get_handle(),
            device.get_handle(),
            out_free,
            out_total,
            out_success
        );
    }

    void create_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_stream_internal(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) const noexcept
    {
        m_ops->destroy_stream_internal(get_handle(), device.get_handle(), stream.get_handle());
    }

    void create_stream_dependency(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& dependent,
        const ice::sonic::TF_StreamOps& other,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_stream_dependency(
            get_handle(),
            device.get_handle(),
            dependent.get_handle(),
            other.get_handle(),
            out_status.get_handle()
        );
    }

    void get_stream_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_stream_status(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            out_status.get_handle()
        );
    }

    void create_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_event_internal(
            get_handle(),
            device.get_handle(),
            event.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event
    ) const noexcept
    {
        m_ops->destroy_event_internal(get_handle(), device.get_handle(), event.get_handle());
    }

    void get_event_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        TF_EventStatus* out_event_status
    ) const noexcept
    {
        m_ops->get_event_status(
            get_handle(),
            device.get_handle(),
            event.get_handle(),
            out_event_status
        );
    }

    void record_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->record_event(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            event.get_handle(),
            out_status.get_handle()
        );
    }

    void wait_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->wait_for_event(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            event.get_handle(),
            out_status.get_handle()
        );
    }

    void create_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_timer_internal(
            get_handle(),
            device.get_handle(),
            timer.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer
    ) const noexcept
    {
        m_ops->destroy_timer_internal(get_handle(), device.get_handle(), timer.get_handle());
    }

    void start_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->start_timer(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            timer.get_handle(),
            out_status.get_handle()
        );
    }

    void stop_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->stop_timer(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            timer.get_handle(),
            out_status.get_handle()
        );
    }

    void memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->memcpy_dtoh(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            host_dst,
            device_src,
            size,
            out_status.get_handle()
        );
    }

    void memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->memcpy_htod(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            device_dst,
            host_src,
            size,
            out_status.get_handle()
        );
    }

    void memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->memcpy_dtod(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            device_dst,
            device_src,
            size,
            out_status.get_handle()
        );
    }

    void sync_memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->sync_memcpy_dtoh(
            get_handle(),
            device.get_handle(),
            host_dst,
            device_src,
            size,
            out_status.get_handle()
        );
    }

    void sync_memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->sync_memcpy_htod(
            get_handle(),
            device.get_handle(),
            device_dst,
            host_src,
            size,
            out_status.get_handle()
        );
    }

    void sync_memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->sync_memcpy_dtod(
            get_handle(),
            device.get_handle(),
            device_dst,
            device_src,
            size,
            out_status.get_handle()
        );
    }

    void block_host_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->block_host_for_event(
            get_handle(),
            device.get_handle(),
            event.get_handle(),
            out_status.get_handle()
        );
    }

    void block_host_until_done(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->block_host_until_done(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            out_status.get_handle()
        );
    }

    void synchronize_all_activity(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->synchronize_all_activity(get_handle(), device.get_handle(), out_status.get_handle());
    }

    void mem_zero(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->mem_zero(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            location,
            size,
            out_status.get_handle()
        );
    }

    void memset(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->memset(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            location,
            pattern,
            size,
            out_status.get_handle()
        );
    }

    void memset32(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->memset32(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            location,
            pattern,
            size,
            out_status.get_handle()
        );
    }

    void host_callback(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        _Bool* out_success
    ) const noexcept
    {
        m_ops->host_callback(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            callback_fn,
            callback_arg,
            out_success
        );
    }

    void create_stream_with_options(
        const ice::sonic::TF_DeviceOps& device,
        const TF_StreamOptions* options,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_stream_with_options(
            get_handle(),
            device.get_handle(),
            options,
            stream.get_handle(),
            out_status.get_handle()
        );
    }

    void get_stream_from_pool(
        const ice::sonic::TF_DeviceOps& device,
        int32_t priority,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_stream_from_pool(
            get_handle(),
            device.get_handle(),
            priority,
            out_stream.get_handle(),
            out_status.get_handle()
        );
    }

    void get_current_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_current_stream(
            get_handle(),
            device.get_handle(),
            out_stream.get_handle(),
            out_status.get_handle()
        );
    }

    void set_current_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_current_stream(
            get_handle(),
            device.get_handle(),
            stream.get_handle(),
            out_status.get_handle()
        );
    }

    void create_stream_from_native(
        const ice::sonic::TF_DeviceOps& device,
        void* native_handle,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_stream_from_native(
            get_handle(),
            device.get_handle(),
            native_handle,
            out_stream.get_handle(),
            out_status.get_handle()
        );
    }

    void create_event_with_options_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_EventOptions* options,
        const ice::sonic::TF_EventOps& out_event,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_event_with_options_internal(
            get_handle(),
            device.get_handle(),
            options,
            out_event.get_handle(),
            out_status.get_handle()
        );
    }

    void create_event_from_ipc_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_IpcEventHandle* handle,
        const ice::sonic::TF_EventOps& out_event,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_event_from_ipc_internal(
            get_handle(),
            device.get_handle(),
            handle,
            out_event.get_handle(),
            out_status.get_handle()
        );
    }

    void create_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& out_allocator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_allocator_internal(
            get_handle(),
            device.get_handle(),
            out_allocator.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& allocator
    ) const noexcept
    {
        m_ops
            ->destroy_allocator_internal(get_handle(), device.get_handle(), allocator.get_handle());
    }

    void create_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t seed,
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_random_generator_internal(
            get_handle(),
            device.get_handle(),
            seed,
            out_generator.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& generator
    ) const noexcept
    {
        m_ops->destroy_random_generator_internal(
            get_handle(),
            device.get_handle(),
            generator.get_handle()
        );
    }

    void get_default_random_generator(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_default_random_generator(
            get_handle(),
            device.get_handle(),
            out_generator.get_handle(),
            out_status.get_handle()
        );
    }

    void get_native_handle(const ice::sonic::TF_DeviceOps& device, void** out_handle) const noexcept
    {
        m_ops->get_native_handle(get_handle(), device.get_handle(), out_handle);
    }
};

} // namespace ice::sonic
