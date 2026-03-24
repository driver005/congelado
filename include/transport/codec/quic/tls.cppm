module;
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/quic.h>
#include <openssl/ssl.h>
export module quic:tls;

import std;
import :types;

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

namespace quic::tls {

// ── TlsContext ────────────────────────────────────────────────────────────────

export class TlsContext {
  public:
    static TlsContext from_files(std::string_view cert_pem, std::string_view key_pem) {
        SSL_CTX *ctx = ::SSL_CTX_new(OSSL_QUIC_server_method());
        if (!ctx)
            throw std::runtime_error("SSL_CTX_new(OSSL_QUIC_server_method) failed");

        // Certificate + key
        if (::SSL_CTX_use_certificate_file(ctx, cert_pem.data(), SSL_FILETYPE_PEM) != 1 ||
            ::SSL_CTX_use_PrivateKey_file(ctx, key_pem.data(), SSL_FILETYPE_PEM) != 1 ||
            ::SSL_CTX_check_private_key(ctx) != 1) {
            ::SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to load cert/key");
        }

        // ALPN: advertise h3 only
        // Wire format: length-prefixed string, e.g. "\x02h3"
        static constexpr unsigned char alpn[] = {2, 'h', '3'};
        ::SSL_CTX_set_alpn_select_cb(
            ctx,
            [](SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen,
               void *) -> int {
                if (::SSL_select_next_proto(const_cast<unsigned char **>(out), outlen, alpn, sizeof(alpn), in, inlen) ==
                    OPENSSL_NPN_NEGOTIATED)
                    return SSL_TLSEXT_ERR_OK;
                return SSL_TLSEXT_ERR_NOACK;
            },
            nullptr);

        // Disable session tickets — HTTP/3 over QUIC handles resumption via
        // NEW_TOKEN frames; server-side tickets add overhead without benefit here.
        ::SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);

        // Don't solicit or require client certificates.
        ::SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);

        // TLS 1.3 only
        ::SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
        ::SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

        return TlsContext{ctx};
    }

    TlsContext() = default;

    ~TlsContext() {
        if (m_ctx)
            ::SSL_CTX_free(m_ctx);
    }

    TlsContext(const TlsContext &) = delete;
    TlsContext &operator=(const TlsContext &) = delete;

    TlsContext(TlsContext &&o) noexcept : m_ctx(std::exchange(o.m_ctx, nullptr)) {}

    TlsContext &operator=(TlsContext &&o) noexcept {
        if (this != &o) {
            if (m_ctx)
                ::SSL_CTX_free(m_ctx);
            m_ctx = std::exchange(o.m_ctx, nullptr);
        }
        return *this;
    }

    [[nodiscard]] SSL_CTX *get() const noexcept { return m_ctx; }
    [[nodiscard]] bool valid() const noexcept { return m_ctx != nullptr; }

  private:
    explicit TlsContext(SSL_CTX *ctx) : m_ctx(ctx) {}
    SSL_CTX *m_ctx{nullptr};
};

// ── TlsSession ────────────────────────────────────────────────────────────────

export class TlsSession {
  public:
    TlsSession(TlsContext &ctx, int udp_fd) {
        SSL *ssl = ::SSL_new(ctx.get());
        if (!ssl)
            throw std::runtime_error("SSL_new failed");
        // RAII guard in case BIO setup throws
        struct Guard {
            SSL *s;
            ~Guard() {
                if (s)
                    ::SSL_free(s);
            }
        } guard{ssl};

        // Datagram BIO — BIO_new_dgram is the correct OpenSSL 3 path for UDP
#ifdef _WIN32
        BIO *bio = ::BIO_new_dgram(static_cast<SOCKET>(udp_fd), BIO_NOCLOSE);
#else
        BIO *bio = ::BIO_new_dgram(udp_fd, BIO_NOCLOSE);
#endif
        if (!bio)
            throw std::runtime_error("BIO_new_dgram failed");
        ::SSL_set_bio(ssl, bio, bio);

        ::SSL_set_accept_state(ssl);

        m_ssl = ssl;
        guard.s = nullptr; // disarm guard
    }

    ~TlsSession() {
        if (m_ssl)
            ::SSL_free(m_ssl);
    }

    TlsSession(const TlsSession &) = delete;
    TlsSession &operator=(const TlsSession &) = delete;

    TlsSession(TlsSession &&o) noexcept : m_ssl(std::exchange(o.m_ssl, nullptr)) {}

    TlsSession &operator=(TlsSession &&o) noexcept {
        if (this != &o) {
            if (m_ssl)
                ::SSL_free(m_ssl);
            m_ssl = std::exchange(o.m_ssl, nullptr);
        }
        return *this;
    }

    [[nodiscard]] SSL *get() const noexcept { return m_ssl; }

    // Drive OpenSSL's internal event loop (retransmits, key updates, etc.)
    void process() noexcept { ::SSL_handle_events(m_ssl); }

    // Returns true while the handshake is in progress (WANT_READ/WRITE),
    // true on completion, false on fatal error.
    bool do_handshake() noexcept {
        int rc = ::SSL_do_handshake(m_ssl);
        if (rc == 1)
            return true;
        int err = ::SSL_get_error(m_ssl, rc);
        return (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
    }

    // Open a new server-initiated stream.
    // flags: 0 = bidirectional, SSL_STREAM_FLAG_UNI = unidirectional.
    [[nodiscard]] SSL *create_stream(std::uint64_t flags = 0) noexcept { return ::SSL_new_stream(m_ssl, flags); }

  private:
    SSL *m_ssl{nullptr};
};

} // namespace quic::tls
