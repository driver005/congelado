// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/executor.h"

export module cc_ice_extern_stream_executor_builder:executor;

import std;

export namespace ice::builder {

class TF_ExecutorOps
{
public:
    TF_ExecutorOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status> device_memory_usage(
        const ice::sonic::TF_DeviceOps& device,
        int64_t* out_free,
        int64_t* out_total,
        _Bool* out_success
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> destroy_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_stream_dependency(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& dependent,
        const ice::sonic::TF_StreamOps& other
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_stream_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> destroy_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_event_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        TF_EventStatus* out_event_status
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> record_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> wait_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> destroy_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> start_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stop_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> sync_memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> sync_memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> sync_memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> block_host_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> block_host_until_done(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    synchronize_all_activity(const ice::sonic::TF_DeviceOps& device) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> mem_zero(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memset(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memset32(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> host_callback(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        _Bool* out_success
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_stream_with_options(
        const ice::sonic::TF_DeviceOps& device,
        const TF_StreamOptions* options,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_stream_from_pool(
        const ice::sonic::TF_DeviceOps& device,
        int32_t priority,
        const ice::sonic::TF_StreamOps& out_stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_current_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& out_stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_current_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_stream_from_native(
        const ice::sonic::TF_DeviceOps& device,
        void* native_handle,
        const ice::sonic::TF_StreamOps& out_stream
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_event_with_options_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_EventOptions* options,
        const ice::sonic::TF_EventOps& out_event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_event_from_ipc_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_IpcEventHandle* handle,
        const ice::sonic::TF_EventOps& out_event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& out_allocator
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> destroy_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& allocator
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t seed,
        const ice::sonic::TF_RandomGeneratorOps& out_generator
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> destroy_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& generator
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_default_random_generator(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& out_generator
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_native_handle(const ice::sonic::TF_DeviceOps& device, void** out_handle) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ExecutorOps{
            .struct_size = TF_EXECUTOR_STRUCT_SIZE,
            .device_memory_usage =
                [](TF_Executor* executor,
                   TF_Device* device,
                   int64_t* out_free,
                   int64_t* out_total,
                   _Bool* out_success) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).device_memory_usage(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    out_free,
                    out_total,
                    out_success
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_stream_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_stream_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_stream_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Stream* stream) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).destroy_stream_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_stream_dependency =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* dependent,
                   TF_Stream* other,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_stream_dependency(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(dependent),
                    ice::sonic::TF_StreamOps::wrap(other)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stream_status =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).get_stream_status(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_event_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_event_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_EventOps::wrap(event)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_event_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Event* event) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).destroy_event_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_EventOps::wrap(event)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_event_status =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Event* event,
                   TF_EventStatus* out_event_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).get_event_status(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_EventOps::wrap(event),
                    out_event_status
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .record_event =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).record_event(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    ice::sonic::TF_EventOps::wrap(event)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .wait_for_event =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).wait_for_event(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    ice::sonic::TF_EventOps::wrap(event)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_timer_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Timer* timer,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_timer_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_TimerOps::wrap(timer)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_timer_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Timer* timer) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).destroy_timer_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_TimerOps::wrap(timer)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start_timer =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Timer* timer,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).start_timer(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    ice::sonic::TF_TimerOps::wrap(timer)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop_timer =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Timer* timer,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).stop_timer(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    ice::sonic::TF_TimerOps::wrap(timer)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TF_ExecutorOps::from_handle(executor).memcpy_dtoh(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    host_dst,
                    device_src,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TF_ExecutorOps::from_handle(executor).memcpy_htod(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    device_dst,
                    host_src,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TF_ExecutorOps::from_handle(executor).memcpy_dtod(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    device_dst,
                    device_src,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .sync_memcpy_dtoh =
                [](TF_Executor* executor,
                   TF_Device* device,
                   void* host_dst,
                   const TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).sync_memcpy_dtoh(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    host_dst,
                    device_src,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .sync_memcpy_htod =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_DeviceMemoryBase* device_dst,
                   const void* host_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).sync_memcpy_htod(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    device_dst,
                    host_src,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .sync_memcpy_dtod =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_DeviceMemoryBase* device_dst,
                   const TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).sync_memcpy_dtod(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    device_dst,
                    device_src,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .block_host_for_event =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Event* event,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).block_host_for_event(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_EventOps::wrap(event)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .block_host_until_done =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).block_host_until_done(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .synchronize_all_activity =
                [](TF_Executor* executor, TF_Device* device, TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).synchronize_all_activity(
                    ice::sonic::TF_DeviceOps::wrap(device)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .mem_zero =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_DeviceMemoryBase* location,
                   uint64_t size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).mem_zero(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    location,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TF_ExecutorOps::from_handle(executor).memset(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    location,
                    pattern,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
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
                auto res = TF_ExecutorOps::from_handle(executor).memset32(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    location,
                    pattern,
                    size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .host_callback =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_StatusCallbackFn callback_fn,
                   void* callback_arg,
                   _Bool* out_success) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).host_callback(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream),
                    callback_fn,
                    callback_arg,
                    out_success
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_stream_with_options =
                [](TF_Executor* executor,
                   TF_Device* device,
                   const TF_StreamOptions* options,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_stream_with_options(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    options,
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stream_from_pool =
                [](TF_Executor* executor,
                   TF_Device* device,
                   int32_t priority,
                   TF_Stream* out_stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).get_stream_from_pool(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    priority,
                    ice::sonic::TF_StreamOps::wrap(out_stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_current_stream =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* out_stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).get_current_stream(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(out_stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_current_stream =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Stream* stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).set_current_stream(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_StreamOps::wrap(stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_stream_from_native =
                [](TF_Executor* executor,
                   TF_Device* device,
                   void* native_handle,
                   TF_Stream* out_stream,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_stream_from_native(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    native_handle,
                    ice::sonic::TF_StreamOps::wrap(out_stream)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_event_with_options_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   const TF_EventOptions* options,
                   TF_Event* out_event,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_event_with_options_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    options,
                    ice::sonic::TF_EventOps::wrap(out_event)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_event_from_ipc_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   const TF_IpcEventHandle* handle,
                   TF_Event* out_event,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_event_from_ipc_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    handle,
                    ice::sonic::TF_EventOps::wrap(out_event)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_allocator_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_Allocator* out_allocator,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_allocator_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_AllocatorOps::wrap(out_allocator)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_allocator_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Allocator* allocator) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).destroy_allocator_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_AllocatorOps::wrap(allocator)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_random_generator_internal =
                [](TF_Executor* executor,
                   TF_Device* device,
                   uint64_t seed,
                   TF_RandomGenerator* out_generator,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).create_random_generator_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    seed,
                    ice::sonic::TF_RandomGeneratorOps::wrap(out_generator)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_random_generator_internal =
                [](TF_Executor* executor, TF_Device* device, TF_RandomGenerator* generator) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).destroy_random_generator_internal(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_RandomGeneratorOps::wrap(generator)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_default_random_generator =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_RandomGenerator* out_generator,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).get_default_random_generator(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TF_RandomGeneratorOps::wrap(out_generator)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Executor* executor, TF_Device* device, void** out_handle) noexcept
            {
                auto res = TF_ExecutorOps::from_handle(executor).get_native_handle(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    out_handle
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_ExecutorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Executor& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ExecutorOps m_vtable;
    TF_Executor m_handle;
};

} // namespace ice::builder
