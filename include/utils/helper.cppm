module;

#ifdef _WIN32
#    include <malloc.h>
#endif

export module helper;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export template<typename T>
struct AlignedManager
{
    /**
     * @brief Allocates `size` bytes, routing through an alignment-aware allocator only when `T`
     * actually needs more alignment than `std::max_align_t` already guarantees — otherwise it's
     * just a plain `std::malloc`.
     * @warning Read this alongside operator delete() below — on Windows the two do NOT use
     * matching allocators when `needs_align` is false. This overload calls plain `std::malloc()`
     * in that branch, but operator delete() unconditionally calls `_aligned_free()` on Windows no
     * matter which branch allocated the memory. Mixing `malloc`/`_aligned_free` is undefined
     * behavior on the Windows CRT — that's a real bug sitting in this pairing, not vibes, only
     * dodged today because most instantiations probably do need the aligned path. Flagging hard
     * since this pass is comment-only and the logic isn't getting touched.
     * @param size how many bytes to allocate.
     * @throws std::bad_alloc if the underlying allocator returns null.
     * @return a pointer to the allocated (and possibly over-aligned) memory.
     */
    static void* operator new(std::size_t size)
    {
        constexpr std::size_t ALIGNMENT = alignof(T);
        constexpr bool NEEDS_ALIGNMENT = ALIGNMENT > alignof(std::max_align_t);
        if constexpr (NEEDS_ALIGNMENT) {
            // T needs more alignment than a plain malloc guarantees — round the size up to a
            // multiple of the alignment first, then route through the platform's aligned
            // allocator.
            std::size_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

            void* ptr = nullptr;
#ifdef _WIN32
            ptr = _aligned_malloc(aligned_size, ALIGNMENT);
#else
            ptr = std::aligned_alloc(
                ALIGNMENT, aligned_size
            ); // NOLINT(cppcoreguidelines-owning-memory) — would need gsl::owner<> annotation; no
               // GSL dependency in this codebase
#endif

            if (ptr == nullptr) {
                throw std::bad_alloc{};
            }

            return ptr;
        } else {
            // Default alignment is plenty — plain malloc does the job, no cap.
            void* ptr = std::malloc(
                size
            ); // NOLINT(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
               // — would need gsl::owner<> annotation; no GSL dependency in
               // this codebase, and this class *is* the low-level allocator
            if (ptr == nullptr) {
                throw std::bad_alloc{};
            }
            return ptr;
        }
    }

    /**
     * @brief Frees memory allocated by the operator new() above.
     * @warning On Windows this always calls `_aligned_free()`, even for allocations that came out
     * of the plain `std::malloc()` branch (when `needs_align` was false at allocation time). See
     * the @warning on operator new() — that's a genuine allocator mismatch bug on Windows, not
     * something to sleep on. POSIX is fine here since `std::free()` is spec'd to accept whatever
     * `std::aligned_alloc()` hands back too.
     * @param ptr the pointer to free.
     */
    static void operator delete(void* ptr) noexcept
    {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        std::free(ptr); // NOLINT(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc) —
                        // would need gsl::owner<> annotation; no GSL dependency in this codebase,
                        // and this class *is* the low-level allocator
#endif
    }

    /**
     * @brief Placement-new passthrough — just hands back `address` untouched, standard
     * placement-new semantics, bet.
     * @param address the address to "allocate" at.
     * @return `address`, unchanged.
     */
    static void* operator new([[maybe_unused]] std::size_t size, void* address) noexcept
    {
        return address;
    }

    /**
     * @brief Placement-delete counterpart — no-op, since placement new never actually owned
     * anything to free. Only ever invoked by the compiler if the matching placement-new's
     * constructor throws.
     */
    static void
    operator delete([[maybe_unused]] void* pointer, [[maybe_unused]] void* address) noexcept
    {
    }
};

#ifdef CONGELADO_TEST
namespace tests {
using namespace boost::ut;

struct alignas(64) OverAligned
{
    std::byte data[64];
};

suite<"AlignedManager"> aligned_manager_suite = [] {
    "over-aligned type allocates memory aligned to its own requirement"_test = [] {
        void* ptr = AlignedManager<OverAligned>::operator new(sizeof(OverAligned));
        expect(ptr != nullptr);
        expect((reinterpret_cast<std::uintptr_t>(ptr) % alignof(OverAligned)) == 0);
        AlignedManager<OverAligned>::operator delete(ptr);
    };
    "default-aligned type still allocates usable memory"_test = [] {
        void* ptr = AlignedManager<int>::operator new(sizeof(int));
        expect(ptr != nullptr);
        AlignedManager<int>::operator delete(ptr);
    };
    "placement new is a passthrough, placement delete is a no-op"_test = [] {
        alignas(int) std::byte storage[sizeof(int)];
        void* ptr = AlignedManager<int>::operator new(sizeof(int), static_cast<void*>(storage));
        expect(ptr == static_cast<void*>(storage));
        AlignedManager<int>::operator delete(ptr, static_cast<void*>(storage));
    };
};

} // namespace tests
#endif
