export module io_base_buffering:pool;

import std;
import :node;
import :view;


export namespace io::base::buffering {

class BufferPool {
  public:
    explicit BufferPool(std::size_t min_size = 8 * 1024, std::size_t max_size = 64 * 1024)
        : m_min_size{min_size}, m_max_size{max_size}, m_current_size{min_size}, m_tail{new BufferNode{min_size}},
          m_view{m_tail, 0, min_size} {}

    BufferPool(const BufferPool &) = delete;
    BufferPool &operator=(const BufferPool &) = delete;
    BufferPool(BufferPool &&) = delete;
    BufferPool &operator=(BufferPool &&) = delete;

    [[nodiscard]] std::optional<BufferNode *> acquire() noexcept {
        if (m_tail->get_remaining() > 0) {
            return m_tail;
        } else {
            auto *new_node = new BufferNode{m_current_size};
            m_tail->set_next(new_node);
            m_tail = new_node;
            m_view.expand(m_current_size);
        }
        return std::nullopt;
    }

    void push(BufferNode node) noexcept {
        m_tail->set_next(&node);
        m_tail = &node;
        m_view.expand(node.get_size());
    }

    void notify_read(std::size_t bytes_read) noexcept {
        if (bytes_read == m_current_size)
            m_current_size = std::clamp(m_current_size * 2, m_min_size, m_max_size);
        else
            m_current_size = std::clamp(bytes_read, m_min_size, m_max_size);
    }

    [[nodiscard]] std::size_t get_size() const noexcept { return m_current_size; }
    [[nodiscard]] BufferView &get_view() noexcept { return m_view; }

  private:
    std::size_t m_min_size;
    std::size_t m_max_size;
    std::size_t m_current_size;

    BufferNode *m_tail;
    BufferView m_view;
};


} // namespace io::base::buffering
