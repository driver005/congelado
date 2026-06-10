module utils.buffering.view;
@nogc nothrow:

import utils.buffering.node;
import util.alloc : make, dispose;

// ---------------------------------------------------------------------------
// NodeView — non-owning view into a region of a BufferNode [start, start+length).
// Holds an acquire reference on the node so the data stays alive.
// ---------------------------------------------------------------------------
class NodeView {
    @disable this(this);

    this(BufferNode* node, size_t start, size_t length, NodeView* next = null) {
        m_node   = node;
        m_start  = start;
        m_length = length;
        m_next   = next;
        node.acquire();
    }

    ~this() { m_node.release(); }

    ref ubyte opIndex(size_t index) { return (*m_node)[m_start + index]; }

    void acquire()           { m_node.acquire(); }
    void release()           { m_node.release(); }
    void set_next(NodeView* next) { m_next = next; }

    NodeView*   get_next()   const { return m_next; }
    BufferNode* get_node()   const { return m_node; }
    ref const(size_t) get_length() const { return m_length; }
    ref size_t  get_length()       { return m_length; }
    ref const(size_t) get_start()  const { return m_start; }
    ref size_t  get_start()        { return m_start; }

  private:
    BufferNode* m_node;
    size_t      m_start;
    size_t      m_length;
    NodeView*   m_next;
}

// ---------------------------------------------------------------------------
// BufferView — intrusive singly-linked list of NodeView slices.
// Supports forward-range iteration over bytes.
// ---------------------------------------------------------------------------
class BufferView {
    @disable this(this);

    // -- Iterator ------------------------------------------------------------
    class Iterator {
        this() { m_node = null; m_offset = 0; }

        this(NodeView* node, size_t offset) {
            m_node   = node;
            m_offset = offset;
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
            if (m_node is null) return;

            if (++m_offset >= m_node.get_length()) {
                auto next = m_node.get_next();
                if (next !is null)
                    next.acquire();

                m_node.release();
                m_node   = next;
                m_offset = 0;
            }
        }

        void opOpAssign(string op : "+")(size_t till) {
            while (till > 0 && m_node !is null) {
                size_t remaining = m_node.get_length() - m_offset;

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

      private:
        NodeView* m_node;
        size_t    m_offset;
    }
    // -- End Iterator --------------------------------------------------------

    this() {
        m_head = null;
        m_tail = null;
        m_size = 0;
    }

    ~this() { release(); }

    Iterator begin() const { return make!Iterator(get_head(), cast(size_t) 0); }
    Iterator end()   const { return make!Iterator(); }

    size_t size()  const { return m_size; }
    bool   empty() const { return get_head() is null; }

    void push_back(NodeView* node_view) {
        node_view.acquire();
        m_size += node_view.get_length();

        if (m_tail !is null) {
            m_tail.set_next(node_view);
            m_tail = node_view;
        } else {
            m_head = node_view;
            m_tail = m_head;
        }
    }

    void push_back(BufferNode* node, size_t start, size_t length) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        push_back(make!NodeView(node, start, length));
    }

    void release() {
        NodeView* current = m_head;
        while (current !is null) {
            NodeView* next = current.get_next();
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            dispose(current);
            current = next;
        }
        m_head = null;
        m_tail = null;
        m_size = 0;
    }

    NodeView* get_head() const { return m_head; }
    NodeView* get_tail() const { return m_tail; }

  private:
    NodeView* m_head;
    NodeView* m_tail;
    size_t    m_size;
}
