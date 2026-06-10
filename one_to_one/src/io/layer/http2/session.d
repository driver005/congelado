module io.layer.http2.session;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.session.
// C++ Session used std::map<uint32_t, std::unique_ptr<Stream<>>> and
// std::set<uint32_t> for closed streams.  D port uses dynamic arrays of
// DataStream* (GC-managed for Run 1; Run 3: use SwissHashMap + util.alloc).
// C++ threw on error paths; D returns Http2ErrorCode / logs and continues.
// std::optional<FrameHeader<RECEIVER>> m_safe_header → nullable pointer.

import io.layer.shared.types;
import io.layer.http2.consts;
import io.layer.http2.settings;
import io.layer.http2.frame;
import io.layer.http2.stream;
import io.layer.http2.req : HttpRequest, write_http_request;
import io.layer.http2.res : HttpResponse, write_http_response;
import io.codec.hpack.hpack : HPackTable;
import io.error.http : Http2ErrorCode, get_http2_error_code;
import utils.buffering.reader : BufferReader;
import utils.buffering.node   : BufferNode;
import utils.buffering.view   : BufferView;
import shared.flow : SendCallback, CloseCallback;
import interfaces.protocol : ReceiveDispatchFn;
import util.alloc : make, dispose;

// ─── Session ─────────────────────────────────────────────────────────────────

/// io::layer::http2::Session
class Session {
  public:
    this(SendCallback send_callback, CloseCallback close_callback,
         ReceiveDispatchFn dispatch = ReceiveDispatchFn.init) {
        m_running                 = true;
        m_last_server_stream_id   = 0;
        m_last_client_stream_id   = 1;
        m_local_settings          = make!Settings();
        m_remote_settings         = make!Settings();
        m_decoding_table          = make!HPackTable();
        m_encoding_table          = make!HPackTable();
        m_connection_stream       = make!ConnectionStream(m_local_settings,
                                                          m_remote_settings);
        m_submitter               = send_callback;
        m_closer                  = close_callback;
        m_safe_header             = null;
        m_dispatch                = dispatch;
        m_streams_count           = 0;
    }

    ~this() {
        dispose(m_local_settings);
        dispose(m_remote_settings);
        dispose(m_decoding_table);
        dispose(m_encoding_table);
        dispose(m_connection_stream);
        foreach (i; 0 .. m_streams_count)
            dispose(m_streams_buf[i]);
    }

    void send(HttpRequest request) {
        auto stream = next_client_stream();
        const uint sid = stream.get_stream_id();
        request.set_stream_id(sid);

        // PORT-NOTE: C++ uses std::vector; D uses fixed-size ubyte[4096] to avoid GC.
        ubyte[4096] wire_buf;
        size_t wire_pos = 0;
        ubyte[] wire_slice = wire_buf[];
        write_http_request(request, m_encoding_table, m_local_settings.get_max_frame_size(),
                           wire_slice, wire_pos);

        auto node = make!BufferNode(cast(const(ubyte)[]) wire_buf[0 .. wire_pos]);
        stream.advance_send(FrameType.HEADERS, Flags.END_STREAM);
        send_node(node);
    }

    void receive(ref BufferReader reader) {
        if (!m_running) return;

        // PORT-NOTE: C++ had try/catch for ConnectionError/StreamError.
        // D: error handling via return codes; partial — full wiring in Run 3.

        if (m_safe_header is null) {
            if (reader.size() < HEADER_SIZE)
                return;

            FrameHeader hdr;
            auto err = read_frame_header_from_reader(reader,
                m_local_settings.get_max_frame_size(), hdr);
            if (err != Http2ErrorCode.NO_ERROR_CODE) {
                close(err);
                return;
            }

            if (m_remote_settings.is_acknowledged()) {
                if (m_remote_settings.get_delta_window_on_settings() > 0) {
                    foreach (i; 0 .. m_streams_count) {
                        if (m_streams_buf[i] !is null)
                            m_streams_buf[i].update_send_window(
                                cast(uint) m_remote_settings.get_delta_window_on_settings());
                    }
                    m_remote_settings.set_delta_window_on_settings(0);
                }
                m_remote_settings.set_state(SettingsState.IMPLEMENTED);
            }

            m_safe_header = hdr;
        }

        if (reader.size() < m_safe_header.get_length())
            return;

        auto header = m_safe_header;
        m_safe_header = null;

        uint stream_id = header.get_stream_id();

        if (stream_id == 0) {
            auto resp = m_connection_stream.receive(header, reader);
            if (resp !is null)
                send_frame(resp);
        } else {
            auto stream = get_or_create_stream(stream_id);
            if (stream is null) {
                reader.consume(header.get_length());
                return;
            }
            auto err = stream.receive(header, reader);
            if (err != Http2ErrorCode.NO_ERROR_CODE) {
                // Send RST_STREAM.
                ubyte[4] payload;
                uint code_val = cast(uint) err;
                payload[0] = cast(ubyte)(code_val >> 24);
                payload[1] = cast(ubyte)(code_val >> 16);
                payload[2] = cast(ubyte)(code_val >>  8);
                payload[3] = cast(ubyte)(code_val);
                auto rst = make!FrameBuilder();
                rst.add_type(FrameType.RST_STREAM).add_flags(0)
                   .add_stream_id(stream_id).add_payload(payload[]).build();
                send_frame(rst);
                dispose(rst);
                mark_stream_closed(stream_id);
            } else if (stream.is_remote_done()) {
                dispatch_response(stream_id);
            }
        }
    }

    void send_node(BufferNode node) {
        if (m_submitter.fn !is null)
            m_submitter.fn(m_submitter.ctx, node);
    }

    void send_frame(FrameBuilder frame) {
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size ubyte[4096] to avoid GC.
        ubyte[4096] wire_buf;
        size_t wire_pos = 0;
        ubyte[] wire_slice = wire_buf[];
        write_frame_builder(frame, m_local_settings.get_max_frame_size(), wire_slice, wire_pos);
        auto node = make!BufferNode(cast(const(ubyte)[]) wire_buf[0 .. wire_pos]);
        send_node(node);
    }

    void close(Http2ErrorCode code, uint stream_id = 0) {
        m_running = false;

        ubyte[8] payload;
        payload[0] = cast(ubyte)(stream_id >> 24);
        payload[1] = cast(ubyte)(stream_id >> 16);
        payload[2] = cast(ubyte)(stream_id >>  8);
        payload[3] = cast(ubyte)(stream_id);
        uint code_val = cast(uint) code;
        payload[4] = cast(ubyte)(code_val >> 24);
        payload[5] = cast(ubyte)(code_val >> 16);
        payload[6] = cast(ubyte)(code_val >>  8);
        payload[7] = cast(ubyte)(code_val);

        auto frame = make!FrameBuilder();
        frame.add_type(FrameType.GOAWAY).add_flags(0).add_stream_id(0)
             .add_payload(payload[]).build();
        send_frame(frame);
        dispose(frame);

        m_safe_header = null;
        // Remove streams with id > stream_id
        foreach (i; 0 .. m_streams_count) {
            if (m_streams_buf[i] !is null && m_streams_buf[i].get_stream_id() > stream_id)
                m_streams_buf[i] = null;
        }

        if (m_closer.fn !is null)
            m_closer.fn(m_closer.ctx);
    }

    uint get_last_client_stream_id() const { return m_last_client_stream_id; }
    Settings get_local_settings()          { return m_local_settings; }
    const(Settings) get_local_settings_c() const { return m_local_settings; }

  private:
    void dispatch_response(uint stream_id) {
        auto stream = find_stream(stream_id);
        if (stream is null) return;

        auto req = stream.get_request();
        auto res = stream.get_response();
        res.set_status(interfaces.status.Status.NOT_FOUND);

        if (m_dispatch.fn !is null)
            m_dispatch.fn(m_dispatch.ctx, *req, *res);

        // PORT-NOTE: C++ uses std::vector; D uses fixed-size ubyte[4096] to avoid GC.
        ubyte[4096] wire_buf;
        size_t wire_pos = 0;
        ubyte[] wire_slice = wire_buf[];
        write_http_response(res, m_encoding_table,
                            m_local_settings.get_max_frame_size(), wire_slice, wire_pos);
        auto node = make!BufferNode(cast(const(ubyte)[]) wire_buf[0 .. wire_pos]);
        send_node(node);
    }

    DataStream next_client_stream() {
        m_last_client_stream_id += 2;
        return get_or_create_stream(m_last_client_stream_id);
    }

    DataStream get_or_create_stream(uint stream_id) {
        if (stream_id == 0) return null;

        // Linear scan — Run 3 will replace with SwissHashMap.
        foreach (i; 0 .. m_streams_count) {
            if (m_streams_buf[i] !is null && m_streams_buf[i].get_stream_id() == stream_id)
                return m_streams_buf[i];
        }

        // PORT-NOTE: C++ uses std::map<uint32_t, unique_ptr<Stream<>>>; D uses fixed-size DataStream[256] to avoid GC.
        assert(m_streams_count < m_streams_buf.length, "Session: too many streams");
        auto stream = make!DataStream(stream_id, m_connection_stream,
                                      m_decoding_table, m_encoding_table,
                                      m_local_settings, m_remote_settings);
        m_streams_buf[m_streams_count++] = stream;
        return stream;
    }

    DataStream find_stream(uint stream_id) {
        foreach (i; 0 .. m_streams_count) {
            if (m_streams_buf[i] !is null && m_streams_buf[i].get_stream_id() == stream_id)
                return m_streams_buf[i];
        }
        return null;
    }

    void mark_stream_closed(uint stream_id) {
        foreach (i; 0 .. m_streams_count) {
            if (m_streams_buf[i] !is null && m_streams_buf[i].get_stream_id() == stream_id) {
                m_streams_buf[i] = null;
                return;
            }
        }
    }

    // PORT-NOTE: C++ used BufferReader.take(HEADER_SIZE) then ReadFrameHeaderAdaptor.
    // D: copy 9 bytes from reader into a local buffer, then parse.
    static Http2ErrorCode read_frame_header_from_reader(ref BufferReader reader,
                                                        uint max_frame_size,
                                                        out FrameHeader header) {
        ubyte[HEADER_SIZE] buf;
        size_t pos = 0;
        while (pos < HEADER_SIZE && !reader.empty()) {
            auto fr = reader.front();
            if (fr.ptr is null) break;
            size_t take = (HEADER_SIZE - pos) < fr.len ? (HEADER_SIZE - pos) : fr.len;
            buf[pos .. pos + take] = fr.ptr[0 .. take];
            pos += take;
            reader.consume(take);
        }
        if (pos < HEADER_SIZE)
            return Http2ErrorCode.FRAME_SIZE_ERROR;

        return read_frame_header(buf[], max_frame_size, header);
    }

    bool              m_running;
    uint              m_last_server_stream_id;
    uint              m_last_client_stream_id;
    Settings          m_local_settings;
    Settings          m_remote_settings;
    // PORT-NOTE: C++ std::map<uint32_t, unique_ptr<Stream<>>>; D uses fixed-size DataStream[256] to avoid GC.
    DataStream[256]   m_streams_buf;
    size_t            m_streams_count;
    // PORT-NOTE: C++ std::set<uint32_t> closed_streams → not needed (null slots).
    // PORT-NOTE: HPackTable is a class (reference type); * removed for correct D semantics.
    HPackTable        m_decoding_table;
    HPackTable        m_encoding_table;
    ConnectionStream  m_connection_stream;
    SendCallback      m_submitter;
    CloseCallback     m_closer;
    // PORT-NOTE: C++ std::optional<FrameHeader> → nullable pointer.
    FrameHeader       m_safe_header;
    ReceiveDispatchFn m_dispatch;
}
