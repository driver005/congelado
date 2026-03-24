export module queue;

import std;
import pager;
import node;
import atomic_list;
import consts;

export template <typename T>
struct ThreadNode : Node {
    std::size_t m_slot_id;
    Pager<T> *m_pager;

    explicit ThreadNode(std::size_t slot, AtomicList *recycle_list)
        : Node{}, m_slot_id{slot}, m_pager{new Pager<T>{recycle_list}} {}
};

export template <typename T>
class ConcurrentQueue {
  public:
    explicit ConcurrentQueue(AtomicList *queue_list, AtomicList *recycle_list)
        : m_queue_list(queue_list), m_recycle_list(recycle_list) {}

    bool enqueue(ThreadNode<T> const &node, T *item) { node.m_pager->enqueue<AllocationMode::CanAlloc>(item); }

    bool enqueue_bulk(ThreadNode<T> const &node, T **items, std::size_t count) {
        node.m_pager->enqueue_bulk<AllocationMode::CanAlloc>(items, count);
    };

    bool try_enqueue(ThreadNode<T> const &node, T *item) { node.m_pager->enqueue<AllocationMode::CannotAlloc>(item); };

    bool try_enqueue_bulk(ThreadNode<T> const &node, T **items, std::size_t count) {
        node.m_pager->enqueue_bulk<AllocationMode::CannotAlloc>(items, count);
    };

    bool try_dequeue(T *&item) {
        auto head = static_cast<ThreadNode<T> *>(m_recycle_list->get_head());
        auto start_head = head;
        auto winner = head->m_pager;

        head = head->m_next;

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

        if (winner->m_pager->dequeue(item)) {
            return true;
        };

        return false;
    };

    std::size_t try_dequeue_bulk(T **items, std::size_t max) {
        auto head = static_cast<ThreadNode<T> *>(m_recycle_list->get_head());
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

        if (winner->size() == 0) {
            return 0;
        }

        return winner->m_pager->dequeue_bulk(items, max);
    };

    std::size_t size_approx() {
        std::size_t count = 0;
        auto node = m_queue_list->get_head();
        while (node != nullptr) {
            count += static_cast<Pager<T>>(node).size();
            node = node->m_next.load(std::memory_order_relaxed);
        }

        return count;
    }

    // -----------------------------------------------------------------------------
    // Thread management
    // -----------------------------------------------------------------------------

    std::size_t thread_slot() { return std::hash<std::thread::id>{}(std::this_thread::get_id()); }

    ThreadNode<T> register_thread() {
        auto *node = new ThreadNode<T>{m_recycle_list, thread_slot()};
        m_queue_list->add(node);
        return *node;
    }

    void add_producer(std::function<void(ThreadNode<T> &)> work) {
        std::lock_guard lock(m_threads_mu);
        m_producers.emplace_back([this, w = std::move(work)](std::stop_token st) {
            auto node = register_thread();
            while (!st.stop_requested()) {
                w(node);
            }
        });
    }

    void add_consumer(std::function<void()> work) {
        std::lock_guard lock(m_threads_mu);
        m_consumers.emplace_back([this, w = std::move(work)](std::stop_token st) {
            while (!st.stop_requested()) {
                w();
            }
        });
    }

    void stop_all() {
        std::lock_guard lock(m_threads_mu);
        for (auto &t : m_producers)
            t.request_stop();
        for (auto &t : m_consumers)
            t.request_stop();
    }

  private:
    AtomicList *m_queue_list;
    AtomicList *m_recycle_list;

    std::mutex m_threads_mu; // guards vectors only, not hot path
    std::vector<std::jthread> m_producers;
    std::vector<std::jthread> m_consumers;
};
