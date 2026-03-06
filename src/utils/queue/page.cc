module page;

template <typename T> bool Page<T>::is_empty() const {
  if (dequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
    std::atomic_thread_fence(std::memory_order_acquire);
    return true;
  }
  assert(dequeued.load(std::memory_order_relaxed) <= BLOCK_SIZE);
  return false;
}

template <typename T> bool Page<T>::set_empty() {
  auto prev = dequeued.fetch_add(1, std::memory_order_acq_rel);
  assert(prev < BLOCK_SIZE);
  return prev == BLOCK_SIZE - 1;
}

template <typename T> bool Page<T>::set_many_empty(std::size_t count) {
  auto prev = dequeued.fetch_add(count, std::memory_order_acq_rel);
  assert(prev + count <= BLOCK_SIZE);
  return prev + count == BLOCK_SIZE;
}

template <typename T> void Page<T>::set_full_empty() { dequeued.store(BLOCK_SIZE, std::memory_order_relaxed); }

template <typename T> void Page<T>::reset_empty() { dequeued.store(0, std::memory_order_relaxed); }

template <typename T> inline T *Page<T>::operator[](std::size_t idx) noexcept {
  return static_cast<T *>(static_cast<void *>(elements)) +
         static_cast<std::size_t>(idx & static_cast<std::size_t>(BLOCK_SIZE - 1));
};

template <typename T> inline T const *Page<T>::operator[](std::size_t idx) const noexcept {
  return static_cast<T const *>(static_cast<void const *>(elements)) +
         static_cast<std::size_t>(idx & static_cast<std::size_t>(BLOCK_SIZE - 1));
};
