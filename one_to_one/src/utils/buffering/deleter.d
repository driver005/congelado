module utils.buffering.deleter;
@nogc nothrow:

import util.alloc : make, dispose;

// ---------------------------------------------------------------------------
// Deleter — ref-counted type-erased cleanup callback.
// Mirrors C++ utils::buffering::Deleter exactly.
// ---------------------------------------------------------------------------
class Deleter {
    @disable this(this);

    // Internal — abstract base for the concrete typed action holder.
    static class Internal {
        int m_ref_count = 1;
        abstract void destroy();
    }

    // ConcreteInternal — holds the actual callable via a function pointer
    // and a void* context pointer (no std::function, @nogc).
    // PORT-NOTE: C++ used std::invocable concept; D version uses a raw
    // function pointer + context pair to stay @nogc nothrow.
    static class ConcreteInternal : Internal {
        void* m_ctx;
        void function(void*) m_fn;

        this(void* ctx, void function(void*) fn) {
            m_ctx = ctx;
            m_fn  = fn;
        }

        override void destroy() { m_fn(m_ctx); }
    }

    // Default constructor — empty deleter.
    this() { m_internal = null; }

    // Construct with a context pointer and a function pointer.
    this(void* ctx, void function(void*) fn) {
        m_internal = make!ConcreteInternal(ctx, fn);
    }

    // Copy constructor — shares ownership.
    this(Deleter other) {
        m_internal = other.m_internal;
        if (m_internal !is null)
            ++m_internal.m_ref_count;
    }

    ~this() { release(); }

    void release() {
        if (m_internal !is null && --m_internal.m_ref_count == 0) {
            m_internal.destroy();
            dispose(m_internal);
            m_internal = null;
        }
    }

    bool empty()     const { return m_internal is null; }
    int  use_count() const { return (m_internal !is null) ? m_internal.m_ref_count : 0; }

  private:
    Internal* m_internal; // PORT-NOTE: raw pointer, manual ref-count
}
