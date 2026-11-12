export module aten_xpu_intern:profile_memory_event;

import std;

export namespace aten_xpu {

// One allocator action (alloc, free, segment map, out of memory, ...) at a wall clock time.
class SyclProfileMemoryEvent
{
public:
    SyclProfileMemoryEvent(
        std::string action,
        std::uintptr_t address,
        std::uint64_t bytes,
        int device_index,
        std::int64_t wall_ns
    ) :
        m_action{std::move(action)},
        m_address{address},
        m_bytes{bytes},
        m_device_index{device_index},
        m_wall_ns{wall_ns}
    {
    }

    [[nodiscard]] const std::string& getAction() const noexcept
    {
        return m_action;
    }

    [[nodiscard]] std::uintptr_t getAddress() const noexcept
    {
        return m_address;
    }

    [[nodiscard]] std::uint64_t getBytes() const noexcept
    {
        return m_bytes;
    }

    // -1 when the allocator did not know the queue (for example an out of memory report).
    [[nodiscard]] int getDeviceIndex() const noexcept
    {
        return m_device_index;
    }

    [[nodiscard]] std::int64_t getWallNs() const noexcept
    {
        return m_wall_ns;
    }

private:
    std::string m_action;
    std::uintptr_t m_address;
    std::uint64_t m_bytes;
    int m_device_index;
    std::int64_t m_wall_ns;
};

} // namespace aten_xpu
