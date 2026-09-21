// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/executor.h"

export module cc_ice_builder_stream_executor:executor;

import std;

export namespace ice::builder {

class TF_ExecutorOps
{
public:
    static TF_ExecutorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ExecutorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ExecutorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ExecutorOps*>(handle->plugin_data);
    }

    virtual ~TF_ExecutorOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> allocate(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t size,
        int64_t memory_space,
        TF_DeviceMemoryBase* mem
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    deallocate(const ice::sonic::TF_DeviceOps& device, TF_DeviceMemoryBase* memory) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> host_memory_allocate(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t size,
        void** out_mem
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    host_memory_deallocate(const ice::sonic::TF_DeviceOps& device, void* mem) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> unified_memory_allocate(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t bytes,
        void** out_location
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    unified_memory_deallocate(const ice::sonic::TF_DeviceOps& device, void* location) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_allocator_stats(
        const ice::sonic::TF_DeviceOps& device,
        TF_AllocatorStats* stats,
        _Bool* out_success
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> device_memory_usage(
        const ice::sonic::TF_DeviceOps& device,
        int64_t* out_free,
        int64_t* out_total,
        _Bool* out_success
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_stream_internal(const ice::sonic::TF_DeviceOps& device, TF_Stream* stream) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_stream_internal(const ice::sonic::TF_DeviceOps& device, TF_Stream* stream) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_stream_dependency(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* dependent,
        TF_Stream* other
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stream_status(const ice::sonic::TF_DeviceOps& device, TF_Stream* stream) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_event_internal(const ice::sonic::TF_DeviceOps& device, TF_Event* event) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_event_internal(const ice::sonic::TF_DeviceOps& device, TF_Event* event) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_event_status(
        const ice::sonic::TF_DeviceOps& device,
        TF_Event* event,
        TF_EventStatus* out_event_status
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> record_event(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_Event* event
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> wait_for_event(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_Event* event
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
        TF_Stream* stream,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stop_timer(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    block_host_for_event(const ice::sonic::TF_DeviceOps& device, TF_Event* event) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    block_host_until_done(const ice::sonic::TF_DeviceOps& device, TF_Stream* stream) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    synchronize_all_activity(const ice::sonic::TF_DeviceOps& device) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> mem_zero(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_DeviceMemoryBase* location,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memset(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> memset32(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> host_callback(
        const ice::sonic::TF_DeviceOps& device,
        TF_Stream* stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        _Bool* out_success
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_stream_with_options(
        const ice::sonic::TF_DeviceOps& device,
        const TF_StreamOptions* options,
        TF_Stream* stream
    ) noexcept = 0;

    static TF_ExecutorOps* get_generic_vtable()
    {
        static TF_ExecutorOps vtable = {
            .struct_size = TF_EXECUTOR_STRUCT_SIZE,
            .allocate =
                [](TF_Executor* executor,
                   TF_Device* device,
                   uint64_t size,
                   int64_t memory_space,
                   TF_DeviceMemoryBase* mem) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->allocate(ice::sonic::TF_DeviceOps::wrap(device), size, memory_space, mem);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .deallocate =
                [](TF_Executor* executor, TF_Device* device, TF_DeviceMemoryBase* memory) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->deallocate(ice::sonic::TF_DeviceOps::wrap(device), memory);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .host_memory_allocate =
                [](TF_Executor* executor, TF_Device* device, uint64_t size, void** out_mem) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->host_memory_allocate(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    size,
                    out_mem
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .host_memory_deallocate =
                [](TF_Executor* executor, TF_Device* device, void* mem) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->host_memory_deallocate(ice::sonic::TF_DeviceOps::wrap(device), mem);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .unified_memory_allocate =
                [](TF_Executor* executor,
                   TF_Device* device,
                   uint64_t bytes,
                   void** out_location) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->unified_memory_allocate(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    bytes,
                    out_location
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .unified_memory_deallocate =
                [](TF_Executor* executor, TF_Device* device, void* location) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->unified_memory_deallocate(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    location
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_allocator_stats =
                [](TF_Executor* executor,
                   TF_Device* device,
                   TF_AllocatorStats* stats,
                   _Bool* out_success) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->get_allocator_stats(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stats,
                    out_success
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .device_memory_usage =
                [](TF_Executor* executor,
                   TF_Device* device,
                   int64_t* out_free,
                   int64_t* out_total,
                   _Bool* out_success) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->device_memory_usage(
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->create_stream_internal(ice::sonic::TF_DeviceOps::wrap(device), stream);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_stream_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Stream* stream) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->destroy_stream_internal(ice::sonic::TF_DeviceOps::wrap(device), stream);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->create_stream_dependency(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    dependent,
                    other
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->get_stream_status(ice::sonic::TF_DeviceOps::wrap(device), stream);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->create_event_internal(ice::sonic::TF_DeviceOps::wrap(device), event);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_event_internal =
                [](TF_Executor* executor, TF_Device* device, TF_Event* event) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->destroy_event_internal(ice::sonic::TF_DeviceOps::wrap(device), event);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->get_event_status(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    event,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->record_event(ice::sonic::TF_DeviceOps::wrap(device), stream, event);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->wait_for_event(ice::sonic::TF_DeviceOps::wrap(device), stream, event);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->create_timer_internal(
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->destroy_timer_internal(
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->start_timer(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->stop_timer(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->memcpy_dtoh(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->memcpy_htod(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->memcpy_dtod(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->sync_memcpy_dtoh(
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->sync_memcpy_htod(
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->sync_memcpy_dtod(
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->block_host_for_event(ice::sonic::TF_DeviceOps::wrap(device), event);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->block_host_until_done(ice::sonic::TF_DeviceOps::wrap(device), stream);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .synchronize_all_activity =
                [](TF_Executor* executor, TF_Device* device, TF_Status* out_status) noexcept
            {
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->synchronize_all_activity(ice::sonic::TF_DeviceOps::wrap(device));
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res =
                    self->mem_zero(ice::sonic::TF_DeviceOps::wrap(device), stream, location, size);
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->memset(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->memset32(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->host_callback(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    stream,
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
                auto* self = TF_ExecutorOps::create(executor);
                auto res = self->create_stream_with_options(
                    ice::sonic::TF_DeviceOps::wrap(device),
                    options,
                    stream
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
