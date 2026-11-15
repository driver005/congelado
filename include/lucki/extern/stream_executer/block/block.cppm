module;

#include <sycl/sycl.hpp>

export module lucki_extern_stream_executor:block;

import std;

export namespace lucki::stream_executor::block {
// Predefine here since there is no other way to solve this cycle import
class BlockPool;

class Block
{
public:
    Block(sycl::queue* queue, size_t size) noexcept :
        m_queue{queue},
        m_size{size}
    {
    }

    Block(sycl::queue* queue, size_t size, BlockPool* pool, std::byte* data) noexcept :
        m_queue{queue},
        m_size{size},
        m_pool{pool},
        m_data{data, [](std::byte*) { /* no-op */ }}
    {
    }

    void split(Block* head, Block* tail) noexcept
    {
        if (head != nullptr) {
            head->m_tail = this;
        }

        m_head = head;

        if (tail != nullptr) {
            tail->m_head = this;
        }

        m_tail = tail;
    }

    bool is_split() const noexcept
    {
        return m_head != nullptr || m_tail != nullptr;
    }

    bool is_free() const noexcept
    {
        return !m_allocated && m_ref_count == 0 && m_queues.empty()
    }

    void set_queue(sycl::queue* queue)
    {
        m_queue = queue;
    }

    void set_data(std::byte* data)
    {
        m_data = data;
    }

    void set_head(Block* head)
    {
        m_head = head;
    }

    void set_tail(Block* tail)
    {
        m_tail = tail;
    }

    void set_size(std::size_t size)
    {
        m_size = size;
    }

    void set_request_size(std::size_t request_size)
    {
        m_request_size = request_size;
    }

    void set_allocate(bool allocated)
    {
        m_allocated = allocated;
    }

    void set_mapped(bool mapped)
    {
        m_mapped = mapped;
    }

    void get_ref_count(std::size_t m_ref_count)
    {
        m_ref_count = m_ref_count;
    }

    void append_queue(sycl::queue* queue)
    {
        m_queues.insert(queue);
    }

    [[nodiscard]] sycl::queue* get_queue() const noexcept
    {
        return m_queue;
    }

    [[nodiscard]] std::byte* get_data() const noexcept
    {
        return m_data.get();
    }

    [[nodiscard]] Block* get_head() const noexcept
    {
        return m_head;
    }

    [[nodiscard]] Block* get_tail() const noexcept
    {
        return m_tail;
    }

    [[nodiscard]] std::size_t get_size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] std::size_t get_requested_size() const noexcept
    {
        return m_requested_size;
    }

    [[nodiscard]] bool get_allocated() const noexcept
    {
        return m_allocated;
    }

    [[nodiscard]] bool get_mapped() const noexcept
    {
        return m_mapped;
    }

    [[nodiscard]] std::size_t get_ref_count() const noexcept
    {
        return m_ref_count;
    }

    [[nodiscard]] std::set<sycl::queue*>& get_queues() const noexcept
    {
        return m_queues;
    }

private:
    // the queue the block was allocated for
    sycl::queue* m_queue{nullptr};
    // start pointer to the data
    std::unique_ptr<std::byte[], std::function<void(std::byte*)>> m_data{
        nullptr,
        [](std::byte*) { /* no-op */ }
    };
    // previous block of the linked list
    Block* m_head{nullptr};
    // next block of the linked list
    Block* m_tail{nullptr};
    // real size of the block in bytes
    std::size_t m_size{0};
    // size the caller asked for
    std::size_t m_requested_size{0};
    // is the block allocated and in use?
    bool m_allocated{false};
    // physical memory or reserved address space
    bool m_mapped{true};
    // how many events use the it
    std::size_t m_ref_count{0};
    // queues that used the block. Clear before recycling the block
    std::set<sycl::queue*> m_queues;
    // pool that owns the block, freed block goes back into pool.
    BlockPool* m_pool{nullptr};

    // The expandable segment the block belongs to, or nullptr for a normal block. It is used
    // for map and unmap. ExpandableSegment* m_segment{nullptr};
    //
};

} // namespace lucki::stream_executor::block
