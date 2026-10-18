module;

#include "include/c/extern/stream_executor/executor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:executor;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;
import aten_xpu_extern_random_generator;
import :device;
import :stream;
import :stream_pool;
import :event;
import :timer;
import :allocator;

export namespace aten_xpu {

using SyclCurrentStream =
    std::tuple<std::shared_ptr<sycl::queue>, std::shared_ptr<std::string>, int32_t>;

// Per calling thread, as the C header documents for get/set_current_stream.
inline thread_local std::vector<SyclCurrentStream> t_current_streams;

class SyclExecutor : public ice::builder::Executor
{
public:
    using PeerAccessCallback = std::function<bool(int, int)>;

    explicit SyclExecutor(const SyclOpsTable& ops) noexcept :
        ice::builder::Executor{
            ops.getAllocatorOps(),
            ops.getDeviceOps(),
            ops.getEventOps(),
            ops.getRandomGeneratorOps(),
            ops.getStatusOps(),
            ops.getStreamOps(),
            ops.getTimerOps()
        },
        m_status{ops},
        m_queue_factory{ops}
    {
    }

    ~SyclExecutor() override = default;
    SyclExecutor(const SyclExecutor&) = delete;
    SyclExecutor& operator=(const SyclExecutor&) = delete;
    SyclExecutor(SyclExecutor&&) = delete;
    SyclExecutor& operator=(SyclExecutor&&) = delete;

    void bind(const sycl::context& context, PeerAccessCallback enable_peer)
    {
        m_context = context;
        m_enable_peer = std::move(enable_peer);
    }

    void release() noexcept
    {
        m_stream_pool = SyclStreamPool{};
        m_owned_queues.clear();
        m_default_generators.clear();
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void device_memory_usage(
        const ice::sonic::Device& device,
        std::int64_t& out_free,
        std::int64_t& out_total,
        bool& out_success
    ) noexcept override
    {
        const auto& native = native_device(device);
        out_total = static_cast<int64_t>(native.get_info<sycl::info::device::global_mem_size>());
        if (!native.has(sycl::aspect::ext_intel_free_memory)) {
            out_success = false;
            return;
        }
        out_free =
            static_cast<int64_t>(native.get_info<sycl::ext::intel::info::device::free_memory>());
        out_success = true;
    }

    void create_stream_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        bind_new_stream(device, stream, 0, out_status);
    }

    void destroy_stream_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream
    ) noexcept override
    {
        static_cast<void>(device);
        static_cast<SyclStream&>(SyclStream::from_handle(stream.get_handle())).release();
    }

    void create_stream_dependency(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& dependent,
        const ice::sonic::Stream& other,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            auto barrier = queue_of(other).ext_oneapi_submit_barrier();
            queue_of(dependent).ext_oneapi_submit_barrier({barrier});
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void get_stream_status(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        const auto& error =
            static_cast<SyclStream&>(SyclStream::from_handle(stream.get_handle())).getAsyncError();
        if (!error.empty()) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, error);
        }
    }

    void create_event_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        const ice::sonic::EventOptions options;
        static_cast<SyclEvent&>(SyclEvent::from_handle(event.get_handle()))
            .bind(options, device_index(device));
    }

    void destroy_event_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event
    ) noexcept override
    {
        static_cast<void>(device);
        static_cast<void>(event);
    }

    void get_event_status(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event,
        ice::sonic::EventStatus& out_event_status
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            out_event_status =
                static_cast<SyclEvent&>(SyclEvent::from_handle(event.get_handle())).is_complete()
                    ? ice::sonic::EventStatus::k_complete
                    : ice::sonic::EventStatus::k_pending;
        } catch (const sycl::exception&) {
            out_event_status = ice::sonic::EventStatus::k_error;
        }
    }

    void record_event(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        auto& target = static_cast<SyclEvent&>(SyclEvent::from_handle(event.get_handle()));
        if (target.getDeviceIndex() != -1 && target.getDeviceIndex() != device_index(device)) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "event device does not match stream"
            );
            return;
        }

        try {
            target.record(queue_of(stream));
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void wait_for_event(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        const auto& native =
            static_cast<SyclEvent&>(SyclEvent::from_handle(event.get_handle())).getNativeEvent();
        if (!native) {
            return;
        }

        try {
            queue_of(stream).ext_oneapi_submit_barrier({*native});
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void create_timer_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Timer& timer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        static_cast<void>(timer);
        static_cast<void>(out_status);
    }

    void destroy_timer_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Timer& timer
    ) noexcept override
    {
        static_cast<void>(device);
        static_cast<void>(timer);
    }

    void start_timer(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Timer& timer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            static_cast<SyclTimer&>(SyclTimer::from_handle(timer.get_handle()))
                .setStartEvent(queue_of(stream).ext_oneapi_submit_barrier());
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void stop_timer(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Timer& timer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            static_cast<SyclTimer&>(SyclTimer::from_handle(timer.get_handle()))
                .setStopEvent(queue_of(stream).ext_oneapi_submit_barrier());
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void memcpy_dtoh(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        void* host_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        submit_copy(stream, host_dst, device_src->getOpaque(), size, out_status);
    }

    void memcpy_htod(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        submit_copy(stream, device_dst->getOpaque(), host_src, size, out_status);
    }

    void memcpy_dtod(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* device_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        submit_copy(stream, device_dst->getOpaque(), device_src->getOpaque(), size, out_status);
    }

    void sync_memcpy_dtoh(
        const ice::sonic::Device& device,
        void* host_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        blocking_copy(device, host_dst, device_src->getOpaque(), size, out_status);
    }

    void sync_memcpy_htod(
        const ice::sonic::Device& device,
        ice::sonic::DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        blocking_copy(device, device_dst->getOpaque(), host_src, size, out_status);
    }

    void sync_memcpy_dtod(
        const ice::sonic::Device& device,
        ice::sonic::DeviceMemoryBase* device_dst,
        const ice::sonic::DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        blocking_copy(device, device_dst->getOpaque(), device_src->getOpaque(), size, out_status);
    }

    void block_host_for_event(
        const ice::sonic::Device& device,
        const ice::sonic::Event& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        const auto& native =
            static_cast<SyclEvent&>(SyclEvent::from_handle(event.get_handle())).getNativeEvent();
        if (!native) {
            return;
        }

        try {
            native->wait_and_throw();
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void block_host_until_done(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            queue_of(stream).wait_and_throw();
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void synchronize_all_activity(
        const ice::sonic::Device& device,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            m_stream_pool.synchronize(device_index(device));
            for (const auto& [index, queue]: m_owned_queues) {
                if (index == device_index(device)) {
                    queue->wait_and_throw();
                }
            }
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void mem_zero(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        submit_fill(device, stream, location, 0, size, out_status);
    }

    void memset(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        submit_fill(device, stream, location, pattern, size, out_status);
    }

    void memset32(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            queue_of(stream).fill(
                static_cast<uint32_t*>(location->getOpaque()),
                pattern,
                size / sizeof(uint32_t)
            );
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void host_callback(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        bool& out_success
    ) noexcept override
    {
        static_cast<void>(device);
        try {
            const auto* status_ops = m_status.getOps().getStatusOps();
            queue_of(stream).submit(
                [callback_fn, callback_arg, status_ops](sycl::handler& handler)
                {
                    handler.host_task(
                        [callback_fn, callback_arg, status_ops]()
                        {
                            ice::sonic::Status status{status_ops};
                            status.create();
                            callback_fn(callback_arg, status.get_handle());
                            status.destroy();
                        }
                    );
                }
            );
            out_success = true;
        } catch (const sycl::exception&) {
            out_success = false;
        }
    }

    void create_stream_with_options(
        const ice::sonic::Device& device,
        const ice::sonic::StreamOptions* options,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        bind_new_stream(
            device,
            stream,
            options == nullptr ? 0 : options->getPriority(),
            out_status
        );
    }

    void get_stream_from_pool(
        const ice::sonic::Device& device,
        int32_t priority,
        const ice::sonic::Stream& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            m_stream_pool.acquire_into(
                static_cast<SyclStream&>(SyclStream::from_handle(out_stream.get_handle())),
                m_queue_factory,
                m_context,
                native_device(device),
                device_index(device),
                priority
            );
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void get_current_stream(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        auto& target = static_cast<SyclStream&>(SyclStream::from_handle(out_stream.get_handle()));
        auto& [queue, async_error, priority] = current_stream_slot(device_index(device));
        if (!queue) {
            get_stream_from_pool(device, 0, out_stream, out_status);
            queue = target.getSharedQueue();
            async_error = target.getSharedAsyncError();
            priority = 0;
            return;
        }
        target.bind(queue, async_error, device_index(device), priority);
    }

    void set_current_stream(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        auto& source = static_cast<SyclStream&>(SyclStream::from_handle(stream.get_handle()));
        auto& [queue, async_error, priority] = current_stream_slot(device_index(device));
        queue = source.getSharedQueue();
        async_error = source.getSharedAsyncError();
        source.get_priority(priority);
    }

    void create_stream_from_native(
        const ice::sonic::Device& device,
        void* native_handle,
        const ice::sonic::Stream& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        auto* native = static_cast<sycl::queue*>(native_handle);
        if (native == nullptr || !native->is_in_order()) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "external queue must be in-order"
            );
            return;
        }
        static_cast<SyclStream&>(SyclStream::from_handle(out_stream.get_handle()))
            .bind(
                std::make_shared<sycl::queue>(*native),
                std::make_shared<std::string>(),
                device_index(device),
                0
            );
    }

    void create_event_with_options_internal(
        const ice::sonic::Device& device,
        const ice::sonic::EventOptions* options,
        const ice::sonic::Event& out_event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (options->getEnableIpc() && options->getEnableTiming()) {
            m_status.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "ipc events cannot enable timing"
            );
            return;
        }
        static_cast<SyclEvent&>(SyclEvent::from_handle(out_event.get_handle()))
            .bind(*options, device_index(device));
    }

    void create_event_from_ipc_internal(
        const ice::sonic::Device& device,
        const ice::sonic::IpcEventHandle* handle,
        const ice::sonic::Event& out_event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            static_cast<SyclEvent&>(SyclEvent::from_handle(out_event.get_handle()))
                .bind_from_ipc(*handle, m_context, native_device(device), device_index(device));
        } catch (const std::exception& error) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, error.what());
        }
    }

    void create_allocator_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Allocator& out_allocator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        try {
            static_cast<SyclAllocator&>(SyclAllocator::from_handle(out_allocator.get_handle()))
                .bind(m_context, native_device(device), device_index(device), m_enable_peer);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void destroy_allocator_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Allocator& allocator
    ) noexcept override
    {
        static_cast<void>(device);
        static_cast<SyclAllocator&>(SyclAllocator::from_handle(allocator.get_handle())).release();
    }

    void create_random_generator_internal(
        const ice::sonic::Device& device,
        uint64_t seed,
        const ice::sonic::RandomGenerator& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        static_cast<SyclRandomGenerator&>(
            SyclRandomGenerator::from_handle(out_generator.get_handle())
        )
            .bind(std::make_shared<SyclPhiloxState>(seed), device_index(device));
    }

    void destroy_random_generator_internal(
        const ice::sonic::Device& device,
        const ice::sonic::RandomGenerator& generator
    ) noexcept override
    {
        static_cast<void>(device);
        static_cast<SyclRandomGenerator&>(SyclRandomGenerator::from_handle(generator.get_handle()))
            .release();
    }

    void get_default_random_generator(
        const ice::sonic::Device& device,
        const ice::sonic::RandomGenerator& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        static_cast<SyclRandomGenerator&>(
            SyclRandomGenerator::from_handle(out_generator.get_handle())
        )
            .bind(getDefaultGeneratorState(device_index(device)), device_index(device));
    }

    void get_native_handle(const ice::sonic::Device& device, void** out_handle) noexcept override
    {
        static_cast<void>(device);
        *out_handle = &m_context;
    }

    const std::shared_ptr<SyclPhiloxState>& getDefaultGeneratorState(int device_index)
    {
        const auto index = static_cast<std::size_t>(device_index);
        if (index >= m_default_generators.size()) {
            m_default_generators.resize(index + 1);
        }
        auto& state = m_default_generators[index];
        if (!state) {
            state = std::make_shared<SyclPhiloxState>(SyclPhiloxState::k_default_seed);
        }
        return state;
    }

    const sycl::context& getContext() const noexcept
    {
        return m_context;
    }

private:
    using CurrentStream = SyclCurrentStream;

    CurrentStream& current_stream_slot(int device_index) const
    {
        const auto index = static_cast<std::size_t>(device_index);
        if (index >= t_current_streams.size()) {
            t_current_streams.resize(index + 1);
        }
        return t_current_streams[index];
    }

    const sycl::device& native_device(const ice::sonic::Device& device) const
    {
        return static_cast<SyclDevice&>(SyclDevice::from_handle(device.get_handle()))
            .getNativeDevice();
    }

    int device_index(const ice::sonic::Device& device) const
    {
        return static_cast<SyclDevice&>(SyclDevice::from_handle(device.get_handle()))
            .getDeviceIndex();
    }

    sycl::queue& queue_of(const ice::sonic::Stream& stream) const
    {
        return static_cast<SyclStream&>(SyclStream::from_handle(stream.get_handle()))
            .getNativeQueue();
    }

    void bind_new_stream(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        int32_t priority,
        const ice::sonic::Status& out_status
    )
    {
        try {
            auto sink = std::make_shared<std::string>();
            auto queue =
                m_queue_factory.create_queue(m_context, native_device(device), priority, sink);
            m_owned_queues.emplace_back(device_index(device), queue);
            static_cast<SyclStream&>(SyclStream::from_handle(stream.get_handle()))
                .bind(std::move(queue), std::move(sink), device_index(device), priority);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void submit_copy(
        const ice::sonic::Stream& stream,
        void* destination,
        const void* source,
        uint64_t size,
        const ice::sonic::Status& out_status
    )
    {
        try {
            queue_of(stream).memcpy(destination, source, size);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void submit_fill(
        const ice::sonic::Device& device,
        const ice::sonic::Stream& stream,
        ice::sonic::DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    )
    {
        static_cast<void>(device);
        try {
            queue_of(stream).memset(location->getOpaque(), pattern, size);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    void blocking_copy(
        const ice::sonic::Device& device,
        void* destination,
        const void* source,
        uint64_t size,
        const ice::sonic::Status& out_status
    )
    {
        try {
            sycl::queue queue{m_context, native_device(device)};
            queue.memcpy(destination, source, size).wait_and_throw();
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }
    }

    SyclStatus m_status;
    SyclQueueFactory m_queue_factory;
    sycl::context m_context;
    PeerAccessCallback m_enable_peer;
    SyclStreamPool m_stream_pool;
    std::vector<std::pair<int, std::shared_ptr<sycl::queue>>> m_owned_queues;
    std::vector<std::shared_ptr<SyclPhiloxState>> m_default_generators;
};

} // namespace aten_xpu
