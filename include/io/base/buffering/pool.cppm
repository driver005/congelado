export module io_base_buffering:pool;

import std;
import :node;
import :view;


export namespace io::base::buffering {

class BufferPool {
  public:
    explicit BufferPool(std::size_t min_size = 8ull * 1024ull, std::size_t max_size = 64 * 1024)
        : m_min_size{min_size}, m_max_size{max_size}, m_current_size{min_size} {}

    ~BufferPool() = default;

    BufferPool(const BufferPool &) = delete;
    BufferPool &operator=(const BufferPool &) = delete;
    BufferPool(BufferPool &&) = delete;
    BufferPool &operator=(BufferPool &&) = delete;

    [[nodiscard]] BufferNode *acquire() noexcept {
        auto *tail = m_view.get_tail();

        if ((tail == nullptr) || tail->get_remaining() == 0) {

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *node = new BufferNode{m_current_size};

            node->acquire();
            m_view.push_back(node);

            node->acquire();

            return node;
        }

        tail->acquire();
        return tail;
    }

    void push(BufferNode node) noexcept { m_view.push_back(new BufferNode{std::move(node)}); }

    void notify_read(BufferNode *node, std::size_t bytes_read) noexcept {
        if (node == nullptr) {
            return;
        }

        node->expand_written(bytes_read);
        m_view.expand(bytes_read);
        node->release();

        if (bytes_read == m_current_size) {
            m_current_size = std::clamp(m_current_size * 2, m_min_size, m_max_size);
        } else {
            m_current_size = std::clamp(bytes_read, m_min_size, m_max_size);
        }
    }

    [[nodiscard]] std::size_t get_predicted_size() const noexcept { return m_current_size; }
    [[nodiscard]] BufferView &get_view() noexcept { return m_view; }

  private:
    std::size_t m_min_size;
    std::size_t m_max_size;
    std::size_t m_current_size;
    BufferView m_view;
};


} // namespace io::base::buffering
