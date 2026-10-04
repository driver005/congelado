module;

#include "include/c/extern/stream_executor/mem_pool.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_trace;

import std;

export namespace aten_xpu {

class SyclAllocatorTrace
{
public:
    enum class Action : std::uint8_t
    {
        alloc,
        free_requested,
        free_completed,
        segment_alloc,
        segment_free,
        segment_map,
        segment_unmap,
        snapshot,
        out_of_memory
    };

    using Entry = std::tuple<Action, std::uintptr_t, std::size_t, sycl::queue*, TF_PoolId>;
    using Tracker = std::function<void(const Entry&)>;

    void addTracker(Tracker tracker) { m_trackers.push_back(std::move(tracker)); }

    void setMaxEntries(std::size_t max_entries)
    {

        m_max_entries = max_entries;
        m_entries.clear();
        m_next = 0;

    }

    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }

    bool getEnabled() const noexcept { return m_enabled; }

    void record(
        Action action,
        const void* address,
        std::size_t size,
        sycl::queue* queue,
        TF_PoolId pool
    )
    {

        const Entry entry{action, reinterpret_cast<std::uintptr_t>(address), size, queue, pool};
        for (const auto& tracker: m_trackers) {
            tracker(entry);
        }
        if (!m_enabled || m_max_entries == 0) {
            return;
        }
        if (m_entries.size() < m_max_entries) {
            m_entries.push_back(entry);
            return;
        }
        m_entries[m_next] = entry;
        m_next = (m_next + 1) % m_max_entries;

    }

    template<typename Visitor>
    void for_each(Visitor&& visitor) const
    {

        for (std::size_t offset = 0; offset < m_entries.size(); ++offset) {
            visitor(m_entries[(m_next + offset) % m_entries.size()]);
        }

    }

    static std::string_view action_name(Action action) noexcept
    {

        switch (action) {
            case Action::alloc:
                return "alloc";
            case Action::free_requested:
                return "free_requested";
            case Action::free_completed:
                return "free_completed";
            case Action::segment_alloc:
                return "segment_alloc";
            case Action::segment_free:
                return "segment_free";
            case Action::segment_map:
                return "segment_map";
            case Action::segment_unmap:
                return "segment_unmap";
            case Action::snapshot:
                return "snapshot";
            case Action::out_of_memory:
                return "oom";
        }
        return "unknown";

    }

private:
    bool m_enabled{false};
    std::size_t m_max_entries{0};
    std::size_t m_next{0};
    std::vector<Entry> m_entries;
    std::vector<Tracker> m_trackers;
};

} // namespace aten_xpu
