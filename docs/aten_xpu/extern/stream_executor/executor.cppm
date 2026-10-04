module;

#include "include/c/extern/stream_executor/executor.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:executor;

import std;
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

class SyclExecutor : public ice::builder::TF_ExecutorOps
{
public:
    using PeerAccessCallback = std::function<bool(int, int)>;

    explicit SyclExecutor(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_ExecutorOps{
            ops.getAllocatorOps(),
            ops.getDeviceOps(),
            ops.getEventOps(),
            ops.getRandomGeneratorOps(),
            ops.getStatusOps(),
            ops.getStreamOps(),
            ops.getTimerOps()
        },
        m_status{ops}
    {
    }

    ~SyclExecutor() override = default;
    SyclExecutor(const SyclExecutor&) = delete;
    SyclExecutor& operator=(const SyclExecutor&) = delete;
    SyclExecutor(SyclExecutor&&) = delete;
    SyclExecutor& operator=(SyclExecutor&&) = delete;

    static void create(::TF_Executor* handle)
    {

        auto* executor = new SyclExecutor{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *executor);

    }

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

    void destroy() noexcept override { delete this; }

    void device_memory_usage(
        const ice::sonic::TF_DeviceOps& device,
        int64_t* out_free,
        int64_t* out_total,
        _Bool* out_success
    ) noexcept override
    {

        const auto& native = native_device(device);
        *out_total = static_cast<int64_t>(native.get_info<sycl::info::device::global_mem_size>());
        if (!native.has(sycl::aspect::ext_intel_free_memory)) {
            *out_success = false;
            return;
        }
        *out_free = static_cast<int64_t>(native.get_info<sycl::ext::intel::info::device::free_memory>());
        *out_success = true;

    }

    void create_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        bind_new_stream(device, stream, 0, out_status);

    }

    void destroy_stream_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream
    ) noexcept override
    {

        static_cast<void>(device);
        SyclHandle::resolve<SyclStream>(stream).release();

    }

    void create_stream_dependency(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& dependent,
        const ice::sonic::TF_StreamOps& other,
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        const auto& error = SyclHandle::resolve<SyclStream>(stream).getAsyncError();
        if (!error.empty()) {
            m_status.fail(out_status, TF_INTERNAL, error);
        }

    }

    void create_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        const TF_EventOptions options{.struct_size = sizeof(TF_EventOptions)};
        SyclHandle::resolve<SyclEvent>(event).bind(options, device_index(device));

    }

    void destroy_event_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event
    ) noexcept override
    {

        static_cast<void>(device);
        static_cast<void>(event);

    }

    void get_event_status(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        TF_EventStatus* out_event_status
    ) noexcept override
    {

        static_cast<void>(device);
        try {
            *out_event_status = SyclHandle::resolve<SyclEvent>(event).is_complete() ? TF_EVENT_COMPLETE
                                                                                   : TF_EVENT_PENDING;
        } catch (const sycl::exception&) {
            *out_event_status = TF_EVENT_ERROR;
        }

    }

    void record_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        auto& target = SyclHandle::resolve<SyclEvent>(event);
        if (target.getDeviceIndex() != -1 && target.getDeviceIndex() != device_index(device)) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "event device does not match stream");
            return;
        }

        try {
            target.record(queue_of(stream));
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void wait_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        const auto& native = SyclHandle::resolve<SyclEvent>(event).getNativeEvent();
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        static_cast<void>(timer);
        static_cast<void>(out_status);

    }

    void destroy_timer_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_TimerOps& timer
    ) noexcept override
    {

        static_cast<void>(device);
        static_cast<void>(timer);

    }

    void start_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        try {
            SyclHandle::resolve<SyclTimer>(timer).setStartEvent(queue_of(stream).ext_oneapi_submit_barrier());
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void stop_timer(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::TF_TimerOps& timer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        try {
            SyclHandle::resolve<SyclTimer>(timer).setStopEvent(queue_of(stream).ext_oneapi_submit_barrier());
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        submit_copy(stream, host_dst, device_src->opaque, size, out_status);

    }

    void memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        submit_copy(stream, device_dst->opaque, host_src, size, out_status);

    }

    void memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        submit_copy(stream, device_dst->opaque, device_src->opaque, size, out_status);

    }

    void sync_memcpy_dtoh(
        const ice::sonic::TF_DeviceOps& device,
        void* host_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        blocking_copy(device, host_dst, device_src->opaque, size, out_status);

    }

    void sync_memcpy_htod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const void* host_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        blocking_copy(device, device_dst->opaque, host_src, size, out_status);

    }

    void sync_memcpy_dtod(
        const ice::sonic::TF_DeviceOps& device,
        TF_DeviceMemoryBase* device_dst,
        const TF_DeviceMemoryBase* device_src,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        blocking_copy(device, device_dst->opaque, device_src->opaque, size, out_status);

    }

    void block_host_for_event(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_EventOps& event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        const auto& native = SyclHandle::resolve<SyclEvent>(event).getNativeEvent();
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
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
        const ice::sonic::TF_DeviceOps& device,
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        submit_fill(device, stream, location, 0, size, out_status);

    }

    void memset(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        submit_fill(device, stream, location, pattern, size, out_status);

    }

    void memset32(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint32_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(device);
        try {
            queue_of(stream).fill(static_cast<uint32_t*>(location->opaque), pattern, size / sizeof(uint32_t));
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void host_callback(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_StatusCallbackFn callback_fn,
        void* callback_arg,
        _Bool* out_success
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
            *out_success = true;
        } catch (const sycl::exception&) {
            *out_success = false;
        }

    }

    void create_stream_with_options(
        const ice::sonic::TF_DeviceOps& device,
        const TF_StreamOptions* options,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        bind_new_stream(device, stream, options == nullptr ? 0 : options->priority, out_status);

    }

    void get_stream_from_pool(
        const ice::sonic::TF_DeviceOps& device,
        int32_t priority,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            m_stream_pool.acquire_into(
                SyclHandle::resolve<SyclStream>(out_stream),
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        auto& target = SyclHandle::resolve<SyclStream>(out_stream);
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        auto& source = SyclHandle::resolve<SyclStream>(stream);
        auto& [queue, async_error, priority] = current_stream_slot(device_index(device));
        queue = source.getSharedQueue();
        async_error = source.getSharedAsyncError();
        source.get_priority(&priority);

    }

    void create_stream_from_native(
        const ice::sonic::TF_DeviceOps& device,
        void* native_handle,
        const ice::sonic::TF_StreamOps& out_stream,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        auto* native = static_cast<sycl::queue*>(native_handle);
        if (native == nullptr || !native->is_in_order()) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "external queue must be in-order");
            return;
        }
        SyclHandle::resolve<SyclStream>(out_stream).bind(
            std::make_shared<sycl::queue>(*native),
            std::make_shared<std::string>(),
            device_index(device),
            0
        );

    }

    void create_event_with_options_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_EventOptions* options,
        const ice::sonic::TF_EventOps& out_event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (options->enable_ipc && options->enable_timing) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "ipc events cannot enable timing");
            return;
        }
        SyclHandle::resolve<SyclEvent>(out_event).bind(*options, device_index(device));

    }

    void create_event_from_ipc_internal(
        const ice::sonic::TF_DeviceOps& device,
        const TF_IpcEventHandle* handle,
        const ice::sonic::TF_EventOps& out_event,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            SyclHandle::resolve<SyclEvent>(out_event).bind_from_ipc(
                *handle,
                m_context,
                native_device(device),
                device_index(device)
            );
        } catch (const std::exception& error) {
            m_status.fail(out_status, TF_INTERNAL, error.what());
        }

    }

    void create_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& out_allocator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        try {
            SyclHandle::resolve<SyclAllocator>(out_allocator).bind(
                m_context,
                native_device(device),
                device_index(device),
                m_enable_peer
            );
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void destroy_allocator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_AllocatorOps& allocator
    ) noexcept override
    {

        static_cast<void>(device);
        SyclHandle::resolve<SyclAllocator>(allocator).release();

    }

    void create_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        uint64_t seed,
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        SyclHandle::resolve<SyclRandomGenerator>(out_generator)
            .bind(std::make_shared<SyclPhiloxState>(seed), device_index(device));

    }

    void destroy_random_generator_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& generator
    ) noexcept override
    {

        static_cast<void>(device);
        SyclHandle::resolve<SyclRandomGenerator>(generator).release();

    }

    void get_default_random_generator(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        SyclHandle::resolve<SyclRandomGenerator>(out_generator)
            .bind(getDefaultGeneratorState(device_index(device)), device_index(device));

    }

    void get_native_handle(const ice::sonic::TF_DeviceOps& device, void** out_handle) noexcept override
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

    const sycl::context& getContext() const noexcept { return m_context; }

private:
    using CurrentStream = std::tuple<std::shared_ptr<sycl::queue>, std::shared_ptr<std::string>, int32_t>;

    static CurrentStream& current_stream_slot(int device_index)
    {

        const auto index = static_cast<std::size_t>(device_index);
        if (index >= s_current_streams.size()) {
            s_current_streams.resize(index + 1);
        }
        return s_current_streams[index];

    }

    static const sycl::device& native_device(const ice::sonic::TF_DeviceOps& device)
    {

        return SyclHandle::resolve<SyclDevice>(device).getNativeDevice();

    }

    static int device_index(const ice::sonic::TF_DeviceOps& device)
    {

        return SyclHandle::resolve<SyclDevice>(device).getDeviceIndex();

    }

    static sycl::queue& queue_of(const ice::sonic::TF_StreamOps& stream)
    {

        return SyclHandle::resolve<SyclStream>(stream).getNativeQueue();

    }

    void bind_new_stream(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        int32_t priority,
        const ice::sonic::Status& out_status
    )
    {

        try {
            auto sink = std::make_shared<std::string>();
            auto queue = SyclStream::create_queue(m_context, native_device(device), priority, sink);
            m_owned_queues.emplace_back(device_index(device), queue);
            SyclHandle::resolve<SyclStream>(stream).bind(std::move(queue), std::move(sink), device_index(device), priority);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void submit_copy(
        const ice::sonic::TF_StreamOps& stream,
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
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TF_StreamOps& stream,
        TF_DeviceMemoryBase* location,
        uint8_t pattern,
        uint64_t size,
        const ice::sonic::Status& out_status
    )
    {

        static_cast<void>(device);
        try {
            queue_of(stream).memset(location->opaque, pattern, size);
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void blocking_copy(
        const ice::sonic::TF_DeviceOps& device,
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
    sycl::context m_context;
    PeerAccessCallback m_enable_peer;
    SyclStreamPool m_stream_pool;
    std::vector<std::pair<int, std::shared_ptr<sycl::queue>>> m_owned_queues;
    std::vector<std::shared_ptr<SyclPhiloxState>> m_default_generators;

    // Per calling thread, as the C header documents for get/set_current_stream.
    static thread_local std::vector<CurrentStream> s_current_streams;
};

inline thread_local std::vector<SyclExecutor::CurrentStream> SyclExecutor::s_current_streams;

} // namespace aten_xpu
