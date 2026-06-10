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
        auto r = new HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.GET));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_head(uint stream_id, const(char)[] path) {
        auto r = new HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.HEAD));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_post(uint stream_id, const(char)[] path) {
        auto r = new HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.POST));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_put(uint stream_id, const(char)[] path) {
        auto r = new HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.PUT));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_del(uint stream_id, const(char)[] path) {
        auto r = new HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.DELETE));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_patch(uint stream_id, const(char)[] path) {
        auto r = new HttpRequest(stream_id);
        r.insert_token(Token.METHOD, method_str(HttpMethod.PATCH));
        r.insert_token(Token.PATH, path);
        return r;
    }
    static HttpRequest make_options(uint stream_id, const(char)[] path) {
        auto r = new HttpRequest(stream_id);
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
        // PORT-NOTE: returns GC slice; Run 3 will use fixed-size stack-based array.
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
    override const(char)[] find_header(scope const(char)[] name) const {
        // PORT-NOTE: Full tokenize + custom lookup deferred to Run 3.
        return [];
    }

    uint get_stream_id() const { return m_stream_id; }

  private:
    void insert_token(Token token, const(char)[] value) {
        // PORT-NOTE: C++ make_shared<HeaderField<true>>; D GC allocation for Run 1.
        auto f = new HeaderFieldStatic();
        f.m_name  = token;
        f.m_value = value;
        m_static_headers[cast(size_t) token] = f;
    }

    void insert_str(const(char)[] name, const(char)[] value) {
        // PORT-NOTE: tokenize + custom hashmap deferred to Run 3.
    }

    enum size_t STATIC_HEADER_COUNT = cast(size_t) Token.CUSTOM + 1;

    uint                                  m_stream_id;
    HeaderFieldStatic*[STATIC_HEADER_COUNT] m_static_headers;
    // PORT-NOTE: SwissHashMap for custom headers deferred to Run 3.
    BufferView                            m_body;
}

// ─── write_http_request ───────────────────────────────────────────────────────
// PORT-NOTE: C++ WriteHttpRequestAdaptor was a range_adaptor_closure.
// CRITICAL: END_STREAM MUST NEVER be set on HEADERS frames — only on DATA frames.

void write_http_request(HttpRequest req, HPackTable* table,
                        size_t max_frame_size, ref ubyte[] output,
                        ubyte extra_flags = 0) {
    const uint stream_id = req.get_stream_id();
    auto entries = req.get_header();
    bool first_frame = true;

    // Build header entries slice for HPACK encoder.
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
    if (req.get_body().empty()) {
        ubyte data_flags = extra_flags | Flags.END_STREAM;
        auto frame = new FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(data_flags)
             .add_stream_id(stream_id).build();
        write_frame_builder(frame, max_frame_size, output);
    } else {
        // PORT-NOTE: body → DATA frames with END_STREAM on last chunk.
        // BufferView iteration deferred to Run 3; placeholder stub.
        auto frame = new FrameBuilder();
        frame.add_type(FrameType.DATA).add_flags(extra_flags)
             .add_stream_id(stream_id).build();
        // TODO: append body bytes from BufferView (Run 3).
        write_frame_builder(frame, max_frame_size, output);
    }
}
