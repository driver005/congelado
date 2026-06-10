module io.codec.quic.tls;
@nogc nothrow:

import io.codec.quic.types;
import modules.openssl;

// OpenSSL 3.6 native QUIC TLS layer.
//
// Optimisations vs. previous version:
//   - TlsContext: ALPN "h3" registered at context level (not per-session)
//   - TlsContext: move-assign added; copy explicitly deleted
//   - TlsContext: session tickets disabled (saves ~200 bytes/handshake
//     for short-lived HTTP/3 connections that won't resume)
//   - TlsSession: RAII on SSL* — if constructor throws after SSL_new,
//     the guard ensures SSL_free is called
//   - TlsSession: BIO_new_dgram replaced with BIO_new(BIO_s_datagram())
//     to avoid the deprecated UDP-only path; BIO_set_fd used directly
//   - TlsSession: non-copyable, explicitly movable

// ── TlsContext ────────────────────────────────────────────────────────────────

// PORT-NOTE: C++ class TlsContext → D class TlsContext (RAII, has behavior).
class TlsContext {
  public:
    // PORT-NOTE: C++ static TlsContext from_files(…) throws → D returns null on error.
    static TlsContext from_files(const(char)[] cert_pem, const(char)[] key_pem) {
        SSL_CTX* ctx = SSL_CTX_new(OSSL_QUIC_server_method());
        if (!ctx) return null;

        // Certificate + key
        if (SSL_CTX_use_certificate_file(ctx, cert_pem.ptr, SSL_FILETYPE_PEM) != 1 ||
            SSL_CTX_use_PrivateKey_file(ctx, key_pem.ptr, SSL_FILETYPE_PEM) != 1 ||
            SSL_CTX_check_private_key(ctx) != 1) {
            SSL_CTX_free(ctx);
            return null;
        }

        // ALPN: advertise h3 only
        // Wire format: length-prefixed string, e.g. "\x02h3"
        static immutable ubyte[3] alpn = [2, 'h', '3'];
        // PORT-NOTE: ALPN select callback dropped (function pointer with lambda not trivial in @nogc)
        // TODO: wire ALPN callback in improvement pass

        // Disable session tickets — HTTP/3 over QUIC handles resumption via
        // NEW_TOKEN frames; server-side tickets add overhead without benefit here.
        SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);

        // Don't solicit or require client certificates.
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, null);

        // TLS 1.3 only
        SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
        SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

        return new TlsContext(ctx);
    }

    this() { m_ctx = null; }

    ~this() {
        if (m_ctx !is null)
            SSL_CTX_free(m_ctx);
    }

    @disable this(this); // non-copyable

    // PORT-NOTE: C++ move constructor/assign → D: implement move via constructor
    // taking an rvalue pointer (manual, since D classes are reference types and
    // "move" is handled by nulling the source reference at the call site).

    SSL_CTX* get() const pure { return m_ctx; }
    bool valid()   const pure { return m_ctx !is null; }

  private:
    this(SSL_CTX* ctx) { m_ctx = ctx; }
    SSL_CTX* m_ctx;
}

// ── TlsSession ────────────────────────────────────────────────────────────────

// PORT-NOTE: C++ class TlsSession → D class TlsSession (RAII, has behavior).
class TlsSession {
  public:
    // PORT-NOTE: C++ constructor throws → D constructor sets m_ssl = null on failure.
    this(TlsContext ctx, int udp_fd) {
        m_ssl = null;
        if (!ctx.valid()) return;
        SSL* ssl = SSL_new(ctx.get());
        if (!ssl) return;

        // Datagram BIO — BIO_new_dgram is the correct OpenSSL 3 path for UDP
        BIO* bio = BIO_new_dgram(udp_fd, BIO_NOCLOSE);
        if (!bio) {
            SSL_free(ssl);
            return;
        }
        SSL_set_bio(ssl, bio, bio);
        SSL_set_accept_state(ssl);
        m_ssl = ssl;
    }

    ~this() {
        if (m_ssl !is null)
            SSL_free(m_ssl);
    }

    @disable this(this); // non-copyable

    SSL* get() const pure { return m_ssl; }

    // Drive OpenSSL's internal event loop (retransmits, key updates, etc.)
    void process() { SSL_handle_events(m_ssl); }

    // Returns true while the handshake is in progress (WANT_READ/WRITE),
    // true on completion, false on fatal error.
    bool do_handshake() {
        int rc = SSL_do_handshake(m_ssl);
        if (rc == 1) return true;
        int err = SSL_get_error(m_ssl, rc);
        return (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
    }

    // Open a new server-initiated stream.
    // flags: 0 = bidirectional, SSL_STREAM_FLAG_UNI = unidirectional.
    SSL* create_stream(ulong flags = 0) { return SSL_new_stream(m_ssl, flags); }

  private:
    SSL* m_ssl;
}
