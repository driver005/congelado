module queue;

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue(AtomicList *queue_list, AtomicList *recycle_list)
    : m_queue_list(queue_list), m_recycle_list(recycle_list) {}

template <typename T> bool ConcurrentQueue<T>::enqueue(ThreadNode<T> const &node, T *item) {
  node.m_pager->enqueue(item);
}

template <typename T> std::size_t ConcurrentQueue<T>::size_approx() {
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

template <typename T> std::size_t ConcurrentQueue<T>::thread_slot() {
  return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

template <typename T> ThreadNode<T> ConcurrentQueue<T>::register_thread() {
  auto *node = new ThreadNode<T>{m_recycle_list, thread_slot()};
  m_queue_list->add(node);
  return *node;
}

template <typename T> void ConcurrentQueue<T>::add_producer(std::function<void(ThreadNode<T> &)> work) {
  std::lock_guard lock(m_threads_mu);
  m_producers.emplace_back([this, w = std::move(work)](std::stop_token st) {
    auto node = register_thread();
    while (!st.stop_requested()) {
      w(node);
    }
  });
}

template <typename T> void ConcurrentQueue<T>::add_consumer(std::function<void(ThreadNode<T> &)> work) {
  std::lock_guard lock(m_threads_mu);
  m_consumers.emplace_back([this, w = std::move(work)](std::stop_token st) {
    auto node = register_thread();
    while (!st.stop_requested()) {
      w(node);
    }
  });
}

template <typename T> void ConcurrentQueue<T>::stop_all() {
  std::lock_guard lock(m_threads_mu);
  for (auto &t : m_producers)
    t.request_stop();
  for (auto &t : m_consumers)
    t.request_stop();
}
