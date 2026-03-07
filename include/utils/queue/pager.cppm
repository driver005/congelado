module; // Start of Global Module Fragment

#include <climits>

export module pager;

import std;
import consts;
import page;
import atomic_list;
import consts;

export enum class AllocationMode { CanAlloc = 0, CannotAlloc = 1 };

/**
 * Circular comparison for unsigned integers.
 * Useful for handling sequence number wrap-around in lock-free queues.
 */
export template <typename T> inline bool circular_less_than(T a, T b) {
  static_assert(std::is_integral_v<T> && !std::is_signed_v<T>,
                "circular_less_than is intended for unsigned integer types only");

  constexpr T shift = (sizeof(T) * CHAR_BIT) - 1;
  return static_cast<T>(a - b) > (static_cast<T>(1) << shift);
}

export template <typename T> class Pager {
public:
  explicit Pager(AtomicList *recycle_list)
      : m_base(0), m_reader(0), m_writer(0), m_head{nullptr}, m_tail{nullptr}, m_recycle_list{recycle_list} {}

  // Disable copying for safety in concurrent contexts
  Pager(const Pager &) = delete;
  Pager &operator=(const Pager &) = delete;

  template <AllocationMode WishAlloc> bool enqueue(const T &item);
  template <AllocationMode WishAlloc> bool enqueue_bulk(const T *items, std::size_t count);
  template <AllocationMode WishAlloc> bool dequeue(T &item);
  template <AllocationMode WishAlloc> std::size_t dequeue_bulk(T *items, std::size_t max_count);

  [[nodiscard]] std::size_t size() const noexcept {
    return m_writer.load(std::memory_order_relaxed) - m_reader.load(std::memory_order_relaxed);
  }

private:
  template <AllocationMode WishAlloc> void remove_page();
  template <AllocationMode WishAlloc> bool add_new_page();
  bool recycle_page();
  bool allocate_page();

  std::atomic<std::size_t> m_base;
  std::atomic<std::size_t> m_reader;
  std::atomic<std::size_t> m_writer;
  std::atomic<std::size_t> m_reader_optimistic{0};
  std::atomic<std::size_t> m_reader_overcommit{0};

  Page<T> *m_head;
  Page<T> *m_tail;
  AtomicList *m_recycle_list;
};
