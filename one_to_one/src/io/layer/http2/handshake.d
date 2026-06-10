module io.layer.http2.handshake;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.handshake.
// C++ Handshake<bool IsServer> was a template; D port uses a runtime bool m_is_server.
// shared::SendCallback is a fn+ctx pair in D; see shared.flow.
// BufferReader.consume() / .size() match the D BufferReader interface.

import io.layer.shared.types : FrameType, Flags;
import io.layer.http2.consts;
import io.layer.http2.settings;
import io.layer.http2.frame;
import utils.buffering.reader : BufferReader;
import utils.buffering.node   : BufferNode;
// PORT-NOTE: shared.flow uses the path "shared.flow" (module named shared.flow
// but directory is one_to_one/src/shared/).
import shared.flow : SendCallback;
import util.alloc : make, dispose;

/// io::layer::http2::HandshakeState
enum HandshakeState : ubyte {
    AWAITING_PREFACE,
    PREFACE_RECEIVED,
    PREFACE_ERROR,
    COMPLETED,
}

/// io::layer::http2::Handshake<IsServer>
/// PORT-NOTE: C++ template on bool IsServer → D class with m_is_server flag.
class Handshake {
  public:
    this(Settings settings, SendCallback submitter, bool is_server = true) {
        m_local_settings = settings;
        m_submitter      = submitter;
        m_sent_settings  = false;
        m_is_server      = is_server;
    }

    /// Server path: first send our SETTINGS, then validate the client preface.
    HandshakeState process_server(ref BufferReader view) {
        send_handshake();
        return is_valid_preface(view);
    }

    /// Client path: send preface + SETTINGS, report COMPLETED immediately.
    HandshakeState process_client() {
        send_handshake();
        return HandshakeState.COMPLETED;
    }

    /// Generic process() — dispatches to server or client path based on m_is_server.
    HandshakeState process(ref BufferReader view) {
        if (m_is_server)
            return process_server(view);
        else
            return process_client();
    }

  private:
    HandshakeState is_valid_preface(ref BufferReader view) const {
        const preface = HTTP2_CONNECTION_PREFACE[];
        if (view.size() < preface.length) {
            return HandshakeState.AWAITING_PREFACE;
        }

        // PORT-NOTE: C++ used std::views::take | std::ranges::equal.
        // D BufferReader exposes front() which returns the first contiguous chunk.
        // We copy up to preface.length bytes from the reader into a local buffer
        // for comparison, without consuming them.
        // PORT-NOTE: This is an approximation — the full comparison across node
        // boundaries is deferred to Run 3.  For now we check the first contiguous chunk.
        bool match = true;
        auto fr = view.front();
        if (fr.ptr !is null) {
            size_t check = preface.length < fr.len ? preface.length : fr.len;
            foreach (i; 0 .. check) {
                if (fr.ptr[i] != preface[i]) {
                    match = false;
                    break;
                }
            }
            if (check < preface.length) match = false;
        } else {
            return HandshakeState.AWAITING_PREFACE;
        }

        if (match) {
            view.consume(preface.length);
            return HandshakeState.COMPLETED;
        }
        return HandshakeState.PREFACE_ERROR;
    }

    void send_handshake() {
        if (m_sent_settings)
            return;

        // Build a SETTINGS payload.
        ubyte[128] settings_buf;
        size_t settings_len = write_settings(m_local_settings, settings_buf[]);

        auto frame = make!FrameBuilder();
        frame.add_type(FrameType.SETTINGS).add_flags(0).add_stream_id(0)
             .add_payload(settings_buf[0 .. settings_len]).build();

        if (!m_is_server) {
            // Client: send HTTP2_CONNECTION_PREFACE + SETTINGS frame.
            // PORT-NOTE: C++ uses std::vector; D uses stack buffer to avoid GC.
            ubyte[64] wire_buf;
            size_t wire_len = 0;
            // Copy preface into wire_buf.
            wire_buf[wire_len .. wire_len + HTTP2_CONNECTION_PREFACE.length] = HTTP2_CONNECTION_PREFACE[];
            wire_len += HTTP2_CONNECTION_PREFACE.length;
            // Write SETTINGS frame into wire_buf after preface.
            ubyte[] wire_slice = wire_buf[];
            write_frame_builder(frame, m_local_settings.get_max_frame_size(), wire_slice, wire_len);

            auto node = make!BufferNode(cast(const(ubyte)[]) wire_buf[0 .. wire_len]);
            if (m_submitter.fn !is null)
                m_submitter.fn(m_submitter.ctx, node);
        } else {
            // Server: send only SETTINGS frame.
            ubyte[64] wire_buf;
            size_t wire_len = 0;
            ubyte[] wire_slice = wire_buf[];
            write_frame_builder(frame, m_local_settings.get_max_frame_size(), wire_slice, wire_len);

            auto node = make!BufferNode(cast(const(ubyte)[]) wire_buf[0 .. wire_len]);
            if (m_submitter.fn !is null)
                m_submitter.fn(m_submitter.ctx, node);
        }

        dispose(frame);
        m_sent_settings = true;
    }

    Settings     m_local_settings;
    SendCallback m_submitter;
    bool         m_sent_settings;
    bool         m_is_server;
}
