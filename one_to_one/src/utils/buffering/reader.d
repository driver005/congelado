module utils.buffering.reader;
@nogc nothrow:

import core.atomic;
import utils.buffering.node;
import utils.buffering.view;
import util.alloc : make, dispose;

// ---------------------------------------------------------------------------
// NodeReader — ref-counted wrapper around a BufferNode that also acts as a
// singly-linked list node (via m_next).
// ---------------------------------------------------------------------------
class NodeReader {
    @disable this(this);

    this(BufferNode* node, NodeReader* next = null) {
        m_node = node;
        atomicStore!(MemoryOrder.raw)(m_next, next);
        node.acquire();
    }

    ~this() { get_node().release(); }

    ref ubyte opIndex(size_t index) { return (*m_node)[index]; }

    void set_next(NodeReader* node) { atomicStore!(MemoryOrder.rel)(m_next, node); }
    void expand_written(size_t size) const { get_node().expand_written(size); }
    void acquire() const { get_node().acquire(); }
    void release() const { get_node().release(); }

    ubyte*      get_data()      const { return get_node().get_data(); }
    NodeReader* get_next()      const { return atomicLoad!(MemoryOrder.acq)(m_next); }
    BufferNode* get_node()      const { return m_node; }
    size_t      get_written()   const { return get_node().get_written(); }
    size_t      get_remaining() const { return get_node().get_remaining(); }
    size_t      get_limit()     const { return get_node().get_limit(); }

  private:
    BufferNode*           m_node;
    shared NodeReader*    m_next;
}

// ---------------------------------------------------------------------------
// BufferReader — lock-free linked list of NodeReader slices, with an
// atomic head/tail/offset/size.  Supports forward-range iteration.
// ---------------------------------------------------------------------------
class BufferReader {
    @disable this(this);

    // -- Iterator (forward range) ------------------------------------------
    class Iterator {
        this() { m_node = null; m_offset = 0; }

        this(NodeReader* node, size_t offset) {
            m_node   = node;
            m_offset = offset;
            if (m_node !is null)
                m_node.acquire();
        }

        this(Iterator other) {
            m_node   = other.m_node;
            m_offset = other.m_offset;
            if (m_node !is null)
                m_node.acquire();
        }

        ~this() {
            if (m_node !is null)
                m_node.release();
        }

        ref const(ubyte) front() const { return (*m_node)[m_offset]; }

        bool empty() const { return m_node is null; }

        void popFront() {
            if (m_node is null)
                return;

            if (++m_offset >= m_node.get_written()) {
                auto next = m_node.get_next();
                if (next !is null)
                    next.acquire();

                m_node.release();
                m_node   = next;
                m_offset = 0;
            }
        }

        // operator+= — advance by `till` bytes across node boundaries.
        void opOpAssign(string op : "+")(size_t till) {
            while (till > 0 && m_node !is null) {
                size_t remaining = m_node.get_written() - m_offset;

                if (till < remaining) {
                    m_offset += till;
                    till = 0;
                } else {
                    till -= remaining;
                    auto next = m_node.get_next();
                    if (next !is null)
                        next.acquire();

                    m_node.release();
                    m_node   = next;
                    m_offset = 0;
                }
            }
        }

        bool opEquals(const Iterator other) const {
            return m_node == other.m_node && m_offset == other.m_offset;
        }

        NodeReader* get_node()   const { return m_node; }
        size_t      get_offset() const { return m_offset; }

        size_t chunk_size() const {
            if (m_node is null) return 0;
            return m_node.get_written() - m_offset;
        }

      private:
        NodeReader* m_node;
        size_t      m_offset;
    }
    // -- End Iterator -------------------------------------------------------

    this() {
        atomicStore!(MemoryOrder.raw)(m_head,   cast(NodeReader*) null);
        atomicStore!(MemoryOrder.raw)(m_tail,   cast(NodeReader*) null);
        atomicStore!(MemoryOrder.raw)(m_offset, cast(size_t) 0);
        atomicStore!(MemoryOrder.raw)(m_size,   cast(size_t) 0);
    }

    ~this() { _release(); }

    Iterator begin() const {
        return make!Iterator(get_head(),
                             atomicLoad!(MemoryOrder.raw)(m_offset));
    }
    // end() — null iterator signals end-of-range
    Iterator end() const { return make!Iterator(); }

    size_t size()  const { return atomicLoad!(MemoryOrder.raw)(m_size); }
    bool   empty() const { return atomicLoad!(MemoryOrder.acq)(m_size) == 0; }

    // peek() — returns head node pointer with an extra acquire, or null.
    // PORT-NOTE: null = empty (replaces std::optional)
    NodeReader* peek() const {
        auto head = get_head();
        if (head is null) return null;
        head.acquire();
        return head;
    }

    // front() — first contiguous chunk in the reader.
    // Returns (null, 0) if empty.
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    struct FrontResult {
        // PORT-NOTE: value wrapper, exempt from classes-only rule
        const(ubyte)* ptr;
        size_t        len;
    }

    FrontResult front() const {
        if (empty()) return FrontResult(null, 0);
        auto it = begin();
        return FrontResult(&it.front(), it.chunk_size());
    }

    void consume(size_t bytes) {
        atomicFetchSub!(MemoryOrder.raw)(m_size, bytes);

        while (bytes > 0) {
            auto head = get_head();
            if (head is null) break;

            size_t offset    = atomicLoad!(MemoryOrder.raw)(m_offset);
            size_t available = head.get_written() - offset;

            if (bytes < available) {
                atomicStore!(MemoryOrder.raw)(m_offset, offset + bytes);
                break;
            }

            bytes -= available;
            auto next = head.get_next();
            atomicStore!(MemoryOrder.rel)(m_head, next);
            atomicStore!(MemoryOrder.raw)(m_offset, cast(size_t) 0);
            if (next is null)
                atomicStore!(MemoryOrder.rel)(m_tail, cast(NodeReader*) null);
            head.release();
        }
    }

    void expand(size_t bytes) { atomicFetchAdd!(MemoryOrder.rel)(m_size, bytes); }

    void push_back(NodeReader* node) {
        node.acquire();

        auto old = atomicExchange!(MemoryOrder.acq_rel)(m_tail, node);
        if (old !is null) {
            old.set_next(node);
        } else {
            atomicStore!(MemoryOrder.rel)(m_head, node);
        }

        expand(node.get_written());
    }

    NodeReader* push_back(BufferNode* node) {
        auto reader = make!NodeReader(node);
        push_back(reader);
        return reader;
    }

    void expand_view(BufferView view, size_t length) {
        while (length > 0) {
            auto head = get_head();
            if (head is null) break;

            size_t offset    = atomicLoad!(MemoryOrder.raw)(m_offset);
            size_t available = head.get_written() - offset;
            size_t to_take   = available < length ? available : length;

            view.push_back(head.get_node(), offset, to_take);

            if (to_take < available) {
                atomicStore!(MemoryOrder.raw)(m_offset, offset + to_take);
                length = 0;
            } else {
                length -= to_take;
                auto next = head.get_next();
                atomicStore!(MemoryOrder.rel)(m_head, next);
                atomicStore!(MemoryOrder.raw)(m_offset, cast(size_t) 0);
                if (next is null)
                    atomicStore!(MemoryOrder.rel)(m_tail, cast(NodeReader*) null);
                head.release();
            }
        }
    }

    NodeReader* get_head() const { return atomicLoad!(MemoryOrder.acq)(m_head); }
    NodeReader* get_tail() const { return atomicLoad!(MemoryOrder.acq)(m_tail); }

  private:
    void _release() {
        auto current = atomicLoad!(MemoryOrder.acq)(m_head);
        while (current !is null) {
            auto next = current.get_next();
            current.release();
            current = next;
        }
    }

    shared NodeReader* m_head;
    shared NodeReader* m_tail;
    shared size_t      m_offset;
    shared size_t      m_size;
}

// ---------------------------------------------------------------------------
// AdvanceReaderAdaptor — consumes `count` bytes from a BufferReader after
// a pipeline result is forwarded.  Mirrors C++ range_adaptor_closure.
// ---------------------------------------------------------------------------
class AdvanceReaderAdaptor {
    @disable this(this);

    this(BufferReader view, size_t count) {
        m_view  = view;
        m_count = count;
    }

    // opCall — pass result through and advance the reader.
    T opCall(T)(T result) {
        m_view.consume(m_count);
        return result;
    }

    BufferReader m_view;   // PORT-NOTE: null = empty (reference semantics via class)
    size_t       m_count;
}
