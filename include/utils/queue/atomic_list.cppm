// https://github.com/cameron314/concurrentqueue/concurrentqueue.h#L1450
export module atomic_list;

import std;
import node;
import consts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif


export class AtomicList {
  public:
    /**
     * @brief Lock-free pop off the freelist — this is the whole ABA-proofing trick from
     * moodycamel's concurrentqueue: bump a node's refcount before trying to unlink it, so if
     * another thread wins the race to reclaim/re-add that same node in between, this thread's
     * still holding a live reference and can safely back out instead of touching freed memory.
     * @warning Not a simple CAS loop, don't skim it like one. Every failed attempt has to walk
     * back through fetch_sub-ing the refcount it optimistically bumped, and if that fetch_sub
     * reveals it was the last ref *and* the should-be-on-list bit says so, it re-adds the node
     * itself via add_internal() before looping again. Miss that path in a refactor and nodes
     * silently stop making it back onto the list — a leak that only shows up under real
     * contention, not exactly a fun one to chase down.
     * @return a node pulled off the freelist, or nullptr if the list was empty.
     */
    Node *try_get() noexcept {
        auto *head = m_head.load(std::memory_order_acquire);
        while (head != nullptr) {
            auto *prev_head = head;
            // Try to bump the observed head's refcount before touching anything else, lowkey the
            // whole ABA-proofing trick — a zero-refcount head means it's already being reclaimed
            // elsewhere, and a failed CAS means someone else raced us to it. Either way, reload
            // and try again instead of pressing on with a node we don't safely own yet.
            auto refs = head->m_refs.load(std::memory_order_relaxed);
            if ((refs & REFS_MASK) == 0 ||
                !head->m_refs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire)) {
                head = m_head.load(std::memory_order_acquire);
                continue;
            }

            // assert(head->m_refs.load(std::memory_order_relaxed) == 2);

            // Good, reference count has been incremented (it wasn't at zero), which means we can read the
            // next and not worry about it changing between now and the time we do the CAS
            auto *next = head->m_next.load(std::memory_order_relaxed);
            if (m_head.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)) {
                // Yay, got the node. This means it was on the list, which means shouldBeOnAtomicList must be false no
                // matter the refcount (because nobody else knows it's been taken off yet, it can't have been put back
                // on). assert((head->m_refs.load(std::memory_order_relaxed) & SHOULD_BE_ON_LIST) == 0); Decrease
                // refcount twice, once for our ref, and once for the list's ref
                head->m_refs.fetch_sub(2, std::memory_order_release);
                return head;
            }

            // OK, the head must have changed on us, but we still need to decrease the refcount we increased.
            // Note that we don't need to release any memory effects, but we do need to ensure that the reference
            // count decrement happens-after the CAS on the head.
            refs = prev_head->m_refs.fetch_sub(1, std::memory_order_acq_rel);
            if (refs == SHOULD_BE_ON_LIST + 1) {
                // assert(refs == SHOULD_BE_ON_LIST + 1);
                add_internal(prev_head);
            }
        }

        return nullptr;
    }

    /**
     * @brief Marks `node` as wanting to be back on the freelist and, if this call turns out to be
     * the last reference standing, actually pushes it on via add_internal(). If someone else is
     * still holding a ref, they're the one who'll add it once they drop theirs — no double-add,
     * no cap.
     * @param node the node to return to the freelist.
     */
    void add(Node *node) noexcept {
        // We know that the should-be-on-freelist bit is 0 at this point, so it's safe to
        // set it using a fetch_add
        if (node->m_refs.fetch_add(SHOULD_BE_ON_LIST, std::memory_order_acq_rel) == 0) {
            // Oh look! We were the last ones referencing this node, and we know
            // we want to add it to the free list, so let's do it!
            // assert(node->m_refs.load(std::memory_order_relaxed) == SHOULD_BE_ON_LIST);
            add_internal(node);
        }
    }

    /**
     * @brief Peeks the current head without taking a reference or removing anything.
     * @warning Relaxed load, no synchronization — the pointer you get back can be immediately
     * stale if another thread pops it concurrently. Fine for a rough peek, not fine to treat as
     * "the node I now own." Straight vibes-based, no ownership guarantee attached.
     * @return the current head node, or nullptr if the list is empty.
     */
    [[nodiscard]] Node *get_head() const noexcept { return m_head.load(std::memory_order_relaxed); }

  private:
    /**
     * @brief The actual freelist push — CAS-loops `node` onto the head, and if it loses the race,
     * retries by walking the refcount back down until it's safe to try again. Straightforward
     * CAS-retry motion once you see past the refcount bookkeeping.
     * @param node the node to push, must already have its should-be-on-list bit set by the
     * caller.
     */
    void add_internal(Node *node) noexcept {
        auto *head = m_head.load(std::memory_order_relaxed);
        while (true) {
            // Stage `node` in front of the last-observed head and reset its refcount to 1 (just
            // the list's own ownership) before attempting the CAS that actually splices it in.
            node->m_next.store(head, std::memory_order_relaxed);
            node->m_refs.store(1, std::memory_order_release);
            if (!m_head.compare_exchange_strong(head, node, std::memory_order_release, std::memory_order_relaxed)) {
                // Hmm, the add failed, but we can only try again when the refcount goes back to zero
                if (node->m_refs.fetch_add(SHOULD_BE_ON_LIST - 1, std::memory_order_acq_rel) == 1) {
                    // assert(node->m_refs.load(std::memory_order_relaxed) == SHOULD_BE_ON_LIST - 1);
                    continue;
                }
            }

            // assert(node->m_refs.load(std::memory_order_relaxed) == 1);
            return;
        }
    }

    std::atomic<Node *> m_head{nullptr};
};

#ifdef CONGELADO_TEST
namespace {
using namespace boost::ut;

suite<"AtomicList"> atomic_list_suite = [] {
    "starts empty"_test = [] {
        AtomicList list;
        expect(list.get_head() == nullptr);
        expect(list.try_get() == nullptr);
    };
    "add then try_get round-trips a single node"_test = [] {
        AtomicList list;
        Node node;

        list.add(&node);
        expect(list.get_head() == &node);

        auto *got = list.try_get();
        expect(got == &node);
        expect(list.get_head() == nullptr);
    };
    "add is LIFO — try_get drains most-recently-added first"_test = [] {
        AtomicList list;
        Node first;
        Node second;

        list.add(&first);
        list.add(&second);
        expect(list.get_head() == &second);

        expect(list.try_get() == &second);
        expect(list.try_get() == &first);
        expect(list.try_get() == nullptr);
    };
    "a node can be returned to the list after being taken off"_test = [] {
        AtomicList list;
        Node node;

        list.add(&node);
        auto *got = list.try_get();
        expect(got == &node);

        list.add(got);
        expect(list.get_head() == &node);
        expect(list.try_get() == &node);
    };
};

} // namespace
#endif
