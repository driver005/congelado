// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/allocator.h"

export module cc_ice_extern_stream_executor_sonic:allocator_stats;

import std;

export namespace ice::sonic {

class AllocatorStats
{
public:
    AllocatorStats() noexcept
    {
        m_value.struct_size = sizeof(::TF_AllocatorStats);
    }

    explicit AllocatorStats(const ::TF_AllocatorStats& value) noexcept :
        m_value{value}
    {
    }

    AllocatorStats(
        std::int64_t num_allocs,
        std::int64_t bytes_in_use,
        std::int64_t peak_bytes_in_use,
        std::int64_t largest_alloc_size,
        std::int8_t has_bytes_limit,
        std::int64_t bytes_limit,
        std::int64_t bytes_reserved,
        std::int64_t peak_bytes_reserved,
        std::int8_t has_bytes_reservable_limit,
        std::int64_t bytes_reservable_limit,
        std::int64_t largest_free_block_bytes
    ) noexcept
    {
        m_value.struct_size = sizeof(::TF_AllocatorStats);
        m_value.num_allocs = num_allocs;
        m_value.bytes_in_use = bytes_in_use;
        m_value.peak_bytes_in_use = peak_bytes_in_use;
        m_value.largest_alloc_size = largest_alloc_size;
        m_value.has_bytes_limit = has_bytes_limit;
        m_value.bytes_limit = bytes_limit;
        m_value.bytes_reserved = bytes_reserved;
        m_value.peak_bytes_reserved = peak_bytes_reserved;
        m_value.has_bytes_reservable_limit = has_bytes_reservable_limit;
        m_value.bytes_reservable_limit = bytes_reservable_limit;
        m_value.largest_free_block_bytes = largest_free_block_bytes;
    }

    std::int64_t getNumAllocs() const noexcept
    {
        return m_value.num_allocs;
    }

    void setNumAllocs(std::int64_t value) noexcept
    {
        m_value.num_allocs = value;
    }

    std::int64_t getBytesInUse() const noexcept
    {
        return m_value.bytes_in_use;
    }

    void setBytesInUse(std::int64_t value) noexcept
    {
        m_value.bytes_in_use = value;
    }

    std::int64_t getPeakBytesInUse() const noexcept
    {
        return m_value.peak_bytes_in_use;
    }

    void setPeakBytesInUse(std::int64_t value) noexcept
    {
        m_value.peak_bytes_in_use = value;
    }

    std::int64_t getLargestAllocSize() const noexcept
    {
        return m_value.largest_alloc_size;
    }

    void setLargestAllocSize(std::int64_t value) noexcept
    {
        m_value.largest_alloc_size = value;
    }

    std::int8_t getHasBytesLimit() const noexcept
    {
        return m_value.has_bytes_limit;
    }

    void setHasBytesLimit(std::int8_t value) noexcept
    {
        m_value.has_bytes_limit = value;
    }

    std::int64_t getBytesLimit() const noexcept
    {
        return m_value.bytes_limit;
    }

    void setBytesLimit(std::int64_t value) noexcept
    {
        m_value.bytes_limit = value;
    }

    std::int64_t getBytesReserved() const noexcept
    {
        return m_value.bytes_reserved;
    }

    void setBytesReserved(std::int64_t value) noexcept
    {
        m_value.bytes_reserved = value;
    }

    std::int64_t getPeakBytesReserved() const noexcept
    {
        return m_value.peak_bytes_reserved;
    }

    void setPeakBytesReserved(std::int64_t value) noexcept
    {
        m_value.peak_bytes_reserved = value;
    }

    std::int8_t getHasBytesReservableLimit() const noexcept
    {
        return m_value.has_bytes_reservable_limit;
    }

    void setHasBytesReservableLimit(std::int8_t value) noexcept
    {
        m_value.has_bytes_reservable_limit = value;
    }

    std::int64_t getBytesReservableLimit() const noexcept
    {
        return m_value.bytes_reservable_limit;
    }

    void setBytesReservableLimit(std::int64_t value) noexcept
    {
        m_value.bytes_reservable_limit = value;
    }

    std::int64_t getLargestFreeBlockBytes() const noexcept
    {
        return m_value.largest_free_block_bytes;
    }

    void setLargestFreeBlockBytes(std::int64_t value) noexcept
    {
        m_value.largest_free_block_bytes = value;
    }

    ::TF_AllocatorStats* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_AllocatorStats* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_AllocatorStats m_value{};
};

} // namespace ice::sonic
