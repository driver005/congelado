export module io_base_buffering:view;

import std;
import :node;
import :deleter;

export namespace io::base::buffering {

class BufferView {
  public:
    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::byte;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::byte *;
        using reference = const std::byte &;

        Iterator() : m_node(nullptr), m_off(0) {}

        // Accept a raw pointer here
        Iterator(BufferNode *node, std::size_t offset) : m_node(node), m_off(offset) {}

        reference operator*() const noexcept { return m_node->get_data()[m_off]; }
        pointer operator->() const noexcept { return &(m_node->get_data()[m_off]); }

        Iterator &operator++() noexcept {
            if (!m_node)
                return *this;

            if (++m_off >= m_node->get_size()) {
                m_node = m_node->get_next();
                m_off = 0;
            }
            return *this;
        }

        Iterator operator++(int) noexcept {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const Iterator &other) const noexcept { return m_node == other.m_node && m_off == other.m_off; }
        bool operator==(std::default_sentinel_t) const noexcept { return m_node == nullptr; }


        [[nodiscard]] BufferNode *get_node() const noexcept { return m_node; }
        [[nodiscard]] const std::size_t &get_offset() const noexcept { return m_off; }

      private:
        BufferNode *m_node; // Raw pointer for maximum speed
        std::size_t m_off;
    };

    struct Sentinel {
        bool operator==(const Iterator &it) const noexcept { return it == std::default_sentinel; }
    };

    BufferView() : m_begin{}, m_size{0} {}

    BufferView(BufferNode *head, std::size_t offset, std::size_t total_size)
        : m_begin{std::move(head), offset}, m_size{total_size} {}


    void expand(std::size_t additional_bytes) noexcept { m_size += additional_bytes; }

    [[nodiscard]] Iterator begin() const noexcept { return m_begin; }
    [[nodiscard]] Sentinel end() const noexcept { return {}; }
    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

    [[nodiscard]] std::optional<BufferNode *> peek() const noexcept {
        if (m_begin == end())
            return std::nullopt;
        return m_begin.get_node();
    }

    [[nodiscard]] std::optional<BufferNode> next() noexcept {
        if (m_begin == end())
            return std::nullopt;
        BufferNode *node = m_begin.get_node();
        ++m_begin;
        auto result = std::make_optional(std::move(*node));
        delete node;
        return result;
    }

    void pop_front() noexcept {
        if (m_begin == end())
            return;
        BufferNode *node = m_begin.get_node();
        ++m_begin;
        delete node;
    }

    [[nodiscard]] const std::byte *peek_contiguous(std::size_t size) const {
        // Ensure we have at least one node and it is large enough
        if (m_size >= size && m_begin != end() && m_begin.get_node()->get_size() >= size) {
            return reinterpret_cast<const std::byte *>(m_begin.get_node()->get_data());
        }
        return nullptr;
    }

  private:
    Iterator m_begin;
    std::size_t m_size;
};

} // namespace io::base::buffering
