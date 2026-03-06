#include <cstddef>
#include <cstdlib>
#include <new>

template <typename T> struct AlignedManager {
  static void *operator new(std::size_t size) {
    constexpr std::size_t alignment = alignof(T);
    constexpr bool needs_align = alignment > alignof(std::max_align_t);
    if constexpr (needs_align) {
      std::size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
      void *ptr = std::aligned_alloc(alignment, aligned_size);
      if (!ptr)
        throw std::bad_alloc{};
      return ptr;
    } else {
      void *ptr = std::malloc(size);
      if (!ptr)
        throw std::bad_alloc{};
      return ptr;
    }
  }

  static void operator delete(void *ptr) noexcept { std::free(ptr); }
};
