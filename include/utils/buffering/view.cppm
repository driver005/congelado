export module utils_buffering:view;

import std;
import :node;

export namespace utils::buffering {

class NodeView {
  public:
    /**
     * @brief Wraps `[start, start + length)` of `node`, acquiring a reference immediately, and
     * links to `next` if given. Zero-copy slicing, that's the whole motion.
     * @param node the buffer node this view slices into.
     * @param start the starting byte offset within `node`.
     * @param length how many bytes this view covers.
     * @param next the next view in the chain, defaults to none.
     */
    explicit NodeView(BufferNode *node, std::size_t start, std::size_t length,
                      NodeView *next = nullptr)
        : m_node{node}, m_start{start}, m_length{length}, m_next{next} {
        node->acquire();
    }

    /**
     * @brief Releases the wrapped node's reference.
     */
    ~NodeView() { m_node->release(); }

    /**
     * @brief Deleted — no copying, same ownership-sharing story as `NodeReader`.
     */
    NodeView(const NodeView &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    NodeView &operator=(const NodeView &) = delete;

    /**
     * @brief Move ctor — steals `other`'s node/start/length/next outright, no ref-count churn.
     * @param other the view to move from, reset to empty after.
     */
    NodeView(NodeView &&other) noexcept
        : m_node{other.m_node}, m_start{other.m_start}, m_length{other.m_length},
          m_next{other.m_next} {
        other.m_node = nullptr;
        other.m_start = 0;
        other.m_length = 0;
        other.m_next = nullptr;
    }
    /**
     * @brief Move assignment, same steal-don't-churn approach as the move ctor.
     * @param other the view to move from, reset to empty after.
     * @return `*this`, now holding `other`'s state.
     */
    NodeView &operator=(NodeView &&other) noexcept {
        // Self-assignment guard, then steal every field outright and reset other to the empty
        // state — no ref-count churn on a move.
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

    /**
     * @brief Indexes into the view's slice, offset from `m_start` — index 0 here means the first
     * byte of the *view*, not the underlying node.
     * @warning No bounds check against `m_length` — go past it and you're reading bytes outside
     * the intended slice, straight into whatever else lives in the node. Cooked if you're not
     * careful with the index.
     * @param index the offset within this view's slice.
     * @return a mutable reference to the byte at `m_start + index`.
     */
    [[nodiscard]] std::byte &operator[](std::size_t index) noexcept {
        return (*m_node)[m_start + index];
    } // FIXME(clang-tidy): unchecked operator[], consider .at()

    /**
     * @brief Bumps the wrapped node's ref count.
     */
    void acquire() noexcept { m_node->acquire(); }
    /**
     * @brief Drops the wrapped node's ref count. Same self-deletion energy as
     * `BufferNode::release()` — the node's gone once the count hits zero, don't touch it after.
     */
    void release() noexcept { m_node->release(); }
    /**
     * @brief Links this view to the next one in the chain. Plain pointer store, no atomics — this
     * one's not built for concurrent linking like its `BufferReader` cousin.
     * @param next the next view to link to.
     */
    void set_next(NodeView *next) noexcept { m_next = next; }

    /**
     * @brief Grabs the next view in the chain.
     * @return the next `NodeView`, or nullptr if this is the tail.
     */
    [[nodiscard]] NodeView *get_next() const noexcept { return m_next; }
    /**
     * @brief Grabs the wrapped node.
     * @return the underlying `BufferNode` this view slices into.
     */
    [[nodiscard]] BufferNode *get_node() const noexcept { return m_node; }
    /**
     * @brief Grabs the slice length.
     * @return how many bytes this view covers.
     */
    [[nodiscard]] const std::size_t &get_length() const noexcept { return m_length; }
    /**
     * @brief Mutable overload — lets callers resize the slice in place.
     * @return a mutable reference to the slice length.
     */
    std::size_t &get_length() noexcept { return m_length; }
    /**
     * @brief Grabs the slice's starting offset within the underlying node.
     * @return the start offset.
     */
    [[nodiscard]] const std::size_t &get_start() const noexcept { return m_start; }
    /**
     * @brief Mutable overload — lets callers shift the slice's start in place.
     * @return a mutable reference to the start offset.
     */
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

        /**
         * @brief Default ctor, builds the past-the-end/empty iterator.
         */
        Iterator() : m_node{nullptr}, m_offset{0} {}

        /**
         * @brief Builds an iterator over `node` starting at `offset`.
         * @warning Unlike `BufferReader::Iterator`'s equivalent ctor, this one does NOT acquire a
         * reference on construction — only the copy ctor/assignment do that. Kinda inconsistent
         * with its `BufferReader` sibling, so don't assume symmetric refcount behavior between
         * the two, that's an easy mixup.
         * @param node the view to start iterating from.
         * @param offset the starting byte offset within `node`'s slice.
         */
        Iterator(NodeView *node, std::size_t offset) : m_node{node}, m_offset{offset} {
            if (m_node != nullptr) {
                m_node->acquire();
            }
        }

        /**
         * @brief Default dtor — no reference to release, construction didn't acquire one (see the
         * ctor's @warning).
         */
        ~Iterator() {
            if (m_node != nullptr) {
                m_node->release();
            }
        };

        /**
         * @brief Copy ctor — acquires a fresh reference on the shared node.
         * @param other the iterator to copy.
         */
        Iterator(const Iterator &other) : m_node(other.m_node), m_offset(other.m_offset) {
            if (m_node != nullptr) {
                m_node->acquire();
            }
        }

        /**
         * @brief Copy assignment — acquires the new node before releasing the old one, avoiding a
         * premature drop on self-assignment-adjacent edge cases.
         * @param other the iterator to copy from.
         * @return `*this`, now pointing at `other`'s position.
         */
        Iterator &operator=(const Iterator &other) noexcept {
            if (this != &other) {
                // Acquire the incoming node's ref before releasing the current one — same
                // ordering trick as BufferReader::Iterator, dodges dropping the last reference on
                // a self-referential edge case.
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

        /**
         * @brief Move ctor — steals `other`'s node/offset, no ref-count churn.
         * @param other the iterator to move from, reset to empty after.
         */
        Iterator(Iterator &&other) noexcept : m_node{other.m_node}, m_offset{other.m_offset} {
            other.m_node = nullptr;
            other.m_offset = 0;
        }

        /**
         * @brief Move assignment, same steal-don't-churn approach as the move ctor.
         * @param other the iterator to move from, reset to empty after.
         * @return `*this`, now holding `other`'s state.
         */
        Iterator &operator=(Iterator &&other) noexcept {
            // Self-assignment guard, then just steal other's position and leave it empty.
            if (this != &other) {
                m_node = other.m_node;
                m_offset = other.m_offset;

                other.m_node = nullptr;
                other.m_offset = 0;
            }
            return *this;
        }

        /**
         * @brief Dereferences the byte at the current position.
         * @return the byte under the iterator, read-only.
         */
        reference operator*() const noexcept {
            return (*m_node)[m_offset];
        } // FIXME(clang-tidy): unchecked operator[], consider .at()
        /**
         * @brief Arrow overload, mirrors operator*().
         * @return a pointer to the byte under the iterator.
         */
        pointer operator->() const noexcept {
            return &((*m_node)[m_offset]);
        } // FIXME(clang-tidy): unchecked operator[], consider .at()

        /**
         * @brief Advances one byte, hopping to the next slice (acquiring it, releasing the old
         * one) once the current slice's length runs out.
         * @return `*this`, advanced.
         */
        Iterator &operator++() noexcept {
            // Already run off the chain — nothing to do.
            if (m_node == nullptr) {
                return *this;
            }

            // Bump the offset; once it runs past this slice's length, hop to the next slice,
            // acquiring the new one before releasing the old to keep a live reference throughout.
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

        /**
         * @brief Postfix advance — copies the current state out before stepping forward.
         * @return the iterator's state before this call.
         */
        Iterator operator++(int) noexcept {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * @brief Skips forward `till` bytes, hopping across as many slices as needed.
         * @param till how many bytes to advance by.
         * @return `*this`, advanced by `till` bytes (or to the end, if the chain runs out first).
         */
        Iterator &operator+=(std::size_t till) noexcept {
            // Chew through slices until `till` bytes are skipped or the chain's exhausted.
            while (till > 0 && (m_node != nullptr)) {
                std::size_t remaining = m_node->get_length() - m_offset;

                if (till < remaining) {
                    // Fits inside the current slice — just nudge the offset.
                    m_offset += till;
                    till = 0;
                } else {
                    // Eats the rest of this slice — hop to the next one.
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

        /**
         * @brief Equality check against another iterator.
         * @param other the iterator to compare against.
         * @return true if both point at the same node and offset.
         */
        bool operator==(const Iterator &other) const noexcept {
            return m_node == other.m_node && m_offset == other.m_offset;
        }

        /**
         * @brief Equality check against the end sentinel.
         * @return true if this iterator has run off the end of the chain (null node).
         */
        bool operator==(std::default_sentinel_t /*unused*/) const noexcept {
            return m_node == nullptr;
        }

      private:
        NodeView *m_node;
        std::size_t m_offset;
    };

    /**
     * @brief Builds an empty view — no slices, size zero. Clean slate, nothing cooking yet.
     */
    BufferView() = default;

    /**
     * @brief Releases and deletes every slice still linked into the chain, via release().
     */
    ~BufferView() { release(); };

    /**
     * @brief Deleted — no copying, the chain's ownership doesn't duplicate implicitly.
     */
    BufferView(const BufferView &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    BufferView &operator=(const BufferView &) = delete;

    /**
     * @brief Move ctor — steals `other`'s chain outright, leaving it empty.
     * @param other the view to move from.
     */
    BufferView(BufferView &&other) noexcept
        : m_head{other.m_head}, m_tail{other.m_tail}, m_size{other.m_size} {
        other.m_head = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }

    /**
     * @brief Move assignment — releases whatever chain `*this` already owned first, then steals
     * `other`'s.
     * @param other the view to move from, left empty after.
     * @return `*this`, now holding `other`'s chain.
     */
    BufferView &operator=(BufferView &&other) noexcept {
        if (this != &other) {
            // Tear down whatever chain `*this` already owned first — this thing owns its nodes
            // outright, so skipping this would leak the old chain — then steal other's.
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


    /**
     * @brief Builds an iterator starting at the head of the chain.
     * @return an iterator positioned at the first byte of the first slice.
     */
    [[nodiscard]] Iterator begin() const noexcept { return Iterator{get_head(), 0}; }
    /**
     * @brief The end sentinel every Iterator compares against.
     * @return `std::default_sentinel`.
     */
    [[nodiscard]] static std::default_sentinel_t end() noexcept { return std::default_sentinel; }

    /**
     * @brief Grabs the total byte count covered across every slice in the chain.
     * @return the combined slice length.
     */
    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    /**
     * @brief Checks whether the chain has any slices at all.
     * @return true if there's no head node.
     */
    [[nodiscard]] bool empty() const noexcept { return get_head() == nullptr; }

    /**
     * @brief Appends an already-constructed `NodeView` onto the tail of the chain, acquiring a
     * reference and folding its length into the running size. Straightforward tail-link, no cap.
     * @param node_view the view to append, ownership of its lifetime is now this chain's job.
     */
    void push_back(NodeView *node_view) noexcept {
        // Do NOT acquire here — the fresh NodeView already holds the chain's one BufferNode ref
        // via its ctor (view.cppm:21), released by ~NodeView on unlink/release(). Bumping again
        // would pin the BufferNode forever after the view is torn down (same bug fixed on the
        // BufferReader side — see reader.cppm push_back(NodeReader*)). Just fold in the length.
        m_size += node_view->get_length();

        // Link onto the existing tail, or become the head/tail both if this is the first slice.
        if (m_tail != nullptr) {
            m_tail->set_next(node_view);
            m_tail = node_view;
        } else {
            m_head = node_view;
            m_tail = m_head;
        }
    }

    /**
     * @brief Convenience overload — carves out `[start, start + length)` of `node` as a
     * freshly-allocated `NodeView` and appends that.
     * @param node the buffer node to slice.
     * @param start the starting byte offset within `node`.
     * @param length how many bytes the new slice covers.
     */
    // FIXME(clang-tidy): bugprone-unhandled-exception-at-new — noexcept push_back() would
    // terminate on bad_alloc; this whole buffering subsystem has no error-return channel (every
    // push_back()/acquire() across reader/view/writter is noexcept, raw-pointer,
    // terminate-on-OOM by convention) — leaving as-is rather than inventing one locally.
    void push_back(BufferNode *node, std::size_t start, std::size_t length) noexcept {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        push_back(new NodeView{node, start, length});
    }


    /**
     * @brief Walks the whole chain, deleting every `NodeView` node (each dtor drops its ref on
     * the underlying `BufferNode`), then resets the chain to empty.
     * @note Unlike `BufferReader::release()`, which only drops references, this one actually
     * `delete`s the `NodeView` nodes — `BufferView` owns them outright, `BufferReader` doesn't
     * own its `NodeReader`s the same way. Don't get the two release() semantics mixed up, that's
     * an easy L to take if you're jumping between the two types.
     */
    void release() noexcept {
        // Grab the next pointer before deleting `current` — reading m_next off a deleted node
        // would be straight cooked UB. Unlike BufferReader::release(), this actually owns and
        // frees the nodes, not just their references.
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

    /**
     * @brief Grabs the current head of the chain.
     * @return the head `NodeView`, or nullptr if the chain's empty.
     */
    [[nodiscard]] NodeView *get_head() const noexcept { return m_head; }
    /**
     * @brief Grabs the current tail of the chain.
     * @return the tail `NodeView`, or nullptr if the chain's empty.
     */
    [[nodiscard]] NodeView *get_tail() const noexcept { return m_tail; }

  private:
    NodeView *m_head{nullptr};
    NodeView *m_tail{nullptr};
    std::size_t m_size{0};
};

} // namespace utils::buffering
