module;

#include "include/c/extern/stream_executor/allocator.h"

export module aten_xpu_extern_stream_executor:allocator_stats;

import std;
import cc_ice_extern_stream_executor_sonic;

export namespace aten_xpu {

class SyclAllocatorStats
{
public:
    enum class Kind : std::uint8_t
    {
        allocation,
        segment,
        active,
        inactive_split,
        allocated_bytes,
        reserved_bytes,
        active_bytes,
        inactive_split_bytes,
        requested_bytes,
        count
    };

    enum class Scope : std::uint8_t
    {
        aggregate,
        small_pool,
        large_pool,
        count
    };

    void setBytesLimit(std::optional<int64_t> limit) noexcept
    {
        m_bytes_limit = limit;
    }

    int64_t getCurrent(Kind kind) const noexcept
    {
        return entry(kind, Scope::aggregate)[k_current];
    }

    int64_t getPeak(Kind kind) const noexcept
    {
        return entry(kind, Scope::aggregate)[k_peak];
    }

    int64_t getAllocRetries() const noexcept
    {
        return m_alloc_retries;
    }

    int64_t getOutOfMemoryCount() const noexcept
    {
        return m_out_of_memory_count;
    }

    void increase(Kind kind, bool small_pool, int64_t amount) noexcept
    {
        apply(kind, Scope::aggregate, amount);
        apply(kind, small_pool ? Scope::small_pool : Scope::large_pool, amount);
    }

    void decrease(Kind kind, bool small_pool, int64_t amount) noexcept
    {
        increase(kind, small_pool, -amount);
    }

    void record_alloc_retry() noexcept
    {
        ++m_alloc_retries;
    }

    void record_out_of_memory() noexcept
    {
        ++m_out_of_memory_count;
    }

    void observe_allocation_size(int64_t size) noexcept
    {
        m_largest_alloc_size = std::max(m_largest_alloc_size, size);
    }

    void reset_accumulated() noexcept
    {
        for (auto& value: m_entries) {
            value[k_allocated] = 0;
            value[k_freed] = 0;
        }
        m_alloc_retries = 0;
        m_out_of_memory_count = 0;
    }

    void reset_peak() noexcept
    {
        for (auto& value: m_entries) {
            value[k_peak] = value[k_current];
        }
        m_largest_alloc_size = 0;
    }

    void fill(ice::sonic::AllocatorStats& stats, int64_t largest_free_block) const noexcept
    {
        stats = ice::sonic::AllocatorStats{};
        stats.setNumAllocs(entry(Kind::allocation, Scope::aggregate)[k_allocated]);
        stats.setBytesInUse(getCurrent(Kind::allocated_bytes));
        stats.setPeakBytesInUse(getPeak(Kind::allocated_bytes));
        stats.setLargestAllocSize(m_largest_alloc_size);
        stats.setBytesReserved(getCurrent(Kind::reserved_bytes));
        stats.setPeakBytesReserved(getPeak(Kind::reserved_bytes));
        stats.setLargestFreeBlockBytes(largest_free_block);
        if (m_bytes_limit) {
            stats.setHasBytesLimit(1);
            stats.setBytesLimit(*m_bytes_limit);
            stats.setHasBytesReservableLimit(1);
            stats.setBytesReservableLimit(*m_bytes_limit);
        }
    }

private:
    using Entry = std::array<int64_t, 4>;

    enum Slot : std::size_t
    {
        k_current = 0,
        k_peak = 1,
        k_allocated = 2,
        k_freed = 3
    };

    enum Extent : std::size_t
    {
        k_scope_count = static_cast<std::size_t>(Scope::count)
    };

    const Entry& entry(Kind kind, Scope scope) const noexcept
    {
        return m_entries
            [static_cast<std::size_t>(kind) * k_scope_count + static_cast<std::size_t>(scope)];
    }

    void apply(Kind kind, Scope scope, int64_t amount) noexcept
    {
        auto& value = m_entries
            [static_cast<std::size_t>(kind) * k_scope_count + static_cast<std::size_t>(scope)];
        value[k_current] += amount;
        value[k_peak] = std::max(value[k_peak], value[k_current]);
        if (amount > 0) {
            value[k_allocated] += amount;
        } else {
            value[k_freed] -= amount;
        }
    }

    std::array<Entry, static_cast<std::size_t>(Kind::count) * k_scope_count> m_entries{};
    int64_t m_alloc_retries{0};
    int64_t m_out_of_memory_count{0};
    int64_t m_largest_alloc_size{0};
    std::optional<int64_t> m_bytes_limit;
};

} // namespace aten_xpu
