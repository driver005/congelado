module io.codec.quic.connection;
@nogc nothrow:

import io.codec.quic.types;
import modules.openssl;

// Per-connection QUIC wrapper around an OpenSSL connection SSL*.
//
// A Connection SSL* is obtained from SSL_accept_connection() on a listener.
// OpenSSL owns: QUIC packet I/O, handshake, ACK, loss recovery, flow control,
// stream multiplexing. We drive it with SSL_handle_events() and read/write
// streams via SSL_accept_stream / SSL_read_ex / SSL_write_ex.
//
// Stream read state machine (SSL_get_stream_read_state):
//   SSL_STREAM_STATE_NONE      — no data yet, don't call SSL_read_ex
//   SSL_STREAM_STATE_OK        — data available
//   SSL_STREAM_STATE_FINISHED  — FIN received, drain remaining data then free
//   SSL_STREAM_STATE_RESET_*   — peer reset, discard and free
//   SSL_STREAM_STATE_CONN_CLOSED — connection gone

// PORT-NOTE: C++ std::function<void(uint64_t, vector<byte>, bool)> StreamDataFn
// → D fn+ctx pair to stay @nogc.
struct StreamDataFn {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    void function(void* ctx, ulong stream_id, const(ubyte)[] data, bool fin) @nogc nothrow fn;
    void* ctx;
}

// PORT-NOTE: std::function<void()> → fn+ctx pair
struct ConnectedFn {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    void function(void* ctx) @nogc nothrow fn;
    void* ctx;
}

// PORT-NOTE: std::unordered_map<uint64_t, SSL*> m_streams → D dynamic array
// of stream entries (improvement: replace with SwissHashMap in Run 3).
private struct StreamEntry {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    ulong stream_id;
    SSL*  ssl;
}

// PORT-NOTE: C++ class Connection → D class Connection (RAII, has behavior).
class Connection {
  public:
    // Takes ownership of a connection SSL* from SSL_accept_connection().
    this(SSL* conn_ssl, StreamDataFn on_stream_data = StreamDataFn.init) {
        m_ssl            = conn_ssl;
        m_on_stream_data = on_stream_data;
        m_state          = ConnState.Handshaking;
        m_handshake_done = false;
    }

    ~this() {
        foreach (ref entry; m_streams) {
            if (entry.ssl !is null)
                SSL_free(entry.ssl);
        }
        if (m_ssl !is null)
            SSL_free(m_ssl);
    }

    @disable this(this); // non-copyable

    // Drive the QUIC engine. Call whenever SSL_get_event_timeout fires,
    // or after new UDP data is available on the fd.
    void tick() {
        SSL_handle_events(m_ssl);

        if (!m_handshake_done) {
            const int rc = SSL_do_handshake(m_ssl);
            if (rc == 1) {
                m_handshake_done = true;
                m_state = ConnState.Connected;
                // Fire once — lets the application open server-initiated streams
                // (e.g. H3 control stream) before poll_streams runs.
                if (m_on_connected.fn !is null)
                    m_on_connected.fn(m_on_connected.ctx);
            } else {
                const int err = SSL_get_error(m_ssl, rc);
                if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
                    m_state = ConnState.Closed;
            }
        }

        if (m_state == ConnState.Connected)
            poll_streams();

        if (SSL_get_shutdown(m_ssl) & SSL_RECEIVED_SHUTDOWN)
            m_state = ConnState.Closed;
    }

    // Open a persistent server-initiated stream stored in m_streams.
    // Returns its QUIC stream id, or ulong.max on failure.
    // Used for long-lived streams (e.g. H3 control stream).
    ulong open_stream(bool unidirectional = false) {
        if (m_state != ConnState.Connected) return ulong.max;
        SSL* s = SSL_new_stream(m_ssl, unidirectional ? SSL_STREAM_FLAG_UNI : 0);
        if (!s) return ulong.max;
        const ulong id = SSL_get_stream_id(s);
        m_streams ~= StreamEntry(id, s);
        return id;
    }

    // Open a stream, write data, and optionally send FIN.
    // If fin=false the stream is kept alive in m_streams under its id.
    bool send_stream(const(ubyte)[] data, bool fin = true, bool unidirectional = false) {
        if (m_state != ConnState.Connected) return false;
        SSL* s = SSL_new_stream(m_ssl, unidirectional ? SSL_STREAM_FLAG_UNI : 0);
        if (!s) return false;
        size_t written = 0;
        SSL_write_ex(s, data.ptr, data.length, &written);
        if (fin) {
            SSL_shutdown(s);
            SSL_free(s);
        } else {
            m_streams ~= StreamEntry(SSL_get_stream_id(s), s);
        }
        return written == data.length;
    }

    // Write to a specific already-open stream by id.
    bool write_stream(ulong stream_id, const(ubyte)[] data, bool fin = false) {
        foreach (i, ref entry; m_streams) {
            if (entry.stream_id != stream_id) continue;
            size_t written = 0;
            SSL_write_ex(entry.ssl, data.ptr, data.length, &written);
            if (fin) {
                SSL_shutdown(entry.ssl);
                SSL_free(entry.ssl);
                // remove from m_streams
                m_streams[i] = m_streams[$ - 1];
                m_streams = m_streams[0 .. $ - 1];
            }
            return written == data.length;
        }
        return false;
    }

    void on_connected(ConnectedFn fn) { m_on_connected = fn; }
    void on_stream(StreamDataFn fn)   { m_on_stream_data = fn; }

    // Needed by quic::Server to schedule SSL_get_event_timeout.
    SSL*      native() const pure { return m_ssl; }
    ConnState state()  const pure { return m_state; }
    bool connected()   const pure { return m_state == ConnState.Connected; }

  private:
    void poll_streams() {
        // Drain the accept queue — non-blocking.
        SSL* incoming = null;
        while ((incoming = SSL_accept_stream(m_ssl, SSL_ACCEPT_STREAM_NO_BLOCK)) !is null)
            m_streams ~= StreamEntry(SSL_get_stream_id(incoming), incoming);

        size_t write_idx = 0;
        for (size_t i = 0; i < m_streams.length; ++i) {
            SSL* s = m_streams[i].ssl;
            const int rs = SSL_get_stream_read_state(s);

            // Skip streams with nothing to read yet.
            if (rs == SSL_STREAM_STATE_NONE) {
                m_streams[write_idx++] = m_streams[i];
                continue;
            }

            // Discard reset streams immediately.
            if (rs == SSL_STREAM_STATE_RESET_REMOTE || rs == SSL_STREAM_STATE_RESET_LOCAL ||
                rs == SSL_STREAM_STATE_CONN_CLOSED) {
                SSL_free(s);
                continue;
            }

            // SSL_STREAM_STATE_OK or SSL_STREAM_STATE_FINISHED — read all data.
            // PORT-NOTE: C++ vector<byte> buf(65536) → stack buffer + temp GC alloc note
            // TODO: replace with @nogc ring-buffer in improvement pass
            ubyte[65536] buf;
            size_t total = 0;
            for (;;) {
                size_t n = 0;
                if (SSL_read_ex(s, buf.ptr + total, buf.sizeof - total, &n) != 1)
                    break;
                total += n;
                if (total == buf.sizeof) break; // prevent overrun
            }

            const bool fin =
                (rs == SSL_STREAM_STATE_FINISHED) ||
                (SSL_get_stream_read_state(s) == SSL_STREAM_STATE_FINISHED);

            if (total > 0 && m_on_stream_data.fn !is null)
                m_on_stream_data.fn(m_on_stream_data.ctx, m_streams[i].stream_id, buf[0 .. total], fin);

            if (fin) {
                SSL_free(s);
                continue;
            }
            m_streams[write_idx++] = m_streams[i];
        }
        m_streams = m_streams[0 .. write_idx];
    }

    SSL*           m_ssl;
    ConnectedFn    m_on_connected;
    StreamDataFn   m_on_stream_data;
    StreamEntry[]  m_streams;
    ConnState      m_state;
    bool           m_handshake_done;
}
