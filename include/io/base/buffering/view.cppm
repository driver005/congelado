module;
#include <cassert>
#include <functional>
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

        Iterator() : m_node{nullptr}, m_offset{0} {}

        Iterator(BufferNode *node, std::size_t offset) : m_node{node}, m_offset{offset} {
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

        reference operator*() const noexcept { return m_node->get_data()[m_offset]; }
        pointer operator->() const noexcept { return &(m_node->get_data()[m_offset]); }

        Iterator &operator++() noexcept {
            if (m_node == nullptr) {
                return *this;
            }

            if (++m_offset >= m_node->get_written()) {
                BufferNode *next = m_node->get_next();
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
                    BufferNode *next = m_node->get_next();
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

        [[nodiscard]] BufferNode *get_node() const noexcept { return m_node; }
        [[nodiscard]] std::size_t get_offset() const noexcept { return m_offset; }

        [[nodiscard]] std::size_t chunk_size() const noexcept {
            if (m_node == nullptr) {
                return 0;
            }
            return m_node->get_written() - m_offset;
        }

      private:
        BufferNode *m_node;
        std::size_t m_offset;
    };

    BufferView() : m_head{nullptr}, m_tail{nullptr}, m_offset{0}, m_size{0} {}

    ~BufferView() = default;

    BufferView(const BufferView &) = delete;
    BufferView &operator=(const BufferView &) = delete;
    BufferView(BufferView &&) = delete;
    BufferView &operator=(BufferView &&) = delete;

    [[nodiscard]] Iterator begin() const noexcept {
        return Iterator{get_head(), m_offset.load(std::memory_order_relaxed)};
    }
    [[nodiscard]] static std::default_sentinel_t end() noexcept { return std::default_sentinel; }
    [[nodiscard]] std::size_t size() const noexcept { return m_size.load(std::memory_order_relaxed); }
    [[nodiscard]] bool empty() const noexcept { return get_head() == nullptr; }

    [[nodiscard]] std::optional<BufferNode *> peek() const noexcept {
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

            std::size_t available = head->get_written() - m_offset.load(std::memory_order_relaxed);

            if (bytes < available) {
                m_offset.fetch_add(bytes, std::memory_order_relaxed);
                break;
            }

            bytes -= available;
            BufferNode *next = head->get_next();

            if (m_head.compare_exchange_strong(head, next)) {
                m_offset.store(0, std::memory_order_relaxed);
                BufferNode *expected_tail = head;
                m_tail.compare_exchange_strong(expected_tail, nullptr);
                head->release();
            }
        }
    }

    void expand(std::size_t bytes) noexcept { m_size.fetch_add(bytes, std::memory_order_relaxed); }

    void push_back(BufferNode *node) noexcept {
        node->acquire();

        BufferNode *old = m_tail.exchange(node, std::memory_order_acq_rel);
        if (old != nullptr) {
            old->set_next(node);
        } else {
            m_head.store(node, std::memory_order_release);
        }
    }

    [[nodiscard]] BufferNode *get_head() const noexcept { return m_head.load(std::memory_order_acquire); }
    [[nodiscard]] BufferNode *get_tail() const noexcept { return m_tail.load(std::memory_order_acquire); }

  private:
    std::atomic<BufferNode *> m_head;
    std::atomic<BufferNode *> m_tail;
    std::atomic<std::size_t> m_offset;
    std::atomic<std::size_t> m_size;
};

struct AdvanceViewAdaptor : std::ranges::range_adaptor_closure<AdvanceViewAdaptor> {
    explicit constexpr AdvanceViewAdaptor(BufferView &view, std::size_t count) : m_view(view), m_count(count) {}

    template <typename T>
    T operator()(T &&result) const {
        m_view.get().consume(m_count);
        return std::forward<T>(result);
    }

    std::reference_wrapper<BufferView> m_view;
    std::size_t m_count;
};

} // namespace io::base::buffering
