module utils.queue.atomic_list;
@nogc nothrow:

// https://github.com/cameron314/concurrentqueue/concurrentqueue.h#L1450

import core.atomic;
import utils.queue.node;
import utils.consts : REFS_MASK, SHOULD_BE_ON_LIST;

class AtomicList {
    @disable this(this);

    this() {
        atomicStore!(MemoryOrder.raw)(m_head, cast(Node*) null);
    }

    Node* try_get() {
        auto head = atomicLoad!(MemoryOrder.acq)(m_head);
        while (head !is null) {
            auto prevHead = head;
            auto refs     = atomicLoad!(MemoryOrder.raw)(head.m_refs);
            if ((refs & REFS_MASK) == 0 ||
                !cas!(MemoryOrder.acq, MemoryOrder.raw)(
                    &head.m_refs, refs, refs + 1)) {
                head = atomicLoad!(MemoryOrder.acq)(m_head);
                continue;
            }

            // assert(head->m_refs.load(std::memory_order_relaxed) == 2);

            // Good, reference count has been incremented (it wasn't at zero), which means we can read the
            // next and not worry about it changing between now and the time we do the CAS
            auto next = atomicLoad!(MemoryOrder.raw)(head.m_next);
            if (cas!(MemoryOrder.acq, MemoryOrder.raw)(
                    &m_head, head, next)) {
                // Yay, got the node. This means it was on the list, which means shouldBeOnAtomicList must be false no
                // matter the refcount (because nobody else knows it's been taken off yet, it can't have been put back
                // on). assert((head->m_refs.load(std::memory_order_relaxed) & SHOULD_BE_ON_LIST) == 0); Decrease
                // refcount twice, once for our ref, and once for the list's ref
                atomicFetchSub!(MemoryOrder.rel)(head.m_refs, cast(size_t) 2);
                return head;
            }

            // OK, the head must have changed on us, but we still need to decrease the refcount we increased.
            // Note that we don't need to release any memory effects, but we do need to ensure that the reference
            // count decrement happens-after the CAS on the head.
            refs = atomicFetchSub!(MemoryOrder.acq_rel)(prevHead.m_refs, cast(size_t) 1);
            if (refs == SHOULD_BE_ON_LIST + 1) {
                // assert(refs == SHOULD_BE_ON_LIST + 1);
                add_internal(prevHead);
            }
        }

        return null;
    }

    void add(Node* node) {
        // We know that the should-be-on-freelist bit is 0 at this point, so it's safe to
        // set it using a fetch_add
        if (atomicFetchAdd!(MemoryOrder.acq_rel)(node.m_refs, SHOULD_BE_ON_LIST) == 0) {
            // Oh look! We were the last ones referencing this node, and we know
            // we want to add it to the free list, so let's do it!
            // assert(node->m_refs.load(std::memory_order_relaxed) == SHOULD_BE_ON_LIST);
            add_internal(node);
        }
    }

    Node* get_head() const { return atomicLoad!(MemoryOrder.raw)(m_head); }

  private:
    void add_internal(Node* node) {
        auto head = atomicLoad!(MemoryOrder.raw)(m_head);
        while (true) {
            atomicStore!(MemoryOrder.raw)(node.m_next, head);
            atomicStore!(MemoryOrder.rel)(node.m_refs, cast(size_t) 1);
            if (!cas!(MemoryOrder.rel, MemoryOrder.raw)(
                    &m_head, head, node)) {
                // Hmm, the add failed, but we can only try again when the refcount goes back to zero
                if (atomicFetchAdd!(MemoryOrder.acq_rel)(
                        node.m_refs, SHOULD_BE_ON_LIST - 1) == 1) {
                    // assert(node->m_refs.load(std::memory_order_relaxed) == SHOULD_BE_ON_LIST - 1);
                    continue;
                }
            }

            // assert(node->m_refs.load(std::memory_order_relaxed) == 1);
            return;
        }
    }

    shared Node* m_head;
}
