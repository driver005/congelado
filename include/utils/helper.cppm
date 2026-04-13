module;

#if defined(_WIN32)
#include <malloc.h>
#endif

export module helper;

import std;

export template <typename T>
struct AlignedManager {
    static void *operator new(std::size_t size) {
        constexpr std::size_t alignment = alignof(T);
        constexpr bool needs_align = alignment > alignof(std::max_align_t);
        if constexpr (needs_align) {
            std::size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);

            void *ptr = nullptr;
#if defined(_WIN32)
            ptr = _aligned_malloc(aligned_size, alignment);
#else
            ptr = std::aligned_alloc(alignment, aligned_size);
#endif

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

    static void operator delete(void *ptr) noexcept {
#if defined(_WIN32)
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }

    static void *operator new(std::size_t, void *p) noexcept { return p; }
    static void operator delete(void *, void *) noexcept {}
};
