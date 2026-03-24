module;
#include <openssl/quic.h>
#include <openssl/ssl.h>
export module quic:connection;

import std;
import :types;

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

namespace quic {

export class Connection {
  public:
    using StreamDataFn = std::function<void(std::uint64_t stream_id, std::vector<std::byte> data, bool fin)>;

    // Takes ownership of a connection SSL* from SSL_accept_connection().
    explicit Connection(SSL *conn_ssl, StreamDataFn on_stream_data = {})
        : m_ssl(conn_ssl), m_on_stream_data(std::move(on_stream_data)) {}

    ~Connection() {
        for (auto &[id, ssl] : m_streams)
            ::SSL_free(ssl);
        if (m_ssl)
            ::SSL_free(m_ssl);
    }

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    Connection(Connection &&o) noexcept
        : m_ssl(std::exchange(o.m_ssl, nullptr)), m_on_stream_data(std::move(o.m_on_stream_data)),
          m_streams(std::move(o.m_streams)), m_state(o.m_state), m_handshake_done(o.m_handshake_done) {}

    // Drive the QUIC engine. Call whenever SSL_get_event_timeout fires,
    // or after new UDP data is available on the fd.
    void tick() {
        ::SSL_handle_events(m_ssl);

        if (!m_handshake_done) {
            const int rc = ::SSL_do_handshake(m_ssl);
            if (rc == 1) {
                m_handshake_done = true;
                m_state = ConnState::Connected;
                // Fire once — lets the application open server-initiated streams
                // (e.g. H3 control stream) before poll_streams runs.
                if (m_on_connected)
                    m_on_connected();
            } else {
                const int err = ::SSL_get_error(m_ssl, rc);
                if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
                    m_state = ConnState::Closed;
            }
        }

        if (m_state == ConnState::Connected)
            poll_streams();

        if (::SSL_get_shutdown(m_ssl) & SSL_RECEIVED_SHUTDOWN)
            m_state = ConnState::Closed;
    }

    // Open a persistent server-initiated stream stored in m_streams.
    // Returns its QUIC stream id, or UINT64_MAX on failure.
    // Used for long-lived streams (e.g. H3 control stream).
    std::uint64_t open_stream(bool unidirectional = false) {
        if (m_state != ConnState::Connected)
            return UINT64_MAX;
        SSL *s = ::SSL_new_stream(m_ssl, unidirectional ? SSL_STREAM_FLAG_UNI : 0);
        if (!s)
            return UINT64_MAX;
        const std::uint64_t id = ::SSL_get_stream_id(s);
        m_streams[id] = s;
        return id;
    }

    // Open a stream, write data, and optionally send FIN.
    // If fin=false the stream is kept alive in m_streams under its id.
    bool send_stream(std::span<const std::byte> data, bool fin = true, bool unidirectional = false) {
        if (m_state != ConnState::Connected)
            return false;
        SSL *s = ::SSL_new_stream(m_ssl, unidirectional ? SSL_STREAM_FLAG_UNI : 0);
        if (!s)
            return false;
        std::size_t written = 0;
        ::SSL_write_ex(s, data.data(), data.size(), &written);
        if (fin) {
            ::SSL_shutdown(s);
            ::SSL_free(s);
        } else {
            m_streams[::SSL_get_stream_id(s)] = s;
        }
        return written == data.size();
    }

    // Write to a specific already-open stream by id.
    bool write_stream(std::uint64_t stream_id, std::span<const std::byte> data, bool fin = false) {
        auto it = m_streams.find(stream_id);
        if (it == m_streams.end())
            return false;
        std::size_t written = 0;
        ::SSL_write_ex(it->second, data.data(), data.size(), &written);
        if (fin) {
            ::SSL_shutdown(it->second);
            ::SSL_free(it->second);
            m_streams.erase(it);
        }
        return written == data.size();
    }

    void on_connected(std::function<void()> fn) { m_on_connected = std::move(fn); }
    void on_stream(StreamDataFn fn) { m_on_stream_data = std::move(fn); }

    // Needed by quic::Server to schedule SSL_get_event_timeout.
    [[nodiscard]] SSL *native() const noexcept { return m_ssl; }
    [[nodiscard]] ConnState state() const noexcept { return m_state; }
    [[nodiscard]] bool connected() const noexcept { return m_state == ConnState::Connected; }

  private:
    void poll_streams() {
        // Drain the accept queue — non-blocking.
        SSL *incoming = nullptr;
        while ((incoming = ::SSL_accept_stream(m_ssl, SSL_ACCEPT_STREAM_NO_BLOCK)) != nullptr)
            m_streams[::SSL_get_stream_id(incoming)] = incoming;

        for (auto it = m_streams.begin(); it != m_streams.end();) {
            SSL *s = it->second;
            const int rs = ::SSL_get_stream_read_state(s);

            // Skip streams with nothing to read yet.
            if (rs == SSL_STREAM_STATE_NONE) {
                ++it;
                continue;
            }

            // Discard reset streams immediately.
            if (rs == SSL_STREAM_STATE_RESET_REMOTE || rs == SSL_STREAM_STATE_RESET_LOCAL ||
                rs == SSL_STREAM_STATE_CONN_CLOSED) {
                ::SSL_free(s);
                it = m_streams.erase(it);
                continue;
            }

            // SSL_STREAM_STATE_OK or SSL_STREAM_STATE_FINISHED — read all data.
            std::vector<std::byte> buf(65536);
            std::size_t total = 0;
            for (;;) {
                std::size_t n = 0;
                if (::SSL_read_ex(s, buf.data() + total, buf.size() - total, &n) != 1)
                    break;
                total += n;
                if (total == buf.size())
                    buf.resize(buf.size() * 2);
            }

            const bool fin =
                (rs == SSL_STREAM_STATE_FINISHED) || (::SSL_get_stream_read_state(s) == SSL_STREAM_STATE_FINISHED);

            if (total > 0) {
                buf.resize(total);
                if (m_on_stream_data)
                    m_on_stream_data(it->first, std::move(buf), fin);
            }

            if (fin) {
                ::SSL_free(s);
                it = m_streams.erase(it);
            } else {
                ++it;
            }
        }
    }

    SSL *m_ssl{nullptr};
    std::function<void()> m_on_connected;
    StreamDataFn m_on_stream_data;
    std::unordered_map<std::uint64_t, SSL *> m_streams;
    ConnState m_state{ConnState::Handshaking};
    bool m_handshake_done{false};
};

} // namespace quic
