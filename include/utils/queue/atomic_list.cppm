// https://github.com/cameron314/concurrentqueue/concurrentqueue.h#L1450
export module atomic_list;

import std;
import node;
import consts;

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
