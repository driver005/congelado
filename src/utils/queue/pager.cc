module pager;

template <typename T> template <AllocationMode Mode> bool Pager<T>::enqueue(const T &item) {
  auto check_index = m_writer.load(std::memory_order_relaxed);

  // Check if index is equal to block size, if so we need to allocate a new page becouse else we whould be writing to
  // the start of the current page instead of the next page
  // This is a safe operation every thread has its own list of pages
  if ((check_index & (BLOCK_SIZE - 1)) == 0) {
    auto status = add_new_page();
    if (!status) {
      // Failed to add new page, likely due to allocation failure in CannotAlloc mode and no more pages available in
      // recycle list
      return false;
    }
  }

  // It is space in the current page, we can increase the writer index without worrying about a rollback
  auto index = m_writer.fetch_add(1, std::memory_order_acq_rel);

  // Calculate block base index
  auto block_index = index & (BLOCK_SIZE - 1);
  m_tail->operator[](block_index) = item;

  return true;
}

template <typename T> template <AllocationMode Mode> bool Pager<T>::enqueue_bulk(const T *items, std::size_t count) {
  if (count == 0) {
    return true; // Nothing to enqueue, trivially successful
  }

  std::size_t start_index = m_writer.load(std::memory_order_relaxed);
  auto end_index = start_index + count;

  auto *const start_page = m_tail;
  auto *end_page = m_tail;
  Page<T> *first_alloc_page = nullptr;

  std::size_t page_diff = (end_index & ~static_cast<std::size_t>(BLOCK_SIZE - 1)) -
                          (start_index & ~static_cast<std::size_t>(BLOCK_SIZE - 1));

  while (page_diff > 0) {
    page_diff -= static_cast<std::size_t>(BLOCK_SIZE);

    // Oh no, the alloction falied what now? In case of a faile we need to rollback all pages allocated starting with
    // first_alloc_page as long as it has a next page!
    if (!add_new_page<Mode>()) {
      Page<T> *current_page = first_alloc_page;
      while (current_page) {
        auto *next_page = current_page->m_next;
        remove_page<Mode>();
        current_page = next_page;
      }

      if (start_page != nullptr) {
        start_page->m_next = nullptr;
      }

      m_tail = start_page;
      return false;
    }

    end_page = m_tail;
    if (first_alloc_page == nullptr) {
      first_alloc_page = end_page;
    }
  }

  // Rest the tail so we can write the items into the corresponding pages
  if ((start_index & static_cast<std::size_t>(BLOCK_SIZE - 1)) == 0 && first_alloc_page != nullptr) {
    m_tail = first_alloc_page;
  } else {
    m_tail = start_page;
  }

  std::size_t write_index = start_index;
  while (true) {
    // stop_index = first index of the NEXT block (or end_index if
    // the remaining items all fit in this block).
    std::size_t stop_index =
        (write_index & ~static_cast<std::size_t>(BLOCK_SIZE - 1)) + static_cast<std::size_t>(BLOCK_SIZE);
    if (circular_less_than<std::size_t>(end_index, stop_index)) {
      stop_index = end_index;
    }

    while (write_index != stop_index) {
      const std::size_t block_index = write_index & (static_cast<std::size_t>(BLOCK_SIZE) - 1);
      m_tail->operator[](block_index) = items[write_index - start_index];
      ++write_index;
    }

    if (m_tail == end_page) {
      // We've written everything; start_index == end_index.
      break;
    }

    m_tail = m_tail->m_next;
  }

  m_writer.store(end_index, std::memory_order_release);
  return true;
}

template <typename T> template <AllocationMode Mode> bool Pager<T>::dequeue(T &item) {
  auto tail = m_writer.load(std::memory_order_relaxed);
  auto overcommit = m_reader_overcommit.load(std::memory_order_relaxed);

  // Check if there might be elements available using the optimistic formula
  if (circular_less_than<std::size_t>(m_reader_optimistic.load(std::memory_order_relaxed) - overcommit, tail)) {
    // Acquire fence to synchronize with potential overcommit updates
    std::atomic_thread_fence(std::memory_order_acquire);

    // Optimistically claim an element
    auto myDequeueCount = m_reader_optimistic.fetch_add(1, std::memory_order_relaxed);

    // Reload tail in case it changed
    tail = m_writer.load(std::memory_order_acquire);

    // Verify the claim is still valid
    if (circular_less_than<std::size_t>(myDequeueCount - overcommit, tail)) {
      // Successfully claimed an element - proceed with actual dequeue
      auto index = m_reader.fetch_add(1, std::memory_order_acq_rel);
      auto block_index = index & (BLOCK_SIZE - 1);

      // Handle page boundary with proper synchronization
      if ((block_index & (BLOCK_SIZE - 1)) == 0) {
        // Remove old page and get new one
        auto status = remove_page<Mode>();
        if (!status) {
          // Failed to remove page, likely due to allocation failure in CannotAlloc mode no more pages available in
          // recycle list
          // Rollback the optimistic claim and return false
          m_reader_overcommit.fetch_add(1, std::memory_order_release);
          return false;
        }

        // Verify we're on the correct page after removal
        auto current_page_index = m_base.load(std::memory_order_relaxed);
        auto expected_page_index = block_index / BLOCK_SIZE;

        if (current_page_index != expected_page_index) {
          // We're on the wrong page, retry
          // This handles the race condition mentioned in the TODO
          m_reader_overcommit.fetch_add(1, std::memory_order_release);
          return dequeue(item); // Retry
        }
      }

      // Safely read the element
      item = std::move(m_head->operator[](index & (BLOCK_SIZE - 1)));
      return true;
    } else {
      // No element available - rollback the optimistic claim
      m_reader_overcommit.fetch_add(1, std::memory_order_release);
    }
  }

  return false;
}

template <typename T> template <AllocationMode Mode> std::size_t Pager<T>::dequeue_bulk(T *items, std::size_t max) {
  auto tail = m_writer.load(std::memory_order_relaxed);
  auto overcommit = m_reader_overcommit.load(std::memory_order_relaxed);

  auto desired_count =
      static_cast<std::size_t>(tail - (m_reader_optimistic.load(std::memory_order_relaxed) - overcommit));

  if (!circular_less_than<std::size_t>(0, desired_count)) {
    return 0;
  }

  desired_count = desired_count < max ? desired_count : max;

  std::atomic_thread_fence(std::memory_order_acquire);

  auto my_dequeue_count = m_reader_optimistic.fetch_add(desired_count, std::memory_order_relaxed);

  tail = m_writer.load(std::memory_order_acquire);

  auto actual_count = static_cast<std::size_t>(tail - (my_dequeue_count - overcommit));

  if (!circular_less_than<std::size_t>(0, actual_count)) {
    m_reader_overcommit.fetch_add(desired_count, std::memory_order_release);
    return 0;
  }

  actual_count = desired_count < actual_count ? desired_count : actual_count;

  if (actual_count < desired_count) {
    m_reader_overcommit.fetch_add(desired_count - actual_count, std::memory_order_release);
  }

  auto first_index = m_reader.fetch_add(actual_count, std::memory_order_acq_rel);
  auto index = first_index;

  while (index != first_index + actual_count) {
    std::size_t end_index =
        (index & ~static_cast<std::size_t>(Page<T>::CAPACITY - 1)) + static_cast<std::size_t>(Page<T>::CAPACITY);
    end_index =
        circular_less_than<std::size_t>(first_index + actual_count, end_index) ? first_index + actual_count : end_index;

    if constexpr (std::is_nothrow_move_assignable_v<T>) {
      while (index != end_index) {
        auto &el = (*m_head)[index & (Page<T>::CAPACITY - 1)];
        items[index - first_index] = std::move(el);
        el.~T();
        ++index;
      }
    } else {
      try {
        while (index != end_index) {
          auto &el = (*m_head)[index & (Page<T>::CAPACITY - 1)];
          items[index - first_index] = std::move(el);
          el.~T();
          ++index;
        }
      } catch (...) {
        // Destroy all remaining claimed-but-unread elements before rethrowing.
        auto cleanup = index;
        while (cleanup != first_index + actual_count) {
          std::size_t cleanup_end = (cleanup & ~static_cast<std::size_t>(Page<T>::CAPACITY - 1)) +
                                    static_cast<std::size_t>(Page<T>::CAPACITY);
          cleanup_end = circular_less_than<std::size_t>(first_index + actual_count, cleanup_end)
                            ? first_index + actual_count
                            : cleanup_end;

          while (cleanup != cleanup_end) {
            (*m_head)[cleanup & (Page<T>::CAPACITY - 1)].~T();
            ++cleanup;
          }

          if ((cleanup_end & static_cast<std::size_t>(Page<T>::CAPACITY - 1)) == 0) {
            remove_page<Mode>();
          }
        }
        throw;
      }
    }

    // Retire the page once we've consumed through to its end.
    if ((end_index & static_cast<std::size_t>(Page<T>::CAPACITY - 1)) == 0) {
      remove_page<Mode>();
    }
  }

  return actual_count;
}

template <typename T> template <AllocationMode Mode> void Pager<T>::remove_page() {
  auto *old_page = m_head;

  m_head = m_head->m_next;
  m_base.fetch_add(BLOCK_SIZE, std::memory_order_release);

  if constexpr (Mode == AllocationMode::CannotAlloc) {
    if (old_page->dynamicly_allocated) {
      delete old_page;
    }
  } else {
    m_recycle_list->add(old_page);
  }
}

template <typename T> template <AllocationMode Mode> bool Pager<T>::add_new_page() {
  auto page = recycle_page();
  if (page) {
    return true;
  }
  if constexpr (Mode == AllocationMode::CannotAlloc) {
    return allocate_page();
  }

  return false;
}

template <typename T> bool Pager<T>::recycle_page() {
  auto *new_page = m_recycle_list->try_get();

  if (new_page != nullptr) {
    if (m_tail) {
      m_tail->m_next = new_page;
    }
    m_tail = new_page;
    return true;
  }

  return false;
}

template <typename T> bool Pager<T>::allocate_page() {
  auto *new_page = new Page<T>();
  if (m_tail) {
    m_tail->m_next = new_page;
  }
  m_tail = new_page;

  return true;
}
