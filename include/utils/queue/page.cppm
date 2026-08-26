export module page;

import std;
import node;
import consts;
import helper;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export template<typename T>
class Page : Node, public AlignedManager<Page<T>>
{
public:
    /**
     * @brief Builds an empty page — not dynamically allocated by default, zero elements
     * dequeued.
     */
    Page() :
        m_dequeued{0}
    {
    }

    /**
     * @brief Checks whether every slot in this page has been dequeued.
     * @note Fires an acquire fence right before returning true, so whoever sees "empty" is
     * synchronized with whatever the last dequeuer wrote — pairs with the acq_rel fetch_add()s
     * in set_empty()/set_many_empty().
     * @return true if all `BLOCK_SIZE` elements have been dequeued.
     */
    [[nodiscard]] bool is_empty() const
    {
        // Every slot's been dequeued — fire the acquire fence before reporting true, so whoever
        // sees this synchronizes with the last dequeuer's writes.
        if (m_dequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
            std::atomic_thread_fence(std::memory_order_acquire);
            return true;
        }
        return false;
    }

    /**
     * @brief Marks one more element as dequeued.
     * @return true if this call was the one that pushed the count over the edge to fully empty
     * (i.e. `BLOCK_SIZE - 1` elements were already dequeued before this call).
     */
    bool set_empty()
    {
        // Bump the count, then report whether THIS call was the one that tipped it over into
        // fully-empty (i.e. it was one short beforehand).
        auto prev = m_dequeued.fetch_add(1, std::memory_order_acq_rel);
        return prev == BLOCK_SIZE - 1;
    }

    /**
     * @brief Marks `count` more elements as dequeued in one shot, for bulk dequeue paths.
     * @param count how many elements to mark dequeued.
     * @return true if this call pushed the page to fully empty.
     */
    bool set_many_empty(std::size_t count)
    {
        // Same idea as set_empty(), just bumping by `count` in one shot for bulk dequeues.
        auto prev = m_dequeued.fetch_add(count, std::memory_order_acq_rel);
        return prev + count == BLOCK_SIZE;
    }

    /**
     * @brief Force-marks the whole page as fully dequeued in one relaxed store, bypassing the
     * incremental fetch_add() path entirely.
     */
    void set_full_empty()
    {
        m_dequeued.store(BLOCK_SIZE, std::memory_order_relaxed);
    }

    /**
     * @brief Resets the dequeued counter back to zero — recycling a page for reuse.
     */
    void reset_empty()
    {
        m_dequeued.store(0, std::memory_order_relaxed);
    }

    /**
     * @brief Indexes into the page's raw element storage, wrapping `idx` down to a valid slot via
     * a `BLOCK_SIZE - 1` mask (`BLOCK_SIZE` being a power of two, so this is a cheap modulo).
     * @warning No bounds/liveness check — this reinterprets raw `m_elements` bytes as `T*` and
     * hands back a pointer regardless of whether a `T` was actually constructed at that slot.
     * Straight cooked if you dereference a slot nobody's placement-new'd into yet.
     * @param idx the logical index to resolve into a slot.
     * @return a pointer to the element at the wrapped slot.
     */
    T* operator[](std::size_t idx) noexcept
    {
        return reinterpret_cast<T*>(m_elements) + // FIXME(clang-tidy): reinterpret_cast usage
               (idx & static_cast<std::size_t>(BLOCK_SIZE - 1));
    };

    /**
     * @brief Const overload of operator[](), same masking and same liveness caveat.
     * @param idx the logical index to resolve into a slot.
     * @return a read-only pointer to the element at the wrapped slot.
     */
    const T* operator[](std::size_t idx) const noexcept
    {
        return reinterpret_cast<const T*>(m_elements) + // FIXME(clang-tidy): reinterpret_cast usage
               (idx & static_cast<std::size_t>(BLOCK_SIZE - 1));
    };

private:
    alignas(T) char m_elements[sizeof(T) * BLOCK_SIZE]{};
    bool m_dynamicly_allocated{false};
    std::atomic<std::size_t> m_dequeued;
};

#ifdef CONGELADO_TEST
namespace {
using namespace boost::ut;

suite<"Page"> page_suite = [] {
    "starts with nothing dequeued"_test = [] {
        Page<int> page;
        expect(not page.is_empty());
    };
    "operator[] wraps the index modulo BLOCK_SIZE"_test = [] {
        Page<int> page;
        expect(page[0] == page[BLOCK_SIZE]);
        expect(page[3] == page[BLOCK_SIZE + 3]);
    };
    "set_empty reports true exactly on the call that reaches BLOCK_SIZE"_test = [] {
        Page<int> page;
        for (std::size_t i = 0; i < BLOCK_SIZE - 1; ++i) {
            expect(not page.set_empty());
        }
        expect(page.set_empty());
        expect(page.is_empty());
    };
    "set_many_empty reports true only once the count reaches BLOCK_SIZE"_test = [] {
        Page<int> page;
        expect(not page.set_many_empty(BLOCK_SIZE - 1));
        expect(page.set_many_empty(1));
        expect(page.is_empty());
    };
    "set_full_empty marks the page fully dequeued in one shot"_test = [] {
        Page<int> page;
        page.set_full_empty();
        expect(page.is_empty());
    };
    "reset_empty clears the counter back to not-empty"_test = [] {
        Page<int> page;
        page.set_full_empty();
        expect(page.is_empty());

        page.reset_empty();
        expect(not page.is_empty());
    };
};

} // namespace
#endif
