// SYCL reference plugin — TF_ExecutorOps: stream/event/timer lifecycle, memcpy/memset, sync,
// stream pool, current stream, and the random-generator/allocator child-handle factories that
// live on TF_ExecutorOps/TF_MemoryOps.
//
// Not built by Bazel (docs/ only). Replaces c10/XPUStream.cpp's stream pool + thread_local
// current stream and c10/XPUFunctions.cpp's device_synchronize. One SyclExecutor per
// TF_Executor, created by SyclPlatform::create_executor_internal; it does not own a separate
// sycl::context — every executor on this platform shares SyclPlatform::get_shared_context(),
// matching c10/XPUFunctions.cpp's Note [Device Management] (one context per platform, not per
// executor).
//
// See platform.cppm's file-level note on the create_*_internal signature assumption: this file
// takes a raw TF_Stream*/TF_Event*/TF_Timer*/TF_RandomGenerator* for every slot that creates a
// fresh handle (the plugin must set its plugin_data), and a dereferenced ice::builder::X& for
// every slot that operates on an already-created handle.

module;

#include "include/c/extern/stream_executor/executor.h"

export module sycl_backend:executor;

import std;
import cc_ice_extern_stream_executor_builder;
import :device;
import :stream;
import :event;
import :timer;
import :random_generator;

export namespace sycl_backend {

class SyclPlatform;

class SyclExecutor : public ice::builder::Executor
{
public:
    // Three priorities (LOW/NORMAL/HIGH), a fixed pool size per priority — matches
    // c10/XPUStream.cpp's Note [XPU Stream priorities] without its bit-packed StreamId encoding,
    // which this ABI does not need: stream identity is just the SyclStream* behind plugin_data.
    static constexpr std::size_t POOL_SIZE_PER_PRIORITY = 32;

    explicit SyclExecutor(SyclPlatform& platform) noexcept :
        m_platform{platform}
    {
    }

    ~SyclExecutor() override = default;
    SyclExecutor(const SyclExecutor&) = delete;
    SyclExecutor& operator=(const SyclExecutor&) = delete;
    SyclExecutor(SyclExecutor&&) = delete;
    SyclExecutor& operator=(SyclExecutor&&) = delete;

    // Every Device/Stream/Event/Timer this executor is ever handed was created by this same
    // plugin (create_*_internal above), so casting back to the concrete type is safe — the same
    // pattern the generated ::create(handle) already relies on.
    static SyclDevice& as_device(ice::builder::Device& device) noexcept
    {
        return static_cast<SyclDevice&>(device);
    }

    static SyclStream& as_stream(ice::builder::Stream& stream) noexcept
    {
        return static_cast<SyclStream&>(stream);
    }

    static SyclEvent& as_event(ice::builder::Event& event) noexcept
    {
        return static_cast<SyclEvent&>(event);
    }

    static SyclTimer& as_timer(ice::builder::Timer& timer) noexcept
    {
        return static_cast<SyclTimer&>(timer);
    }

    void device_memory_usage(
        ice::builder::Device& device,
        int64_t* out_free,
        int64_t* out_total,
        bool* out_success
    ) noexcept override
    {
        const sycl::device& native = as_device(device).get_native_device();

        if (!native.has(sycl::aspect::ext_intel_free_memory)) {
            *out_success = false;
            return;
        }

        *out_free =
            static_cast<int64_t>(native.get_info<sycl::ext::intel::info::device::free_memory>());
        *out_total = static_cast<int64_t>(native.get_info<sycl::info::device::global_mem_size>());
        *out_success = true;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create_stream_internal(ice::builder::Device& device, TF_Stream* stream) noexcept
    {
        return create_stream_with_priority(device, /*priority=*/0, stream);
    }

    void destroy_stream_internal(
        ice::builder::Device& device,
        ice::builder::Stream& stream
    ) noexcept override
    {
        (void)device;
        remove_owned_stream(&stream);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_stream_dependency(
        ice::builder::Device& device,
        ice::builder::Stream& dependent,
        ice::builder::Stream& other
    ) noexcept override
    {
        (void)device;

        try {
            sycl::event barrier = as_stream(other).get_native_queue().ext_oneapi_submit_barrier();
            as_stream(dependent).get_native_queue().ext_oneapi_submit_barrier({barrier});
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_stream_status(ice::builder::Device& device, ice::builder::Stream& stream) noexcept override
    {
        (void)device;
        (void)stream;
        // No standing async error to surface in this reference backend; a real backend would
        // report the last exception its queue's async_handler caught.
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create_event_internal(ice::builder::Device& device, TF_Event* event) noexcept
    {
        (void)device;
        auto owned = std::make_unique<SyclEvent>(/*enable_timing=*/false);
        event->plugin_data = owned.get();
        m_events.push_back(std::move(owned));
        return {};
    }

    void destroy_event_internal(
        ice::builder::Device& device,
        ice::builder::Event& event
    ) noexcept override
    {
        (void)device;
        remove_owned_event(&event);
    }

    void get_event_status(
        ice::builder::Device& device,
        ice::builder::Event& event,
        TF_EventStatus* out_event_status
    ) noexcept override
    {
        (void)device;

        const auto status = as_event(event)
                                .get_native_event()
                                .get_info<sycl::info::event::command_execution_status>();

        *out_event_status = status == sycl::info::event_command_status::complete ? TF_EVENT_COMPLETE
                                                                                 : TF_EVENT_PENDING;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> record_event(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        ice::builder::Event& event
    ) noexcept override
    {
        (void)device;

        try {
            as_event(event).set_native_event(
                as_stream(stream).get_native_queue().ext_oneapi_submit_barrier()
            );
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> wait_for_event(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        ice::builder::Event& event
    ) noexcept override
    {
        (void)device;

        try {
            as_stream(stream).get_native_queue().ext_oneapi_submit_barrier(
                {as_event(event).get_native_event()}
            );
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create_timer_internal(ice::builder::Device& device, TF_Timer* timer) noexcept
    {
        (void)device;
        auto owned = std::make_unique<SyclTimer>();
        timer->plugin_data = owned.get();
        m_timers.push_back(std::move(owned));
        return {};
    }

    void destroy_timer_internal(
        ice::builder::Device& device,
        ice::builder::Timer& timer
    ) noexcept override
    {
        (void)device;
        remove_owned_timer(&timer);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> start_timer(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        ice::builder::Timer& timer
    ) noexcept override
    {
        (void)device;

        auto& native_timer = as_timer(timer);
        try {
            native_timer.set_start_event(
                as_stream(stream).get_native_queue().ext_oneapi_submit_barrier()
            );
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> stop_timer(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        ice::builder::Timer& timer
    ) noexcept override
    {
        (void)device;

        auto& native_timer = as_timer(timer);
        try {
            native_timer.set_stop_event(
                as_stream(stream).get_native_queue().ext_oneapi_submit_barrier()
            );
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> memcpy_dtoh(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept override
    {
        (void)device;
        return submit_memcpy(stream, host_dst, device_src->opaque, size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> memcpy_htod(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size
    ) noexcept override
    {
        (void)device;
        return submit_memcpy(stream, device_dst->opaque, host_src, size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> memcpy_dtod(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept override
    {
        (void)device;
        return submit_memcpy(stream, device_dst->opaque, device_src->opaque, size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> sync_memcpy_dtoh(
        ice::builder::Device& device,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept override
    {
        return blocking_memcpy(device, host_dst, device_src->opaque, size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> sync_memcpy_htod(
        ice::builder::Device& device,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size
    ) noexcept override
    {
        return blocking_memcpy(device, device_dst->opaque, host_src, size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> sync_memcpy_dtod(
        ice::builder::Device& device,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size
    ) noexcept override
    {
        return blocking_memcpy(device, device_dst->opaque, device_src->opaque, size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    block_host_for_event(ice::builder::Device& device, ice::builder::Event& event) noexcept override
    {
        (void)device;

        try {
            as_event(event).get_native_event().wait_and_throw();
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> block_host_until_done(
        ice::builder::Device& device,
        ice::builder::Stream& stream
    ) noexcept override
    {
        (void)device;

        try {
            as_stream(stream).get_native_queue().wait_and_throw();
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    synchronize_all_activity(ice::builder::Device& device) noexcept override
    {
        (void)device;

        try {
            for (auto& stream: m_owned_streams) {
                stream->get_native_queue().wait_and_throw();
            }
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> mem_zero(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        TF_DeviceMemoryBase* location,
        uint64_t size
    ) noexcept override
    {
        (void)device;

        try {
            as_stream(stream).get_native_queue().memset(location->opaque, 0, size);
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> memset(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size
    ) noexcept override
    {
        (void)device;

        try {
            as_stream(stream).get_native_queue().memset(location->opaque, pattern, size);
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> memset32(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size
    ) noexcept override
    {
        (void)device;

        try {
            auto* typed_destination = static_cast<uint32_t*>(location->opaque);
            const uint64_t element_count = size / sizeof(uint32_t);

            as_stream(stream).get_native_queue().parallel_for(
                sycl::range<1>{element_count},
                [typed_destination, pattern](sycl::id<1> index)
                {
                    typed_destination[index] = pattern;
                }
            );
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    void host_callback(
        ice::builder::Device& device,
        ice::builder::Stream& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        bool* out_success
    ) noexcept override
    {
        (void)device;

        try {
            as_stream(stream).get_native_queue().submit(
                [&](sycl::handler& handler)
                {
                    handler.host_task(
                        [callback_fn, callback_arg]()
                        {
                            ice::sonic::Status status;
                            callback_fn(callback_arg, status.get_handle());
                        }
                    );
                }
            );
            *out_success = true;
        } catch (const sycl::exception&) {
            *out_success = false;
        }
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_stream_with_options(
        ice::builder::Device& device,
        const TF_StreamOptions* options,
        TF_Stream* stream
    ) noexcept
    {
        return create_stream_with_priority(device, options->priority, stream);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_stream_from_pool(
        ice::builder::Device& device,
        int32_t priority,
        TF_Stream* out_stream
    ) noexcept
    {
        auto& pool = pool_for(priority);

        if (pool.empty()) {
            for (std::size_t index = 0; index < POOL_SIZE_PER_PRIORITY; ++index) {
                auto created = create_pooled_stream(device, priority);
                if (!created) {
                    return std::unexpected{created.error()};
                }
                pool.push_back(*created);
            }
        }

        auto& counter = counter_for(priority);
        out_stream->plugin_data = pool[counter % pool.size()];
        ++counter;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_current_stream(ice::builder::Device& device, TF_Stream* out_stream) noexcept
    {
        auto& current = current_stream_slot();

        if (current == nullptr) {
            auto assigned = get_stream_from_pool(device, /*priority=*/0, out_stream);
            if (!assigned) {
                return std::unexpected{assigned.error()};
            }

            current = static_cast<SyclStream*>(out_stream->plugin_data);
            return {};
        }

        out_stream->plugin_data = current;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_current_stream(ice::builder::Device& device, ice::builder::Stream& stream) noexcept override
    {
        (void)device;
        current_stream_slot() = &as_stream(stream);
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_stream_from_native(
        ice::builder::Device& device,
        void* native_handle,
        TF_Stream* out_stream
    ) noexcept
    {
        (void)device;

        auto owned = std::make_unique<SyclStream>(
            sycl::queue{*static_cast<sycl::queue*>(native_handle)},
            0,
            0
        );
        out_stream->plugin_data = owned.get();
        m_owned_streams.push_back(std::move(owned));
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_event_with_options_internal(
        ice::builder::Device& device,
        const TF_EventOptions* options,
        TF_Event* out_event
    ) noexcept
    {
        (void)device;
        auto owned = std::make_unique<SyclEvent>(options->enable_timing);
        out_event->plugin_data = owned.get();
        m_events.push_back(std::move(owned));
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_event_from_ipc_internal(
        ice::builder::Device& device,
        const TF_IpcEventHandle* handle,
        TF_Event* out_event
    ) noexcept
    {
        (void)device;
        (void)handle;
        (void)out_event;
        return std::unexpected{
            ice::sonic::Status::from_message("SyclExecutor: IPC events not implemented")
        };
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_random_generator_internal(
        ice::builder::Device& device,
        uint64_t seed,
        TF_RandomGenerator* out_generator
    ) noexcept
    {
        auto owned = std::make_unique<SyclRandomGenerator>(device_index_of(device), seed);
        out_generator->plugin_data = owned.get();
        m_random_generators.push_back(std::move(owned));
        return {};
    }

    void destroy_random_generator_internal(
        ice::builder::Device& device,
        ice::builder::RandomGenerator& generator
    ) noexcept override
    {
        (void)device;

        if (&generator == m_default_random_generator.get()) {
            // The default generator is owned for the lifetime of the executor; ATen's
            // XPUGeneratorImpl calling destroy on it is a no-op, same as c10::xpu::detail's
            // getDefaultXPUGenerator.
            return;
        }

        std::erase_if(
            m_random_generators,
            [&generator](const std::unique_ptr<SyclRandomGenerator>& candidate)
            {
                return candidate.get() == &generator;
            }
        );
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_default_random_generator(
        ice::builder::Device& device,
        TF_RandomGenerator* out_generator
    ) noexcept
    {
        if (!m_default_random_generator) {
            m_default_random_generator =
                std::make_unique<SyclRandomGenerator>(device_index_of(device), /*seed=*/0);
        }

        out_generator->plugin_data = m_default_random_generator.get();
        return {};
    }

    void get_native_handle(ice::builder::Device& device, void** out_handle) noexcept override
    {
        (void)device;
        *out_handle = const_cast<sycl::context*>(&m_platform.get_shared_context());
    }

private:
    [[nodiscard]] std::expected<void, ice::sonic::Status> create_stream_with_priority(
        ice::builder::Device& device,
        int32_t priority,
        TF_Stream* stream
    ) noexcept
    {
        try {
            auto queue = sycl::queue{
                m_platform.get_shared_context(),
                as_device(device).get_native_device(),
                sycl::property_list{sycl::property::queue::in_order{}}
            };

            auto owned =
                std::make_unique<SyclStream>(std::move(queue), device_index_of(device), priority);
            stream->plugin_data = owned.get();
            m_owned_streams.push_back(std::move(owned));
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    std::expected<SyclStream*, ice::sonic::Status>
    create_pooled_stream(ice::builder::Device& device, int32_t priority)
    {
        TF_Stream stream{};
        auto created = create_stream_with_priority(device, priority, &stream);
        if (!created) {
            return std::unexpected{created.error()};
        }
        return static_cast<SyclStream*>(stream.plugin_data);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    submit_memcpy(ice::builder::Stream& stream, void* dst, const void* src, uint64_t size) noexcept
    {
        try {
            as_stream(stream).get_native_queue().memcpy(dst, src, size);
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> blocking_memcpy(
        ice::builder::Device& device,
        void* dst,
        const void* src,
        uint64_t size
    ) noexcept
    {
        try {
            sycl::queue temporary{
                m_platform.get_shared_context(),
                as_device(device).get_native_device()
            };
            temporary.memcpy(dst, src, size).wait_and_throw();
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    int device_index_of(ice::builder::Device& device) const noexcept
    {
        return m_platform.get_index_of(as_device(device));
    }

    void remove_owned_stream(ice::builder::Stream* stream) noexcept
    {
        std::erase_if(
            m_owned_streams,
            [stream](const std::unique_ptr<SyclStream>& candidate)
            {
                return candidate.get() == stream;
            }
        );
    }

    void remove_owned_event(ice::builder::Event* event) noexcept
    {
        std::erase_if(
            m_events,
            [event](const std::unique_ptr<SyclEvent>& candidate)
            {
                return candidate.get() == event;
            }
        );
    }

    void remove_owned_timer(ice::builder::Timer* timer) noexcept
    {
        std::erase_if(
            m_timers,
            [timer](const std::unique_ptr<SyclTimer>& candidate)
            {
                return candidate.get() == timer;
            }
        );
    }

    std::vector<SyclStream*>& pool_for(int32_t priority) noexcept
    {
        return m_stream_pools.at(pool_index(priority));
    }

    std::size_t& counter_for(int32_t priority) noexcept
    {
        return m_pool_counters.at(pool_index(priority));
    }

    static std::size_t pool_index(int32_t priority) noexcept
    {
        // priority < 0 == HIGH, 0 == NORMAL, > 0 == LOW — matches c10/XPUStream.h's priority
        // range and Note [XPU Stream priorities] (higher priority number == lower priority).
        if (priority < 0) {
            return 2;
        }
        if (priority > 0) {
            return 0;
        }
        return 1;
    }

    // Per calling thread, matching c10/XPUStream.cpp's thread_local current_streams.
    SyclStream*& current_stream_slot() noexcept
    {
        return s_current_stream;
    }

    SyclPlatform& m_platform;
    std::vector<std::unique_ptr<SyclStream>> m_owned_streams;
    std::vector<std::unique_ptr<SyclEvent>> m_events;
    std::vector<std::unique_ptr<SyclTimer>> m_timers;
    std::vector<std::unique_ptr<SyclRandomGenerator>> m_random_generators;
    std::unique_ptr<SyclRandomGenerator> m_default_random_generator;
    std::array<std::vector<SyclStream*>, 3> m_stream_pools;
    std::array<std::size_t, 3> m_pool_counters{};

    static thread_local SyclStream* s_current_stream;
};

inline thread_local SyclStream* SyclExecutor::s_current_stream = nullptr;

} // namespace sycl_backend
