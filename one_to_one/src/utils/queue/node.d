module utils.queue.node;
@nogc nothrow:

import core.atomic;

struct Node {
    shared Node*   m_next;
    shared size_t  m_refs;

    // Use the Initializer List for direct construction
    this(int /*unused*/) {
        atomicStore!(MemoryOrder.raw)(m_next, cast(Node*) null);
        atomicStore!(MemoryOrder.raw)(m_refs, cast(size_t) 0);
    }

    // WARNING: virtual ~Node() adds a VTABLE pointer to the struct.
    // This increases the size by 8 bytes and breaks cache-line alignment.
    // For maximum speed, remove 'virtual' and use a flat struct.
    ~this() {}
}
