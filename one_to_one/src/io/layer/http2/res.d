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
        auto r = new HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "200");
        return r;
    }
    static HttpResponse created(uint stream_id) {
        auto r = new HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "201");
        return r;
    }
    static HttpResponse no_content(uint stream_id) {
        auto r = new HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "204");
        return r;
    }
    static HttpResponse bad_request(uint stream_id) {
        auto r = new HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "400");
        return r;
    }
    static HttpResponse not_found(uint stream_id) {
        auto r = new HttpResponse(stream_id);
        r.insert_token(Token.STATUS, "404");
        return r;
    }
    static HttpResponse internal_error(uint stream_id) {
        auto r = new HttpResponse(stream_id);
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
        char[6] buf;
        size_t len = fmt_uint(buf[], cast(uint) status);
        // PORT-NOTE: idup allocates; Run 3 will use stack-local slice.
        insert_token(Token.STATUS, buf[0 .. len].idup);
    }
    override void set_body(ubyte[] body) { m_body = body; }

    override HeaderEntry[] get_header() const {
        HeaderEntry[] result;
        foreach (f; m_static_headers) {
            if (f !is null) {
                HeaderEntry e;
                e.kind         = HeaderEntryKind.Static;
                e.static_field = f;
                result ~= e;
            }
        }
        return result;
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
        // PORT-NOTE: GC allocation for Run 1; Run 3: util.alloc make!/dispose.
        auto f = new HeaderFieldStatic();
        f.m_name  = token;
        f.m_value = value;
        m_static_headers[cast(size_t) token] = f;
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
    HeaderFieldStatic*[STATIC_HEADER_COUNT] m_static_headers;
    // PORT-NOTE: SwissHashMap for custom headers deferred to Run 3.
    ubyte[]                                 m_body;
}

// ─── write_http_response ─────────────────────────────────────────────────────
// PORT-NOTE: C++ WriteHttpResponseAdaptor was a range_adaptor_closure.
// CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames — only on DATA frames.

void write_http_response(HttpResponse res, HPackTable* table,
                         size_t max_frame_size, ref ubyte[] output,
                         ubyte extra_flags = 0) {
    const uint stream_id = res.get_stream_id();
    auto entries = res.get_header();
    bool first_frame = true;

    struct FlushCtx {
        ubyte[]* out_;
        uint     stream_id;
        bool*    first_frame;
    }
    FlushCtx fctx;
    fctx.out_        = &output;
    fctx.stream_id   = stream_id;
    fctx.first_frame = &first_frame;

    auto encoder = new HpackEncoder!()(
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
                *c.out_ ~= hdr[];
                *c.out_ ~= data;
                *c.first_frame = false;
            })
    );
    encoder();

    // Emit trailing DATA frame with END_STREAM.
    if (res.get_body().length == 0) {
        ubyte data_flags = extra_flags | Flags.END_STREAM;
        auto frame = new FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(data_flags)
             .add_stream_id(stream_id).build();
        write_frame_builder(frame, max_frame_size, output);
    } else {
        auto frame = new FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(extra_flags)
             .add_stream_id(stream_id)
             .add_payload(res.get_body()).build();
        write_frame_builder(frame, max_frame_size, output);
    }
}
