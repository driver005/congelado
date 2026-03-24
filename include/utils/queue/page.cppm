export module page;

import std;
import node;
import consts;
import helper;

export template <typename T>
class Page : Node, public AlignedManager<Page<T>> {
  public:
    Page() : dynamicly_allocated(false), dequeued(0) {}

    bool is_empty() const {
        if (dequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
            std::atomic_thread_fence(std::memory_order_acquire);
            return true;
        }
        return false;
    }

    bool set_empty() {
        auto prev = dequeued.fetch_add(1, std::memory_order_acq_rel);
        return prev == BLOCK_SIZE - 1;
    }

    bool set_many_empty(std::size_t count) {
        auto prev = dequeued.fetch_add(count, std::memory_order_acq_rel);
        return prev + count == BLOCK_SIZE;
    }

    void set_full_empty() { dequeued.store(BLOCK_SIZE, std::memory_order_relaxed); }

    void reset_empty() { dequeued.store(0, std::memory_order_relaxed); }

    T *operator[](std::size_t idx) noexcept {
        return static_cast<T *>(static_cast<void *>(elements)) +
               static_cast<std::size_t>(idx & static_cast<std::size_t>(BLOCK_SIZE - 1));
    };

    T const *operator[](std::size_t idx) const noexcept {
        return static_cast<T const *>(static_cast<void const *>(elements)) +
               static_cast<std::size_t>(idx & static_cast<std::size_t>(BLOCK_SIZE - 1));
    };

  private:
    alignas(T) char elements[sizeof(T) * BLOCK_SIZE];
    bool dynamicly_allocated;
    std::atomic<std::size_t> dequeued;
};
