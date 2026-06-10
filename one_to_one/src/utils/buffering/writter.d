module utils.buffering.writter;
@nogc nothrow:

import utils.buffering.node;
import utils.buffering.reader;
import util.alloc : make, dispose;

// ---------------------------------------------------------------------------
// BufferWriter — manages a BufferReader's backing storage by allocating new
// BufferNode pages on demand and tracking the "predicted" next allocation size.
// ---------------------------------------------------------------------------
class BufferWriter {
    @disable this(this);

    this(size_t min_size = 8UL * 1024UL, size_t max_size = 64 * 1024) {
        m_min_size     = min_size;
        m_max_size     = max_size;
        m_current_size = min_size;
        m_view         = make!BufferReader();
    }

    ~this() {
        if (m_view !is null)
            dispose(m_view);
    }

    NodeReader* acquire() {
        auto tail = m_view.get_tail();

        if (tail is null || tail.get_remaining() == 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto node = m_view.push_back(make!BufferNode(m_current_size));
            return node;
        }

        tail.acquire();
        return tail;
    }

    void push(BufferNode* node) { m_view.push_back(node); }

    void notify_read(NodeReader* node, size_t bytes_read) {
        if (node is null) return;

        node.expand_written(bytes_read);
        m_view.expand(bytes_read);
        node.release();

        if (bytes_read == m_current_size) {
            size_t doubled = m_current_size * 2;
            m_current_size = doubled < m_max_size ? doubled : m_max_size;
            if (m_current_size < m_min_size) m_current_size = m_min_size;
        } else {
            m_current_size = bytes_read < m_min_size ? m_min_size
                           : bytes_read > m_max_size ? m_max_size
                           : bytes_read;
        }
    }

    size_t       get_predicted_size() const { return m_current_size; }
    BufferReader get_view()                 { return m_view; }

  private:
    size_t       m_min_size;
    size_t       m_max_size;
    size_t       m_current_size;
    BufferReader m_view;
}
