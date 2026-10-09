module;

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_block;

import std;

export namespace aten_xpu {

class SyclBlockPool;
class SyclExpandableSegment;

class SyclBlock
{
public:
    SyclBlock(
        sycl::queue* queue,
        std::size_t size,
        SyclBlockPool* pool,
        std::byte* pointer
    ) noexcept :
        m_queue{queue},
        m_size{size},
        m_pool{pool},
        m_pointer{pointer}
    {
    }

    SyclBlock(sycl::queue* queue, std::size_t size) noexcept :
        m_queue{queue},
        m_size{size}
    {
    }

    void addStreamUse(sycl::queue* queue)
    {
        m_stream_uses.insert(queue);
    }

    void setSize(std::size_t size) noexcept
    {
        m_size = size;
    }

    void setRequestedSize(std::size_t size) noexcept
    {
        m_requested_size = size;
    }

    void setPointer(std::byte* pointer) noexcept
    {
        m_pointer = pointer;
    }

    void setAllocated(bool allocated) noexcept
    {
        m_allocated = allocated;
    }

    void setMapped(bool mapped) noexcept
    {
        m_mapped = mapped;
    }

    void setPrevious(SyclBlock* previous) noexcept
    {
        m_previous = previous;
    }

    void setNext(SyclBlock* next) noexcept
    {
        m_next = next;
    }

    void setEventCount(int count) noexcept
    {
        m_event_count = count;
    }

    void setExpandableSegment(SyclExpandableSegment* segment) noexcept
    {
        m_expandable_segment = segment;
    }

    sycl::queue* getQueue() const noexcept
    {
        return m_queue;
    }

    std::size_t getSize() const noexcept
    {
        return m_size;
    }

    std::size_t getRequestedSize() const noexcept
    {
        return m_requested_size;
    }

    SyclBlockPool* getPool() const noexcept
    {
        return m_pool;
    }

    std::byte* getPointer() const noexcept
    {
        return m_pointer;
    }

    bool getAllocated() const noexcept
    {
        return m_allocated;
    }

    bool getMapped() const noexcept
    {
        return m_mapped;
    }

    SyclBlock* getPrevious() const noexcept
    {
        return m_previous;
    }

    SyclBlock* getNext() const noexcept
    {
        return m_next;
    }

    int getEventCount() const noexcept
    {
        return m_event_count;
    }

    SyclExpandableSegment* getExpandableSegment() const noexcept
    {
        return m_expandable_segment;
    }

    std::set<sycl::queue*>& getStreamUses() noexcept
    {
        return m_stream_uses;
    }

    bool is_split() const noexcept
    {
        return m_previous != nullptr || m_next != nullptr;
    }

    bool is_free() const noexcept
    {
        return !m_allocated && m_event_count == 0 && m_stream_uses.empty();
    }

    void splice(SyclBlock* before, SyclBlock* after) noexcept
    {
        if (before != nullptr) {
            before->m_next = this;
        }
        m_previous = before;
        if (after != nullptr) {
            after->m_previous = this;
        }
        m_next = after;
    }

    static bool compare_size(const SyclBlock* left, const SyclBlock* right) noexcept
    {
        if (left->m_queue != right->m_queue) {
            return std::less<>{}(left->m_queue, right->m_queue);
        }
        if (left->m_size != right->m_size) {
            return left->m_size < right->m_size;
        }
        return std::less<>{}(left->m_pointer, right->m_pointer);
    }

    static bool compare_address(const SyclBlock* left, const SyclBlock* right) noexcept
    {
        if (left->m_queue != right->m_queue) {
            return std::less<>{}(left->m_queue, right->m_queue);
        }
        return std::less<>{}(left->m_pointer, right->m_pointer);
    }

private:
    sycl::queue* m_queue{nullptr};
    std::size_t m_size{0};
    std::size_t m_requested_size{0};
    SyclBlockPool* m_pool{nullptr};
    std::byte* m_pointer{nullptr};
    bool m_allocated{false};
    bool m_mapped{true};
    SyclBlock* m_previous{nullptr};
    SyclBlock* m_next{nullptr};
    int m_event_count{0};
    SyclExpandableSegment* m_expandable_segment{nullptr};
    std::set<sycl::queue*> m_stream_uses;
};

} // namespace aten_xpu
