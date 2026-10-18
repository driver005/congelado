// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:executor;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Executor
{
public:
    explicit Executor(
        const ::TF_AllocatorOps* Allocator_ops,
        const ::TF_DeviceOps* Device_ops,
        const ::TF_EventOps* Event_ops,
        const ::TF_RandomGeneratorOps* RandomGenerator_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StreamOps* Stream_ops,
        const ::TF_TimerOps* Timer_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Allocator_ops = Allocator_ops;
        m_Device_ops = Device_ops;
        m_Event_ops = Event_ops;
        m_RandomGenerator_ops = RandomGenerator_ops;
        m_Status_ops = Status_ops;
        m_Stream_ops = Stream_ops;
        m_Timer_ops = Timer_ops;
    }

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    static Executor& from_handle(void* ctx) noexcept
    {
        return *static_cast<Executor*>(ctx);
    }

    template<typename HandleT>
    static Executor& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Executor*>(handle->plugin_data);
    }

    virtual ~Executor() = default;
    virtual void destroy() noexcept = 0;
    virtual void device_memory_usage(
        const ice::sonic::Device& device,
        std::int64_t& out_free,
        std::int64_t& out_total,
        bool& out_success
    ) noexcept = 0;
    virtual void create_stream_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_stream_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream
    ) noexcept = 0;
    virtual void create_stream_dependency(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& dependent,
        const ice::sonic::Stream& other,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stream_status(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_event_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_event_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event
    ) noexcept = 0;
    virtual void get_event_status(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event,
        ice::sonic::EventStatus& out_event_status
    ) noexcept = 0;
    virtual void record_event(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void wait_for_event(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_timer_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Timer& timer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_timer_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Timer& timer
    ) noexcept = 0;
    virtual void start_timer(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Timer& timer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void stop_timer(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Timer& timer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memcpy_dtoh(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        void* host_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memcpy_htod(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memcpy_dtod(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* device_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void sync_memcpy_dtoh(
        const ice::sonic::Device& device,
        void* host_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void sync_memcpy_htod(
        const ice::sonic::Device& device,
        ice::sonic::DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void sync_memcpy_dtod(
        const ice::sonic::Device& device,
        ice::sonic::DeviceMemoryBase* device_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void block_host_for_event(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void block_host_until_done(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void synchronize_all_activity(
        const ice::sonic::Device& device,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void mem_zero(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memset(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void memset32(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void host_callback(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        bool& out_success
    ) noexcept = 0;
    virtual void create_stream_with_options(
        const ice::sonic::Device& device,
        const ice::sonic::StreamOptions* options,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stream_from_pool(
        const ice::sonic::Device& device,
        int32_t priority,
        const ice::sonic::Stream& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_current_stream(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_current_stream(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_stream_from_native(
        const ice::sonic::Device& device,
        void* native_handle,
        const ice::sonic::Stream& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_event_with_options_internal(
        const ice::sonic::Device& device,
        const ice::sonic::EventOptions* options,
        const ice::sonic::Event& out_event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_event_from_ipc_internal(
        const ice::sonic::Device& device,
        const ice::sonic::IpcEventHandle* handle,
        const ice::sonic::Event& out_event,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_allocator_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Allocator& out_allocator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_allocator_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Allocator& allocator
    ) noexcept = 0;
    virtual void create_random_generator_internal(
        const ice::sonic::Device& device,
        uint64_t seed,
        const ice::sonic::RandomGenerator& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_random_generator_internal(
        const ice::sonic::Device& device,
        const ice::sonic::RandomGenerator& generator
    ) noexcept = 0;
    virtual void get_default_random_generator(
        const ice::sonic::Device& device,
        const ice::sonic::RandomGenerator& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_native_handle(const ice::sonic::Device& device, void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Executor*)) noexcept
    {
        m_vtable = ::TF_ExecutorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ExecutorOps, get_native_handle),

            .create = create,
            .destroy =
                [](::TF_Executor* handle) noexcept
            {
                auto& self = Executor::from_handle(handle);
                self.destroy();
            },
            .device_memory_usage =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   int64_t* out_free,
                   int64_t* out_total,
                   _Bool* out_success) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.device_memory_usage(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    *out_free,
                    *out_total,
                    *out_success
                );
            },
            .create_stream_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_stream_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_stream_internal =
                [](::TF_Executor* executor, ::TF_Device* device, ::TF_Stream* stream) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.destroy_stream_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream)
                );
            },
            .create_stream_dependency =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* dependent,
                   ::TF_Stream* other,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_stream_dependency(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, dependent),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, other),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stream_status =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.get_stream_status(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_event_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Event* event,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_event_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Event>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_event_internal =
                [](::TF_Executor* executor, ::TF_Device* device, ::TF_Event* event) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.destroy_event_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Event>{}, event)
                );
            },
            .get_event_status =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Event* event,
                   ::TF_EventStatus* out_event_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                ice::sonic::EventStatus wrapped_out_event_status{*out_event_status};
                self.get_event_status(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Event>{}, event),
                    wrapped_out_event_status
                );
                *out_event_status = wrapped_out_event_status.get_value();
            },
            .record_event =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Event* event,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.record_event(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Event>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .wait_for_event =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Event* event,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.wait_for_event(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Event>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_timer_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Timer* timer,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_timer_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Timer>{}, timer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_timer_internal =
                [](::TF_Executor* executor, ::TF_Device* device, ::TF_Timer* timer) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.destroy_timer_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Timer>{}, timer)
                );
            },
            .start_timer =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Timer* timer,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.start_timer(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Timer>{}, timer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .stop_timer =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Timer* timer,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.stop_timer(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Timer>{}, timer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memcpy_dtoh =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   void* host_dst,
                   const ::TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_src;
                if (device_src != nullptr) {
                    wrapped_device_src.emplace(*device_src);
                }
                self.memcpy_dtoh(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    host_dst,
                    wrapped_device_src ? &*wrapped_device_src : nullptr,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .memcpy_htod =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_DeviceMemoryBase* device_dst,
                   const void* host_src,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_dst;
                if (device_dst != nullptr) {
                    wrapped_device_dst.emplace(*device_dst);
                }
                self.memcpy_htod(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    wrapped_device_dst ? &*wrapped_device_dst : nullptr,
                    host_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (device_dst != nullptr) {
                    *device_dst = *wrapped_device_dst->get_handle();
                }
            },
            .memcpy_dtod =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_DeviceMemoryBase* device_dst,
                   const ::TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_dst;
                if (device_dst != nullptr) {
                    wrapped_device_dst.emplace(*device_dst);
                }
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_src;
                if (device_src != nullptr) {
                    wrapped_device_src.emplace(*device_src);
                }
                self.memcpy_dtod(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    wrapped_device_dst ? &*wrapped_device_dst : nullptr,
                    wrapped_device_src ? &*wrapped_device_src : nullptr,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (device_dst != nullptr) {
                    *device_dst = *wrapped_device_dst->get_handle();
                }
            },
            .sync_memcpy_dtoh =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   void* host_dst,
                   const ::TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_src;
                if (device_src != nullptr) {
                    wrapped_device_src.emplace(*device_src);
                }
                self.sync_memcpy_dtoh(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    host_dst,
                    wrapped_device_src ? &*wrapped_device_src : nullptr,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .sync_memcpy_htod =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_DeviceMemoryBase* device_dst,
                   const void* host_src,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_dst;
                if (device_dst != nullptr) {
                    wrapped_device_dst.emplace(*device_dst);
                }
                self.sync_memcpy_htod(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    wrapped_device_dst ? &*wrapped_device_dst : nullptr,
                    host_src,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (device_dst != nullptr) {
                    *device_dst = *wrapped_device_dst->get_handle();
                }
            },
            .sync_memcpy_dtod =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_DeviceMemoryBase* device_dst,
                   const ::TF_DeviceMemoryBase* device_src,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_dst;
                if (device_dst != nullptr) {
                    wrapped_device_dst.emplace(*device_dst);
                }
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_device_src;
                if (device_src != nullptr) {
                    wrapped_device_src.emplace(*device_src);
                }
                self.sync_memcpy_dtod(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    wrapped_device_dst ? &*wrapped_device_dst : nullptr,
                    wrapped_device_src ? &*wrapped_device_src : nullptr,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (device_dst != nullptr) {
                    *device_dst = *wrapped_device_dst->get_handle();
                }
            },
            .block_host_for_event =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Event* event,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.block_host_for_event(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Event>{}, event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .block_host_until_done =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.block_host_until_done(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .synchronize_all_activity =
                [](::TF_Executor* executor, ::TF_Device* device, ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.synchronize_all_activity(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .mem_zero =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_DeviceMemoryBase* location,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_location;
                if (location != nullptr) {
                    wrapped_location.emplace(*location);
                }
                self.mem_zero(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    wrapped_location ? &*wrapped_location : nullptr,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (location != nullptr) {
                    *location = *wrapped_location->get_handle();
                }
            },
            .memset =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_DeviceMemoryBase* location,
                   uint8_t pattern,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_location;
                if (location != nullptr) {
                    wrapped_location.emplace(*location);
                }
                self.memset(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    wrapped_location ? &*wrapped_location : nullptr,
                    pattern,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (location != nullptr) {
                    *location = *wrapped_location->get_handle();
                }
            },
            .memset32 =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_DeviceMemoryBase* location,
                   uint32_t pattern,
                   uint64_t size,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::DeviceMemoryBase> wrapped_location;
                if (location != nullptr) {
                    wrapped_location.emplace(*location);
                }
                self.memset32(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    wrapped_location ? &*wrapped_location : nullptr,
                    pattern,
                    size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (location != nullptr) {
                    *location = *wrapped_location->get_handle();
                }
            },
            .host_callback =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   TF_StatusCallbackFn callback_fn,
                   void* callback_arg,
                   _Bool* out_success) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.host_callback(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    callback_fn,
                    callback_arg,
                    *out_success
                );
            },
            .create_stream_with_options =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   const ::TF_StreamOptions* options,
                   ::TF_Stream* stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::StreamOptions> wrapped_options;
                if (options != nullptr) {
                    wrapped_options.emplace(*options);
                }
                self.create_stream_with_options(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    wrapped_options ? &*wrapped_options : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stream_from_pool =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   int32_t priority,
                   ::TF_Stream* out_stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.get_stream_from_pool(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    priority,
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, out_stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_current_stream =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* out_stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.get_current_stream(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, out_stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_current_stream =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Stream* stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.set_current_stream(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_stream_from_native =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   void* native_handle,
                   ::TF_Stream* out_stream,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_stream_from_native(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    native_handle,
                    self.wrap(std::type_identity<ice::sonic::Stream>{}, out_stream),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_event_with_options_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   const ::TF_EventOptions* options,
                   ::TF_Event* out_event,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::EventOptions> wrapped_options;
                if (options != nullptr) {
                    wrapped_options.emplace(*options);
                }
                self.create_event_with_options_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    wrapped_options ? &*wrapped_options : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Event>{}, out_event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_event_from_ipc_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   const ::TF_IpcEventHandle* handle,
                   ::TF_Event* out_event,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                std::optional<ice::sonic::IpcEventHandle> wrapped_handle;
                if (handle != nullptr) {
                    wrapped_handle.emplace(*handle);
                }
                self.create_event_from_ipc_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    wrapped_handle ? &*wrapped_handle : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Event>{}, out_event),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_allocator_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_Allocator* out_allocator,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_allocator_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Allocator>{}, out_allocator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_allocator_internal =
                [](::TF_Executor* executor, ::TF_Device* device, ::TF_Allocator* allocator) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.destroy_allocator_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Allocator>{}, allocator)
                );
            },
            .create_random_generator_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   uint64_t seed,
                   ::TF_RandomGenerator* out_generator,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.create_random_generator_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    seed,
                    self.wrap(std::type_identity<ice::sonic::RandomGenerator>{}, out_generator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_random_generator_internal =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_RandomGenerator* generator) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.destroy_random_generator_internal(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::RandomGenerator>{}, generator)
                );
            },
            .get_default_random_generator =
                [](::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TF_RandomGenerator* out_generator,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.get_default_random_generator(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::RandomGenerator>{}, out_generator),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_native_handle =
                [](::TF_Executor* executor, ::TF_Device* device, void** out_handle) noexcept
            {
                auto& self = Executor::from_handle(executor);
                self.get_native_handle(
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    out_handle
                );
            },

        };
    }

    ice::sonic::Allocator
    wrap(std::type_identity<ice::sonic::Allocator>, const ::TF_Allocator* handle) const noexcept
    {
        return ice::sonic::Allocator{m_Allocator_ops, handle};
    }

    ice::sonic::Device
    wrap(std::type_identity<ice::sonic::Device>, const ::TF_Device* handle) const noexcept
    {
        return ice::sonic::Device{m_Device_ops, handle};
    }

    ice::sonic::Event
    wrap(std::type_identity<ice::sonic::Event>, const ::TF_Event* handle) const noexcept
    {
        return ice::sonic::Event{m_Event_ops, handle};
    }

    ice::sonic::RandomGenerator wrap(
        std::type_identity<ice::sonic::RandomGenerator>,
        const ::TF_RandomGenerator* handle
    ) const noexcept
    {
        return ice::sonic::RandomGenerator{m_RandomGenerator_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::Stream
    wrap(std::type_identity<ice::sonic::Stream>, const ::TF_Stream* handle) const noexcept
    {
        return ice::sonic::Stream{m_Stream_ops, handle};
    }

    ice::sonic::Timer
    wrap(std::type_identity<ice::sonic::Timer>, const ::TF_Timer* handle) const noexcept
    {
        return ice::sonic::Timer{m_Timer_ops, handle};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ExecutorOps*>(&m_vtable));
    }

private:
    ::TF_ExecutorOps m_vtable;
    ::TF_Executor m_handle;

    const ::TF_AllocatorOps* m_Allocator_ops{nullptr};

    const ::TF_DeviceOps* m_Device_ops{nullptr};

    const ::TF_EventOps* m_Event_ops{nullptr};

    const ::TF_RandomGeneratorOps* m_RandomGenerator_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StreamOps* m_Stream_ops{nullptr};

    const ::TF_TimerOps* m_Timer_ops{nullptr};
};

} // namespace ice::builder
