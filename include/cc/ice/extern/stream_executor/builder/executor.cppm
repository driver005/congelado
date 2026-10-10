// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:executor;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ExecutorOps
{
public:
    explicit TF_ExecutorOps(
        const ::TF_AllocatorOps* TF_AllocatorOps_ops,
        const ::TF_DeviceOps* TF_DeviceOps_ops,
        const ::TF_EventOps* TF_EventOps_ops,
        const ::TF_RandomGeneratorOps* TF_RandomGeneratorOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StreamOps* TF_StreamOps_ops,
        const ::TF_TimerOps* TF_TimerOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_AllocatorOps_ops = TF_AllocatorOps_ops;
        m_TF_DeviceOps_ops = TF_DeviceOps_ops;
        m_TF_EventOps_ops = TF_EventOps_ops;
        m_TF_RandomGeneratorOps_ops = TF_RandomGeneratorOps_ops;
        m_Status_ops = Status_ops;
        m_TF_StreamOps_ops = TF_StreamOps_ops;
        m_TF_TimerOps_ops = TF_TimerOps_ops;
    }

    TF_ExecutorOps(const TF_ExecutorOps&) = delete;
    TF_ExecutorOps& operator=(const TF_ExecutorOps&) = delete;

    static TF_ExecutorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ExecutorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ExecutorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ExecutorOps*>(handle->plugin_data);
    }

    virtual ~TF_ExecutorOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void device_memory_usage(
        const ice::sonic::TF_DeviceOps& device,
        int64_t* out_free,
        int64_t* out_total,
        _Bool* out_success
    ) noexcept = 0;
    virtual void create_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    virtual void create_stream_dependency(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& dependent,
        const ice::sonic::TF_StreamOps& other,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stream_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event
    ) noexcept = 0;
    virtual void get_event_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        TF_EventStatus* out_event_status
    ) noexcept = 0;
    virtual void record_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void wait_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    virtual void start_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void stop_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void sync_memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void sync_memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void sync_memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void block_host_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void block_host_until_done(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void synchronize_all_activity(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void mem_zero(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memset(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memset32(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void host_callback(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        _Bool* out_success
    ) noexcept = 0;
    virtual void create_stream_with_options(
        const ice::sonic::TF_DeviceOps& device,
        const TF_StreamOptions* options,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stream_from_pool(
        const ice::sonic::TF_DeviceOps& device,
        int32_t priority,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_current_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_current_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_stream_from_native(
        const ice::sonic::TF_DeviceOps& device,
        void* native_handle,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_event_with_options_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_EventOptions* options,
        const ice::sonic::TF_EventOps& out_event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_event_from_ipc_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_IpcEventHandle* handle,
        const ice::sonic::TF_EventOps& out_event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& out_allocator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& allocator
    ) noexcept = 0;
    virtual void create_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t seed,
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& generator
    ) noexcept = 0;
    virtual void get_default_random_generator(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_native_handle(const ice::sonic::TF_DeviceOps& device, void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Executor*)) noexcept
    {
        m_vtable = ::TF_ExecutorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ExecutorOps, get_native_handle),

            .create = create,
            .destroy =
                [](TF_Executor* handle) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(handle);
                self.destroy();
            },
            .device_memory_usage =
                [](TF_Executor* executor,
                   TF_Device* device,
                   int64_t* out_free,
                   int64_t* out_total,
                   _Bool* out_success) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.device_memory_usage(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    out_free,
                    out_total,
                    out_success
                );
            },
            .create_stream_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_stream_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_stream_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Stream* stream) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.destroy_stream_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream)
                );
            },
            .create_stream_dependency =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* dependent,
                   TF_Stream* other,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_stream_dependency(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, dependent),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, other),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stream_status =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.get_stream_status(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_event_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_event_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_event_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Event* event) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.destroy_event_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, event)
                );
            },
            .get_event_status =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Event* event,
                   TF_EventStatus* out_event_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.get_event_status(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, event),
                    out_event_status
                );
            },
            .record_event =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.record_event(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .wait_for_event =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.wait_for_event(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_timer_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Timer* timer,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_timer_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_TimerOps>{}, timer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_timer_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Timer* timer) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.destroy_timer_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_TimerOps>{}, timer)
                );
            },
            .start_timer =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Timer* timer,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.start_timer(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::TF_TimerOps>{}, timer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .stop_timer =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Timer* timer,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.stop_timer(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::TF_TimerOps>{}, timer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memcpy_dtoh =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   void* host_dst,
                   const TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.memcpy_dtoh(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    host_dst,
                    device_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memcpy_htod =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* device_dst,
                   const void* host_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.memcpy_htod(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    device_dst,
                    host_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memcpy_dtod =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* device_dst,
                   const TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.memcpy_dtod(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    device_dst,
                    device_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .sync_memcpy_dtoh =
                [](TF_Executor* executor,
                   TF_Device* device,
                   void* host_dst,
                   const TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.sync_memcpy_dtoh(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    host_dst,
                    device_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .sync_memcpy_htod =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_DeviceMemoryBase* device_dst,
                   const void* host_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.sync_memcpy_htod(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    device_dst,
                    host_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .sync_memcpy_dtod =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_DeviceMemoryBase* device_dst,
                   const TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.sync_memcpy_dtod(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    device_dst,
                    device_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .block_host_for_event =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.block_host_for_event(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .block_host_until_done =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.block_host_until_done(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .synchronize_all_activity =
                [](TF_Executor* executor, TF_Device* device, TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.synchronize_all_activity(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .mem_zero =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* location,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.mem_zero(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    location,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memset =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* location,
                   uint8_t pattern,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.memset(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    location,
                    pattern,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memset32 =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* location,
                   uint32_t pattern,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.memset32(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    location,
                    pattern,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .host_callback =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_StatusCallbackFn callback_fn,
                   void* callback_arg,
                   _Bool* out_success) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.host_callback(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    callback_fn,
                    callback_arg,
                    out_success
                );
            },
            .create_stream_with_options =
                [](TF_Executor* executor,
                   TF_Device* device,
                   const TF_StreamOptions* options,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_stream_with_options(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    options,
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stream_from_pool =
                [](TF_Executor* executor,
                   TF_Device* device,
                   int32_t priority,
                   TF_Stream* out_stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.get_stream_from_pool(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    priority,
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, out_stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_current_stream =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* out_stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.get_current_stream(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, out_stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_current_stream =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.set_current_stream(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_stream_from_native =
                [](TF_Executor* executor,
                   TF_Device* device,
                   void* native_handle,
                   TF_Stream* out_stream,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_stream_from_native(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    native_handle,
                    self.wrap(std::type_identity<ice::sonic::TF_StreamOps>{}, out_stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_event_with_options_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   const TF_EventOptions* options,
                   TF_Event* out_event,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_event_with_options_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    options,
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, out_event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_event_from_ipc_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   const TF_IpcEventHandle* handle,
                   TF_Event* out_event,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_event_from_ipc_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    handle,
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, out_event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_allocator_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Allocator* out_allocator,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_allocator_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_AllocatorOps>{}, out_allocator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_allocator_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Allocator* allocator) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.destroy_allocator_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_AllocatorOps>{}, allocator)
                );
            },
            .create_random_generator_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   uint64_t seed,
                   TF_RandomGenerator* out_generator,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.create_random_generator_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    seed,
                    self.wrap(
                        std::type_identity<ice::sonic::TF_RandomGeneratorOps>{},
                        out_generator
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_random_generator_internal =
                [](TF_Executor* executor, TF_Device* device, TF_RandomGenerator* generator) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.destroy_random_generator_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::TF_RandomGeneratorOps>{}, generator)
                );
            },
            .get_default_random_generator =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_RandomGenerator* out_generator,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.get_default_random_generator(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(
                        std::type_identity<ice::sonic::TF_RandomGeneratorOps>{},
                        out_generator
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_native_handle =
                [](TF_Executor* executor, TF_Device* device, void** out_handle) noexcept
            {
                auto& self = TF_ExecutorOps::from_handle(executor);
                self.get_native_handle(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    out_handle
                );
            },

        };
    }

    ice::sonic::TF_AllocatorOps wrap(
        std::type_identity<ice::sonic::TF_AllocatorOps>,
        const ::TF_Allocator* handle
    ) const noexcept
    {
        return ice::sonic::TF_AllocatorOps{
            m_TF_AllocatorOps_ops,
            const_cast<::TF_Allocator*>(handle)
        };
    }

    ice::sonic::TF_DeviceOps
    wrap(std::type_identity<ice::sonic::TF_DeviceOps>, const ::TF_Device* handle) const noexcept
    {
        return ice::sonic::TF_DeviceOps{m_TF_DeviceOps_ops, const_cast<::TF_Device*>(handle)};
    }

    ice::sonic::TF_EventOps
    wrap(std::type_identity<ice::sonic::TF_EventOps>, const ::TF_Event* handle) const noexcept
    {
        return ice::sonic::TF_EventOps{m_TF_EventOps_ops, const_cast<::TF_Event*>(handle)};
    }

    ice::sonic::TF_RandomGeneratorOps wrap(
        std::type_identity<ice::sonic::TF_RandomGeneratorOps>,
        const ::TF_RandomGenerator* handle
    ) const noexcept
    {
        return ice::sonic::TF_RandomGeneratorOps{
            m_TF_RandomGeneratorOps_ops,
            const_cast<::TF_RandomGenerator*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::TF_StreamOps
    wrap(std::type_identity<ice::sonic::TF_StreamOps>, const ::TF_Stream* handle) const noexcept
    {
        return ice::sonic::TF_StreamOps{m_TF_StreamOps_ops, const_cast<::TF_Stream*>(handle)};
    }

    ice::sonic::TF_TimerOps
    wrap(std::type_identity<ice::sonic::TF_TimerOps>, const ::TF_Timer* handle) const noexcept
    {
        return ice::sonic::TF_TimerOps{m_TF_TimerOps_ops, const_cast<::TF_Timer*>(handle)};
    }

    const ::TF_ExecutorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Executor& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_ExecutorOps*>(&m_vtable)
        );
    }

private:
    ::TF_ExecutorOps m_vtable;
    ::TF_Executor m_handle;

    const ::TF_AllocatorOps* m_TF_AllocatorOps_ops{nullptr};

    const ::TF_DeviceOps* m_TF_DeviceOps_ops{nullptr};

    const ::TF_EventOps* m_TF_EventOps_ops{nullptr};

    const ::TF_RandomGeneratorOps* m_TF_RandomGeneratorOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StreamOps* m_TF_StreamOps_ops{nullptr};

    const ::TF_TimerOps* m_TF_TimerOps_ops{nullptr};
};

} // namespace ice::builder
