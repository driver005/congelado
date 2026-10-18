module;

#include "include/c/extern/stream_executor/mem_pool.h"

export module aten_xpu_extern_stream_executor:allocator_block_pool;

import std;
import cc_ice_extern_stream_executor_sonic;
import :allocator_block;

export namespace aten_xpu {

class SyclBlockPool
{
public:
    using SizeOrderedSet = std::set<SyclBlock*, SyclBlockSizeLess>;
    using AddressOrderedSet = std::set<SyclBlock*, SyclBlockAddressLess>;

    SyclBlockPool(bool is_small, ice::sonic::PoolId owner) noexcept :
        m_is_small{is_small},
        m_owner{owner}
    {
    }

    SyclBlockPool(const SyclBlockPool&) = delete;
    SyclBlockPool& operator=(const SyclBlockPool&) = delete;
    SyclBlockPool(SyclBlockPool&&) = delete;
    SyclBlockPool& operator=(SyclBlockPool&&) = delete;

    ~SyclBlockPool()
    {
        for (auto* block: m_blocks) {
            delete block;
        }
        for (auto* block: m_unmapped) {
            delete block;
        }
    }

    void addAllocation() noexcept
    {
        ++m_allocation_count;
    }

    bool getIsSmall() const noexcept
    {
        return m_is_small;
    }

    const ice::sonic::PoolId& getOwner() const noexcept
    {
        return m_owner;
    }

    int getAllocationCount() const noexcept
    {
        return m_allocation_count;
    }

    SizeOrderedSet& getBlocks() noexcept
    {
        return m_blocks;
    }

    AddressOrderedSet& getUnmapped() noexcept
    {
        return m_unmapped;
    }

    void remove_allocation() noexcept
    {
        if (m_allocation_count > 0) {
            --m_allocation_count;
        }
    }

    bool is_default() const noexcept
    {
        return m_owner.getFirst() == 0 && m_owner.getSecond() == 0;
    }

    SyclBlock* take_best_fit(sycl::queue* queue, std::size_t size, bool use_expandable)
    {
        SyclBlock key{queue, size};
        auto found = m_blocks.lower_bound(&key);
        if (found == m_blocks.end() || (*found)->getQueue() != queue) {
            return nullptr;
        }

        if ((*found)->getExpandableSegment() != nullptr && !use_expandable) {
            do {
                ++found;
            } while (found != m_blocks.end() && (*found)->getExpandableSegment() != nullptr &&
                     (*found)->getQueue() == queue);
            if (found == m_blocks.end() || (*found)->getQueue() != queue) {
                return nullptr;
            }
        }

        auto* block = *found;
        m_blocks.erase(found);
        return block;
    }

private:
    bool m_is_small;
    ice::sonic::PoolId m_owner;
    int m_allocation_count{0};
    SizeOrderedSet m_blocks;
    AddressOrderedSet m_unmapped;
};

} // namespace aten_xpu
