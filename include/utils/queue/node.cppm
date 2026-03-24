export module node;

import std;

export struct Node {
    std::atomic<Node *> m_next;
    std::atomic<std::size_t> m_refs;

    // Use the Initializer List for direct construction
    Node() : m_next(nullptr), m_refs(0) {}

    // WARNING: virtual ~Node() adds a VTABLE pointer to the struct.
    // This increases the size by 8 bytes and breaks cache-line alignment.
    // For maximum speed, remove 'virtual' and use a flat struct.
    ~Node() = default;
};
