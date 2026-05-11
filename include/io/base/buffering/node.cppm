module;
#include <atomic>
export module io_base_buffering:node;

import std;

export namespace io::base::buffering {

class BufferNode {
  public:
    BufferNode()
        : m_data{nullptr, std::default_delete<std::byte[]>()}, m_limit{0}, m_written{0}, m_next{nullptr}, m_refs{0} {}

    explicit BufferNode(std::size_t size)
        : m_data{new std::byte[size], std::default_delete<std::byte[]>()}, m_limit{size}, m_written{0}, m_next{nullptr},
          m_refs{0} {}

    explicit BufferNode(std::byte *data, std::size_t size)
        : m_data{data, [](std::byte *) { /* no-op */ }}, m_limit{size}, m_written{size}, m_next{nullptr}, m_refs{0} {}

    template <std::ranges::forward_range R>
    BufferNode(std::from_range_t /*unused*/, R &&range)
        : BufferNode{static_cast<std::size_t>(std::ranges::distance(range))} {
        for (auto byte : std::forward<R>(range)) {
            push_back(byte);
        }
    }

    ~BufferNode() = default;

    BufferNode(const BufferNode &) = delete;
    BufferNode &operator=(const BufferNode &) = delete;

    // Custom Move Constructor needed because std::atomic is not trivially movable
    BufferNode(BufferNode &&other) noexcept
        : m_data{std::move(other.m_data)}, m_limit{other.m_limit},
          m_written{other.m_written.load(std::memory_order_relaxed)},
          m_next{other.m_next.load(std::memory_order_relaxed)}, m_refs{other.m_refs.load(std::memory_order_relaxed)} {}

    BufferNode &operator=(BufferNode &&other) noexcept {
        m_data = std::move(other.m_data);
        m_limit = other.m_limit;
        m_written.store(other.m_written.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_next.store(other.m_next.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_refs.store(other.m_refs.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    [[nodiscard]] std::byte *begin() noexcept { return m_data.get(); }
    [[nodiscard]] std::byte *end() noexcept { return m_data.get() + m_limit; }
    [[nodiscard]] const std::byte *begin() const noexcept { return m_data.get(); }
    [[nodiscard]] const std::byte *end() const noexcept { return m_data.get() + m_limit; }

    [[nodiscard]] std::byte *get_data() const noexcept { return m_data.get(); }
    [[nodiscard]] BufferNode *get_next() const noexcept { return m_next.load(std::memory_order_acquire); }
    [[nodiscard]] std::size_t get_limit() const noexcept { return m_limit; }
    [[nodiscard]] std::size_t get_written() const noexcept { return m_written.load(std::memory_order_acquire); }
    [[nodiscard]] std::size_t get_remaining() const noexcept { return m_limit - get_written(); }

    void set_next(BufferNode *node) noexcept { m_next.store(node, std::memory_order_release); }
    void push_back(std::byte byte) noexcept { m_data.get()[m_written.fetch_add(1, std::memory_order_acq_rel)] = byte; }
    void set_written(std::size_t size) noexcept { m_written.store(size, std::memory_order_release); }
    void expand_written(std::size_t size) noexcept { m_written.fetch_add(size, std::memory_order_release); }
    void acquire() noexcept { m_refs.fetch_add(1, std::memory_order_relaxed); }

    void release() noexcept {
        if (m_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

  private:
    std::unique_ptr<std::byte[], std::function<void(std::byte *)>> m_data;
    std::size_t m_limit;
    std::atomic<std::size_t> m_written;
    std::atomic<BufferNode *> m_next;
    std::atomic<std::size_t> m_refs;
};

} // namespace io::base::buffering
