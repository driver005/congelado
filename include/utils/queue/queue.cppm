export module queue;

import std;
import pager;
import node;
import atomic_list;

export template <typename T> struct ThreadNode : Node {
  std::size_t m_slot_id;
  Pager<T> *m_pager;

  explicit ThreadNode(std::size_t slot, AtomicList *recycle_list)
      : Node{}, m_slot_id{slot}, m_pager{new Pager<T>{recycle_list}} {}
};

export template <typename T> class ConcurrentQueue {
public:
  explicit ConcurrentQueue(AtomicList *queue_list, AtomicList *recycle_list);
  ~ConcurrentQueue() = default; // jthreads auto-join

  // --- enqueue (allocates if necessary) ---
  bool enqueue(ThreadNode<T> const &node, T *item);
  bool enqueue_bulk(T **items, std::size_t count);

  // --- try_enqueue (never allocates) ---
  bool try_enqueue(T *item);
  bool try_enqueue_bulk(T **items, std::size_t count);

  // --- try_dequeue (never allocates) ---
  bool try_dequeue(T *&item);
  std::size_t try_dequeue_bulk(T **items, std::size_t max);

  // --- misc ---
  // Approximate size of the queue. May be inaccurate due to concurrent modifications.
  std::size_t size_approx();

  // Register the calling thread into m_queue_list via its hashed thread id.
  // Idempotent — safe to call multiple times from the same thread.
  ThreadNode<T> register_thread();

  // Returns the slot index for the calling thread.
  static std::size_t thread_slot();

  // Spawn a producer jthread. `work` is called in a loop until stop is requested.
  void add_producer(std::function<void(ThreadNode<T> &)> work);

  // Spawn a consumer jthread. `work` is called in a loop until stop is requested.
  void add_consumer(std::function<void(ThreadNode<T> &)> work);

  // Cooperatively request all threads to stop.
  // jthread destructors will join them automatically.
  void stop_all();

private:
  AtomicList *m_queue_list;
  AtomicList *m_recycle_list;

  std::mutex m_threads_mu; // guards vectors only, not hot path
  std::vector<std::jthread> m_producers;
  std::vector<std::jthread> m_consumers;
};
