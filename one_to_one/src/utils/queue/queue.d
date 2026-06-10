module utils.queue.queue;
@nogc nothrow:

import utils.queue.pager;
import utils.queue.node;
import utils.queue.atomic_list;
import utils.consts : BLOCK_SIZE;
import util.alloc : make, dispose;

struct ThreadNode(T) {
    Node     m_node;
    size_t   m_slot_id;
    Pager!T* m_pager;

    this(size_t slot, AtomicList recycle_list) {
        m_node    = Node(0);
        m_slot_id = slot;
        m_pager   = make!(Pager!T)(recycle_list);
    }
}

class ConcurrentQueue(T) {
    @disable this(this);

    this(AtomicList queue_list, AtomicList recycle_list) {
        m_queue_list   = queue_list;
        m_recycle_list = recycle_list;
    }

    bool enqueue(ref const ThreadNode!T node, T* item) {
        return node.m_pager.enqueue!(AllocationMode.CanAlloc)(*item);
    }

    bool enqueue_bulk(ref const ThreadNode!T node, T** items, size_t count) {
        return node.m_pager.enqueue_bulk!(AllocationMode.CanAlloc)(items, count);
    }

    bool try_enqueue(ref const ThreadNode!T node, T* item) {
        return node.m_pager.enqueue!(AllocationMode.CannotAlloc)(*item);
    }

    bool try_enqueue_bulk(ref const ThreadNode!T node, T** items, size_t count) {
        return node.m_pager.enqueue_bulk!(AllocationMode.CannotAlloc)(items, count);
    }

    bool try_dequeue(ref T* item) {
        auto head       = cast(ThreadNode!T*) m_recycle_list.get_head();
        auto start_head = head;
        auto winner     = head.m_pager;

        head = cast(ThreadNode!T*) head.m_node.m_next;

        while (head.m_slot_id != start_head.m_slot_id) {
            auto sz = head.m_pager.size();
            if (sz == BLOCK_SIZE) {
                winner = head.m_pager;
                break;
            }
            if (sz > winner.size()) {
                winner = head.m_pager;
            }
            head = cast(ThreadNode!T*) head.m_node.m_next;
        }

        T dequeued;
        if (winner.dequeue!(AllocationMode.CanAlloc)(dequeued)) {
            item = &dequeued;
            return true;
        }

        return false;
    }

    size_t try_dequeue_bulk(T** items, size_t max) {
        auto head       = cast(ThreadNode!T*) m_recycle_list.get_head();
        auto start_head = head;
        auto winner     = head.m_pager;

        head = cast(ThreadNode!T*) head.m_node.m_next;

        while (head.m_slot_id != start_head.m_slot_id) {
            auto sz = head.m_pager.size();
            if (sz != BLOCK_SIZE) {
                if (sz > winner.size())
                    winner = head.m_pager;
                head = cast(ThreadNode!T*) head.m_node.m_next;
                continue;
            }
            break;
        }

        if (winner.size() == 0)
            return 0;

        // PORT-NOTE: dequeue_bulk takes T* items not T** — cast items array
        return winner.dequeue_bulk!(AllocationMode.CanAlloc)(
            cast(T*) items, max);
    }

    size_t size_approx() {
        size_t count = 0;
        auto node = m_queue_list.get_head();
        while (node !is null) {
            // PORT-NOTE: C++ cast from Node* to Pager<T> is inconsistent in
            // original source (likely a bug). Preserved as-is, cast to ThreadNode.
            auto tn = cast(ThreadNode!T*) node;
            if (tn.m_pager !is null)
                count += tn.m_pager.size();
            import core.atomic : atomicLoad, MemoryOrder;
            node = cast(Node*) atomicLoad!(MemoryOrder.raw)(node.m_next);
        }
        return count;
    }

    // -----------------------------------------------------------------------------
    // Thread management
    // -----------------------------------------------------------------------------

    size_t thread_slot() {
        // PORT-NOTE: C++ used std::hash<std::thread::id>; D uses pthread ID.
        import core.thread : Thread;
        return cast(size_t) cast(void*) Thread.getThis();
    }

    ThreadNode!T* register_thread() {
        auto node = make!(ThreadNode!T)(thread_slot(), m_recycle_list);
        m_queue_list.add(cast(Node*) node);
        return node;
    }

    // PORT-NOTE: C++ used std::function<void(ThreadNode<T>&)> for producers/consumers.
    // D @nogc equivalent: function pointer + context.
    void add_producer(void* ctx, void function(void*, ref ThreadNode!T) @nogc nothrow work) {
        // TODO: spawn @nogc thread with stop-token equivalent
    }

    void add_consumer(void* ctx, void function(void*) @nogc nothrow work) {
        // TODO: spawn @nogc thread with stop-token equivalent
    }

    void stop_all() {
        // TODO: signal all producer/consumer threads to stop
    }

  private:
    AtomicList m_queue_list;
    AtomicList m_recycle_list;

    // PORT-NOTE: C++ used std::mutex + std::vector<std::jthread>.
    // Thread management fields omitted; @nogc thread pools handled externally.
}
