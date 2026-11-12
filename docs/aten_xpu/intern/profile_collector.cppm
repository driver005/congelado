module;

#include <sycl/sycl.hpp>

export module aten_xpu_intern:profile_collector;

import std;
import :profile_activity;
import :profile_memory_event;

export namespace aten_xpu {

// Collects the events of copies, fills and primitives while a profiler session runs. Kernels are
// launched from host threads the plugin does not own, so the activity list is mutex guarded.
class SyclProfileCollector
{
public:
    // Queues only get SYCL profiling when this is set before they are created.
    [[nodiscard]] static bool isRequested() noexcept
    {
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        const char* value = std::getenv("ATEN_XPU_PROFILING");

        return value != nullptr && std::string_view{value} == "1";
    }

    // True when the device can report command_submit/start/end for its queues.
    [[nodiscard]] static bool supportsDevice(const sycl::device& device)
    {
        return device.has(sycl::aspect::queue_profiling);
    }

    void setEnabled(bool enabled) noexcept
    {
        m_enabled.store(enabled);
    }

    [[nodiscard]] bool getEnabled() const noexcept
    {
        return m_enabled.load();
    }

    void record(
        const sycl::queue& queue,
        std::string name,
        SyclProfileActivity::Kind kind,
        std::uint64_t bytes,
        const sycl::event& event
    )
    {
        if (!m_enabled.load()) {
            return;
        }

        const auto submit_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                   std::chrono::system_clock::now().time_since_epoch()
        )
                                   .count();

        const std::lock_guard<std::mutex> lock{m_mutex};
        m_activities.emplace_back(
            std::move(name),
            kind,
            device_index_of(queue.get_device()),
            queue_id_of(queue),
            bytes,
            submit_ns,
            event
        );
    }

    // Allocator actions arrive without an event; `queue` may be null when the allocator has none.
    void recordMemory(
        std::string action,
        std::uintptr_t address,
        std::uint64_t bytes,
        const sycl::queue* queue
    )
    {
        if (!m_enabled.load()) {
            return;
        }

        const auto wall_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();

        const std::lock_guard<std::mutex> lock{m_mutex};
        m_memory_events.emplace_back(
            std::move(action),
            address,
            bytes,
            queue == nullptr ? -1 : device_index_of(queue->get_device()),
            wall_ns
        );
    }

    [[nodiscard]] std::vector<SyclProfileMemoryEvent> drainMemory()
    {
        const std::lock_guard<std::mutex> lock{m_mutex};

        return std::exchange(m_memory_events, {});
    }

    // Hands over everything recorded so far and starts an empty list.
    [[nodiscard]] std::vector<SyclProfileActivity> drain()
    {
        const std::lock_guard<std::mutex> lock{m_mutex};

        return std::exchange(m_activities, {});
    }

    // Every SYCL device in a stable order; the index is the XSpace plane number.
    [[nodiscard]] std::vector<sycl::device> getDevices()
    {
        const std::lock_guard<std::mutex> lock{m_mutex};
        load_devices();

        return m_devices;
    }

private:
    void load_devices()
    {
        if (m_devices.empty()) {
            m_devices = sycl::device::get_devices();
        }
    }

    int device_index_of(const sycl::device& device)
    {
        load_devices();
        const auto found = std::ranges::find(m_devices, device);

        return found == m_devices.end() ? -1 : static_cast<int>(found - m_devices.begin());
    }

    int queue_id_of(const sycl::queue& queue)
    {
        const auto [position, inserted] =
            m_queue_ids.try_emplace(queue, static_cast<int>(m_queue_ids.size()));

        return position->second;
    }

    std::atomic<bool> m_enabled{false};
    std::mutex m_mutex;
    std::vector<sycl::device> m_devices;
    std::unordered_map<sycl::queue, int> m_queue_ids;
    std::vector<SyclProfileActivity> m_activities;
    std::vector<SyclProfileMemoryEvent> m_memory_events;
};

} // namespace aten_xpu
