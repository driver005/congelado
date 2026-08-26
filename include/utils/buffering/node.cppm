export module utils_buffering:node;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::buffering {

class BufferNode
{
public:
    /**
     * @brief Allocates `size` bytes on the heap, owned outright (default array delete). Starts
     * empty — `m_written` is 0 until something actually writes into it.
     * @param size how many bytes to allocate.
     */
    explicit BufferNode(std::size_t size) :
        m_data{new std::byte[size], std::default_delete<std::byte[]>()},
        m_limit{size},
        m_written{0},
        m_refs{0}
    {
    }

    /**
     * @brief Wraps an externally-owned buffer instead of allocating — the no-op deleter means
     * this node never frees `data`, that's on whoever handed it over. Also marks the whole
     * thing as already written (`m_written == size`), since it's assumed to already hold real
     * data.
     * @param data the externally-owned bytes to wrap, not freed by this node.
     * @param size how many bytes are available (and considered written) at `data`.
     */
    explicit BufferNode(std::byte* data, std::size_t size) :
        m_data{data, [](std::byte*) { /* no-op */ }},
        m_limit{size},
        m_written{size},
        m_refs{0}
    {
    }

    /**
     * @brief Adopts a moved-in vector's buffer instead of copying — heap-owns the vector,
     * points `m_data` at its `data()`, and stashes a deleter that frees the owning vector.
     * O(1), no byte copy at any body size. The node is marked fully written (`m_written ==
     * size`).
     * @param bytes the vector whose buffer this node takes ownership of.
     */
    explicit BufferNode(std::vector<std::byte>&& bytes) :
        m_data{nullptr, [](std::byte*) { /* no-op, replaced below */ }},
        m_limit{0},
        m_written{0},
        m_refs{0}
    {
        m_limit = bytes.size();
        auto* owned = new std::vector<std::byte>(std::move(bytes));
        m_data = decltype(m_data){owned->data(), [owned](std::byte*) {
                                      delete owned;
                                  }};
        m_written.store(m_limit, std::memory_order_relaxed);
    }

    /**
     * @brief Builds a node by copying an entire forward range in, byte by byte via push_back().
     * Allocates exactly enough room for the range up front.
     * @tparam R a forward range of bytes.
     * @param range the source range to copy in.
     */
    template<std::ranges::forward_range R>
    BufferNode(std::from_range_t /*unused*/, R&& range) :
        BufferNode{static_cast<std::size_t>(std::ranges::distance(range))}
    {
        // Buffer's already sized right via the delegating ctor above — just walk the range and
        // push each byte in, one at a time. No batching trick, straight copy.
        for (auto byte: std::forward<R>(range)) {
            push_back(byte);
        }
    }

    /**
     * @brief Default dtor — the real cleanup work already lives in the deleter stashed inside
     * `m_data`, nothing extra needed here.
     */
    ~BufferNode() = default;

    /**
     * @brief Deleted — no copying, this thing is meant to be shared through
     * acquire()/release(), not duplicated. Copying would double-own the underlying bytes.
     */
    BufferNode(const BufferNode&) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor — no copy assignment either.
     */
    BufferNode& operator=(const BufferNode&) = delete;

    // Custom Move Constructor needed because std::atomic is not trivially movable
    /**
     * @brief Move ctor, needed by hand since `std::atomic` isn't trivially movable. Loads
     * `other`'s atomics with relaxed ordering — fine since this assumes no concurrent access
     * during the move itself, don't be moving a node another thread's still poking at.
     * @param other the node to move from.
     */
    BufferNode(BufferNode&& other) noexcept :
        m_data{std::move(other.m_data)},
        m_limit{other.m_limit},
        m_written{other.m_written.load(std::memory_order_relaxed)},
        m_refs{other.m_refs.load(std::memory_order_relaxed)}
    {
    }

    /**
     * @brief Move assignment, same relaxed-load reasoning as the move ctor.
     * @param other the node to move from.
     * @return `*this`, now holding `other`'s state.
     */
    BufferNode& operator=(BufferNode&& other) noexcept
    {
        // Steal the backing storage and capacity outright, then carry the atomics over with
        // relaxed loads/stores — same no-concurrent-access assumption as the move ctor.
        m_data = std::move(other.m_data);
        m_limit = other.m_limit;
        m_written.store(other.m_written.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_refs.store(other.m_refs.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    /**
     * @brief Appends an entire input range one element at a time via push_back().
     * Straightforward loop, no batching trick.
     * @tparam R an input range of bytes.
     * @param range the bytes to append.
     */
    template<std::ranges::input_range R>
    void append_range(R&& range)
    {
        // Bet — just push every element from the incoming range through push_back(), one at a
        // time.
        for (auto&& element: std::forward<R>(range)) {
            this->push_back(element);
        }
    }

    /**
     * @brief Indexes into the raw buffer.
     * @warning Zero bounds checking against `m_limit` — go past it and it's straight UB, no
     * safety net.
     * @param index the byte offset to grab.
     * @return a mutable reference to the byte at `index`.
     */
    [[nodiscard]] std::byte& operator[](std::size_t index) noexcept
    {
        return m_data.get()[index];
    }

    /**
     * @brief Const overload — same deal, look but don't touch.
     * @warning Same unchecked-bounds situation as the non-const overload.
     * @param index the byte offset to grab.
     * @return a read-only reference to the byte at `index`.
     */
    [[nodiscard]] const std::byte& operator[](std::size_t index) const noexcept
    {
        return m_data.get()[index];
    }

    /**
     * @brief Start of the raw buffer.
     * @return a pointer to the first byte.
     */
    [[nodiscard]] std::byte* begin() noexcept
    {
        return m_data.get();
    }

    /**
     * @brief One-past-the-end of the buffer's full capacity — note this is `m_limit`, not
     * `m_written`, so it walks past whatever's actually been written into if the buffer isn't
     * full yet.
     * @return a pointer just past the last allocated byte.
     */
    [[nodiscard]] std::byte* end() noexcept
    {
        return m_data.get() + m_limit;
    }

    /**
     * @brief Const overload of begin().
     * @return a read-only pointer to the first byte.
     */
    [[nodiscard]] const std::byte* begin() const noexcept
    {
        return m_data.get();
    }

    /**
     * @brief Const overload of end(), same `m_limit`-not-`m_written` caveat applies.
     * @return a read-only pointer just past the last allocated byte.
     */
    [[nodiscard]] const std::byte* end() const noexcept
    {
        return m_data.get() + m_limit;
    }

    /**
     * @brief Grabs the raw backing pointer.
     * @return the raw byte pointer this node wraps.
     */
    [[nodiscard]] std::byte* get_data() const noexcept
    {
        return m_data.get();
    }

    /**
     * @brief Grabs the total allocated capacity.
     * @return the buffer's full capacity in bytes.
     */
    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    /**
     * @brief Grabs how many bytes have actually been written so far.
     * @return the written byte count, read with acquire ordering.
     */
    [[nodiscard]] std::size_t get_written() const noexcept
    {
        return m_written.load(std::memory_order_acquire);
    }

    /**
     * @brief Grabs the remaining unwritten capacity.
     * @return `get_limit() - get_written()`.
     */
    [[nodiscard]] std::size_t get_remaining() const noexcept
    {
        return m_limit - get_written();
    }

    /**
     * @brief Writes one byte at the current write position and bumps the write cursor forward.
     * @warning No bounds check against `m_limit` — `fetch_add` just keeps incrementing the
     * index no matter what. Call this more times than the node has capacity for and you're
     * writing past the allocation, straight cooked. Safe for concurrent pushers to land on
     * distinct indices (that's the whole point of `fetch_add`), but only if the total stays
     * under `m_limit` — nothing here stops you from blowing past it.
     * @param byte the byte to write.
     */
    void push_back(std::byte byte) noexcept
    {
        m_data.get()[m_written.fetch_add(1, std::memory_order_acq_rel)] = byte;
    }

    /**
     * @brief Directly sets the written-byte count, bypassing push_back() entirely.
     * @param size the new written-byte count.
     */
    void set_written(std::size_t size) noexcept
    {
        m_written.store(size, std::memory_order_release);
    }

    /**
     * @brief Bumps the written-byte count up by `size` atomically, for when bytes landed in the
     * buffer through some path other than push_back() (bulk I/O reads, for instance).
     * @param size how many bytes to add to the written count.
     */
    void expand_written(std::size_t size) noexcept
    {
        m_written.fetch_add(size, std::memory_order_release);
    }

    /**
     * @brief Bumps the ref count. Pair every acquire() with a release(), no exceptions, or this
     * node either leaks or gets freed while someone's still holding it.
     */
    void acquire() noexcept
    {
        m_refs.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Drops the ref count and self-destructs (`delete this`) once it hits zero.
     * @warning Classic intrusive-refcount self-deletion — once this returns and the count hit
     * zero, `this` is a dangling pointer, full stop. Don't touch the node after calling
     * release() unless you know for a fact another reference is still alive. No cap, this is
     * the kind of bug that only shows up under load.
     */
    void release() noexcept
    {
        if (m_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

private:
    std::unique_ptr<std::byte[], std::function<void(std::byte*)>> m_data;
    std::size_t m_limit;
    std::atomic<std::size_t> m_written;
    std::atomic<std::size_t> m_refs;
};


} // namespace utils::buffering

#ifdef CONGELADO_TEST
namespace utils::buffering::tests {
using namespace boost::ut;

suite<"BufferNode"> buffer_node_suite = [] {
    "heap allocation starts empty with the requested capacity"_test = [] {
        auto* node = new BufferNode(16);
        node->acquire();

        expect(node->get_limit() == 16);
        expect(node->get_written() == 0);
        expect(node->get_remaining() == 16);

        node->release();
    };
    "push_back writes sequentially and advances the write cursor"_test = [] {
        auto* node = new BufferNode(4);
        node->acquire();

        node->push_back(std::byte{1});
        node->push_back(std::byte{2});

        expect(node->get_written() == 2);
        expect(node->get_remaining() == 2);
        expect((*node)[0] == std::byte{1});
        expect((*node)[1] == std::byte{2});

        node->release();
    };
    "set_written/expand_written override the cursor directly"_test = [] {
        auto* node = new BufferNode(8);
        node->acquire();

        node->set_written(3);
        expect(node->get_written() == 3);
        node->expand_written(2);
        expect(node->get_written() == 5);

        node->release();
    };
    "wraps externally-owned data as already fully written"_test = [] {
        std::array<std::byte, 3> data{std::byte{7}, std::byte{8}, std::byte{9}};
        auto* node = new BufferNode(data.data(), data.size());
        node->acquire();

        expect(node->get_limit() == 3);
        expect(node->get_written() == 3);
        expect(node->get_data() == data.data());

        node->release();
    };
    "adopts a moved vector's buffer as fully written"_test = [] {
        std::vector<std::byte> bytes{std::byte{9}, std::byte{8}};
        auto* node = new BufferNode(std::move(bytes));
        node->acquire();

        expect(node->get_limit() == 2);
        expect(node->get_written() == 2);
        expect((*node)[0] == std::byte{9});

        node->release();
    };
    "acquire/release deletes the node once the last reference drops"_test = [] {
        auto* node = new BufferNode(4);
        node->acquire();
        node->acquire();
        expect(node->get_limit() == 4); // still alive with two refs held

        node->release();
        expect(node->get_limit() == 4); // still alive with one ref held

        node->release(); // drops to zero, self-deletes — nothing touches `node` after this
    };
};

} // namespace utils::buffering::tests
#endif
