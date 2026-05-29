module;
#include <cassert>
export module utils_buffering:reader;

import std;
import :node;
import :deleter;
import :view;

export namespace utils::buffering {

class NodeReader {
  public:
    explicit NodeReader(BufferNode *node, NodeReader *next = nullptr) : m_node{node}, m_next{next} { node->acquire(); }

    ~NodeReader() { get_node()->release(); }

    NodeReader(const NodeReader &) = delete;
    NodeReader &operator=(const NodeReader &) = delete;

    NodeReader(NodeReader &&other) noexcept
        : m_node{other.m_node}, m_next{other.m_next.load(std::memory_order_relaxed)} {
        other.m_node = nullptr;
        other.m_next = nullptr;
    }

    NodeReader &operator=(NodeReader &&other) noexcept {
        if (this != &other) {
            m_node = other.m_node;
            m_next.store(other.m_next.load(std::memory_order_relaxed), std::memory_order_relaxed);

            other.m_node = nullptr;
            other.m_next = nullptr;
        }
        return *this;
    }

    [[nodiscard]] std::byte &operator[](std::size_t index) noexcept { return (*m_node)[index]; }

    void set_next(NodeReader *node) noexcept { m_next.store(node, std::memory_order_release); }
    void expand_written(std::size_t size) const noexcept { get_node()->expand_written(size); }
    void acquire() const noexcept { get_node()->acquire(); }
    void release() const noexcept { get_node()->release(); }

    [[nodiscard]] std::byte *get_data() const noexcept { return get_node()->get_data(); }
    [[nodiscard]] NodeReader *get_next() const noexcept { return m_next.load(std::memory_order_acquire); }
    [[nodiscard]] BufferNode *get_node() const noexcept { return m_node; }
    [[nodiscard]] std::size_t get_written() const noexcept { return get_node()->get_written(); }
    [[nodiscard]] std::size_t get_remaining() const noexcept { return get_node()->get_remaining(); }
    [[nodiscard]] std::size_t get_limit() const noexcept { return get_node()->get_limit(); }

  private:
    BufferNode *m_node;
    std::atomic<NodeReader *> m_next;
};

class BufferReader {
  public:
    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::byte;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::byte *;
        using reference = const std::byte &;

        Iterator() : m_node{nullptr}, m_offset{0} {}

        Iterator(NodeReader *node, std::size_t offset) : m_node{node}, m_offset{offset} {
            if (m_node != nullptr) {
                m_node->acquire();
            }
        }

        ~Iterator() {
            if (m_node != nullptr) {
                m_node->release();
            }
        }

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

            if (++m_offset >= m_node->get_written()) {
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
                std::size_t remaining = m_node->get_written() - m_offset;

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

        [[nodiscard]] NodeReader *get_node() const noexcept { return m_node; }
        [[nodiscard]] std::size_t get_offset() const noexcept { return m_offset; }

        [[nodiscard]] std::size_t chunk_size() const noexcept {
            if (m_node == nullptr) {
                return 0;
            }
            return m_node->get_written() - m_offset;
        }

      private:
        NodeReader *m_node;
        std::size_t m_offset;
    };

    BufferReader() : m_head{nullptr}, m_tail{nullptr}, m_offset{0}, m_size{0} {}

    ~BufferReader() { release(); }

    BufferReader(const BufferReader &) = delete;
    BufferReader &operator=(const BufferReader &) = delete;
    BufferReader(BufferReader &&) = delete;
    BufferReader &operator=(BufferReader &&) = delete;

    [[nodiscard]] Iterator begin() const noexcept {
        return Iterator{get_head(), m_offset.load(std::memory_order_relaxed)};
    }
    [[nodiscard]] static std::default_sentinel_t end() noexcept { return std::default_sentinel; }
    [[nodiscard]] std::size_t size() const noexcept { return m_size.load(std::memory_order_relaxed); }
    [[nodiscard]] bool empty() const noexcept { return m_size.load(std::memory_order_acquire) == 0; }

    [[nodiscard]] std::optional<NodeReader *> peek() const noexcept {
        auto *head = get_head();
        if (head == nullptr) {
            return std::nullopt;
        }
        head->acquire();
        return head;
    }

    [[nodiscard]] std::pair<const std::byte *, std::size_t> front() const noexcept {
        if (empty()) {
            return {nullptr, 0};
        }
        auto it = begin();
        return {&(*it), it.chunk_size()};
    }

    void consume(std::size_t bytes) noexcept {
        m_size.fetch_sub(bytes, std::memory_order_relaxed);

        while (bytes > 0) {
            auto *head = get_head();
            if (head == nullptr) {
                break;
            }

            std::size_t offset = m_offset.load(std::memory_order_relaxed);
            std::size_t available = head->get_written() - offset;

            if (bytes < available) {
                m_offset.store(offset + bytes, std::memory_order_relaxed);
                break;
            }

            bytes -= available;
            auto *next = head->get_next();
            m_head.store(next, std::memory_order_release);
            m_offset.store(0, std::memory_order_relaxed);
            if (next == nullptr) {
                m_tail.store(nullptr, std::memory_order_release);
            }
            head->release();
        }
    }

    void expand(std::size_t bytes) noexcept { m_size.fetch_add(bytes, std::memory_order_release); }

    void push_back(NodeReader *node) noexcept {
        node->acquire();

        auto *old = m_tail.exchange(node, std::memory_order_acq_rel);
        if (old != nullptr) {
            old->set_next(node);
        } else {
            m_head.store(node, std::memory_order_release);
        }

        expand(node->get_written());
    }

    NodeReader *push_back(BufferNode *node) noexcept {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *reader = new NodeReader{node};
        push_back(reader);
        return reader;
    }

    void expand_view(BufferView &view, std::size_t length) noexcept {
        while (length > 0) {
            auto *head = get_head();
            if (head == nullptr) {
                break;
            }

            std::size_t offset = m_offset.load(std::memory_order_relaxed);
            std::size_t available = head->get_written() - offset;
            std::size_t to_take = std::min(available, length);

            view.push_back(head->get_node(), offset, to_take);

            if (to_take < available) {
                m_offset.store(offset + to_take, std::memory_order_relaxed);
                length = 0;
            } else {
                length -= to_take;
                auto *next = head->get_next();
                m_head.store(next, std::memory_order_release);
                m_offset.store(0, std::memory_order_relaxed);
                if (next == nullptr) {
                    m_tail.store(nullptr, std::memory_order_release);
                }
                head->release();
            }
        }
    }

    [[nodiscard]] NodeReader *get_head() const noexcept { return m_head.load(std::memory_order_acquire); }
    [[nodiscard]] NodeReader *get_tail() const noexcept { return m_tail.load(std::memory_order_acquire); }

  private:
    void release() noexcept {
        auto *current = m_head.load(std::memory_order_acquire);
        while (current != nullptr) {
            auto *next = current->get_next();
            current->release();
            current = next;
        }
    }

    std::atomic<NodeReader *> m_head;
    std::atomic<NodeReader *> m_tail;
    std::atomic<std::size_t> m_offset;
    std::atomic<std::size_t> m_size;
};

struct AdvanceReaderAdaptor : std::ranges::range_adaptor_closure<AdvanceReaderAdaptor> {
    explicit constexpr AdvanceReaderAdaptor(BufferReader &view, std::size_t count) : m_view{view}, m_count{count} {}

    template <typename T>
    T operator()(T &&result) const {
        m_view.get().consume(m_count);
        return std::forward<T>(result);
    }

    std::reference_wrapper<BufferReader> m_view;
    std::size_t m_count;
};

}; // namespace utils::buffering
