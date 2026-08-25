export module node;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export struct Node {
    std::atomic<Node *> m_next;
    std::atomic<std::size_t> m_refs;

    // Use the Initializer List for direct construction
    /**
     * @brief Builds a node with no next link and a zeroed refcount, ready to get slotted into an
     * `AtomicList`.
     */
    Node() : m_next(nullptr), m_refs(0) {}

    // Nodes live inside a lock-free freelist/list and are linked via `m_next`, which other
    // threads may be racing to read through `std::atomic`. Copying or moving a `Node` would
    // duplicate or invalidate that linkage out from under concurrent readers, and `std::atomic`
    // itself isn't copyable or movable anyway — so all four are explicitly deleted rather than
    // left to an implicit (and misleading) default.
    Node(Node const &) = delete;
    Node &operator=(Node const &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node &&) = delete;

    // WARNING: virtual ~Node() adds a VTABLE pointer to the struct.
    // This increases the size by 8 bytes and breaks cache-line alignment.
    // For maximum speed, remove 'virtual' and use a flat struct.
    /**
     * @brief Default dtor, deliberately non-virtual — see the WARNING comment right above, that's
     * not decoration, going virtual here tanks the cache-line-friendly layout this whole
     * lock-free freelist motion depends on.
     */
    ~Node() = default;
};

#ifdef CONGELADO_TEST
namespace {
using namespace boost::ut;

suite<"Node"> node_suite = [] {
    "default-constructed node has no next link and a zeroed refcount"_test = [] {
        Node node;
        expect(node.m_next.load() == nullptr);
        expect(node.m_refs.load() == 0);
    };
};

} // namespace
#endif
