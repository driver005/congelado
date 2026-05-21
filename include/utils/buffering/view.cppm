export module utils_buffering:view;

import std;
import :node;

export namespace utils::buffering {

class NodeView {
  public:
    explicit NodeView(BufferNode *node, std::size_t start, std::size_t length, NodeView *next = nullptr)
        : m_node{node}, m_start{start}, m_length{length}, m_next{next} {
        node->acquire();
    }

    ~NodeView() { m_node->release(); }

    NodeView(const NodeView &) = delete;
    NodeView &operator=(const NodeView &) = delete;

    NodeView(NodeView &&other) noexcept
        : m_node{other.m_node}, m_start{other.m_start}, m_length{other.m_length}, m_next{other.m_next} {
        other.m_node = nullptr;
        other.m_start = 0;
        other.m_length = 0;
        other.m_next = nullptr;
    }
    NodeView &operator=(NodeView &&other) noexcept {
        if (this != &other) {
            m_node = other.m_node;
            m_start = other.m_start;
            m_length = other.m_length;
            m_next = other.m_next;

            other.m_node = nullptr;
            other.m_start = 0;
            other.m_length = 0;
            other.m_next = nullptr;
        }
        return *this;
    }

    [[nodiscard]] std::byte &operator[](std::size_t index) noexcept { return (*m_node)[m_start + index]; }

    void acquire() noexcept { m_node->acquire(); }
    void release() noexcept { m_node->release(); }
    void set_next(NodeView *next) noexcept { m_next = next; }

    [[nodiscard]] NodeView *get_next() const noexcept { return m_next; }
    [[nodiscard]] BufferNode *get_node() const noexcept { return m_node; }
    [[nodiscard]] const std::size_t &get_length() const noexcept { return m_length; }
    std::size_t &get_length() noexcept { return m_length; }
    [[nodiscard]] const std::size_t &get_start() const noexcept { return m_start; }
    std::size_t &get_start() noexcept { return m_start; }

  private:
    BufferNode *m_node;
    std::size_t m_start;
    std::size_t m_length;
    NodeView *m_next;
};


class BufferView {
  public:
    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::byte;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::byte *;
        using reference = const std::byte &;

        Iterator() : m_node{nullptr}, m_offset{0} {}

        Iterator(NodeView *node, std::size_t offset) : m_node{node}, m_offset{offset} {}

        ~Iterator() = default;

        Iterator(const Iterator &other) : m_node(other.m_node), m_offset(other.m_offset) {
            if (m_node != nullptr) {
                m_node->acquire();
            }
        }

        Iterator &operator=(const Iterator &other) noexcept {
            if (this != &other) {
                if (other.m_node != nullptr) {
                    other.m_node->acquire();
                }

                m_node = other.m_node;
                m_offset = other.m_offset;

                if (m_node != nullptr) {
                    m_node->release();
                }
            }
            return *this;
        }

        Iterator(Iterator &&other) noexcept : m_node{other.m_node}, m_offset{other.m_offset} {
            other.m_node = nullptr;
            other.m_offset = 0;
        }

        Iterator &operator=(Iterator &&other) noexcept {
            if (this != &other) {
                m_node = other.m_node;
                m_offset = other.m_offset;

                other.m_node = nullptr;
                other.m_offset = 0;
            }
            return *this;
        }

        reference operator*() const noexcept { return (*m_node)[m_offset]; }
        pointer operator->() const noexcept { return &((*m_node)[m_offset]); }

        Iterator &operator++() noexcept {
            if (m_node == nullptr) {
                return *this;
            }

            if (++m_offset >= m_node->get_length()) {
                auto *next = m_node->get_next();
                if (next != nullptr) {
                    next->acquire();
                }

                m_node->release();
                m_node = next;
                m_offset = 0;
            }

            return *this;
        }

        Iterator operator++(int) noexcept {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        Iterator &operator+=(std::size_t till) noexcept {
            while (till > 0 && (m_node != nullptr)) {
                std::size_t remaining = m_node->get_length() - m_offset;

                if (till < remaining) {
                    m_offset += till;
                    till = 0;
                } else {
                    till -= remaining;
                    auto *next = m_node->get_next();
                    if (next != nullptr) {
                        next->acquire();
                    }

                    m_node->release();
                    m_node = next;
                    m_offset = 0;
                }
            }
            return *this;
        }

        bool operator==(const Iterator &other) const noexcept {
            return m_node == other.m_node && m_offset == other.m_offset;
        }

        bool operator==(std::default_sentinel_t /*unused*/) const noexcept { return m_node == nullptr; }

      private:
        NodeView *m_node;
        std::size_t m_offset;
    };

    BufferView() : m_head{nullptr}, m_tail{nullptr}, m_size{0} {}

    ~BufferView() { release(); };

    BufferView(const BufferView &) = delete;
    BufferView &operator=(const BufferView &) = delete;

    BufferView(BufferView &&other) noexcept
        : m_head{other.m_head}, m_tail{other.m_tail}, m_size{other.m_size} {
        other.m_head = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }

    BufferView &operator=(BufferView &&other) noexcept {
        if (this != &other) {
            release();
            m_head = other.m_head;
            m_tail = other.m_tail;
            m_size = other.m_size;
            other.m_head = nullptr;
            other.m_tail = nullptr;
            other.m_size = 0;
        }
        return *this;
    }


    [[nodiscard]] Iterator begin() const noexcept { return Iterator{get_head(), 0}; }
    [[nodiscard]] static std::default_sentinel_t end() noexcept { return std::default_sentinel; }

    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return get_head() == nullptr; }

    void push_back(NodeView *node_view) noexcept {
        node_view->acquire();
        m_size += node_view->get_length();

        if (m_tail != nullptr) {
            m_tail->set_next(node_view);
            m_tail = node_view;
        } else {
            m_head = node_view;
            m_tail = m_head;
        }
    }

    void push_back(BufferNode *node, std::size_t start, std::size_t length) noexcept {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        push_back(new NodeView{node, start, length});
    }


    void release() noexcept {
        NodeView *current = m_head;
        while (current != nullptr) {
            NodeView *next = current->get_next();
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete current;
            current = next;
        }
        m_head = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    [[nodiscard]] NodeView *get_head() const noexcept { return m_head; }
    [[nodiscard]] NodeView *get_tail() const noexcept { return m_tail; }

  private:
    NodeView *m_head;
    NodeView *m_tail;
    std::size_t m_size;
};

} // namespace utils::buffering
