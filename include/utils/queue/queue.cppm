export module queue;

import std;
import pager;
import node;
import atomic_list;
import consts;

export template<typename T>
struct ThreadNode : Node
{
    std::size_t m_slot_id;
    Pager<T>* m_pager;

    /**
     * @brief Builds a per-thread queue node — tags it with `slot` and spins up a fresh `Pager<T>`
     * wired to the shared recycle list.
     * @warning `new Pager<T>{recycle_list}` doesn't compile as written — `Pager<T>` (in
     * `pager.cppm`) declares no constructors at all, so it only has the implicit default one, and
     * its data members are all private, so it isn't an aggregate either. There's no constructor
     * on `Pager<T>` that accepts an `AtomicList*`. This whole module reads as unfinished/unwired
     * up against `pager.cppm`'s actual API — flagging it hard since this pass is comment-only and
     * nothing here is getting touched.
     * @param slot the thread slot id to tag this node with.
     * @param recycle_list the shared page recycle list this node's pager draws from.
     */
    explicit ThreadNode(std::size_t slot, AtomicList* recycle_list) :
        Node{},
        m_slot_id{slot},
        m_pager{new Pager<T>{recycle_list}}
    {
    }
};

export template<typename T>
class ConcurrentQueue
{
public:
    /**
     * @brief Binds this queue to its shared thread-registry list and page-recycle list.
     * @param queue_list the list of registered `ThreadNode`s (producers/consumers).
     * @param recycle_list the shared freelist of pages available for reuse.
     */
    explicit ConcurrentQueue(AtomicList* queue_list, AtomicList* recycle_list) :
        m_queue_list(queue_list),
        m_recycle_list(recycle_list)
    {
    }

    /**
     * @brief Supposed to enqueue `item` onto `node`'s pager, allowing allocation if a fresh page's
     * needed.
     * @warning Two real problems here, no cap: first, `node.m_pager->enqueue<AllocationMode::
     * CanAlloc>(item)`'s result is never returned — this function's declared `bool` but falls off
     * the end without a `return`, which is undefined behavior the instant a caller reads the
     * result. Second, `Pager<T>::enqueue()` takes a `const T &`, but `item` here is a `T *` — a
     * pointer where a reference-to-value is expected, which doesn't implicitly convert for an
     * arbitrary `T`. Same pointer-vs-value mismatch shows up in every sibling method below
     * (enqueue_bulk(), try_enqueue(), try_enqueue_bulk()) — this whole class's public surface
     * looks like it was written against a different `Pager<T>` API than the one that actually
     * exists in `pager.cppm`.
     * @param node the thread node whose pager gets the item.
     * @param item the item to enqueue.
     * @return intended to report success/failure, but see the warning — nothing's actually
     * returned.
     */
    bool enqueue(const ThreadNode<T>& node, T* item)
    {
        node.m_pager->template enqueue<AllocationMode::CAN_ALLOC>(item);
    }

    /**
     * @brief Bulk sibling of enqueue().
     * @warning Same missing-`return` issue as enqueue() — declared `bool`, never returns one.
     * Also same pointer-mismatch issue: `Pager<T>::enqueue_bulk()` wants `const T *items`, but
     * this passes `T **items` straight through, a pointer-to-pointer where a pointer-to-`T` is
     * expected.
     * @param node the thread node whose pager gets the items.
     * @param items the items to enqueue.
     * @param count how many items to enqueue.
     * @return intended to report success/failure, but see the warning.
     */
    bool enqueue_bulk(const ThreadNode<T>& node, T** items, std::size_t count)
    {
        node.m_pager->template enqueue_bulk<AllocationMode::CAN_ALLOC>(items, count);
    };

    /**
     * @brief Non-allocating sibling of enqueue() — should fail rather than grow the page pool.
     * @warning Same missing-`return` and pointer-vs-value mismatch as enqueue() above.
     * @param node the thread node whose pager gets the item.
     * @param item the item to enqueue.
     * @return intended to report success/failure, but see the warning.
     */
    bool try_enqueue(const ThreadNode<T>& node, T* item)
    {
        node.m_pager->template enqueue<AllocationMode::CANNOT_ALLOC>(item);
    };

    /**
     * @brief Non-allocating sibling of enqueue_bulk().
     * @warning Same missing-`return` and pointer-vs-value mismatch as enqueue_bulk() above.
     * @param node the thread node whose pager gets the items.
     * @param items the items to enqueue.
     * @param count how many items to enqueue.
     * @return intended to report success/failure, but see the warning.
     */
    bool try_enqueue_bulk(const ThreadNode<T>& node, T** items, std::size_t count)
    {
        node.m_pager->template enqueue_bulk<AllocationMode::CANNOT_ALLOC>(items, count);
    };

    /**
     * @brief Supposed to pick the busiest registered thread's pager and dequeue one element from
     * it — a work-stealing-style "find the fullest queue" scan across every registered
     * `ThreadNode`.
     * @warning Multiple real issues, flagging them all since this pass is comment-only: (1)
     * `head = head->m_next;` assigns a `Node*` (from the inherited `std::atomic<Node*>` member,
     * implicitly loaded) into `head`, which is statically typed `ThreadNode<T> *` — that's an
     * invalid implicit downcast, needs an explicit `static_cast` like the initial assignment two
     * lines up got. (2) `head->m_pager->size()` calls `size()` on a `Pager<T>`, but `Pager<T>`
     * (in `pager.cppm`) has no `size()` member at all. (3) `winner` is already a `Pager<T> *`
     * (from `head->m_pager`), so `winner->m_pager->dequeue(item)` tries to access a `m_pager`
     * member on `Pager<T>` that doesn't exist — should almost certainly just be
     * `winner->dequeue(item)`. This method as written doesn't compile.
     * @param[out] item receives a pointer to the dequeued element on success.
     * @return intended to report whether an element was found, but see the warning — the logic
     * can't actually run as written.
     */
    bool try_dequeue(T*& item)
    {
        // Start the scan at the registry's head, remembering its pager as the current best guess.
        auto head = static_cast<ThreadNode<T>*>(m_recycle_list->get_head());
        auto start_head = head;
        auto winner = head->m_pager;

        head = head->m_next;

        // Walk the ring of registered threads looking for the busiest pager, no cap — a full page
        // short-circuits immediately, otherwise just track whichever one's biggest so far.
        while (head->m_slot_id != start_head->m_slot_id) {
            auto size = head->m_pager->size();
            if (size == BLOCK_SIZE) {
                winner = head->m_pager;
                break;
            }
            if (size > winner->size()) {
                winner = head->m_pager;
            }
            head = head->m_next;
        }

        // Whichever pager won the scan, try pulling one element off it.
        if (winner->m_pager->dequeue(item)) {
            return true;
        };

        return false;
    };

    /**
     * @brief Bulk sibling of try_dequeue() — same busiest-pager scan, then a bulk dequeue off the
     * winner.
     * @warning Same `head->m_next` invalid-downcast issue as try_dequeue(), plus every call to
     * `.size()` on a `Pager<T>*` hits the same missing-member problem noted there. Unlike
     * try_dequeue() though, `winner->m_pager->dequeue_bulk(...)` isn't reached through a
     * `->m_pager->` typo here — wait, it is: `winner` is a `Pager<T> *` and `winner->m_pager`
     * still doesn't exist. Same class of bug as try_dequeue(), this method doesn't compile as
     * written either.
     * @param[out] items receives dequeued elements.
     * @param max the maximum number of elements to dequeue.
     * @return intended to report how many elements were dequeued, but see the warning.
     */
    std::size_t try_dequeue_bulk(T** items, std::size_t max)
    {
        // Same busiest-pager scan as try_dequeue() — walk the ring, track the biggest pager seen,
        // and stop early the moment a full page turns up.
        auto head = static_cast<ThreadNode<T>*>(m_recycle_list->get_head());
        auto start_head = head;
        auto winner = head->m_pager;

        head = head->m_next;

        while (head->m_slot_id != start_head->m_slot_id) {
            auto size = head->m_pager->size();
            if (size != BLOCK_SIZE) {
                if (size > winner->size()) {
                    winner = head->m_pager;
                }
                head = head->m_next;
                continue;
            }
            break;
        }

        // Nothing to dequeue even from the winner — bail before touching it.
        if (winner->size() == 0) {
            return 0;
        }

        return winner->m_pager->dequeue_bulk(items, max);
    };

    /**
     * @brief Supposed to sum up the approximate element count across every registered thread's
     * pager.
     * @warning `static_cast<Pager<T>>(node)` tries to cast a `Node *` into a `Pager<T>` *value* —
     * there's no such conversion, `Pager<T>` isn't constructible from a `Node*` and this isn't a
     * pointer cast (no `*` in the cast target). On top of that, `Pager<T>` has no `size()` member
     * (same gap noted in try_dequeue()). Doesn't compile as written.
     * @return intended to be the approximate total element count, but see the warning.
     */
    std::size_t size_approx()
    {
        // Walk every registered thread node and fold its pager's size into the running total.
        std::size_t count = 0;
        auto* node = m_queue_list->get_head();
        while (node != nullptr) {
            count += static_cast<Pager<T>>(node).size();
            node = node->m_next.load(std::memory_order_relaxed);
        }

        return count;
    }

    // -----------------------------------------------------------------------------
    // Thread management
    // -----------------------------------------------------------------------------

    /**
     * @brief Derives a slot id for the calling thread by hashing its `std::thread::id`.
     * @return a hash-derived slot id for the current thread.
     */
    std::size_t thread_slot()
    {
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    /**
     * @brief Registers the calling thread with a fresh `ThreadNode`, linking it onto the shared
     * queue list.
     * @warning `new ThreadNode<T>{m_recycle_list, thread_slot()}` passes the args in the wrong
     * order — `ThreadNode`'s constructor is `(std::size_t slot, AtomicList *recycle_list)`, but
     * here it's `(m_recycle_list, thread_slot())`, an `AtomicList *` where `std::size_t` is
     * expected and vice versa. Neither converts implicitly, so this doesn't compile. Separately:
     * `return *node;` returns a `ThreadNode<T>` by value, but `ThreadNode` inherits `Node`, which
     * holds `std::atomic` members — atomics have deleted copy AND move constructors, so
     * `ThreadNode<T>` has no viable copy or move constructor either. Returning `*node` by value
     * can't compile regardless of the constructor-argument issue above. This method has two
     * independent reasons it can't build as written.
     * @return intended to be the newly-registered node, but see the warning.
     */
    ThreadNode<T> register_thread()
    {
        // Build a fresh node for the calling thread, link it onto the shared registry, then hand
        // it back to the caller.
        auto* node = new ThreadNode<T>{
            m_recycle_list, thread_slot()
        }; // FIXME(clang-tidy): cppcoreguidelines-owning-memory — would need
           // gsl::owner<ThreadNode<T> *>, but this codebase has no GSL dependency; not a mechanical
           // fix
        m_queue_list->add(node);
        return *node;
    }

    /**
     * @brief Spins up a producer thread that registers itself and repeatedly calls `work` until
     * stopped.
     * @param work the per-iteration producer callback, invoked with the thread's own
     * `ThreadNode`.
     */
    void add_producer(std::function<void(ThreadNode<T>&)> work)
    {
        // Lock just long enough to grow the producer vector — the thread itself registers and
        // loops entirely on its own, outside the lock.
        std::scoped_lock lock(m_threads_mu);
        m_producers.emplace_back([this, work_callback =
                                            std::move(work)](const std::stop_token& stop_token) {
            auto node = register_thread();
            while (!stop_token.stop_requested()) {
                work_callback(node);
            }
        });
    }

    /**
     * @brief Spins up a consumer thread that repeatedly calls `work` until stopped.
     * @param work the per-iteration consumer callback.
     */
    void add_consumer(std::function<void()> work)
    {
        // Same deal as add_producer() — lock only for the vector growth, the loop itself runs
        // unlocked on its own thread.
        std::scoped_lock lock(m_threads_mu);
        m_consumers.emplace_back([this, work_callback =
                                            std::move(work)](const std::stop_token& stop_token) {
            while (!stop_token.stop_requested()) {
                work_callback();
            }
        });
    }

    /**
     * @brief Requests a stop on every producer and consumer thread this queue owns. Doesn't join
     * them — that's `std::jthread`'s job on destruction.
     */
    void stop_all()
    {
        // Signal every producer and every consumer to stop — joining them is std::jthread's job
        // on destruction, not this method's.
        std::scoped_lock lock(m_threads_mu);
        for (auto& producer: m_producers) {
            producer.request_stop();
        }
        for (auto& consumer: m_consumers) {
            consumer.request_stop();
        }
    }

private:
    AtomicList* m_queue_list;
    AtomicList* m_recycle_list;

    std::mutex m_threads_mu; // guards vectors only, not hot path
    std::vector<std::jthread> m_producers;
    std::vector<std::jthread> m_consumers;
};
