module;

#include <sycl/sycl.hpp>

export module aten_xpu_intern:profile_activity;

import std;
import aten_xpu_profiler_common;

export namespace aten_xpu {

// One submission that returned a sycl::event; its device timestamps are read once it completed.
class SyclProfileActivity
{
public:
    using Kind = ProfileKind;

    SyclProfileActivity(
        std::string name,
        Kind kind,
        int device_index,
        int queue_id,
        std::uint64_t bytes,
        std::int64_t host_submit_ns,
        sycl::event event
    ) :
        m_name{std::move(name)},
        m_kind{kind},
        m_device_index{device_index},
        m_queue_id{queue_id},
        m_bytes{bytes},
        m_host_submit_ns{host_submit_ns},
        m_event{std::move(event)}
    {
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] Kind getKind() const noexcept
    {
        return m_kind;
    }

    [[nodiscard]] int getDeviceIndex() const noexcept
    {
        return m_device_index;
    }

    [[nodiscard]] int getQueueId() const noexcept
    {
        return m_queue_id;
    }

    [[nodiscard]] std::uint64_t getBytes() const noexcept
    {
        return m_bytes;
    }

    // Wall clock (system_clock) nanoseconds when the work was handed to the queue.
    [[nodiscard]] std::int64_t getHostSubmitNs() const noexcept
    {
        return m_host_submit_ns;
    }

    [[nodiscard]] const sycl::event& getEvent() const noexcept
    {
        return m_event;
    }

private:
    std::string m_name;
    Kind m_kind;
    int m_device_index;
    int m_queue_id;
    std::uint64_t m_bytes;
    std::int64_t m_host_submit_ns;
    sycl::event m_event;
};

} // namespace aten_xpu
