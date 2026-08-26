module;
#include <cassert>
export module utils_buffering:reader;

import std;
import :node;
import :deleter;
import :view;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::buffering {

class NodeReader
{
public:
    /**
     * @brief Wraps `node`, acquiring a reference to it immediately, and links to `next` if
     * given. Straightforward setup, no motion beyond grabbing the ref.
     * @param node the buffer node to wrap and acquire a reference to.
     * @param next the next reader in the chain, defaults to none.
     */
    explicit NodeReader(BufferNode* node, NodeReader* next = nullptr) :
        m_node{node},
        m_next{next}
    {
        node->acquire();
    }

    /**
     * @brief Releases the wrapped node's reference. Standard RAII payoff for the acquire() up
     * in the ctor.
     */
    ~NodeReader()
    {
        get_node()->release();
    }

    /**
     * @brief Deleted — no copying a reader, ownership of the underlying node reference isn't
     * meant to be duplicated implicitly.
     */
    NodeReader(const NodeReader&) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    NodeReader& operator=(const NodeReader&) = delete;

    /**
     * @brief Move ctor — steals `other`'s node and next-pointer outright, no extra
     * acquire/release churn.
     * @param other the reader to move from, left null after.
     */
    NodeReader(NodeReader&& other) noexcept :
        m_node{other.m_node},
        m_next{other.m_next.load(std::memory_order_relaxed)}
    {
        other.m_node = nullptr;
        other.m_next = nullptr;
    }

    /**
     * @brief Move assignment, same steal-don't-churn approach as the move ctor.
     * @param other the reader to move from, left null after.
     * @return `*this`, now holding `other`'s state.
     */
    NodeReader& operator=(NodeReader&& other) noexcept
    {
        // Self-assignment guard first, then steal other's node/next and leave it pointing at
        // nothing — no acquire/release churn needed since ownership just moves over.
        if (this != &other) {
            m_node = other.m_node;
            m_next.store(other.m_next.load(std::memory_order_relaxed), std::memory_order_relaxed);

            other.m_node = nullptr;
            other.m_next = nullptr;
        }
        return *this;
    }

    /**
     * @brief Indexes straight into the wrapped node.
     * @warning Unbounded, just like `BufferNode::operator[]` — no safety net past the node's
     * limit.
     * @param index the byte offset to grab.
     * @return a mutable reference to the byte at `index`.
     */
    [[nodiscard]] std::byte& operator[](std::size_t index) noexcept
    {
        return (*m_node)[index];
    } // FIXME(clang-tidy): unchecked operator[], consider .at()

    /**
     * @brief Links this reader to the next one in the chain, released with acquire-ordering by
     * readers so the chain stays consistent under concurrent traversal.
     * @param node the next reader to link to.
     */
    void set_next(NodeReader* node) noexcept
    {
        m_next.store(node, std::memory_order_release);
    }

    /**
     * @brief Bumps the wrapped node's written-byte count, forwarding straight to
     * `BufferNode::expand_written()`.
     * @param size how many bytes to add to the written count.
     */
    void expand_written(std::size_t size) const noexcept
    {
        get_node()->expand_written(size);
    }

    /**
     * @brief Bumps the wrapped node's ref count. Bet — pair it with a release() or the count
     * never comes back down.
     */
    void acquire() const noexcept
    {
        get_node()->acquire();
    }

    /**
     * @brief Drops the wrapped node's ref count. Same self-deletion warning as
     * `BufferNode::release()` — the node's a goner once the count hits zero.
     */
    void release() const noexcept
    {
        get_node()->release();
    }

    /**
     * @brief Grabs the wrapped node's raw data pointer.
     * @return the underlying byte pointer.
     */
    [[nodiscard]] std::byte* get_data() const noexcept
    {
        return get_node()->get_data();
    }

    /**
     * @brief Grabs the next reader in the chain.
     * @return the next `NodeReader`, or nullptr if this is the tail.
     */
    [[nodiscard]] NodeReader* get_next() const noexcept
    {
        return m_next.load(std::memory_order_acquire);
    }

    /**
     * @brief Grabs the wrapped node.
     * @return the underlying `BufferNode` pointer.
     */
    [[nodiscard]] BufferNode* get_node() const noexcept
    {
        return m_node;
    }

    /**
     * @brief Grabs the wrapped node's written-byte count.
     * @return how many bytes are written in the underlying node.
     */
    [[nodiscard]] std::size_t get_written() const noexcept
    {
        return get_node()->get_written();
    }

    /**
     * @brief Grabs the wrapped node's remaining unwritten capacity.
     * @return the underlying node's remaining capacity.
     */
    [[nodiscard]] std::size_t get_remaining() const noexcept
    {
        return get_node()->get_remaining();
    }

    /**
     * @brief Grabs the wrapped node's total capacity.
     * @return the underlying node's limit.
     */
    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return get_node()->get_limit();
    }

private:
    BufferNode* m_node;
    std::atomic<NodeReader*> m_next;
};

class BufferReader
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::byte;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::byte*;
        using reference = const std::byte&;

        /**
         * @brief Default ctor, builds the past-the-end/empty iterator — no node, offset zero.
         */
        Iterator() :
            m_node{nullptr},
            m_offset{0}
        {
        }

        /**
         * @brief Builds an iterator over `node` starting at `offset`, acquiring a reference if
         * `node` isn't null.
         * @param node the reader to start iterating from.
         * @param offset the starting byte offset within `node`.
         */
        Iterator(NodeReader* node, std::size_t offset) :
            m_node{node},
            m_offset{offset}
        {
            if (m_node != nullptr) {
                m_node->acquire();
            }
        }

        /**
         * @brief Releases the held node's reference, if any.
         */
        ~Iterator()
        {
            if (m_node != nullptr) {
                m_node->release();
            }
        }

        /**
         * @brief Copy ctor — acquires a fresh reference on the shared node, keeping the
         * refcount honest.
         * @param other the iterator to copy.
         */
        Iterator(const Iterator& other) :
            m_node(other.m_node),
            m_offset(other.m_offset)
        {
            if (m_node != nullptr) {
                m_node->acquire();
            }
        }

        /**
         * @brief Copy assignment — acquires the new node before releasing the old one, so a
         * self-referential edge case can't drop the last reference prematurely.
         * @param other the iterator to copy from.
         * @return `*this`, now pointing at `other`'s position.
         */
        Iterator& operator=(const Iterator& other) noexcept
        {
            if (this != &other) {
                // Order matters here — acquire the new node's ref BEFORE releasing the old one.
                // Flip that order and a self-referential edge case could drop the last
                // reference and free the node out from under this same assignment.
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
        Iterator(Iterator&& other) noexcept :
            m_node{other.m_node},
            m_offset{other.m_offset}
        {
            other.m_node = nullptr;
            other.m_offset = 0;
        }

        /**
         * @brief Move assignment, same steal-don't-churn approach as the move ctor.
         * @param other the iterator to move from, reset to empty after.
         * @return `*this`, now holding `other`'s state.
         */
        Iterator& operator=(Iterator&& other) noexcept
        {
            // Self-assignment guard, then just steal other's position outright and leave it
            // empty — no ref-count work needed on a move.
            if (this != &other) {
                m_node = other.m_node;
                m_offset = other.m_offset;

                other.m_node = nullptr;
                other.m_offset = 0;
            }
            return *this;
        }

        /**
         * @brief Dereferences the byte at the current position. Bog standard iterator motion.
         * @return the byte under the iterator, read-only.
         */
        reference operator*() const noexcept
        {
            return (*m_node)[m_offset];
        } // FIXME(clang-tidy): unchecked operator[], consider .at()

        /**
         * @brief Arrow overload, mirrors operator*().
         * @return a pointer to the byte under the iterator.
         */
        pointer operator->() const noexcept
        {
            return &((*m_node)[m_offset]);
        } // FIXME(clang-tidy): unchecked operator[], consider .at()

        /**
         * @brief Advances one byte, hopping to the next node in the chain (acquiring it,
         * releasing the old one) once the current node's written region runs out.
         * @return `*this`, advanced.
         */
        Iterator& operator++() noexcept
        {
            // Already at the end — nothing to advance into.
            if (m_node == nullptr) {
                return *this;
            }

            // Bump the offset; if that runs off the end of what's been written in this node,
            // hop to the next one in the chain — acquire the new node before releasing the old
            // to keep a live reference the whole time.
            if (++m_offset >= m_node->get_written()) {
                auto* next = m_node->get_next();
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
        Iterator operator++(int) noexcept
        {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * @brief Skips forward `till` bytes, hopping across as many chain nodes as needed.
         * @param till how many bytes to advance by.
         * @return `*this`, advanced by `till` bytes (or to the end, if the chain runs out
         * first).
         */
        Iterator& operator+=(std::size_t till) noexcept
        {
            // Keep eating chunks off the chain until `till` bytes are consumed or the chain
            // runs dry.
            while (till > 0 && (m_node != nullptr)) {
                std::size_t remaining = m_node->get_written() - m_offset;

                if (till < remaining) {
                    // Whole skip fits inside the current node — just bump the offset in place.
                    m_offset += till;
                    till = 0;
                } else {
                    // Skip eats the rest of this node — hop to the next one, same
                    // acquire-before-release ordering as operator++().
                    till -= remaining;
                    auto* next = m_node->get_next();
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
        bool operator==(const Iterator& other) const noexcept
        {
            return m_node == other.m_node && m_offset == other.m_offset;
        }

        /**
         * @brief Equality check against the end sentinel.
         * @return true if this iterator has run off the end of the chain (null node).
         */
        bool operator==([[maybe_unused]] std::default_sentinel_t sentinel_value) const noexcept
        {
            return m_node == nullptr;
        }

        /**
         * @brief Grabs the current node.
         * @return the `NodeReader` this iterator currently points into.
         */
        [[nodiscard]] NodeReader* get_node() const noexcept
        {
            return m_node;
        }

        /**
         * @brief Grabs the current byte offset within the current node.
         * @return the offset into `get_node()`.
         */
        [[nodiscard]] std::size_t get_offset() const noexcept
        {
            return m_offset;
        }

        /**
         * @brief How many bytes are left to read in the current node before the iterator has to
         * hop to the next one.
         * @return the remaining bytes in the current chunk, 0 if the iterator's at the end.
         */
        [[nodiscard]] std::size_t chunk_size() const noexcept
        {
            if (m_node == nullptr) {
                return 0;
            }
            return m_node->get_written() - m_offset;
        }

    private:
        NodeReader* m_node;
        std::size_t m_offset;
    };

    /**
     * @brief Builds an empty reader chain — no nodes, size zero.
     */
    BufferReader() :
        m_head{nullptr},
        m_tail{nullptr},
        m_offset{0},
        m_size{0}
    {
    }

    /**
     * @brief Releases every node still linked into the chain.
     */
    ~BufferReader()
    {
        release();
    }

    /**
     * @brief Deleted — this thing owns a live chain of refcounted nodes, no copying that
     * around.
     */
    BufferReader(const BufferReader&) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    BufferReader& operator=(const BufferReader&) = delete;

    /**
     * @brief Move ctor — steals `other`'s whole chain (head/tail/offset/size) and leaves it
     * empty, exactly like `splice()`'s detach step: the moved-from reader owns nothing
     * afterward, so its dtor frees nothing this one now holds. No refcount changes — the
     * `NodeReader`s (and their `BufferNode` stakes) transfer as-is.
     * @param other the reader to move from; emptied.
     */
    BufferReader(BufferReader&& other) noexcept :
        m_head{other.m_head.exchange(nullptr, std::memory_order_acq_rel)},
        m_tail{other.m_tail.exchange(nullptr, std::memory_order_acq_rel)},
        m_offset{other.m_offset.exchange(0, std::memory_order_relaxed)},
        m_size{other.m_size.exchange(0, std::memory_order_acq_rel)}
    {
    }

    /**
     * @brief Move assignment — releases this reader's own chain first, then steals `other`'s
     * the same way the move ctor does. Self-move is a guarded no-op.
     * @param other the reader to move from; emptied.
     * @return `*this`.
     */
    BufferReader& operator=(BufferReader&& other) noexcept
    {
        if (this != &other) {
            // Drop whatever this reader already owned before overwriting the handles.
            release();
            m_head.store(
                other.m_head.exchange(nullptr, std::memory_order_acq_rel), std::memory_order_release
            );
            m_tail.store(
                other.m_tail.exchange(nullptr, std::memory_order_acq_rel), std::memory_order_release
            );
            m_offset.store(
                other.m_offset.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed
            );
            m_size.store(
                other.m_size.exchange(0, std::memory_order_acq_rel), std::memory_order_release
            );
        }
        return *this;
    }

    /**
     * @brief Builds an iterator starting at the current head and read offset.
     * @return an iterator positioned at the first unconsumed byte.
     */
    [[nodiscard]] Iterator begin() const noexcept
    {
        return Iterator{get_head(), m_offset.load(std::memory_order_relaxed)};
    }

    /**
     * @brief The end sentinel every Iterator compares against.
     * @return `std::default_sentinel`.
     */
    [[nodiscard]] static std::default_sentinel_t end() noexcept
    {
        return std::default_sentinel;
    }

    /**
     * @brief Grabs how many unconsumed bytes are sitting in the chain.
     * @return the total unread byte count.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_size.load(std::memory_order_relaxed);
    }

    /**
     * @brief Checks whether there's anything left to read. Quick vibe check before you bother
     * calling front() or begin().
     * @return true if size() is zero.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return m_size.load(std::memory_order_acquire) == 0;
    }

    /**
     * @brief Grabs the head reader without consuming anything, bumping its ref count so it
     * survives past whatever else happens to the chain meanwhile.
     * @warning Caller's on the hook for release()-ing whatever comes back — this hands out an
     * extra reference, not a borrowed peek. Forget to release it and that's a leak, no cap.
     * @return the head `NodeReader`, or nullopt if the chain's empty.
     */
    [[nodiscard]] std::optional<NodeReader*> peek() const noexcept
    {
        // Nothing to peek at on an empty chain.
        auto* head = get_head();
        if (head == nullptr) {
            return std::nullopt;
        }
        // Otherwise hand back an extra reference so the head survives whatever the caller does
        // with it, no cap — that's on them to release() when done.
        head->acquire();
        return head;
    }

    /**
     * @brief Grabs a raw pointer/length pair for the first unread chunk, no ownership transfer
     * involved.
     * @return the front chunk's data pointer and size, or `{nullptr, 0}` if empty.
     */
    [[nodiscard]] std::pair<const std::byte*, std::size_t> front() const noexcept
    {
        // Nothing to hand back on an empty chain.
        if (empty()) {
            return {nullptr, 0};
        }
        // Otherwise just point at whatever the head iterator's currently sitting on and how
        // much of that chunk is left to read.
        auto it = begin();
        return {&(*it), it.chunk_size()};
    }

    /**
     * @brief Advances the read position by `bytes`, releasing and unlinking any nodes that get
     * fully consumed along the way.
     * @warning Single-consumer assumption baked in — nothing here stops two threads from
     * calling consume() concurrently and stomping on `m_offset`/`m_head` with relaxed ordering.
     * This is built for one reader thread, don't be the one who finds out the hard way what
     * happens otherwise.
     * @warning `m_size.fetch_sub(bytes, ...)` fires unconditionally up front, before the loop
     * even checks whether the chain actually has `bytes` worth of data. Call this with more
     * bytes than size() actually reports (empty chain included) and `m_size` — an unsigned
     * atomic — wraps straight around to some huge number instead of clamping at zero. Straight
     * cooked if a caller ever over-consumes, no cap, that's a real bug sitting right here.
     * @param bytes how many bytes to consume off the front of the chain.
     */
    void consume(std::size_t bytes) noexcept
    {
        // Publish the shrink up front — see the doxygen warning, straight up this fires
        // unconditionally before the loop below even checks the chain actually has this much
        // data.
        m_size.fetch_sub(bytes, std::memory_order_relaxed);

        // Walk the chain, eating whole nodes until the remaining `bytes` fit inside one.
        while (bytes > 0) {
            auto* head = get_head();
            if (head == nullptr) {
                break;
            }

            std::size_t offset = m_offset.load(std::memory_order_relaxed);
            std::size_t available = head->get_written() - offset;

            if (bytes < available) {
                // Consume fits entirely inside the current head node — just nudge the offset.
                m_offset.store(offset + bytes, std::memory_order_relaxed);
                break;
            }

            // Head node's fully consumed — unlink it and move on to the next one, clearing the
            // tail too if that was the last node standing.
            bytes -= available;
            auto* next = head->get_next();
            m_head.store(next, std::memory_order_release);
            m_offset.store(0, std::memory_order_relaxed);
            if (next == nullptr) {
                m_tail.store(nullptr, std::memory_order_release);
            }
            // Chain owns the NodeReader object — delete it. ~NodeReader drops the one
            // BufferNode reference it took in its ctor, so DON'T also call release() here
            // (that'd double-drop the BufferNode). Outstanding holders (a writer slot, an
            // iterator) keep their own BufferNode refs, so the underlying bytes survive until
            // they let go. NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete head;
        }
    }

    /**
     * @brief Marks `bytes` more bytes as available for reading, without changing the actual
     * chain structure. Used to reflect writes that landed through a side channel.
     * @param bytes how many bytes' worth of new data to account for.
     */
    void expand(std::size_t bytes) noexcept
    {
        m_size.fetch_add(bytes, std::memory_order_release);
    }

    /**
     * @brief Links `node` onto the tail of the chain and bumps `size()` by whatever it's
     * already got written. Standard lock-free tail-swap motion — `exchange()` first, link
     * second.
     * @param node the reader to append. The chain takes ownership of the NodeReader OBJECT and
     * `delete`s it on unlink (consume/dtor), exactly like `BufferView` owns its
     * `NodeView`s. The one BufferNode reference the NodeReader ctor already took is that chain
     * stake — no extra acquire here, or the BufferNode ends up over-referenced and leaks.
     * @param count_size when true (default) `size()` grows by `node`'s written bytes; pass
     * false to link only — used by `splice()`, which pushes a chain's head node and then folds
     * the whole chain's size in with a single `expand()`.
     */
    void push_back(NodeReader* node, bool count_size = true) noexcept
    {
        // Swap ourselves in as the new tail first — whoever we displaced (if anyone) gets
        // linked to us second. Empty chain means we're the new head too.
        auto* old = m_tail.exchange(node, std::memory_order_acq_rel);
        if (old != nullptr) {
            old->set_next(node);
        } else {
            m_head.store(node, std::memory_order_release);
        }

        // Fold in whatever this node already had written, so size() reflects it immediately —
        // unless the caller is accounting the size itself (see @param count_size).
        if (count_size) {
            expand(node->get_written());
        }
    }

    /**
     * @brief Convenience overload — wraps `node` in a freshly heap-allocated `NodeReader` and
     * appends that.
     * @param node the buffer node to wrap and append.
     * @return the newly-allocated `NodeReader` now owned by the chain.
     */
    NodeReader* push_back(BufferNode* node) noexcept
    {
        // NOLINT(cppcoreguidelines-owning-memory) — would need gsl::owner<> annotation; no GSL
        // dependency in this codebase.
        // FIXME(clang-tidy): bugprone-unhandled-exception-at-new — noexcept push_back() would
        // terminate on bad_alloc; this whole buffering subsystem has no error-return channel
        // (every push_back()/acquire() across reader/view/writter is noexcept, raw-pointer,
        // terminate-on-OOM by convention) — leaving as-is rather than inventing one locally.
        auto* reader = new NodeReader{node}; // NOLINT(cppcoreguidelines-owning-memory)
        push_back(reader);
        return reader;
    }

    /**
     * @brief Moves every node from `src` onto the tail of this chain — zero-copy: relinks the
     * existing `NodeReader` objects (and the single `BufferNode` ref each already holds) rather
     * than copying bytes or allocating new nodes, then leaves `src` empty. O(1) — just a
     * head/tail splice, no per-node walk.
     * @warning `src`'s read offset (bytes already consumed off its head, e.g. a preface the
     * handshake consumed) only travels with the chain when `this` is **empty** — offset applies
     * to a chain's head only, so it can't ride a node relinked onto a non-empty tail. Callers
     * that splice a partially-consumed `src` must do so into an empty destination (the
     * executor's handoff is empty on the first post-handshake feed, which is the only time
     * `src` carries an offset). `src`'s nodes are transferred, not deleted, so no ref is
     * dropped — the moved `NodeReader`s keep their `BufferNode` stakes and this chain frees
     * them on its own `consume()`/dtor.
     * @param src the reader to drain into this one; emptied on return.
     */
    void splice(BufferReader& src) noexcept
    {
        // Nothing to move.
        if (src.empty()) {
            return;
        }

        // Empty destination — take `src` over wholesale via move-assignment, which transfers
        // head/tail/size AND the read offset (so a partially-consumed src, e.g. the
        // handshake-consumed preface, stays consumed instead of resurfacing).
        if (empty()) {
            *this = std::move(src);
            return;
        }

        // Non-empty destination — one-shot relink: push src's head onto our tail (link only, no
        // per-node size count), fix our tail to src's real tail, fold in the whole src size,
        // then clear src so its consume()/dtor frees nothing (ownership moved to us). `src`
        // carries no read offset here — only the first, into-empty feed does (handled by the
        // move path above).
        auto* src_head = src.get_head();
        auto* src_tail = src.get_tail();

        push_back(src_head, false);
        m_tail.store(src_tail, std::memory_order_release);

        const auto SRC_SIZE = src.size();
        expand(SRC_SIZE);

        src.clear();
    }

    /**
     * @brief Resets this reader to empty — drops the head/tail/size/offset handles **without**
     * releasing any node. Only safe once ownership of the chain has already been transferred
     * elsewhere (e.g. `splice()` relinked the nodes into another reader first); calling it on a
     * reader that still owns live nodes leaks them.
     */
    void clear() noexcept
    {
        m_head.store(nullptr, std::memory_order_release);
        m_tail.store(nullptr, std::memory_order_release);
        m_offset.store(0, std::memory_order_relaxed);
        m_size.store(0, std::memory_order_release);
    }

    /**
     * @brief Appends up to `length` bytes from the front of the chain into `view` as `NodeView`
     * slices, each taking its own `BufferNode` ref. Pure read: it does NOT advance or shrink
     * this reader. Callers must follow it with `consume(length)` to actually remove the bytes —
     * the view's `NodeView` refs keep the underlying `BufferNode`s alive after that consume.
     * @param[out] view the buffer view to append the sliced references onto.
     * @param length how many bytes to reference into `view`.
     */
    void grow_view(BufferView& view, std::size_t length) const noexcept
    {
        // Walk a LOCAL cursor over the chain — never touch m_head/m_offset/m_size. Each slice
        // handed to `view` becomes a NodeView holding its own BufferNode ref.
        auto* node = get_head();
        std::size_t offset = m_offset.load(std::memory_order_relaxed);

        while (length > 0 && node != nullptr) {
            std::size_t available = node->get_written() - offset;
            std::size_t to_take = std::min(available, length);

            view.push_back(node->get_node(), offset, to_take);
            length -= to_take;

            if (to_take < available) {
                offset += to_take;
            } else {
                node = node->get_next();
                offset = 0;
            }
        }
    }

    /**
     * @brief Grabs the current head of the chain.
     * @return the head `NodeReader`, or nullptr if the chain's empty.
     */
    [[nodiscard]] NodeReader* get_head() const noexcept
    {
        return m_head.load(std::memory_order_acquire);
    }

    /**
     * @brief Grabs the current tail of the chain.
     * @return the tail `NodeReader`, or nullptr if the chain's empty.
     */
    [[nodiscard]] NodeReader* get_tail() const noexcept
    {
        return m_tail.load(std::memory_order_acquire);
    }

private:
    /**
     * @brief Walks the whole chain from head to tail, deleting every NodeReader the chain owns.
     * Called from the dtor to tear the chain down clean — same ownership model as
     * `BufferView::release()`.
     */
    void release() noexcept
    {
        // Grab the next pointer before deleting — the delete runs ~NodeReader and frees
        // `current` outright, so reading m_next off it after the fact would be a
        // use-after-free.
        auto* current = m_head.load(std::memory_order_acquire);
        while (current != nullptr) {
            auto* next = current->get_next();
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete current;
            current = next;
        }
    }

    std::atomic<NodeReader*> m_head;
    std::atomic<NodeReader*> m_tail;
    std::atomic<std::size_t> m_offset;
    std::atomic<std::size_t> m_size;
};

struct AdvanceReaderAdaptor : std::ranges::range_adaptor_closure<AdvanceReaderAdaptor>
{
    /**
     * @brief Binds the adaptor to a target reader and a byte count to consume once invoked.
     * @param view the reader to consume() from.
     * @param count how many bytes to consume.
     */
    explicit constexpr AdvanceReaderAdaptor(BufferReader& view, std::size_t count) :
        m_view{view},
        m_count{count}
    {
    }

    /**
     * @brief Invocation hook the range-adaptor machinery calls — consumes `m_count` bytes off
     * the bound reader, then just passes `result` straight through untouched.
     * @tparam T whatever type flows through the pipe at this point.
     * @param result the piped-in value, forwarded through unchanged.
     * @return `result`, forwarded straight through — this adaptor's only here for the consume()
     * side effect.
     */
    template<typename T>
    T operator()(T&& result) const
    {
        m_view.get().consume(m_count);
        return std::forward<T>(result);
    }

    std::reference_wrapper<BufferReader> m_view;
    std::size_t m_count;
};

}; // namespace utils::buffering

#ifdef CONGELADO_TEST
namespace utils::buffering::tests {
using namespace boost::ut;

suite<"NodeReader"> node_reader_suite = [] {
    "wraps a node and forwards its written/limit/remaining"_test = [] {
        auto* node = new BufferNode(4);
        node->push_back(std::byte{42});

        auto* reader = new NodeReader{node};
        expect(reader->get_written() == 1);
        expect(reader->get_limit() == 4);
        expect(reader->get_remaining() == 3);
        expect((*reader)[0] == std::byte{42});
        expect(reader->get_next() == nullptr);

        delete reader; // releases the one BufferNode ref taken in the ctor, node self-deletes
    };
};

suite<"BufferReader"> buffer_reader_suite = [] {
    "empty reader has no head/tail and size 0"_test = [] {
        BufferReader reader;
        expect(reader.empty());
        expect(reader.size() == 0);
        expect(reader.get_head() == nullptr);
    };
    "push_back(BufferNode*) wraps it in a NodeReader and sizes by written bytes"_test = [] {
        auto* node = new BufferNode(4);
        node->push_back(std::byte{1});
        node->push_back(std::byte{2});

        BufferReader reader;
        auto* wrapped = reader.push_back(node);

        expect(wrapped != nullptr);
        expect(not reader.empty());
        expect(reader.size() == 2);
        expect(reader.get_head() == wrapped);
        expect(reader.get_tail() == wrapped);
    };
    "consume walks bytes and unlinks fully-consumed nodes"_test = [] {
        auto* node = new BufferNode(2);
        node->push_back(std::byte{5});
        node->push_back(std::byte{6});

        BufferReader reader;
        reader.push_back(node);
        expect(reader.size() == 2);

        reader.consume(1);
        expect(reader.size() == 1);
        expect(not reader.empty());

        reader.consume(1);
        expect(reader.empty());
        expect(reader.get_head() == nullptr);
    };
    "begin/end iterate the written bytes in order"_test = [] {
        auto* node = new BufferNode(3);
        node->push_back(std::byte{10});
        node->push_back(std::byte{20});
        node->push_back(std::byte{30});

        BufferReader reader;
        reader.push_back(node);

        std::vector<std::byte> collected;
        for (auto it = reader.begin(); it != reader.end(); ++it) {
            collected.push_back(*it);
        }

        expect(collected.size() == 3);
        expect(collected[0] == std::byte{10});
        expect(collected[2] == std::byte{30});
    };
    "front reports the head chunk's pointer and remaining length"_test = [] {
        auto* node = new BufferNode(2);
        node->push_back(std::byte{1});
        node->push_back(std::byte{2});

        BufferReader reader;
        reader.push_back(node);

        auto [ptr, length] = reader.front();
        expect(ptr != nullptr);
        expect(length == 2);
    };
    "front on an empty reader reports null/zero"_test = [] {
        BufferReader reader;
        auto [ptr, length] = reader.front();
        expect(ptr == nullptr);
        expect(length == 0);
    };
    "splice into an empty destination moves the whole chain over"_test = [] {
        auto* node = new BufferNode(2);
        node->push_back(std::byte{7});
        node->push_back(std::byte{8});

        BufferReader src;
        src.push_back(node);

        BufferReader dest;
        dest.splice(src);

        expect(src.empty());
        expect(dest.size() == 2);
    };
    "grow_view slices the front bytes into a BufferView without consuming"_test = [] {
        auto* node = new BufferNode(3);
        node->push_back(std::byte{1});
        node->push_back(std::byte{2});
        node->push_back(std::byte{3});

        BufferReader reader;
        reader.push_back(node);

        BufferView view;
        reader.grow_view(view, 2);

        expect(view.size() == 2);
        expect(reader.size() == 3); // grow_view is a pure read, doesn't shrink the reader
    };
};

} // namespace utils::buffering::tests
#endif
