module;
#include <openssl/quic.h>
#include <openssl/ssl.h>
export module io_quic:connection;

import std;
import :types;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
    /**
     * @brief Wraps a fresh connection SSL* and takes ownership of it, bet.
     * @warning `conn_ssl` must come from `SSL_accept_connection()` — this class calls
     * `SSL_free` on it in the destructor, so double-free or a dangling SSL* is on the caller if
     * that contract's broken.
     * @param conn_ssl connection SSL* from `SSL_accept_connection()`; ownership transfers here.
     * @param on_stream_data callback fired with each stream's data as it arrives; optional, can
     * also be wired later via on_stream().
     */
    explicit Connection(SSL *conn_ssl, StreamDataFn on_stream_data = {})
        : m_ssl(conn_ssl), m_on_stream_data(std::move(on_stream_data)) {}

    /**
     * @brief Frees every open stream SSL* still tracked in `m_streams`, then the connection SSL*
     * itself — full teardown, no leaks.
     */
    ~Connection() {
        // Free every tracked stream first, then the connection itself — streams don't outlive
        // their parent connection SSL*.
        for (auto &[id, ssl] : m_streams) {
            ::SSL_free(ssl);
        }
        if (m_ssl != nullptr) {
            ::SSL_free(m_ssl);
        }
    }

    /** @brief Copying's a straight L here — SSL* ownership can't be shared, so it's deleted. */
    Connection(const Connection &) = delete;
    /** @brief Same deal as the copy ctor — SSL* ownership can't be shared, deleted. */
    Connection &operator=(const Connection &) = delete;

    /**
     * @brief Steals the SSL* and all stream/state from `other`, leaving it null so its destructor
     * is a no-op.
     * @param other the connection being moved from; left in a null/empty state after this runs.
     */
    Connection(Connection &&other) noexcept
        : m_ssl(std::exchange(other.m_ssl, nullptr)), m_on_stream_data(std::move(other.m_on_stream_data)),
          m_streams(std::move(other.m_streams)), m_state(other.m_state),
          m_handshake_done(other.m_handshake_done) {}

    /**
     * @brief Steals the SSL* and all stream/state from `other`, freeing whatever this connection
     * already held first, then leaves `other` null so its destructor is a no-op.
     * @param other the connection being moved from; left in a null/empty state after this runs.
     * @return *this.
     */
    Connection &operator=(Connection &&other) noexcept {
        if (this != &other) {
            for (auto &[id, ssl] : m_streams) {
                ::SSL_free(ssl);
            }
            if (m_ssl != nullptr) {
                ::SSL_free(m_ssl);
            }
            m_ssl = std::exchange(other.m_ssl, nullptr);
            m_on_stream_data = std::move(other.m_on_stream_data);
            m_streams = std::move(other.m_streams);
            m_state = other.m_state;
            m_handshake_done = other.m_handshake_done;
        }
        return *this;
    }

    // Drive the QUIC engine. Call whenever SSL_get_event_timeout fires,
    // or after new UDP data is available on the fd.
    /**
     * @brief Pumps the QUIC engine one tick — drains OpenSSL's internal events, drives the
     * handshake to completion if it isn't done yet, then polls streams once connected.
     * @note Call this whenever `SSL_get_event_timeout` fires, or right after new UDP data lands
     * on the fd. Skipping ticks stalls the whole connection — no events, no handshake progress, no
     * stream reads.
     * @warning Handshake failure (any `SSL_get_error` other than WANT_READ/WANT_WRITE) flips
     * state straight to Closed with no retry — that's terminal for this connection, not a
     * transient hiccup.
     */
    void tick() {
        // Always drain OpenSSL's internal event queue first, handshake or not.
        ::SSL_handle_events(m_ssl);

        // Still shaking hands — push it forward one step.
        if (!m_handshake_done) {
            const int RC = ::SSL_do_handshake(m_ssl);
            if (RC == 1) {
                m_handshake_done = true;
                m_state = ConnState::Connected;
                // Fire once — lets the application open server-initiated streams
                // (e.g. H3 control stream) before poll_streams runs.
                if (m_on_connected) {
                    m_on_connected();
                }
            } else {
                // Anything other than WANT_READ/WANT_WRITE is a hard failure — no retry, straight
                // to Closed.
                const int ERR = ::SSL_get_error(m_ssl, RC);
                if (ERR != SSL_ERROR_WANT_READ && ERR != SSL_ERROR_WANT_WRITE) {
                    m_state = ConnState::Closed;
                }
            }
        }

        // Once connected, pull whatever's available off every open stream.
        if (m_state == ConnState::Connected) {
            poll_streams();
        }

        // Peer signaled shutdown — reflect that in our own state.
        if ((::SSL_get_shutdown(m_ssl) & SSL_RECEIVED_SHUTDOWN) != 0) {
            m_state = ConnState::Closed;
        }
    }

    // Open a persistent server-initiated stream stored in m_streams.
    // Returns its QUIC stream id, or UINT64_MAX on failure.
    // Used for long-lived streams (e.g. H3 control stream).
    /**
     * @brief Opens a persistent server-initiated stream and stashes it in `m_streams` so it
     * stays alive across ticks — this is the motion for long-lived streams like the H3 control
     * stream.
     * @param unidirectional true for a uni stream, false (default) for bidi.
     * @return the new stream's QUIC stream id, or `UINT64_MAX` if the connection isn't Connected
     * yet or `SSL_new_stream` came back null.
     */
    std::uint64_t open_stream(bool unidirectional = false) {
        // Can't open streams before the handshake's landed.
        if (m_state != ConnState::Connected) {
            return UINT64_MAX;
        }
        SSL *stream_ssl = ::SSL_new_stream(m_ssl, unidirectional ? SSL_STREAM_FLAG_UNI : 0);
        if (stream_ssl == nullptr) {
            return UINT64_MAX;
        }
        // Stash it under its stream id so it stays alive across ticks.
        const std::uint64_t ID = ::SSL_get_stream_id(stream_ssl);
        m_streams[ID] = stream_ssl;
        return ID;
    }

    // Open a stream, write data, and optionally send FIN.
    // If fin=false the stream is kept alive in m_streams under its id.
    /**
     * @brief Opens a fresh stream, writes `data` to it in one shot, and (by default) sends FIN
     * and frees it — a one-and-done send.
     * @param data the bytes to write to the new stream.
     * @param fin true (default) closes the stream right after writing; false keeps it alive in
     * `m_streams` under its id for further writes via write_stream().
     * @param unidirectional true for a uni stream, false (default) for bidi.
     * @return true if the connection was Connected, stream creation succeeded, and every byte of
     * `data` got written; false otherwise. No partial-write recovery — check the return, don't
     * assume it landed.
     */
    bool send_stream(std::span<const std::byte> data, bool fin = true, bool unidirectional = false) {
        // Same connected-state and stream-creation guards as open_stream().
        if (m_state != ConnState::Connected) {
            return false;
        }
        SSL *stream_ssl = ::SSL_new_stream(m_ssl, unidirectional ? SSL_STREAM_FLAG_UNI : 0);
        if (stream_ssl == nullptr) {
            return false;
        }

        std::size_t written = 0;
        ::SSL_write_ex(stream_ssl, data.data(), data.size(), &written);

        // Default motion is FIN-and-free; opting out keeps it alive for further write_stream()
        // calls under its stream id.
        if (fin) {
            ::SSL_shutdown(stream_ssl);
            ::SSL_free(stream_ssl);
        } else {
            m_streams[::SSL_get_stream_id(stream_ssl)] = stream_ssl;
        }
        return written == data.size();
    }

    // Write to a specific already-open stream by id.
    /**
     * @brief Writes to an already-open stream by id, optionally FIN-ing and freeing it after.
     * @param stream_id id of a stream already tracked in `m_streams` (from open_stream() or a
     * non-FIN send_stream()) — unknown ids are a straight L, no throw, just false back.
     * @param data the bytes to write.
     * @param fin false (default) leaves the stream open for more writes; true shuts it down and
     * erases it from `m_streams` after this write.
     * @return true if `stream_id` was found and every byte of `data` got written; false
     * otherwise.
     */
    bool write_stream(std::uint64_t stream_id, std::span<const std::byte> data, bool fin = false) {
        // Unknown id is a plain L, no throw — just report failure.
        auto it = m_streams.find(stream_id);
        if (it == m_streams.end()) {
            return false;
        }

        std::size_t written = 0;
        ::SSL_write_ex(it->second, data.data(), data.size(), &written);

        // Caller wants this write to be the last one — shut it down and drop our tracking.
        if (fin) {
            ::SSL_shutdown(it->second);
            ::SSL_free(it->second);
            m_streams.erase(it);
        }
        return written == data.size();
    }

    /**
     * @brief Wires up the callback fired once, right when the handshake completes — the hook
     * point for opening server-initiated streams before any peer data shows up.
     * @param callback the callback to run on connect; replaces whatever was set before, no motion if
     * left default-constructed.
     */
    void on_connected(std::function<void()> callback) { m_on_connected = std::move(callback); }
    /**
     * @brief Wires up the callback fired per-stream as data arrives — same slot as the ctor
     * param, this just lets you set/replace it after construction.
     * @param callback the callback to run for each chunk of stream data.
     */
    void on_stream(StreamDataFn callback) { m_on_stream_data = std::move(callback); }

    // Needed by quic::Server to schedule SSL_get_event_timeout.
    /**
     * @brief Gets the raw underlying SSL* — needed by quic::Server to schedule
     * `SSL_get_event_timeout`.
     * @warning This hands out the raw pointer with zero lifetime guarantees beyond "as long as
     * this Connection lives." Don't stash it past that, don't free it yourself.
     * @return the connection's native SSL* handle.
     */
    [[nodiscard]] SSL *native() const noexcept { return m_ssl; }
    /**
     * @brief Gets where this connection is at in the handshake/closing lifecycle.
     * @return the current ConnState.
     */
    [[nodiscard]] ConnState state() const noexcept { return m_state; }
    /**
     * @brief Checks if the connection made it through the handshake and is live.
     * @return true if state is Connected.
     */
    [[nodiscard]] bool connected() const noexcept { return m_state == ConnState::Connected; }

  private:
    /**
     * @brief Drains newly-accepted streams into `m_streams`, then walks every tracked stream and
     * reads whatever's available, firing `m_on_stream_data` and cleaning up finished/reset
     * streams as it goes. This is the whole read side of the connection, no cap.
     * @note Read state gets checked per-stream via `SSL_get_stream_read_state` — NONE gets
     * skipped, RESET_* and CONN_CLOSED get discarded immediately, OK/FINISHED get drained then
     * the stream's freed and erased on FIN.
     * @warning The read loop grows its scratch buffer by doubling until a read returns short —
     * a stream that keeps producing exactly-buffer-sized chunks could, in theory, keep growing
     * that allocation. Not bounded here, just flagging the shape of it.
     */
    void poll_streams() {
        // Drain the accept queue — non-blocking.
        SSL *incoming = nullptr;
        while ((incoming = ::SSL_accept_stream(m_ssl, SSL_ACCEPT_STREAM_NO_BLOCK)) != nullptr) {
            m_streams[::SSL_get_stream_id(incoming)] = incoming;
        }

        for (auto it = m_streams.begin(); it != m_streams.end();) {
            SSL *stream_ssl = it->second;
            const int RS = ::SSL_get_stream_read_state(stream_ssl);

            // Skip streams with nothing to read yet.
            if (RS == SSL_STREAM_STATE_NONE) {
                ++it;
                continue;
            }

            // Discard reset streams immediately.
            if (RS == SSL_STREAM_STATE_RESET_REMOTE || RS == SSL_STREAM_STATE_RESET_LOCAL ||
                RS == SSL_STREAM_STATE_CONN_CLOSED) {
                ::SSL_free(stream_ssl);
                it = m_streams.erase(it);
                continue;
            }

            // SSL_STREAM_STATE_OK or SSL_STREAM_STATE_FINISHED — read all data.
            std::vector<std::byte> buf(65536);
            std::size_t total = 0;
            for (;;) {
                std::size_t read_count = 0;
                if (::SSL_read_ex(stream_ssl, buf.data() + total, buf.size() - total, &read_count) != 1) {
                    break;
                }
                total += read_count;
                if (total == buf.size()) {
                    buf.resize(buf.size() * 2);
                }
            }

            // Re-check read state after the drain loop too — FIN can show up mid-read, not just
            // as the state we walked in with.
            const bool FIN = (RS == SSL_STREAM_STATE_FINISHED) ||
                              (::SSL_get_stream_read_state(stream_ssl) == SSL_STREAM_STATE_FINISHED);

            // Only fire the callback if something actually got read.
            if (total > 0) {
                buf.resize(total);
                if (m_on_stream_data) {
                    m_on_stream_data(it->first, std::move(buf), FIN);
                }
            }

            // FIN means this stream's done — free it and drop it from tracking; otherwise move on
            // to the next stream, this one stays open.
            if (FIN) {
                ::SSL_free(stream_ssl);
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

// Connection's contract requires a real SSL* from SSL_accept_connection() — every method that
// actually touches m_ssl (tick(), poll_streams(), a stream open/write that gets past the
// Connected-state guard) needs a live QUIC handshake, not reproducible here. What IS safe to
// exercise: the state-machine surface that early-returns before ever touching m_ssl when the
// connection isn't Connected yet — true for a freshly-constructed Connection regardless of what
// SSL* it wraps, and its destructor's SSL_free(nullptr) is a documented no-op, so a null SSL*
// is safe to construct with as long as tick()/poll_streams() are never called on it.
#ifdef CONGELADO_TEST
namespace quic::tests {
using namespace boost::ut;

suite<"Connection"> connection_suite = [] {
    "starts Handshaking, not connected"_test = [] {
        Connection conn{nullptr};
        expect(conn.state() == ConnState::Handshaking);
        expect(not conn.connected());
        expect(conn.native() == nullptr);
    };
    "stream operations fail closed before the handshake completes"_test = [] {
        Connection conn{nullptr};

        expect(conn.open_stream() == UINT64_MAX);

        std::array<std::byte, 4> data{};
        expect(not conn.send_stream(data));
        expect(not conn.write_stream(0, data));
    };
    "on_connected/on_stream just replace the stored callbacks, no invocation without a tick"_test =
        [] {
        Connection conn{nullptr};
        bool connected_fired = false;
        conn.on_connected([&connected_fired] { connected_fired = true; });
        conn.on_stream([](std::uint64_t, std::vector<std::byte>, bool) {});

        expect(not connected_fired);
    };
};

} // namespace quic::tests
#endif
