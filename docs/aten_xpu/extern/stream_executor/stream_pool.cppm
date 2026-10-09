module;

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:stream_pool;

import std;
import :stream;

export namespace aten_xpu {

class SyclStreamPool
{
public:
    static constexpr std::size_t k_streams_per_priority = 32;
    static constexpr std::size_t k_priority_count = 3;

    void acquire_into(
        SyclStream& stream,
        const sycl::context& context,
        const sycl::device& device,
        int device_index,
        int32_t priority
    )
    {
        const auto slot = slot_index(device_index, priority);
        ensure_filled(slot, context, device, priority);

        auto& counter = m_counters[slot];
        const auto position = counter % k_streams_per_priority;
        ++counter;
        stream
            .bind(m_queues[slot][position], m_async_errors[slot][position], device_index, priority);
    }

    void synchronize(int device_index)
    {
        for (std::size_t level = 0; level < k_priority_count; ++level) {
            const auto slot = static_cast<std::size_t>(device_index) * k_priority_count + level;
            if (slot >= m_queues.size()) {
                continue;
            }
            for (const auto& queue: m_queues[slot]) {
                queue->wait_and_throw();
            }
        }
    }

    static std::size_t priority_level(int32_t priority) noexcept
    {
        if (priority < 0) {
            return 2;
        }
        if (priority > 0) {
            return 0;
        }
        return 1;
    }

private:
    std::size_t slot_index(int device_index, int32_t priority)
    {
        const auto slot =
            static_cast<std::size_t>(device_index) * k_priority_count + priority_level(priority);
        if (slot >= m_queues.size()) {
            m_queues.resize(slot + 1);
            m_async_errors.resize(slot + 1);
            m_counters.resize(slot + 1, 0);
        }
        return slot;
    }

    void ensure_filled(
        std::size_t slot,
        const sycl::context& context,
        const sycl::device& device,
        int32_t priority
    )
    {
        if (!m_queues[slot].empty()) {
            return;
        }

        m_queues[slot].reserve(k_streams_per_priority);
        m_async_errors[slot].reserve(k_streams_per_priority);
        for (std::size_t index = 0; index < k_streams_per_priority; ++index) {
            auto sink = std::make_shared<std::string>();
            m_queues[slot].push_back(SyclStream::create_queue(context, device, priority, sink));
            m_async_errors[slot].push_back(std::move(sink));
        }
    }

    std::vector<std::vector<std::shared_ptr<sycl::queue>>> m_queues;
    std::vector<std::vector<std::shared_ptr<std::string>>> m_async_errors;
    std::vector<std::size_t> m_counters;
};

} // namespace aten_xpu
