export module aten_xpu_profiler_common:device_activity;

import std;
import :kind;

export namespace aten_xpu {

// One kernel or copy a vendor tracer reported, already on the wall clock.
class ProfileDeviceActivity
{
public:
    ProfileDeviceActivity(
        std::string device,
        std::string name,
        ProfileKind kind,
        std::uint64_t queue,
        std::int64_t start_wall_ns,
        std::int64_t duration_ns,
        std::uint64_t bytes
    ) :
        m_device{std::move(device)},
        m_name{std::move(name)},
        m_kind{kind},
        m_queue{queue},
        m_start_wall_ns{start_wall_ns},
        m_duration_ns{duration_ns},
        m_bytes{bytes}
    {
    }

    // Backend specific device label, such as a PCI address or a CUDA ordinal.
    [[nodiscard]] const std::string& getDevice() const noexcept
    {
        return m_device;
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] ProfileKind getKind() const noexcept
    {
        return m_kind;
    }

    [[nodiscard]] std::uint64_t getQueue() const noexcept
    {
        return m_queue;
    }

    [[nodiscard]] std::int64_t getStartWallNs() const noexcept
    {
        return m_start_wall_ns;
    }

    [[nodiscard]] std::int64_t getDurationNs() const noexcept
    {
        return m_duration_ns;
    }

    [[nodiscard]] std::uint64_t getBytes() const noexcept
    {
        return m_bytes;
    }

private:
    std::string m_device;
    std::string m_name;
    ProfileKind m_kind;
    std::uint64_t m_queue;
    std::int64_t m_start_wall_ns;
    std::int64_t m_duration_ns;
    std::uint64_t m_bytes;
};

} // namespace aten_xpu
