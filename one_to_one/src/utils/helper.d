module utils.helper;
@nogc nothrow:

import core.stdc.stdlib : malloc, free, aligned_alloc;

// AlignedManager: mixin providing aligned operator-new / delete semantics.
// In D, classes are always heap-allocated via GC or make!/dispose.
// This mixin template replicates the custom allocator logic for classes that
// require over-aligned storage (alignment > max_align_t, i.e. > 16 bytes).
//
// PORT-NOTE: C++ template class with overloaded operator new/delete.
// In D there is no operator-new overloading for classes; instead, callers
// that need aligned allocation must use util.alloc.make_aligned!T / dispose.
// This mixin documents the intent and provides aligned_alloc / free wrappers
// that the pool/arena layer can call directly.
mixin template AlignedManager() {
    // alloc: allocate aligned storage of `size` bytes with `alignment`.
    // Returns null on failure (no exceptions — @nogc nothrow).
    static void* alloc(size_t size, size_t alignment) {
        version (Windows) {
            // _aligned_malloc not exposed in core.stdc on all LDC versions;
            // fall back to malloc for Windows until util.alloc wraps it.
            return malloc(size);
        } else {
            // aligned_alloc requires size to be a multiple of alignment.
            size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
            return aligned_alloc(alignment, aligned_size);
        }
    }

    // dealloc: free memory returned by alloc().
    static void dealloc(void* ptr) {
        free(ptr);
    }

    // placement helpers — no-op in D (placement new is emplace in core.lifetime)
    static void* placement_new(size_t, void* p) { return p; }
    static void placement_delete(void*, void*) {}
}
