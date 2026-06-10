module io.layer.http2.req;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.req.
// C++ HttpRequest extended interfaces::IRequest<Protocol>.
// D port preserves the interface hierarchy.
// std::shared_ptr<HeaderField<true/false>> → HeaderFieldStatic*/HeaderField* pointers
// (heap via GC for Run 1; Run 3 upgrades to util.alloc make!/dispose).
// C++ threw on empty name / invalid token; D port silently ignores per @nogc nothrow.
// CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames — see write_http_request.

import io.layer.shared.types : FrameType, Flags;
import io.layer.http2.frame;
import io.codec.hpack.hpack : HPackTable, HpackEncoder, FlushCallback, HpackFlushReason;
import io.shared.http.types : Token, HttpMethod, method_str;
import io.shared.http.header : HeaderFieldStatic, HeaderField, HeaderEntry, HeaderEntryKind;
import interfaces.request : IRequest;
import interfaces.protocol : HttpProtocol;
import utils.buffering.view : BufferView;
import util.alloc : make, dispose;

// ─── HttpRequest ─────────────────────────────────────────────────────────────

/// io::layer::http2::HttpRequest
class HttpRequest : IRequest!HttpProtocol {
  public:
    this(uint stream_id) {
        m_stream_id = stream_id;
        foreach (ref f; m_static_headers)
            f = null;
    }

    // Static factory methods.
    static HttpRequest make_get(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.GET));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_head(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.HEAD));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_post(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.POST));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_put(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.PUT));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_del(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.DELETE));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_patch(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.PATCH));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_options(uint stream_id, const(char)[] path) {
        auto r = make!HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.OPTIONS));
        r.insert_token(Token.PATH, path);
        return r;
    }

    // Builder methods.
    HttpRequest with_method(HttpMethod method) {
        insert_token(Token.METHOD, method_str(method));
        return this;
    }
    HttpRequest with_path(const(char)[] path) {
        insert_token(Token.PATH, path);
        return this;
    }
    HttpRequest with_scheme(const(char)[] schema) {
        insert_token(Token.SCHEME, schema);
        return this;
    }
    HttpRequest with_authority(const(char)[] authority) {
        insert_token(Token.AUTHORITY, authority);
        return this;
    }
    HttpRequest with_header(const(char)[] name, const(char)[] value) {
        insert_str(name, value);
        return this;
    }
    HttpRequest with_header_tok(Token token, const(char)[] value) {
        insert_token(token, value);
        return this;
    }
    // TODO: with_query — URL-encoding via utils.encode; deferred to Run 3.
    // TODO: with_bearer_auth / with_basic_auth — base64; deferred to Run 3.
    HttpRequest with_content_type(const(char)[] mime) {
        insert_token(Token.CONTENT_TYPE, mime);
        return this;
    }
    HttpRequest with_accept(const(char)[] mime) {
        insert_token(Token.ACCEPT, mime);
        return this;
    }
    HttpRequest with_user_agent(const(char)[] user) {
        insert_token(Token.USER_AGENT, user);
        return this;
    }

    void set_stream_id(uint stream_id) { m_stream_id = stream_id; }

    // IRequest interface implementation.
    override void add_header(scope const(char)[] name, scope const(char)[] value) {
        insert_str(name, value);
    }
    override void add_header(Token token, scope const(char)[] value) {
        insert_token(token, value);
    }
    override void remove_header(scope const(char)[] name) {
        // PORT-NOTE: deferred tokenize; for known tokens only (Run 3).
    }
    override void remove_header(Token token) {
        m_static_headers[cast(size_t) token] = null;
    }
    override const(char)[] get_method() const {
        auto f = m_static_headers[cast(size_t) Token.METHOD];
        return f !is null ? f.m_value : [];
    }
    override const(char)[] get_target() const {
        auto f = m_static_headers[cast(size_t) Token.PATH];
        return f !is null ? f.m_value : [];
    }
    override ref BufferView get_body() { return m_body; }
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
    override const(char)[] find_header(scope const(char)[] name) const {
        // PORT-NOTE: Full tokenize + custom lookup deferred to Run 3.
        return [];
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

    enum size_t STATIC_HEADER_COUNT = cast(size_t) Token.CUSTOM + 1;

    uint                                    m_stream_id;
    // PORT-NOTE: HeaderFieldStatic is a struct; store by value in inline array to avoid GC.
    HeaderFieldStatic[STATIC_HEADER_COUNT]  m_static_fields;
    HeaderFieldStatic*[STATIC_HEADER_COUNT] m_static_headers;
    // PORT-NOTE: C++ uses std::vector; D uses fixed-size HeaderEntry[STATIC_HEADER_COUNT] to avoid GC.
    HeaderEntry[STATIC_HEADER_COUNT]        m_header_cache;
    size_t                                  m_header_cache_count;
    // PORT-NOTE: SwissHashMap for custom headers deferred to Run 3.
    BufferView                              m_body;
}

// ─── write_http_request ───────────────────────────────────────────────────────
// PORT-NOTE: C++ WriteHttpRequestAdaptor was a range_adaptor_closure.
// CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames — only on DATA frames.

void write_http_request(HttpRequest req, HPackTable table,
                        size_t max_frame_size, ref ubyte[] output,
                        ref size_t out_pos,
                        ubyte extra_flags = 0) {
    const uint stream_id = req.get_stream_id();
    auto entries = req.get_header();
    bool first_frame = true;

    // Build header entries slice for HPACK encoder.
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
    if (req.get_body().empty()) {
        ubyte data_flags = extra_flags | Flags.END_STREAM;
        auto frame = make!FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(data_flags)
             .add_stream_id(stream_id).build();
        write_frame_builder(frame, max_frame_size, output, out_pos);
        dispose(frame);
    } else {
        // PORT-NOTE: body → DATA frames with END_STREAM on last chunk.
        // BufferView iteration deferred to Run 3; placeholder stub.
        auto frame = make!FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(extra_flags)
             .add_stream_id(stream_id).build();
        // TODO: append body bytes from BufferView (Run 3).
        write_frame_builder(frame, max_frame_size, output, out_pos);
        dispose(frame);
    }
}
