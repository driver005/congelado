module;

#include "include/c/extern/stream_executor/mem_pool.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_snapshot;

import std;
import :allocator_block;
import :allocator_block_pool;
import :allocator_trace;

export namespace aten_xpu {

class SyclAllocatorSnapshot
{
public:
    void begin(int device_index)
    {

        m_scratch_text.clear();
        m_scratch_text += std::format("{{\"device\":{},\"segments\":[", device_index);
        m_first_segment = true;
        m_total_active = 0;

    }

    void add_segment(const SyclBlock& head)
    {

        if (head.getPrevious() != nullptr && head.getPrevious()->getMapped()) {
            return;
        }

        const auto& pool = *head.getPool();
        m_scratch_text += m_first_segment ? "" : ",";
        m_first_segment = false;
        m_scratch_text += std::format(
            "{{\"address\":{},\"stream\":{},\"is_large\":{},\"is_expandable\":{},"
            "\"pool\":[{},{}],\"blocks\":[",
            reinterpret_cast<std::uintptr_t>(head.getPointer()),
            reinterpret_cast<std::uintptr_t>(head.getQueue()),
            !pool.getIsSmall(),
            head.getExpandableSegment() != nullptr,
            pool.getOwner().first,
            pool.getOwner().second
        );

        bool first_block = true;
        std::size_t total_size = 0;
        std::size_t allocated_size = 0;
        std::size_t active_size = 0;
        std::size_t requested_size = 0;
        for (const auto* block = &head; block != nullptr && block->getMapped();
             block = block->getNext())
        {
            const bool active = !block->is_free();
            m_scratch_text += first_block ? "" : ",";
            first_block = false;
            m_scratch_text += std::format(
                "{{\"size\":{},\"requested_size\":{},\"allocated\":{},\"active\":{}}}",
                block->getSize(),
                block->getRequestedSize(),
                block->getAllocated(),
                active
            );
            total_size += block->getSize();
            allocated_size += block->getAllocated() ? block->getSize() : 0;
            active_size += active ? block->getSize() : 0;
            requested_size += active ? block->getRequestedSize() : 0;
        }

        m_scratch_text += std::format(
            "],\"total_size\":{},\"allocated_size\":{},\"active_size\":{},\"requested_size\":{}}}",
            total_size,
            allocated_size,
            active_size,
            requested_size
        );
        m_total_active += active_size;

    }

    std::string_view finish(const SyclAllocatorTrace& trace)
    {

        m_scratch_text += "],\"trace\":[";
        bool first_entry = true;
        trace.for_each(
            [this, &first_entry](const SyclAllocatorTrace::Entry& entry)
            {

                const auto& [action, address, size, queue, pool] = entry;
                m_scratch_text += first_entry ? "" : ",";
                first_entry = false;
                m_scratch_text += std::format(
                    "{{\"action\":\"{}\",\"address\":{},\"size\":{},\"stream\":{},\"pool\":[{},{}]}}",
                    SyclAllocatorTrace::action_name(action),
                    address,
                    size,
                    reinterpret_cast<std::uintptr_t>(queue),
                    pool.first,
                    pool.second
                );

            }
        );
        m_scratch_text += "]}";
        return m_scratch_text;

    }

    std::size_t getTotalActive() const noexcept { return m_total_active; }

private:
    std::string m_scratch_text;
    bool m_first_segment{true};
    std::size_t m_total_active{0};
};

} // namespace aten_xpu
