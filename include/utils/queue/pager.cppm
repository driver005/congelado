module;

#include <climits>

export module pager;

import std;
import consts;
import page;
import atomic_list;
import consts;

export enum class AllocationMode : std::uint8_t
{
    CAN_ALLOC = 0,
    CANNOT_ALLOC = 1
};

/**
 * @brief
 * Circular comparison for unsigned integers.
 * Useful for handling sequence number wrap-around in lock-free queues.
 * Gets the kind of thing a plain `a < b` wrong once a counter rolls past its max — exactly the
 * situation this whole lock-free page-based queue lives in.
 * @note This is a free function sitting at module scope, not a class static method — flagging
 * the convention mismatch (this codebase's rule is class-only, no free functions). Leaving the
 * structure alone, this pass is comment-only.
 * @tparam T an unsigned integral type — asserted at compile time, signed types aren't supported.
 * @param lhs the left-hand sequence number.
 * @param rhs the right-hand sequence number.
 * @return true if `lhs` is "before" `rhs` in circular sequence order.
 */
export template<typename T>
inline bool circular_less_than(T lhs, T rhs)
{
    static_assert(
        std::is_integral_v<T> && !std::is_signed_v<T>,
        "circular_less_than is intended for unsigned integer types only"
    );

    constexpr T SHIFT = (sizeof(T) * CHAR_BIT) - 1;
    return static_cast<T>(lhs - rhs) > (static_cast<T>(1) << SHIFT);
}

export template<typename T>
class Pager
{
public:
    /**
     * @brief Writes `item` into the current tail page, allocating a fresh page first if the
     * current one just filled up.
     * @warning Calls `add_new_page()` below with no explicit template argument, but
     * add_new_page() is itself templated on `AllocationMode` with nothing to deduce it from (zero
     * function parameters). That reads like it should be `add_new_page<Mode>()` — as written this
     * is either a compile error the moment this template actually gets instantiated, or it's
     * quietly resolving to some default nobody intended. Straight cooked, flagging it since this
     * pass is comment-only and the call isn't getting touched.
     * @tparam Mode whether allocation is allowed if no recycled page is available.
     * @param item the item to enqueue, copied into the page.
     * @return true on success, false if a new page was needed and couldn't be obtained (only
     * possible under `AllocationMode::CANNOT_ALLOC` with an empty recycle list).
     */
    template<AllocationMode Mode>
    bool enqueue(const T& item)
    {
        auto check_index = m_writer.load(std::memory_order_relaxed);

        // Check if index is equal to block size, if so we need to allocate a new page becouse else
        // we whould be writing to the start of the current page instead of the next page This is a
        // safe operation every thread has its own list of pages
        if ((check_index & (BLOCK_SIZE - 1)) == 0) {
            auto status = add_new_page();
            if (!status) {
                // Failed to add new page, likely due to allocation failure in CANNOT_ALLOC mode and
                // no more pages available in recycle list
                return false;
            }
        }

        // It is space in the current page, we can increase the writer index without worrying about
        // a rollback
        auto index = m_writer.fetch_add(1, std::memory_order_acq_rel);

        // Calculate block base index
        auto block_index = index & (BLOCK_SIZE - 1);
        m_tail->operator[](block_index) = item;

        return true;
    }

    /**
     * @brief Writes `count` items across as many pages as needed, allocating new pages along the
     * way and rolling back cleanly if allocation fails partway through.
     * @warning The rollback path (on allocation failure) walks `first_alloc_page` forward via
     * `m_next` and calls remove_page() for each, then resets `m_tail` back to `start_page`. Get
     * that unwind wrong in a refactor and you either leak freshly-allocated pages or leave
     * `m_tail` pointing somewhere stale — not a fun one to debug since it only bites on the
     * allocation-failure path, which is rare by definition.
     * @tparam Mode whether allocation is allowed if no recycled page is available.
     * @param items the items to enqueue, copied in order.
     * @param count how many items to enqueue.
     * @return true if all `count` items were written, false (with everything rolled back) if a
     * needed page couldn't be obtained partway through.
     */
    template<AllocationMode Mode>
    bool enqueue_bulk(const T* items, std::size_t count)
    {
        if (count == 0) {
            return true; // Nothing to enqueue, trivially successful
        }

        std::size_t start_index = m_writer.load(std::memory_order_relaxed);
        auto end_index = start_index + count;

        auto* const START_PAGE = m_tail;
        auto* end_page = m_tail;
        Page<T>* first_alloc_page = nullptr;

        // How many whole page boundaries sit between the start and end index — that's how many
        // fresh pages we'll need to walk/allocate below.
        std::size_t page_diff = (end_index & ~static_cast<std::size_t>(BLOCK_SIZE - 1)) -
                                (start_index & ~static_cast<std::size_t>(BLOCK_SIZE - 1));

        while (page_diff > 0) {
            page_diff -= static_cast<std::size_t>(BLOCK_SIZE);

            // Oh no, the alloction falied what now? In case of a faile we need to rollback all
            // pages allocated starting with first_alloc_page as long as it has a next page!
            if (!add_new_page<Mode>()) {
                Page<T>* current_page = first_alloc_page;
                while (current_page) {
                    auto* next_page = current_page->m_next;
                    remove_page<Mode>();
                    current_page = next_page;
                }

                if (START_PAGE != nullptr) {
                    START_PAGE->m_next = nullptr;
                }

                m_tail = START_PAGE;
                return false;
            }

            // Track the newest tail as the end page, and remember the very first page we
            // allocated this call — that's the rollback anchor if a later allocation fails.
            end_page = m_tail;
            if (first_alloc_page == nullptr) {
                first_alloc_page = end_page;
            }
        }

        // Rest the tail so we can write the items into the corresponding pages
        if ((start_index & static_cast<std::size_t>(BLOCK_SIZE - 1)) == 0 &&
            first_alloc_page != nullptr) {
            m_tail = first_alloc_page;
        } else {
            m_tail = START_PAGE;
        }

        std::size_t write_index = start_index;
        while (true) {
            // stop_index = first index of the NEXT block (or end_index if
            // the remaining items all fit in this block).
            std::size_t stop_index = (write_index & ~static_cast<std::size_t>(BLOCK_SIZE - 1)) +
                                     static_cast<std::size_t>(BLOCK_SIZE);
            if (circular_less_than<std::size_t>(end_index, stop_index)) {
                stop_index = end_index;
            }

            while (write_index != stop_index) {
                const std::size_t BLOCK_INDEX =
                    write_index & (static_cast<std::size_t>(BLOCK_SIZE) - 1);
                m_tail->operator[](BLOCK_INDEX) = items[write_index - start_index];
                ++write_index;
            }

            if (m_tail == end_page) {
                // We've written everything; start_index == end_index.
                break;
            }

            m_tail = m_tail->m_next;
        }

        // Every item's landed in its page now — publish the new writer position.
        m_writer.store(end_index, std::memory_order_release);
        return true;
    }

    /**
     * @brief Optimistic-concurrency single dequeue — claims a slot speculatively, verifies the
     * claim's still valid against the writer's current position, and only then actually reads the
     * element. Rolls back the optimistic claim via `m_reader_overcommit` if the verification
     * fails or a page swap can't complete.
     * @warning On a page-boundary race (current page doesn't match the expected one after
     * remove_page()), this retries by recursing into itself (`return dequeue(item);`) rather than
     * looping. Under pathological contention that's unbounded recursion depth, not just an
     * unbounded loop — technically still a footgun if the race keeps losing, worth knowing before
     * you go assuming this has a flat call stack.
     * @tparam Mode whether allocation is allowed if a page needs replacing during the dequeue.
     * @param[out] item where the dequeued element gets moved into, only touched on success.
     * @return true if an element was dequeued into `item`, false if nothing was available (or the
     * page swap failed under `AllocationMode::CANNOT_ALLOC`).
     */
    template<AllocationMode Mode>
    bool dequeue(T& item)
    {
        auto tail = m_writer.load(std::memory_order_relaxed);
        auto overcommit = m_reader_overcommit.load(std::memory_order_relaxed);

        // Check if there might be elements available using the optimistic formula
        if (circular_less_than<std::size_t>(
                m_reader_optimistic.load(std::memory_order_relaxed) - overcommit, tail
            )) {
            // Acquire fence to synchronize with potential overcommit updates
            std::atomic_thread_fence(std::memory_order_acquire);

            // Optimistically claim an element
            auto my_dequeue_count = m_reader_optimistic.fetch_add(1, std::memory_order_relaxed);

            // Reload tail in case it changed
            tail = m_writer.load(std::memory_order_acquire);

            // Verify the claim is still valid
            if (circular_less_than<std::size_t>(my_dequeue_count - overcommit, tail)) {
                // Successfully claimed an element - proceed with actual dequeue
                auto index = m_reader.fetch_add(1, std::memory_order_acq_rel);
                auto block_index = index & (BLOCK_SIZE - 1);

                // Handle page boundary with proper synchronization
                if ((block_index & (BLOCK_SIZE - 1)) == 0) {
                    // Remove old page and get new one
                    auto status = remove_page<Mode>();
                    if (!status) {
                        // Failed to remove page, likely due to allocation failure in CANNOT_ALLOC
                        // mode no more pages available in recycle list Rollback the optimistic
                        // claim and return false
                        m_reader_overcommit.fetch_add(1, std::memory_order_release);
                        return false;
                    }

                    // Verify we're on the correct page after removal
                    auto current_page_index = m_base.load(std::memory_order_relaxed);
                    auto expected_page_index = block_index / BLOCK_SIZE;

                    if (current_page_index != expected_page_index) {
                        // We're on the wrong page, retry
                        // This handles the race condition mentioned in the TODO
                        m_reader_overcommit.fetch_add(1, std::memory_order_release);
                        return dequeue(item); // Retry
                    }
                }

                // Safely read the element
                item = std::move(m_head->operator[](index & (BLOCK_SIZE - 1)));
                return true;
            }

            // No element available - rollback the optimistic claim
            m_reader_overcommit.fetch_add(1, std::memory_order_release);
        }

        return false;
    }

    /**
     * @brief Bulk sibling of dequeue() — claims up to `max` elements optimistically in one shot,
     * verifies the claim, then moves elements out page-by-page, running destructors as it goes.
     * @warning References `Page<T>::CAPACITY` throughout (see the masking arithmetic below), but
     * `Page<T>` (in `page.cppm`) never actually defines a `CAPACITY` member — it only has
     * `BLOCK_SIZE` from `consts.cppm`, used the exact same way everywhere else in this file
     * (enqueue()/dequeue() both mask with `BLOCK_SIZE - 1`, not `Page<T>::CAPACITY - 1`). This
     * reads like dead/never-instantiated code — a template member function only gets compiled
     * when it's actually used, so this bug can sit here indefinitely without anyone hitting a
     * build error. Straight cooked if something ever calls dequeue_bulk() for real; flagging hard
     * since this pass is comment-only and the logic isn't getting touched.
     * @tparam Mode whether allocation is allowed if a page needs replacing during the dequeue.
     * @param[out] items the destination buffer elements get moved into.
     * @param max the maximum number of elements to dequeue.
     * @throws exception whatever `T`'s move-assignment throws, if it isn't
     * `nothrow_move_assignable` — remaining claimed-but-unread elements get destroyed before the
     * exception propagates, so nothing gets fully leaked, but note the throw path exists at all.
     * @return how many elements were actually dequeued into `items` (may be less than `max`).
     */
    // FIXME(clang-tidy): readability-function-cognitive-complexity — this is a lock-free bulk
    // claim/verify/rollback algorithm with an exception-safety cleanup path that mirrors the
    // main move loop's page-boundary walk almost exactly (see the catch block below). It's also
    // already documented above as dead/never-instantiated code with a real latent bug
    // (Page<T>::CAPACITY doesn't exist on Page<T>) — extracting helpers out of logic that's
    // already known-broken and untested risks hiding that bug further or introducing a new one
    // in code nobody can currently exercise to verify. Deferring rather than guessing at a
    // refactor of unreachable, already-buggy code.
    template<AllocationMode Mode>
    std::size_t dequeue_bulk(T* items, std::size_t max)
    {
        auto tail = m_writer.load(std::memory_order_relaxed);
        auto overcommit = m_reader_overcommit.load(std::memory_order_relaxed);

        // Rough estimate of how much is actually available to read right now.
        auto desired_count = static_cast<std::size_t>(
            tail - (m_reader_optimistic.load(std::memory_order_relaxed) - overcommit)
        );

        // Nothing available — bail before touching any atomics that matter.
        if (!circular_less_than<std::size_t>(0, desired_count)) {
            return 0;
        }

        // Cap the ask at whatever the caller's buffer can actually hold.
        desired_count = desired_count < max ? desired_count : max;

        std::atomic_thread_fence(std::memory_order_acquire);

        // Optimistically claim `desired_count` elements up front, then re-check against the
        // writer's current position in case it moved since the estimate above.
        auto my_dequeue_count =
            m_reader_optimistic.fetch_add(desired_count, std::memory_order_relaxed);

        tail = m_writer.load(std::memory_order_acquire);

        auto actual_count = static_cast<std::size_t>(tail - (my_dequeue_count - overcommit));

        // Claim didn't hold up at all, straight L — roll the whole optimistic bump back and
        // report nothing dequeued.
        if (!circular_less_than<std::size_t>(0, actual_count)) {
            m_reader_overcommit.fetch_add(desired_count, std::memory_order_release);
            return 0;
        }

        actual_count = desired_count < actual_count ? desired_count : actual_count;

        // Claimed more than turned out to be available — roll back just the difference.
        if (actual_count < desired_count) {
            m_reader_overcommit.fetch_add(desired_count - actual_count, std::memory_order_release);
        }

        // Claim is settled — grab the real read index range and start moving elements out.
        auto first_index = m_reader.fetch_add(actual_count, std::memory_order_acq_rel);
        auto index = first_index;

        while (index != first_index + actual_count) {
            // Don't walk past the current page's end — stop at whichever comes first, the page
            // boundary or the last claimed index.
            std::size_t end_index = (index & ~static_cast<std::size_t>(Page<T>::CAPACITY - 1)) +
                                    static_cast<std::size_t>(Page<T>::CAPACITY);
            end_index = circular_less_than<std::size_t>(first_index + actual_count, end_index)
                            ? first_index + actual_count
                            : end_index;

            if constexpr (std::is_nothrow_move_assignable_v<T>) {
                // Move-assignable without throwing — no need for exception-safety bookkeeping,
                // just move each element out and destroy the source slot.
                while (index != end_index) {
                    auto& el = (*m_head)[index & (Page<T>::CAPACITY - 1)];
                    items[index - first_index] = std::move(el);
                    el.~T();
                    ++index;
                }
            } else {
                try {
                    while (index != end_index) {
                        auto& el = (*m_head)[index & (Page<T>::CAPACITY - 1)];
                        items[index - first_index] = std::move(el);
                        el.~T();
                        ++index;
                    }
                } catch (...) {
                    // Destroy all remaining claimed-but-unread elements before rethrowing.
                    auto cleanup = index;
                    while (cleanup != first_index + actual_count) {
                        std::size_t cleanup_end =
                            (cleanup & ~static_cast<std::size_t>(Page<T>::CAPACITY - 1)) +
                            static_cast<std::size_t>(Page<T>::CAPACITY);
                        cleanup_end =
                            circular_less_than<std::size_t>(first_index + actual_count, cleanup_end)
                                ? first_index + actual_count
                                : cleanup_end;

                        while (cleanup != cleanup_end) {
                            (*m_head)[cleanup & (Page<T>::CAPACITY - 1)].~T();
                            ++cleanup;
                        }

                        if ((cleanup_end & static_cast<std::size_t>(Page<T>::CAPACITY - 1)) == 0) {
                            remove_page<Mode>();
                        }
                    }
                    throw;
                }
            }

            // Retire the page once we've consumed through to its end.
            if ((end_index & static_cast<std::size_t>(Page<T>::CAPACITY - 1)) == 0) {
                remove_page<Mode>();
            }
        }

        return actual_count;
    }

private:
    /**
     * @brief Retires the current head page — advances `m_head` to the next page, bumps `m_base`
     * to reflect the new page's starting index, and either frees or recycles the old page
     * depending on `Mode`.
     * @tparam Mode `CANNOT_ALLOC` deletes dynamically-allocated pages outright (freeing memory
     * back since no fresh allocation is coming), anything else hands the page back to the
     * recycle list for reuse.
     */
    template<AllocationMode Mode>
    void remove_page()
    {
        auto* old_page = m_head;

        // Advance to the next page and bump the base index to match, so future reads know the
        // new head page's starting position.
        m_head = m_head->m_next;
        m_base.fetch_add(BLOCK_SIZE, std::memory_order_release);

        // CANNOT_ALLOC means no fresh allocation's coming, so free dynamically-allocated pages
        // outright instead of hoarding them — anything else hands the page back to the recycle
        // list for reuse.
        if constexpr (Mode == AllocationMode::CANNOT_ALLOC) {
            if (old_page->m_dynamicly_allocated) {
                delete old_page; // NOLINT(cppcoreguidelines-owning-memory) — would need
                                 // gsl::owner<> annotation; no GSL dependency in this codebase
            }
        } else {
            m_recycle_list->add(old_page);
        }
    }

    /**
     * @brief Gets a new tail page, preferring a recycled one over a fresh allocation.
     * @warning Read the `if constexpr` below closely — it only calls allocate_page() when `Mode
     * == AllocationMode::CANNOT_ALLOC`, and just returns false (no allocation attempt at all) for
     * `CAN_ALLOC`. That's backwards from what the enum names suggest at a glance: the mode that
     * says "can't allocate" is the one branch that actually allocates, the mode that says "can
     * allocate" is the one that never does. Either the naming's misleading or the condition's
     * flipped — either way, don't trust the enumerator name alone to guess this function's
     * behavior, read the branch.
     * @tparam Mode picks which branch above runs — see the warning, it's not the intuitive
     * reading.
     * @return true if a page was obtained (recycled or freshly allocated), false if neither
     * worked.
     */
    template<AllocationMode Mode>
    bool add_new_page()
    {
        // Prefer a recycled page over a fresh allocation, always.
        auto page = recycle_page();
        if (page) {
            return true;
        }
        // See the @warning above — this branch condition reads backwards from what the enum
        // names suggest, but as written only `CANNOT_ALLOC` actually falls through to
        // allocate_page().
        if constexpr (Mode == AllocationMode::CANNOT_ALLOC) {
            return allocate_page();
        }

        return false;
    }

    /**
     * @brief Pulls a page off the recycle freelist, if one's available, and links it on as the
     * new tail.
     * @return true if a recycled page was found and linked in, false if the recycle list was
     * empty.
     */
    bool recycle_page()
    {
        auto* new_page = m_recycle_list->try_get();

        // Nothing sitting on the freelist — nothing to link in.
        if (new_page != nullptr) {
            // Link the recycled page on as the new tail, or make it both head and tail if this
            // is the very first page.
            if (m_tail) {
                m_tail->m_next = new_page;
            }
            m_tail = new_page;
            return true;
        }

        return false;
    }

    /**
     * @brief Heap-allocates a brand-new page and links it on as the new tail.
     * @return true, always — this one doesn't have a failure path (barring `bad_alloc`, which
     * just propagates out uncaught).
     */
    bool allocate_page()
    {
        // Same link-on-as-tail motion as recycle_page(), just against a freshly heap-allocated
        // page instead of one pulled off the freelist.
        auto* new_page =
            new Page<T>(); // NOLINT(cppcoreguidelines-owning-memory) — would need gsl::owner<>
                           // annotation; no GSL dependency in this codebase
        if (m_tail) {
            m_tail->m_next = new_page;
        }
        m_tail = new_page;

        return true;
    }

    std::atomic<std::size_t> m_base;
    std::atomic<std::size_t> m_reader;
    std::atomic<std::size_t> m_writer;
    std::atomic<std::size_t> m_reader_optimistic{0};
    std::atomic<std::size_t> m_reader_overcommit{0};

    Page<T>* m_head;
    Page<T>* m_tail;
    AtomicList* m_recycle_list{nullptr};
};
