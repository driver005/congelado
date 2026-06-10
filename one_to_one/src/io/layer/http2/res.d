module io.layer.http2.res;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.res.
// C++ HttpResponse extended interfaces::IResponse<Protocol>.
// D port preserves the interface hierarchy.
// std::shared_ptr<HeaderField<true/false>> → HeaderFieldStatic*/HeaderField* pointers.
// C++ threw on invalid operations; D silently ignores (nothrow).
// CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames — only on DATA frames.

import io.layer.shared.types : FrameType, Flags;
import io.layer.http2.frame;
import io.codec.hpack.hpack : HPackTable, HpackEncoder, FlushCallback, HpackFlushReason;
import io.shared.http.types : Token;
import io.shared.http.header : HeaderFieldStatic, HeaderEntry, HeaderEntryKind;
import interfaces.response : IResponse;
import interfaces.status : Status;
import interfaces.protocol : HttpProtocol;
import util.alloc : make, dispose;

// ─── HttpResponse ────────────────────────────────────────────────────────────

/// io::layer::http2::HttpResponse
class HttpResponse : IResponse!HttpProtocol {
  public:
    this(uint stream_id) {
        m_stream_id = stream_id;
        foreach (ref f; m_static_headers)
            f = null;
    }

    // Static factory methods.
    static HttpResponse ok(uint stream_id) {
        auto r = make!HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "200");
        return r;
    }
    static HttpResponse created(uint stream_id) {
        auto r = make!HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "201");
        return r;
    }
    static HttpResponse no_content(uint stream_id) {
        auto r = make!HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "204");
        return r;
    }
    static HttpResponse bad_request(uint stream_id) {
        auto r = make!HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "400");
        return r;
    }
    static HttpResponse not_found(uint stream_id) {
        auto r = make!HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "404");
        return r;
    }
    static HttpResponse internal_error(uint stream_id) {
        auto r = make!HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "500");
        return r;
    }

    // Builder methods.
    HttpResponse with_header(const(char)[] name, const(char)[] value) {
        insert_str(name, value);
        return this;
    }
    HttpResponse with_header_tok(Token token, const(char)[] value) {
        insert_token(token, value);
        return this;
    }
    HttpResponse with_content_type(const(char)[] mime) {
        insert_token(Token.CONTENT_TYPE, mime);
        return this;
    }
    HttpResponse with_body(ubyte[] body) {
        m_body = body;
        return this;
    }

    // IResponse interface implementation.
    override void add_header(scope const(char)[] name, scope const(char)[] value) {
        insert_str(name, value);
    }
    override void add_header(Token token, scope const(char)[] value) {
        insert_token(token, value);
    }
    override void remove_header(scope const(char)[] name) {
        // PORT-NOTE: tokenize deferred to Run 3.
    }
    override void remove_header(Token token) {
        m_static_headers[cast(size_t) token] = null;
    }
    override void set_status(Status status) {
        // PORT-NOTE: C++ used std::to_string(status_code(status)).
        // Store formatted status in inline buffer to avoid GC (no idup).
        size_t len = fmt_uint(m_status_buf[], cast(uint) status);
        insert_token(Token.STATUS, m_status_buf[0 .. len]);
    }
    override void set_body(ubyte[] body) { m_body = body; }

    override HeaderEntry[] get_header() const {
        // PORT-NOTE: C++ uses std::vector; D uses fixed-size HeaderEntry[STATIC_HEADER_COUNT] to avoid GC.
        return m_header_cache[0 .. m_header_cache_count];
    }

    // Rebuild the header cache (call after any insert_token / insert_str).
    private void rebuild_header_cache() {
        m_header_cache_count = 0;
        foreach (f; m_static_headers) {
            if (f !is null) {
                HeaderEntry e;
                e.kind         = HeaderEntryKind.Static;
                e.static_field = f;
                // PORT-NOTE: C++ uses std::vector; D uses fixed-size T[N] to avoid GC.
                assert(m_header_cache_count < m_header_cache.length, "get_header: too many headers");
                m_header_cache[m_header_cache_count++] = e;
            }
        }
    }
    override const(ubyte)[] get_body() const { return m_body; }

    /**
     * Returns an approximation of the total on-wire size of the complete HTTP/2 response
     * (header frames + data frames + payload).
     */
    size_t get_size(size_t max_frame_payload) const {
        size_t header_block = 0;
        foreach (f; m_static_headers) {
            if (f !is null)
                header_block += f.size();
        }
        size_t num_header_frames =
            (header_block == 0) ? 1 : (header_block + max_frame_payload - 1) / max_frame_payload;
        size_t total = (num_header_frames * 9) + header_block;

        // --- DATA frames ---
        const size_t BODY_SIZE = m_body.length;
        if (BODY_SIZE > 0) {
            size_t num_data_frames = (BODY_SIZE + max_frame_payload - 1) / max_frame_payload;
            total += (num_data_frames * 9) + BODY_SIZE;
        } else {
            total += 9; // empty DATA frame with END_STREAM
        }
        return total;
    }

    uint get_stream_id() const { return m_stream_id; }

  private:
    void insert_token(Token token, const(char)[] value) {
        // PORT-NOTE: HeaderFieldStatic is a struct; store by value in inline array to avoid GC.
        size_t idx = cast(size_t) token;
        m_static_fields[idx].m_name  = token;
        m_static_fields[idx].m_value = value;
        m_static_headers[idx] = &m_static_fields[idx];
        rebuild_header_cache();
    }
    void insert_str(const(char)[] name, const(char)[] value) {
        // PORT-NOTE: tokenize + custom hashmap deferred to Run 3.
    }

    // PORT-NOTE: format uint as decimal without GC.
    static size_t fmt_uint(char[] buf, uint val) {
        if (val == 0) { buf[0] = '0'; return 1; }
        size_t pos = buf.length;
        while (val > 0) {
            buf[--pos] = cast(char)('0' + val % 10);
            val /= 10;
        }
        size_t len = buf.length - pos;
        foreach (i; 0 .. len) buf[i] = buf[pos + i];
        return len;
    }

    enum size_t STATIC_HEADER_COUNT = cast(size_t) Token.CUSTOM + 1;

    uint                                    m_stream_id;
    char[6]                                 m_status_buf;
    // PORT-NOTE: HeaderFieldStatic is a struct; store by value in inline array to avoid GC.
    HeaderFieldStatic[STATIC_HEADER_COUNT]  m_static_fields;
    HeaderFieldStatic*[STATIC_HEADER_COUNT] m_static_headers;
    // PORT-NOTE: C++ uses std::vector; D uses fixed-size HeaderEntry[STATIC_HEADER_COUNT] to avoid GC.
    HeaderEntry[STATIC_HEADER_COUNT]        m_header_cache;
    size_t                                  m_header_cache_count;
    // PORT-NOTE: SwissHashMap for custom headers deferred to Run 3.
    ubyte[]                                 m_body;
}

// ─── write_http_response ─────────────────────────────────────────────────────
// PORT-NOTE: C++ WriteHttpResponseAdaptor was a range_adaptor_closure.
// CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames — only on DATA frames.

void write_http_response(HttpResponse res, HPackTable table,
                         size_t max_frame_size, ref ubyte[] output,
                         ref size_t out_pos,
                         ubyte extra_flags = 0) {
    const uint stream_id = res.get_stream_id();
    auto entries = res.get_header();
    bool first_frame = true;

    struct FlushCtx {
        ubyte[]* out_;
        size_t*  out_pos_;
        uint     stream_id;
        bool*    first_frame;
    }
    FlushCtx fctx;
    fctx.out_        = &output;
    fctx.out_pos_    = &out_pos;
    fctx.stream_id   = stream_id;
    fctx.first_frame = &first_frame;

    auto encoder = make!(HpackEncoder!())(
        table, entries, max_frame_size,
        FlushCallback(cast(void*)&fctx,
            (void* ctx, const(ubyte)[] data, HpackFlushReason reason) @nogc nothrow {
                auto c = cast(FlushCtx*) ctx;
                FrameType type = *c.first_frame ? FrameType.HEADERS : FrameType.CONTINUATION;
                ubyte fflags   = (reason == HpackFlushReason.END)
                                 ? Flags.END_HEADERS : cast(ubyte) 0;
                // CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames.
                ubyte[HEADER_SIZE] hdr;
                write_frame_header(hdr[], cast(uint) data.length, type, fflags, c.stream_id);
                // PORT-NOTE: C++ uses std::vector append; D writes positionally to avoid GC.
                (*c.out_)[*c.out_pos_ .. *c.out_pos_ + HEADER_SIZE] = hdr[];
                *c.out_pos_ += HEADER_SIZE;
                (*c.out_)[*c.out_pos_ .. *c.out_pos_ + data.length] = data[];
                *c.out_pos_ += data.length;
                *c.first_frame = false;
            })
    );
    encoder();
    dispose(encoder);

    // Emit trailing DATA frame with END_STREAM.
    if (res.get_body().length == 0) {
        ubyte data_flags = extra_flags | Flags.END_STREAM;
        auto frame = make!FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(data_flags)
             .add_stream_id(stream_id).build();
        write_frame_builder(frame, max_frame_size, output, out_pos);
        dispose(frame);
    } else {
        auto frame = make!FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(extra_flags)
             .add_stream_id(stream_id)
             .add_payload(res.get_body()).build();
        write_frame_builder(frame, max_frame_size, output, out_pos);
        dispose(frame);
    }
}
