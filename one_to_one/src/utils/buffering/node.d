module utils.buffering.node;
@nogc nothrow:

import core.atomic;
import core.stdc.stdlib : malloc, free;

class BufferNode {
    @disable this(this);

    // Allocate owned storage of `size` bytes.
    this(size_t size) {
        m_data  = cast(ubyte*) malloc(size);
        m_owned = true;
        m_limit = size;
        atomicStore!(MemoryOrder.raw)(m_written, cast(size_t) 0);
        atomicStore!(MemoryOrder.raw)(m_refs,    cast(size_t) 0);
    }

    // Wrap externally-owned data (no-op deleter).
    this(ubyte* data, size_t size) {
        m_data  = data;
        m_owned = false;
        m_limit = size;
        atomicStore!(MemoryOrder.raw)(m_written, size);
        atomicStore!(MemoryOrder.raw)(m_refs,    cast(size_t) 0);
    }

    // Construct from a ubyte slice (copies data in).
    this(const(ubyte)[] range) {
        m_data  = cast(ubyte*) malloc(range.length);
        m_owned = true;
        m_limit = range.length;
        atomicStore!(MemoryOrder.raw)(m_written, cast(size_t) 0);
        atomicStore!(MemoryOrder.raw)(m_refs,    cast(size_t) 0);
        foreach (b; range)
            push_back(b);
    }

    ~this() {
        if (m_owned && m_data !is null)
            free(m_data);
    }

    // Append a range of ubyte to this node.
    void append_range(const(ubyte)[] range) {
        foreach (b; range)
            push_back(b);
    }

    ref ubyte opIndex(size_t index) { return m_data[index]; }
    ref const(ubyte) opIndex(size_t index) const { return m_data[index]; }

    ubyte* begin() { return m_data; }
    ubyte* end()   { return m_data + m_limit; }
    const(ubyte)* begin() const { return m_data; }
    const(ubyte)* end()   const { return m_data + m_limit; }

    ubyte*  get_data()      const { return m_data; }
    size_t  get_limit()     const { return m_limit; }
    size_t  get_written()   const { return atomicLoad!(MemoryOrder.acq)(m_written); }
    size_t  get_remaining() const { return m_limit - get_written(); }

    void push_back(ubyte b) {
        auto idx = atomicFetchAdd!(MemoryOrder.acq_rel)(m_written, cast(size_t) 1);
        m_data[idx] = b;
    }
    // PORT-NOTE: acq_rel split — store is release
    void set_written(size_t size)    { atomicStore!(MemoryOrder.rel)(m_written, size); }
    void expand_written(size_t size) { atomicFetchAdd!(MemoryOrder.rel)(m_written, size); }

    void acquire() { atomicFetchAdd!(MemoryOrder.raw)(m_refs, cast(size_t) 1); }
    void release() {
        if (atomicFetchSub!(MemoryOrder.acq_rel)(m_refs, cast(size_t) 1) == 1) {
            import util.alloc : dispose;
            dispose(this);
        }
    }

  private:
    ubyte*        m_data;
    bool          m_owned;
    size_t        m_limit;
    shared size_t m_written;
    shared size_t m_refs;
}
