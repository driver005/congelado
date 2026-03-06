module;
// https://github.com/cameron314/concurrentqueue/concurrentqueue.h#L1450

#include <atomic>
#include <cassert>
#include <cstddef>

export module atomic_list;

import node;

static constexpr std::size_t REFS_MASK = ~static_cast<std::size_t>(0) >> 1;
static constexpr std::size_t SHOULD_BE_ON_LIST = ~REFS_MASK;

export class AtomicList {
public:
  AtomicList() : m_head(nullptr) {};
  [[nodiscard]] Node *try_get() noexcept;
  void add(Node *node) noexcept;
  Node *get_head() const noexcept;

private:
  std::atomic<Node *> m_head{nullptr};

  void add_internal(Node *node) noexcept;
};
