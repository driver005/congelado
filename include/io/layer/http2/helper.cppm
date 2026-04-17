export module io_layer_http2:helper;

import std;
import io_layer_shared;
import :frame;

export namespace io::layer::http2 {

class StreamStateMachine {
  public:
    explicit StreamStateMachine(std::uint32_t stream_id) : m_id{stream_id}, m_state{shared_layer::StreamState::Idle} {}

    const shared_layer::StreamState &get_state() const noexcept { return m_state; }
    const std::uint32_t &id() const noexcept { return m_id; }
    bool is_open() const noexcept { return m_state == shared_layer::StreamState::Open; }
    bool can_send_data() const noexcept {
        return m_state == shared_layer::StreamState::Open || m_state == shared_layer::StreamState::HalfClosedRemote;
    }
    bool can_receive_data() const noexcept {
        return m_state == shared_layer::StreamState::Open || m_state == shared_layer::StreamState::HalfClosedLocal;
    }
    bool is_closed() const noexcept { return m_state == shared_layer::StreamState::Closed; }

    template <shared_layer::FrameRole Role>
    shared_layer::StreamState advance(const Frame<Role> &frame, bool is_local) {
        const auto type = frame.get_header().get_type();
        const auto flags = frame.get_header().get_flags();
        const bool end_stream = (flags & shared_layer::Flags::END_STREAM) != 0;

        // PRIORITY and CONTINUATION do not drive state transitions (§5.1).
        // RST_STREAM always moves to Closed regardless of current state.
        if (type == shared_layer::FrameType::PRIORITY)
            return m_state;

        if (type == shared_layer::FrameType::RST_STREAM) {
            require_not_idle(type);
            return apply(shared_layer::StreamState::Closed);
        }

        switch (m_state) {

        case shared_layer::StreamState::Idle:
            if (type == shared_layer::FrameType::HEADERS) {
                // Sending or receiving HEADERS opens the stream.
                // If END_STREAM is also set, jump straight to half-closed.
                if (end_stream)
                    return apply(is_local ? shared_layer::StreamState::HalfClosedLocal
                                          : shared_layer::StreamState::HalfClosedRemote);
                return apply(shared_layer::StreamState::Open);
            }
            if (type == shared_layer::FrameType::PUSH_PROMISE) {
                // PUSH_PROMISE transitions the *promised* (this) stream.
                // Sending → ReservedLocal; receiving → ReservedRemote.
                return apply(is_local ? shared_layer::StreamState::ReservedLocal
                                      : shared_layer::StreamState::ReservedRemote);
            }
            // Anything else on an idle stream is a PROTOCOL_ERROR (§5.1).
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type {} received on idle stream", static_cast<int>(type)), m_id);

        case shared_layer::StreamState::Open:
            if (end_stream)
                return apply(is_local ? shared_layer::StreamState::HalfClosedLocal
                                      : shared_layer::StreamState::HalfClosedRemote);
            return m_state; // stays Open

        case shared_layer::StreamState::HalfClosedLocal:
            if (is_local) {
                // We must not send data or headers frames on a half-closed-local
                // stream (WINDOW_UPDATE and RST_STREAM are handled above / below).
                if (type == shared_layer::FrameType::DATA || type == shared_layer::FrameType::HEADERS) {
                    throw error::http::StreamError(m_id, error::http::Http2ErrorCode::STREAM_CLOSED,
                                                   "Cannot send DATA/HEADERS on half-closed (local) stream");
                }
            } else {
                // Receiving END_STREAM closes the stream.
                if (end_stream)
                    return apply(shared_layer::StreamState::Closed);
            }
            return m_state;

        case shared_layer::StreamState::HalfClosedRemote:
            if (!is_local) {
                // Peer must not send DATA or HEADERS on a half-closed-remote stream.
                if (type == shared_layer::FrameType::DATA || type == shared_layer::FrameType::HEADERS) {
                    throw error::http::StreamError(m_id, error::http::Http2ErrorCode::STREAM_CLOSED,
                                                   "Received DATA/HEADERS on half-closed (remote) stream");
                }
            } else {
                // Sending END_STREAM closes the stream.
                if (end_stream)
                    return apply(shared_layer::StreamState::Closed);
            }
            return m_state;

        case shared_layer::StreamState::ReservedLocal:
            if (is_local && type == shared_layer::FrameType::HEADERS)
                return apply(shared_layer::StreamState::HalfClosedRemote);
            if (!is_local &&
                (type == shared_layer::FrameType::WINDOW_UPDATE || type == shared_layer::FrameType::RST_STREAM))
                return m_state; // peer may send these
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Illegal frame type {} on reserved (local) stream", static_cast<int>(type)), m_id);

        case shared_layer::StreamState::ReservedRemote:
            if (!is_local && type == shared_layer::FrameType::HEADERS)
                return apply(shared_layer::StreamState::HalfClosedLocal);
            if (is_local &&
                (type == shared_layer::FrameType::WINDOW_UPDATE || type == shared_layer::FrameType::RST_STREAM))
                return m_state;
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Illegal frame type {} on reserved (remote) stream", static_cast<int>(type)), m_id);

        case shared_layer::StreamState::Closed:
            // WINDOW_UPDATE and RST_STREAM may arrive briefly after closure
            // due to race conditions — permit them silently (§5.1).
            if (!is_local &&
                (type == shared_layer::FrameType::WINDOW_UPDATE || type == shared_layer::FrameType::RST_STREAM))
                return m_state;

            // DATA or HEADERS on a closed stream → connection error
            if (!is_local && (type == shared_layer::FrameType::DATA || type == shared_layer::FrameType::HEADERS)) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::STREAM_CLOSED,
                                                   std::format("Received {} on closed stream", static_cast<int>(type)),
                                                   m_id);
            }
            // Anything else: connection error per §5.1
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type {} received on closed stream", static_cast<int>(type)), m_id);
        }
    }

  private:
    shared_layer::StreamState apply(shared_layer::StreamState next) noexcept {
        m_state = next;
        return m_state;
    }

    void require_not_idle(shared_layer::FrameType type) const {
        if (m_state == shared_layer::StreamState::Idle)
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type {} received on idle stream", static_cast<int>(type)), m_id);
    }

    std::uint32_t m_id;
    shared_layer::StreamState m_state;
};

} // namespace io::layer::http2
