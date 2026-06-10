module utils.queue.pager;
@nogc nothrow:

import core.atomic;
import utils.queue.node;
import utils.queue.page;
import utils.queue.atomic_list;
import utils.consts : BLOCK_SIZE;

enum AllocationMode { CanAlloc = 0, CannotAlloc = 1 }

/**
 * Circular comparison for unsigned integers.
 * Useful for handling sequence number wrap-around in lock-free queues.
 */
bool circular_less_than(T)(T a, T b) if (__traits(isUnsigned, T)) {
    enum T shift = cast(T)(T.sizeof * 8 - 1);
    return cast(T)(a - b) > (cast(T)(1) << shift);
}

class Pager(T) {
    @disable this(this);

    bool enqueue(AllocationMode Mode)(const ref T item) {
        auto check_index = atomicLoad!(MemoryOrder.raw)(m_writer);

        // Check if index is equal to block size, if so we need to allocate a new page becouse else we whould be writing
        // to the start of the current page instead of the next page This is a safe operation every thread has its own
        // list of pages
        if ((check_index & (BLOCK_SIZE - 1)) == 0) {
            auto status = add_new_page!(Mode)();
            if (!status) {
                // Failed to add new page, likely due to allocation failure in CannotAlloc mode and no more pages
                // available in recycle list
                return false;
            }
        }

        // It is space in the current page, we can increase the writer index without worrying about a rollback
        auto index = atomicFetchAdd!(MemoryOrder.acq_rel)(m_writer, cast(size_t) 1);

        // Calculate block base index
        auto block_index = index & (BLOCK_SIZE - 1);
        *m_tail.opIndex(block_index) = item;

        return true;
    }

    bool enqueue_bulk(AllocationMode Mode)(const T* items, size_t count) {
        if (count == 0) {
            return true; // Nothing to enqueue, trivially successful
        }

        size_t start_index = atomicLoad!(MemoryOrder.raw)(m_writer);
        auto   end_index   = start_index + count;

        auto start_page     = m_tail;
        auto end_page       = m_tail;
        Page!T* first_alloc_page = null;

        size_t page_diff = (end_index & ~cast(size_t)(BLOCK_SIZE - 1)) -
                           (start_index & ~cast(size_t)(BLOCK_SIZE - 1));

        while (page_diff > 0) {
            page_diff -= cast(size_t)(BLOCK_SIZE);

            // Oh no, the alloction falied what now? In case of a faile we need to rollback all pages allocated starting
            // with first_alloc_page as long as it has a next page!
            if (!add_new_page!(Mode)()) {
                auto current_page = first_alloc_page;
                while (current_page !is null) {
                    auto next_page = current_page.m_next;
                    remove_page!(Mode)();
                    current_page = cast(Page!T*) next_page;
                }

                if (start_page !is null)
                    start_page.m_next = atomicLoad!(MemoryOrder.raw)(cast(shared Node**) null);

                m_tail = start_page;
                return false;
            }

            end_page = m_tail;
            if (first_alloc_page is null)
                first_alloc_page = end_page;
        }

        // Rest the tail so we can write the items into the corresponding pages
        if ((start_index & cast(size_t)(BLOCK_SIZE - 1)) == 0 && first_alloc_page !is null)
            m_tail = first_alloc_page;
        else
            m_tail = start_page;

        size_t write_index = start_index;
        while (true) {
            // stop_index = first index of the NEXT block (or end_index if
            // the remaining items all fit in this block).
            size_t stop_index =
                (write_index & ~cast(size_t)(BLOCK_SIZE - 1)) + cast(size_t)(BLOCK_SIZE);
            if (circular_less_than!(size_t)(end_index, stop_index))
                stop_index = end_index;

            while (write_index != stop_index) {
                const size_t block_index = write_index & (cast(size_t)(BLOCK_SIZE) - 1);
                *m_tail.opIndex(block_index) = items[write_index - start_index];
                ++write_index;
            }

            if (m_tail is end_page) {
                // We've written everything; start_index == end_index.
                break;
            }

            m_tail = cast(Page!T*) atomicLoad!(MemoryOrder.raw)(m_tail.m_next);
        }

        atomicStore!(MemoryOrder.rel)(m_writer, end_index);
        return true;
    }

    bool dequeue(AllocationMode Mode)(ref T item) {
        auto tail      = atomicLoad!(MemoryOrder.raw)(m_writer);
        auto overcommit = atomicLoad!(MemoryOrder.raw)(m_reader_overcommit);

        // Check if there might be elements available using the optimistic formula
        if (circular_less_than!(size_t)(
                atomicLoad!(MemoryOrder.raw)(m_reader_optimistic) - overcommit, tail)) {
            // Acquire fence to synchronize with potential overcommit updates
            atomicFence!(MemoryOrder.acq)();

            // Optimistically claim an element
            auto myDequeueCount = atomicFetchAdd!(MemoryOrder.raw)(m_reader_optimistic, cast(size_t) 1);

            // Reload tail in case it changed
            tail = atomicLoad!(MemoryOrder.acq)(m_writer);

            // Verify the claim is still valid
            if (circular_less_than!(size_t)(myDequeueCount - overcommit, tail)) {
                // Successfully claimed an element - proceed with actual dequeue
                auto index      = atomicFetchAdd!(MemoryOrder.acq_rel)(m_reader, cast(size_t) 1);
                auto block_index = index & (BLOCK_SIZE - 1);

                // Handle page boundary with proper synchronization
                if ((block_index & (BLOCK_SIZE - 1)) == 0) {
                    // Remove old page and get new one
                    auto status = remove_page!(Mode)();
                    if (!status) {
                        // Failed to remove page, likely due to allocation failure in CannotAlloc mode no more pages
                        // available in recycle list Rollback the optimistic claim and return false
                        atomicFetchAdd!(MemoryOrder.rel)(m_reader_overcommit, cast(size_t) 1);
                        return false;
                    }

                    // Verify we're on the correct page after removal
                    auto current_page_index  = atomicLoad!(MemoryOrder.raw)(m_base);
                    auto expected_page_index = block_index / BLOCK_SIZE;

                    if (current_page_index != expected_page_index) {
                        // We're on the wrong page, retry
                        // This handles the race condition mentioned in the TODO
                        atomicFetchAdd!(MemoryOrder.rel)(m_reader_overcommit, cast(size_t) 1);
                        return dequeue!(Mode)(item); // Retry
                    }
                }

                // Safely read the element
                item = *m_head.opIndex(index & (BLOCK_SIZE - 1));
                return true;
            } else {
                // No element available - rollback the optimistic claim
                atomicFetchAdd!(MemoryOrder.rel)(m_reader_overcommit, cast(size_t) 1);
            }
        }

        return false;
    }

    size_t dequeue_bulk(AllocationMode Mode)(T* items, size_t max) {
        auto tail       = atomicLoad!(MemoryOrder.raw)(m_writer);
        auto overcommit = atomicLoad!(MemoryOrder.raw)(m_reader_overcommit);

        auto desired_count = cast(size_t)(
            tail - (atomicLoad!(MemoryOrder.raw)(m_reader_optimistic) - overcommit));

        if (!circular_less_than!(size_t)(0, desired_count))
            return 0;

        desired_count = desired_count < max ? desired_count : max;

        atomicFence!(MemoryOrder.acq)();

        auto my_dequeue_count = atomicFetchAdd!(MemoryOrder.raw)(m_reader_optimistic, desired_count);

        tail = atomicLoad!(MemoryOrder.acq)(m_writer);

        auto actual_count = cast(size_t)(tail - (my_dequeue_count - overcommit));

        if (!circular_less_than!(size_t)(0, actual_count)) {
            atomicFetchAdd!(MemoryOrder.rel)(m_reader_overcommit, desired_count);
            return 0;
        }

        actual_count = desired_count < actual_count ? desired_count : actual_count;

        if (actual_count < desired_count)
            atomicFetchAdd!(MemoryOrder.rel)(m_reader_overcommit, desired_count - actual_count);

        auto first_index = atomicFetchAdd!(MemoryOrder.acq_rel)(m_reader, actual_count);
        auto index       = first_index;

        while (index != first_index + actual_count) {
            enum size_t PAGE_CAPACITY = BLOCK_SIZE;
            size_t end_index = (index & ~cast(size_t)(PAGE_CAPACITY - 1)) +
                               cast(size_t)(PAGE_CAPACITY);
            end_index = circular_less_than!(size_t)(first_index + actual_count, end_index)
                        ? first_index + actual_count
                        : end_index;

            while (index != end_index) {
                auto el = m_head.opIndex(index & (PAGE_CAPACITY - 1));
                items[index - first_index] = *el;
                destroy(*el);
                ++index;
            }

            // Retire the page once we've consumed through to its end.
            if ((end_index & cast(size_t)(PAGE_CAPACITY - 1)) == 0)
                remove_page!(Mode)();
        }

        return actual_count;
    }

  private:
    bool remove_page(AllocationMode Mode)() {
        auto old_page = m_head;

        m_head = cast(Page!T*) atomicLoad!(MemoryOrder.raw)(m_head.m_next);
        atomicFetchAdd!(MemoryOrder.rel)(m_base, BLOCK_SIZE);

        static if (Mode == AllocationMode.CannotAlloc) {
            if (old_page.dynamicly_allocated) {
                import util.alloc : dispose;
                dispose(old_page);
            }
        } else {
            m_recycle_list.add(cast(Node*) old_page);
        }
        return true;
    }

    bool add_new_page(AllocationMode Mode)() {
        auto page = recycle_page();
        if (page) return true;

        static if (Mode == AllocationMode.CannotAlloc) {
            return allocate_page();
        }

        return false;
    }

    bool recycle_page() {
        auto new_page = cast(Page!T*) m_recycle_list.try_get();

        if (new_page !is null) {
            if (m_tail !is null)
                atomicStore!(MemoryOrder.raw)(m_tail.m_next, cast(shared Node*) new_page);
            m_tail = new_page;
            return true;
        }

        return false;
    }

    bool allocate_page() {
        import util.alloc : make;
        auto new_page = make!(Page!T)();
        if (m_tail !is null)
            atomicStore!(MemoryOrder.raw)(m_tail.m_next, cast(shared Node*) new_page);
        m_tail = new_page;
        return true;
    }

    shared size_t m_base;
    shared size_t m_reader;
    shared size_t m_writer;
    shared size_t m_reader_optimistic = 0;
    shared size_t m_reader_overcommit = 0;

    Page!T*    m_head;
    Page!T*    m_tail;
    AtomicList m_recycle_list;
}
