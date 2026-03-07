module atomic_list;

void AtomicList::add_internal(Node *node) noexcept {
  auto head = m_head.load(std::memory_order_relaxed);
  while (true) {
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

Node *AtomicList::try_get() noexcept {
  auto head = m_head.load(std::memory_order_acquire);
  while (head != nullptr) {
    auto prevHead = head;
    auto refs = head->m_refs.load(std::memory_order_relaxed);
    if ((refs & REFS_MASK) == 0 || !head->m_refs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire)) {
      head = m_head.load(std::memory_order_acquire);
      continue;
    }

    // assert(head->m_refs.load(std::memory_order_relaxed) == 2);

    // Good, reference count has been incremented (it wasn't at zero), which means we can read the
    // next and not worry about it changing between now and the time we do the CAS
    auto next = head->m_next.load(std::memory_order_relaxed);
    if (m_head.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)) {
      // Yay, got the node. This means it was on the list, which means shouldBeOnAtomicList must be false no
      // matter the refcount (because nobody else knows it's been taken off yet, it can't have been put back on).
      // assert((head->m_refs.load(std::memory_order_relaxed) & SHOULD_BE_ON_LIST) == 0);
      // Decrease refcount twice, once for our ref, and once for the list's ref
      head->m_refs.fetch_sub(2, std::memory_order_release);
      return head;
    }

    // OK, the head must have changed on us, but we still need to decrease the refcount we increased.
    // Note that we don't need to release any memory effects, but we do need to ensure that the reference
    // count decrement happens-after the CAS on the head.
    refs = prevHead->m_refs.fetch_sub(1, std::memory_order_acq_rel);
    if (refs == SHOULD_BE_ON_LIST + 1) {
      // assert(refs == SHOULD_BE_ON_LIST + 1);
      add_internal(prevHead);
    }
  }

  return nullptr;
}

void AtomicList::add(Node *node) noexcept {
  // We know that the should-be-on-freelist bit is 0 at this point, so it's safe to
  // set it using a fetch_add
  if (node->m_refs.fetch_add(SHOULD_BE_ON_LIST, std::memory_order_acq_rel) == 0) {
    // Oh look! We were the last ones referencing this node, and we know
    // we want to add it to the free list, so let's do it!
    // assert(node->m_refs.load(std::memory_order_relaxed) == SHOULD_BE_ON_LIST);
    add_internal(node);
  }
}

Node *AtomicList::get_head() const noexcept { return m_head.load(std::memory_order_relaxed); }
