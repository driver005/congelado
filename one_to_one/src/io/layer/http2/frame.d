module io.layer.http2.frame;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.frame.
// C++ used C++26 range_adaptor_closure pipelines for encode/decode.
// D port replaces them with plain structs + helper functions operating on
// ubyte slices. Template parameter shared_layer::FrameRole was used to
// specialise PUSH_PROMISE handling; D port keeps FrameRole as a runtime tag
// on FrameHeader.
// C++ threw ConnectionError on validation failure; D returns Http2ErrorCode.

import io.layer.shared.types;
import io.layer.http2.consts;
import io.error.http : Http2ErrorCode;

// ─── FrameHeader ─────────────────────────────────────────────────────────────

/// io::layer::http2::FrameHeader<Role>
/// PORT-NOTE: C++ was a template on FrameRole; D stores m_role at construction.
class FrameHeader {
  public:
    this() {
        m_length    = 0;
        m_type      = FrameType.DATA;
        m_flags     = 0;
        m_stream_id = 0;
        m_role      = FrameRole.RECEIVER;
    }

    this(uint length, FrameType type, ubyte flags, uint stream_id,
         FrameRole role = FrameRole.RECEIVER) {
        m_length    = length;
        m_type      = type;
        m_flags     = flags;
        m_stream_id = 0;
        m_role      = role;
        set_stream_id(stream_id);
    }

    FrameHeader add_length(uint len) {
        m_length = len;
        return this;
    }

    FrameHeader add_type(FrameType type) {
        m_type = type;
        return this;
    }

    FrameHeader add_flags(ubyte flags) {
        m_flags = flags;
        return this;
    }

    FrameHeader add_stream_id(uint stream_id) {
        set_stream_id(stream_id);
        return this;
    }

    // PORT-NOTE: C++ validate() threw ConnectionError; D returns Http2ErrorCode.
    Http2ErrorCode validate() {
        if (m_role == FrameRole.SENDER) {
            if (m_stream_id % 2 != 0) {
                if (m_type == FrameType.PUSH_PROMISE)
                    return Http2ErrorCode.INTERNAL_ERROR;
            }
        }
        switch (m_type) {
        case FrameType.DATA:          return validate_data();
        case FrameType.HEADERS:       return validate_headers();
        case FrameType.PRIORITY:      return validate_priority();
        case FrameType.RST_STREAM:    return validate_rst_stream();
        case FrameType.SETTINGS:      return validate_settings();
        case FrameType.PUSH_PROMISE:  return validate_push_promise();
        case FrameType.PING:          return validate_ping();
        case FrameType.GOAWAY:        return validate_goaway();
        case FrameType.WINDOW_UPDATE: return validate_window_update();
        case FrameType.CONTINUATION:  return validate_continuation();
        default:
            return Http2ErrorCode.NO_ERROR_CODE;
        }
    }

    Http2ErrorCode validate_payload_size(size_t actual_size) const {
        if (actual_size != m_length)
            return Http2ErrorCode.INTERNAL_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_padding(uint actual_size) const {
        if (actual_size >= m_length)
            return Http2ErrorCode.PROTOCOL_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    bool is_end_stream() const { return (m_flags & Flags.END_STREAM) != 0; }
    bool is_padded()     const { return (m_flags & Flags.PADDED)     != 0; }

    size_t    get_size()      const { return HEADER_SIZE; }
    uint      get_length()    const { return m_length; }
    FrameType get_type()      const { return m_type; }
    ubyte     get_flags()     const { return m_flags; }
    uint      get_stream_id() const { return m_stream_id; }
    FrameRole get_role()      const { return m_role; }

    void set_length(uint len)       { m_length    = len; }
    void set_type(FrameType type)   { m_type      = type; }
    void set_flags(ubyte flags)     { m_flags     = flags; }
    void set_stream_id(uint new_id) { m_stream_id = new_id & 0x7FFFFFFF; }

  private:
    Http2ErrorCode validate_data() const {
        if (m_stream_id == 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & ~(Flags.END_STREAM | Flags.PADDED)) != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & Flags.PADDED) != 0 && m_length < 1)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_headers() const {
        if (m_stream_id == 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & ~(Flags.END_STREAM | Flags.END_HEADERS |
                         Flags.PADDED | Flags.PRIORITY)) != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        uint min_len = 0;
        if ((m_flags & Flags.PADDED)   != 0) min_len += 1;
        if ((m_flags & Flags.PRIORITY) != 0) min_len += 5;
        if (m_length < min_len)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_priority() const {
        if (m_stream_id == 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_flags != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_length != 5)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_rst_stream() const {
        if (m_stream_id == 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_flags != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_length != 4)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_settings() const {
        if (m_stream_id != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & ~Flags.ACK) != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & Flags.ACK) != 0) {
            if (m_length != 0)
                return Http2ErrorCode.FRAME_SIZE_ERROR;
        } else if (m_length % 6 != 0) {
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        }
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_push_promise() const {
        if (m_role == FrameRole.RECEIVER) {
            // A receiver (server) MUST treat receipt of PUSH_PROMISE as a connection error
            return Http2ErrorCode.PROTOCOL_ERROR;
        } else {
            // Sender validation for server PUSH_PROMISE
            if (m_stream_id == 0)
                return Http2ErrorCode.PROTOCOL_ERROR;
            if ((m_flags & ~(Flags.END_HEADERS | Flags.PADDED)) != 0)
                return Http2ErrorCode.PROTOCOL_ERROR;
        }
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_ping() const {
        if (m_stream_id != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & ~Flags.ACK) != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_length != 8)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_goaway() const {
        if (m_stream_id != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_flags != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_length < 8)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_window_update() const {
        if (m_flags != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if (m_length != 4)
            return Http2ErrorCode.FRAME_SIZE_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    Http2ErrorCode validate_continuation() const {
        if (m_stream_id == 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        if ((m_flags & ~Flags.END_HEADERS) != 0)
            return Http2ErrorCode.PROTOCOL_ERROR;
        return Http2ErrorCode.NO_ERROR_CODE;
    }

    uint      m_length;
    FrameType m_type;
    ubyte     m_flags;
    uint      m_stream_id;
    FrameRole m_role;
}

// ─── read_frame_header ────────────────────────────────────────────────────────
// PORT-NOTE: C++ ReadFrameHeaderAdaptor was a range_adaptor_closure.
// D: plain function that populates header_out on success.
// Returns Http2ErrorCode.NO_ERROR_CODE on success.

Http2ErrorCode read_frame_header(const(ubyte)[] data, uint max_frame_size,
                                 out FrameHeader header_out) {
    if (data.length < HEADER_SIZE)
        return Http2ErrorCode.FRAME_SIZE_ERROR;

    uint len = (cast(uint) data[0] << 16)
             | (cast(uint) data[1] <<  8)
             | (cast(uint) data[2]);

    if (len > max_frame_size)
        return Http2ErrorCode.FRAME_SIZE_ERROR;

    auto frame_type = cast(FrameType) data[3];
    ubyte flags     = data[4];
    uint stream_id  = 0x7FFF_FFFF &
                      ((cast(uint) data[5] << 24)
                      | (cast(uint) data[6] << 16)
                      | (cast(uint) data[7] <<  8)
                      | (cast(uint) data[8]));

    import util.alloc : make;
    header_out = make!FrameHeader(len, frame_type, flags, stream_id,
                                  FrameRole.RECEIVER);
    return Http2ErrorCode.NO_ERROR_CODE;
}

// ─── write_frame_header ───────────────────────────────────────────────────────
// PORT-NOTE: C++ FrameHeaderClosureAdaptor piped big-endian fields.
// D: plain write into a 9-byte caller-provided slice.

void write_frame_header(ubyte[] out_, uint length, FrameType type,
                        ubyte flags, uint stream_id) {
    // 3-byte big-endian length
    out_[0] = cast(ubyte)(length >> 16);
    out_[1] = cast(ubyte)(length >>  8);
    out_[2] = cast(ubyte)(length);
    out_[3] = cast(ubyte) type;
    out_[4] = flags;
    // 4-byte stream id (MSB reserved, mask to 31 bits)
    uint clean = stream_id & 0x7FFFFFFF;
    out_[5] = cast(ubyte)(clean >> 24);
    out_[6] = cast(ubyte)(clean >> 16);
    out_[7] = cast(ubyte)(clean >>  8);
    out_[8] = cast(ubyte)(clean);
}

// ─── read_window_increment ────────────────────────────────────────────────────
// PORT-NOTE: C++ ReadWindowIncrementAdaptor; D plain function.
// Returns the 31-bit increment; sets err on failure.

uint read_window_increment(const(ubyte)[] data, out Http2ErrorCode err) {
    if (data.length < 4) {
        err = Http2ErrorCode.FRAME_SIZE_ERROR;
        return 0;
    }
    uint inc = 0x7FFFFFFF &
               ((cast(uint) data[0] << 24)
               | (cast(uint) data[1] << 16)
               | (cast(uint) data[2] <<  8)
               | (cast(uint) data[3]));
    if (inc == 0) {
        err = Http2ErrorCode.PROTOCOL_ERROR;
        return 0;
    }
    err = Http2ErrorCode.NO_ERROR_CODE;
    return inc;
}

// ─── FrameBuilder ────────────────────────────────────────────────────────────
// PORT-NOTE: C++ FrameBuilder<Role> held a std::vector<std::byte> payload;
// D port uses a fixed-size ubyte[16384] buffer to avoid GC.

class FrameBuilder {
  public:
    this() {
        m_type      = FrameType.DATA;
        m_flags     = 0;
        m_stream_id = 0;
        m_payload_len = 0;
    }

    FrameBuilder add_type(FrameType type) {
        m_type = type;
        return this;
    }

    FrameBuilder add_flags(ubyte flags) {
        m_flags = flags;
        return this;
    }

    FrameBuilder add_stream_id(uint stream_id) {
        m_stream_id = stream_id & 0x7FFFFFFF;
        return this;
    }

    FrameBuilder add_payload(const(ubyte)[] payload) {
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size ubyte[16384] to avoid GC.
        assert(m_payload_len + payload.length <= m_payload_buf.length, "FrameBuilder: payload overflow");
        m_payload_buf[m_payload_len .. m_payload_len + payload.length] = payload[];
        m_payload_len += payload.length;
        return this;
    }

    FrameBuilder build() { return this; }

    void expand_payload(const(ubyte)[] payload) {
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size ubyte[16384] to avoid GC.
        assert(m_payload_len + payload.length <= m_payload_buf.length, "FrameBuilder: payload overflow");
        m_payload_buf[m_payload_len .. m_payload_len + payload.length] = payload[];
        m_payload_len += payload.length;
    }

    size_t         get_size()     const { return HEADER_SIZE + m_payload_len; }
    const(ubyte)[] get_payload()  const { return m_payload_buf[0 .. m_payload_len]; }
    size_t         get_length()   const { return m_payload_len; }
    FrameType      get_type()     const { return m_type; }
    ubyte          get_flags()    const { return m_flags; }
    uint           get_stream_id() const { return m_stream_id; }

  private:
    FrameType      m_type;
    ubyte          m_flags;
    uint           m_stream_id;
    // PORT-NOTE: C++ uses std::vector; D uses fixed-size ubyte[16384] to avoid GC.
    ubyte[16384]   m_payload_buf;
    size_t         m_payload_len;
}

// ─── write_frame_builder ──────────────────────────────────────────────────────
// PORT-NOTE: C++ WriteFrameBuilderAdaptor/WriteFrameClosureAdapter split a payload
// into max_frame_size chunks, emitting HEADERS + END_HEADERS, CONTINUATION, and
// DATA + END_STREAM frames.  D port: plain function writing into caller-owned slice
// at position out_pos.
//
// CRITICAL HTTP/2 INVARIANT: END_STREAM MUST NEVER be set on HEADERS frames.
// Only set on the trailing DATA frame (or on HEADERS when no_data == true, meaning
// no DATA frame follows).  Matches the C++ WriteFrameClosureAdapter exactly.

void write_frame_builder(FrameBuilder frame, size_t max_frame_size,
                         ref ubyte[] out_, ref size_t out_pos,
                         bool end_stream_after_data = false,
                         bool no_data = false) {
    const(ubyte)[] data = frame.get_payload();
    size_t total_len    = data.length;
    size_t slice_size   = (max_frame_size > 0) ? max_frame_size : (total_len > 0 ? total_len : 1);
    size_t total_chunks = (total_len == 0) ? 1 : (total_len + slice_size - 1) / slice_size;

    foreach (chunk_idx; 0 .. total_chunks) {
        size_t offset    = chunk_idx * slice_size;
        size_t remaining = (total_len > offset) ? (total_len - offset) : 0;
        size_t chunk_len = (remaining < slice_size) ? remaining : slice_size;

        FrameType type = frame.get_type();
        ubyte flags    = frame.get_flags();
        bool is_last   = (chunk_idx == total_chunks - 1);

        if (type == FrameType.HEADERS) {
            if (is_last) {
                flags |= Flags.END_HEADERS;
                // CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames.
                // It is only set here when no_data == true, meaning there is no
                // DATA frame to follow (matching C++ WriteFrameClosureAdapter).
                if (no_data)
                    flags |= Flags.END_STREAM;
            }
            if (chunk_idx != 0)
                type = FrameType.CONTINUATION;
        } else if (type == FrameType.DATA && is_last && !end_stream_after_data) {
            flags |= Flags.END_STREAM;
        }

        ubyte[HEADER_SIZE] hdr;
        write_frame_header(hdr[], cast(uint) chunk_len, type, flags,
                           frame.get_stream_id());
        // PORT-NOTE: C++ uses std::vector append; D writes positionally to avoid GC.
        out_[out_pos .. out_pos + HEADER_SIZE] = hdr[];
        out_pos += HEADER_SIZE;
        if (chunk_len > 0) {
            out_[out_pos .. out_pos + chunk_len] = data[offset .. offset + chunk_len];
            out_pos += chunk_len;
        }
    }
}
