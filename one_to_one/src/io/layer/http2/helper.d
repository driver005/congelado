module io.layer.http2.helper;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.helper.
// C++ threw ConnectionError/StreamError; D returns Http2ErrorCode.
// StreamStateMachine is kept as a class matching the C++ semantics.

import io.layer.shared.types;
import io.error.http : Http2ErrorCode;

/// io::layer::http2::StreamStateMachine
class StreamStateMachine {
  public:
    this(uint stream_id) {
        m_id    = stream_id;
        m_state = StreamState.IDLE;
    }

    // PORT-NOTE: C++ advance() threw on invalid transitions.
    // D: returns Http2ErrorCode (NO_ERROR_CODE on success).
    // new_state is set unconditionally (to m_state on error).
    Http2ErrorCode advance(FrameType type, ubyte flags, bool is_local,
                           out StreamState new_state) {
        const bool END_STREAM = (flags & Flags.END_STREAM) != 0;

        // PRIORITY and CONTINUATION do not drive state transitions (§5.1).
        if (type == FrameType.PRIORITY) {
            new_state = m_state;
            return Http2ErrorCode.NO_ERROR_CODE;
        }

        // RST_STREAM always moves to Closed regardless of current state.
        if (type == FrameType.RST_STREAM) {
            if (m_state == StreamState.IDLE) {
                new_state = m_state;
                return Http2ErrorCode.PROTOCOL_ERROR;
            }
            new_state = apply(StreamState.CLOSED);
            return Http2ErrorCode.NO_ERROR_CODE;
        }

        switch (m_state) {

        case StreamState.IDLE:
            if (type == FrameType.HEADERS) {
                // Sending or receiving HEADERS opens the stream.
                // If END_STREAM is also set, jump straight to half-closed.
                if (END_STREAM) {
                    new_state = apply(is_local
                        ? StreamState.HALF_CLOSED_LOCAL
                        : StreamState.HALF_CLOSED_REMOTE);
                    return Http2ErrorCode.NO_ERROR_CODE;
                }
                new_state = apply(StreamState.OPEN);
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            if (type == FrameType.PUSH_PROMISE) {
                // PUSH_PROMISE transitions the *promised* (this) stream.
                // Sending → ReservedLocal; receiving → ReservedRemote.
                new_state = apply(is_local
                    ? StreamState.RESERVED_LOCAL
                    : StreamState.RESERVED_REMOTE);
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            // Anything else on an idle stream is a PROTOCOL_ERROR (§5.1).
            new_state = m_state;
            return Http2ErrorCode.PROTOCOL_ERROR;

        case StreamState.OPEN:
            if (END_STREAM) {
                new_state = apply(is_local
                    ? StreamState.HALF_CLOSED_LOCAL
                    : StreamState.HALF_CLOSED_REMOTE);
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            new_state = m_state; // stays Open
            return Http2ErrorCode.NO_ERROR_CODE;

        case StreamState.HALF_CLOSED_LOCAL:
            if (is_local) {
                // We must not send data or headers frames on a half-closed-local
                // stream (WINDOW_UPDATE and RST_STREAM are handled above / below).
                if (type == FrameType.DATA || type == FrameType.HEADERS) {
                    new_state = m_state;
                    return Http2ErrorCode.STREAM_CLOSED;
                }
            } else {
                // Receiving END_STREAM closes the stream.
                if (END_STREAM) {
                    new_state = apply(StreamState.CLOSED);
                    return Http2ErrorCode.NO_ERROR_CODE;
                }
            }
            new_state = m_state;
            return Http2ErrorCode.NO_ERROR_CODE;

        case StreamState.HALF_CLOSED_REMOTE:
            if (!is_local) {
                // Peer must not send DATA or HEADERS on a half-closed-remote stream.
                if (type == FrameType.DATA || type == FrameType.HEADERS) {
                    new_state = m_state;
                    return Http2ErrorCode.STREAM_CLOSED;
                }
            } else {
                // Sending END_STREAM closes the stream.
                if (END_STREAM) {
                    new_state = apply(StreamState.CLOSED);
                    return Http2ErrorCode.NO_ERROR_CODE;
                }
            }
            new_state = m_state;
            return Http2ErrorCode.NO_ERROR_CODE;

        case StreamState.RESERVED_LOCAL:
            if (is_local && type == FrameType.HEADERS) {
                new_state = apply(StreamState.HALF_CLOSED_REMOTE);
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            if (!is_local &&
                (type == FrameType.WINDOW_UPDATE || type == FrameType.RST_STREAM)) {
                new_state = m_state; // peer may send these
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            new_state = m_state;
            return Http2ErrorCode.PROTOCOL_ERROR;

        case StreamState.RESERVED_REMOTE:
            if (!is_local && type == FrameType.HEADERS) {
                new_state = apply(StreamState.HALF_CLOSED_LOCAL);
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            if (is_local &&
                (type == FrameType.WINDOW_UPDATE || type == FrameType.RST_STREAM)) {
                new_state = m_state;
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            new_state = m_state;
            return Http2ErrorCode.PROTOCOL_ERROR;

        case StreamState.CLOSED:
            // WINDOW_UPDATE and RST_STREAM may arrive briefly after closure
            // due to race conditions — permit them silently (§5.1).
            if (!is_local &&
                (type == FrameType.WINDOW_UPDATE || type == FrameType.RST_STREAM)) {
                new_state = m_state;
                return Http2ErrorCode.NO_ERROR_CODE;
            }
            // DATA or HEADERS on a closed stream → connection error
            if (!is_local &&
                (type == FrameType.DATA || type == FrameType.HEADERS)) {
                new_state = m_state;
                return Http2ErrorCode.STREAM_CLOSED;
            }
            // Anything else: connection error per §5.1
            new_state = m_state;
            return Http2ErrorCode.PROTOCOL_ERROR;

        default:
            new_state = m_state;
            return Http2ErrorCode.NO_ERROR_CODE;
        }
    }

    StreamState get_state() const { return m_state; }
    uint        id()        const { return m_id; }

    bool is_open()          const { return m_state == StreamState.OPEN; }
    bool can_send_data()    const {
        return m_state == StreamState.OPEN
            || m_state == StreamState.HALF_CLOSED_REMOTE;
    }
    bool can_receive_data() const {
        return m_state == StreamState.OPEN
            || m_state == StreamState.HALF_CLOSED_LOCAL;
    }
    bool is_closed()        const { return m_state == StreamState.CLOSED; }

  private:
    StreamState apply(StreamState next) {
        m_state = next;
        return m_state;
    }

    uint        m_id;
    StreamState m_state;
}
