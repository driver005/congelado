export module page;

import std;
import node;
import consts;

export template <typename T> class Page : Node {
public:
  Page() : dynamicly_allocated(false), dequeued(0) {}

  bool is_empty() const;
  bool set_empty();
  bool set_many_empty(std::size_t count);
  void set_full_empty();
  void reset_empty();
  T *operator[](std::size_t idx) noexcept;
  T const *operator[](std::size_t idx) const noexcept;
  std::size_t size_approx();

private:
  alignas(T) char elements[sizeof(T) * BLOCK_SIZE];
  bool dynamicly_allocated;
  std::atomic<std::size_t> dequeued;
};
