module utils.queue.page;
@nogc nothrow:

import core.atomic;
import utils.queue.node;
import utils.consts : BLOCK_SIZE;
import utils.helper;

class Page(T) : Node {
    mixin AlignedManager;

    this() {
        super(0);
        dynamicly_allocated = false;
        atomicStore!(MemoryOrder.raw)(dequeued, cast(size_t) 0);
    }

    bool is_empty() const {
        if (atomicLoad!(MemoryOrder.raw)(dequeued) == BLOCK_SIZE) {
            // std::atomic_thread_fence(std::memory_order_acquire)
            atomicFence!(MemoryOrder.acq)();
            return true;
        }
        return false;
    }

    bool set_empty() {
        auto prev = atomicFetchAdd!(MemoryOrder.acq_rel)(dequeued, cast(size_t) 1);
        return prev == BLOCK_SIZE - 1;
    }

    bool set_many_empty(size_t count) {
        auto prev = atomicFetchAdd!(MemoryOrder.acq_rel)(dequeued, count);
        return prev + count == BLOCK_SIZE;
    }

    void set_full_empty() { atomicStore!(MemoryOrder.raw)(dequeued, BLOCK_SIZE); }

    void reset_empty() { atomicStore!(MemoryOrder.raw)(dequeued, cast(size_t) 0); }

    T* opIndex(size_t idx) {
        return cast(T*)(cast(void*) elements.ptr) +
               (idx & (BLOCK_SIZE - 1));
    }

    const(T)* opIndex(size_t idx) const {
        return cast(const(T)*)(cast(const(void)*) elements.ptr) +
               (idx & (BLOCK_SIZE - 1));
    }

  private:
    align(T.alignof) ubyte[T.sizeof * BLOCK_SIZE] elements;
    bool dynamicly_allocated;
    shared size_t dequeued;
}
