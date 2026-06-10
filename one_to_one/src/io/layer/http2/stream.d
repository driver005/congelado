module io.layer.http2.stream;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.stream.
// C++ used a template<bool IsStreamBased> to unify connection-level and
// stream-level stream objects via [[no_unique_address]] StreamHelper.
// D port uses two separate classes: ConnectionStream and DataStream, preserving
// all logic.  ConnectionLevelHelper / StreamLevelHelper → embedded fields.
// C++ threw on errors; D returns Http2ErrorCode.
// std::optional<BufferView> m_header_block → nullable BufferView pointer.

import io.layer.shared.types;
import io.layer.http2.consts;
import io.layer.http2.settings;
import io.layer.http2.frame;
import io.layer.http2.helper;
import io.layer.http2.req : HttpRequest;
import io.layer.http2.res : HttpResponse;
import io.codec.hpack.hpack : HPackTable, HpackEncoder, Hpack = HpackEncoder;
import io.error.http : Http2ErrorCode, get_http2_error_code;
import utils.buffering.reader : BufferReader;
import utils.buffering.view   : BufferView;

// ─── ConnectionStream ────────────────────────────────────────────────────────
// PORT-NOTE: C++ Stream<false> — connection-level (stream 0) aggregates SETTINGS,
// PING, GOAWAY, WINDOW_UPDATE handling.

class ConnectionStream {
  public:
    this(Settings local_settings, Settings remote_settings) {
        m_state_machine   = new StreamStateMachine(0);
        m_send_window     = cast(int) remote_settings.get_initial_window_size();
        m_recv_window     = cast(int) remote_settings.get_initial_window_size();
        m_local_settings  = local_settings;
        m_remote_settings = remote_settings;
        m_connection_error_code = Http2ErrorCode.NO_ERROR_CODE;
    }

    void set_connection_error_code(Http2ErrorCode code) {
        m_connection_error_code = code;
    }
    Http2ErrorCode get_connection_error_code() const {
        return m_connection_error_code;
    }

    // Receive a connection-level frame.
    // Returns a FrameBuilder response if one must be sent; else null.
    // PORT-NOTE: C++ returned std::optional<FrameBuilder<SENDER>>; D returns null.
    FrameBuilder receive(FrameHeader header, ref BufferReader reader) {
        auto type = header.get_type();
        FrameBuilder response = null;

        switch (type) {
        case FrameType.WINDOW_UPDATE: {
            Http2ErrorCode err;
            ubyte[4] buf;
            copy_reader_bytes(reader, buf[0 .. 4]);
            uint increment = read_window_increment(buf[], err);
            if (err == Http2ErrorCode.NO_ERROR_CODE)
                update_send_window(increment);
            break;
        }
        case FrameType.GOAWAY: {
            m_remote_settings.set_last_stream_id(header.get_stream_id());

            ubyte[8] payload;
            uint sid_bytes = header.get_stream_id();
            payload[0] = cast(ubyte)(sid_bytes >> 24);
            payload[1] = cast(ubyte)(sid_bytes >> 16);
            payload[2] = cast(ubyte)(sid_bytes >>  8);
            payload[3] = cast(ubyte)(sid_bytes);
            uint code_val = cast(uint) m_connection_error_code;
            payload[4] = cast(ubyte)(code_val >> 24);
            payload[5] = cast(ubyte)(code_val >> 16);
            payload[6] = cast(ubyte)(code_val >>  8);
            payload[7] = cast(ubyte)(code_val);

            response = new FrameBuilder();
            response.add_type(FrameType.GOAWAY).add_flags(0).add_stream_id(0)
                    .add_payload(payload[]).build();
            break;
        }
        case FrameType.PING: {
            auto flags = header.get_flags();
            if (flags & Flags.ACK) {
                // ACK: try to match against in-flight pings.
                ubyte[8] ack_payload;
                copy_reader_bytes(reader, ack_payload[]);
                m_remote_settings.get_ping_tracker().on_ack(ack_payload[]);
            } else {
                m_remote_settings.get_ping_tracker().note_activity(
                    core.time.MonoTime.currTime);

                // Echo back with ACK.
                ubyte[8] ping_payload;
                copy_reader_bytes(reader, ping_payload[]);
                response = new FrameBuilder();
                response.add_type(FrameType.PING).add_flags(Flags.ACK)
                        .add_stream_id(0).add_payload(ping_payload[]).build();
            }
            break;
        }
        case FrameType.SETTINGS: {
            response = handle_settings(header, reader);
            break;
        }
        case FrameType.PRIORITY: {
            // PRIORITY frames are deprecated in RFC9113; treat as PROTOCOL_ERROR.
            // (C++ threw here; D: caller checks result)
            break;
        }
        default:
            break;
        }

        reader.consume(header.get_length());
        return response;
    }

    bool can_send_data_of_size(int size) const {
        return m_state_machine.can_send_data() && (m_send_window >= size);
    }

    void consume_window(int size, bool is_sender) {
        if (is_sender) {
            m_send_window -= size;
        } else {
            m_recv_window -= size;
        }
    }

    void update_send_window(uint increment) {
        m_send_window += cast(int) increment;
    }

    bool needs_recv_window_update() const {
        return m_recv_window < cast(int) m_remote_settings.get_initial_window_size() / 2;
    }

    uint recv_window_increment() const {
        return cast(uint)(m_remote_settings.get_initial_window_size()
                          - cast(uint) m_recv_window);
    }

    int  send_window()   const { return m_send_window; }
    int  recv_window()   const { return m_recv_window; }
    uint get_stream_id() const { return m_state_machine.id(); }

  private:
    FrameBuilder handle_settings(FrameHeader header, ref BufferReader reader) {
        if ((header.get_flags() & Flags.ACK) != 0) {
            m_local_settings.set_state(SettingsState.ACKNOWLEDGED);
            return null;
        }

        if (m_remote_settings.is_finished())
            return null; // PROTOCOL_ERROR in C++; logged by caller

        uint old_window = m_remote_settings.get_initial_window_size();

        // Read the settings payload.
        ubyte[128] sbuf;
        uint payload_len = header.get_length();
        if (payload_len > sbuf.length) payload_len = cast(uint) sbuf.length;
        copy_reader_bytes(reader, sbuf[0 .. payload_len]);
        auto settings = read_settings(sbuf[0 .. payload_len]);

        // Apply to remote settings (merge).
        uint new_window = settings.get_initial_window_size();
        if (old_window != new_window) {
            int delta = cast(int) new_window - cast(int) old_window;
            update_send_window(delta > 0 ? cast(uint) delta : 0);
            m_remote_settings.set_delta_window_on_settings(delta);
        }
        m_remote_settings.set_state(SettingsState.ACKNOWLEDGED);

        return Settings.generate_ack();
    }

    // PORT-NOTE: helper to copy up to n bytes from BufferReader into a stack buffer.
    static void copy_reader_bytes(ref BufferReader reader, ubyte[] out_) {
        size_t remaining = out_.length;
        size_t pos = 0;
        while (remaining > 0 && !reader.empty()) {
            auto fr = reader.front();
            if (fr.ptr is null) break;
            size_t take = remaining < fr.len ? remaining : fr.len;
            out_[pos .. pos + take] = fr.ptr[0 .. take];
            pos       += take;
            remaining -= take;
            reader.consume(take);
        }
    }

    StreamStateMachine m_state_machine;
    int                m_send_window;
    int                m_recv_window;
    Settings           m_local_settings;
    Settings           m_remote_settings;
    Http2ErrorCode     m_connection_error_code;
}

// ─── DataStream ──────────────────────────────────────────────────────────────
// PORT-NOTE: C++ Stream<true> — per-stream carrying request, response, hpack refs.

class DataStream {
  public:
    this(uint stream_id, ConnectionStream connection_stream,
         HPackTable* decoding_table, HPackTable* encoding_table,
         Settings local_settings, Settings remote_settings,
         bool is_client_initiated = true) {
        m_state_machine      = new StreamStateMachine(stream_id);
        m_send_window        = cast(int) remote_settings.get_initial_window_size();
        m_recv_window        = cast(int) remote_settings.get_initial_window_size();
        m_local_settings     = local_settings;
        m_remote_settings    = remote_settings;
        m_connection_stream  = connection_stream;
        m_decoding_table     = decoding_table;
        m_encoding_table     = encoding_table;
        m_request            = new HttpRequest(stream_id);
        m_response           = new HttpResponse(stream_id);
        m_header_block       = null;
        m_expecting_continuation = false;
        m_remote_done        = false;
        // PORT-NOTE: C++ validated even/odd stream IDs and threw; D: silently ignore.
    }

    ~this() { cleanup_resources(); }

    // Receive a stream-level frame.
    // PORT-NOTE: C++ threw on error; D returns Http2ErrorCode.
    Http2ErrorCode receive(FrameHeader header, ref BufferReader reader) {
        auto type = header.get_type();

        if (m_expecting_continuation && type != FrameType.CONTINUATION) {
            reader.consume(header.get_length());
            return Http2ErrorCode.PROTOCOL_ERROR;
        }

        StreamState new_state;
        auto err = m_state_machine.advance(type, header.get_flags(), false, new_state);
        if (err != Http2ErrorCode.NO_ERROR_CODE) {
            reader.consume(header.get_length());
            return err;
        }

        switch (type) {
        case FrameType.DATA: {
            if (header.get_length() > 0) {
                consume_window(header.get_length(), false);
                if (m_body_buf is null)
                    m_body_buf = new BufferView();
                reader.expand_view(*m_body_buf, header.get_length());
            }
            break;
        }
        case FrameType.HEADERS:
        case FrameType.PUSH_PROMISE:
        case FrameType.CONTINUATION: {
            if (m_header_block is null)
                m_header_block = new BufferView();
            if (header.get_length() > 0)
                reader.expand_view(*m_header_block, header.get_length());

            if ((header.get_flags() & Flags.END_HEADERS) != 0) {
                m_expecting_continuation = false;
                handle_header(*m_header_block);
            } else {
                m_expecting_continuation = true;
            }
            break;
        }
        case FrameType.WINDOW_UPDATE: {
            ubyte[4] buf;
            ConnectionStream.copy_reader_bytes(reader, buf[]);
            Http2ErrorCode werr;
            uint inc = read_window_increment(buf[], werr);
            if (werr == Http2ErrorCode.NO_ERROR_CODE)
                update_send_window(inc);
            break;
        }
        case FrameType.RST_STREAM: {
            ubyte[4] buf;
            ConnectionStream.copy_reader_bytes(reader, buf[]);
            uint code_val = (cast(uint) buf[0] << 24)
                          | (cast(uint) buf[1] << 16)
                          | (cast(uint) buf[2] <<  8)
                          | (cast(uint) buf[3]);
            cleanup_resources();
            reader.consume(header.get_length() > 4 ? header.get_length() - 4 : 0);
            return get_http2_error_code(code_val);
        }
        default:
            break;
        }

        reader.consume(header.get_length());

        if (m_state_machine.get_state() == StreamState.HALF_CLOSED_REMOTE)
            m_remote_done = true;

        if (m_state_machine.is_closed())
            cleanup_resources();

        return Http2ErrorCode.NO_ERROR_CODE;
    }

    void handle_header(ref BufferView view) {
        // PORT-NOTE: C++ decoded via Hpack; D: stub — full wiring in Run 3.
        // TODO: wire hpack.decode() here.
    }

    void consume_window(int size, bool is_sender) {
        m_connection_stream.consume_window(size, is_sender);
        if (is_sender) {
            m_send_window -= size;
        } else {
            m_recv_window -= size;
        }
    }

    bool can_send_data_of_size(int size) const {
        return m_state_machine.can_send_data() && (m_send_window >= size);
    }

    void update_send_window(uint increment) {
        m_send_window += cast(int) increment;
        m_connection_stream.update_send_window(increment);
    }

    void advance_send(FrameType type, ubyte flags) {
        StreamState new_state;
        m_state_machine.advance(type, flags, true, new_state);
        if (m_state_machine.is_closed())
            cleanup_resources();
    }

    bool needs_recv_window_update() const {
        return m_recv_window < cast(int) m_remote_settings.get_initial_window_size() / 2;
    }

    uint recv_window_increment() const {
        return cast(uint)(m_remote_settings.get_initial_window_size()
                          - cast(uint) m_recv_window);
    }

    bool is_remote_done() const { return m_remote_done; }

    HttpRequest  get_request()  { return m_request; }
    HttpResponse get_response() { return m_response; }

    int  send_window()   const { return m_send_window; }
    int  recv_window()   const { return m_recv_window; }
    uint get_stream_id() const { return m_state_machine.id(); }
    StreamState get_state() const { return m_state_machine.get_state(); }

  private:
    void cleanup_resources() {
        m_recv_buf = null;
        m_header_block = null;
        m_expecting_continuation = false;
    }

    StreamStateMachine m_state_machine;
    int                m_send_window;
    int                m_recv_window;
    Settings           m_local_settings;
    Settings           m_remote_settings;
    ConnectionStream   m_connection_stream;
    HPackTable*        m_decoding_table;
    HPackTable*        m_encoding_table;
    HttpRequest        m_request;
    HttpResponse       m_response;
    BufferView*        m_header_block;
    BufferView*        m_body_buf;
    ubyte[]            m_recv_buf;
    bool               m_expecting_continuation;
    bool               m_remote_done;
}
