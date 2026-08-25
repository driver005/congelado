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
export module io_quic:tls;

import std;
import :types;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
    /**
     * @brief Stands up a server-side `SSL_CTX` wired for QUIC: loads the cert/key pair, pins ALPN
     * to "h3" only, disables session tickets, skips client-cert verification, and locks the
     * proto range to TLS 1.3. This is the whole security posture for every connection born from
     * the resulting context, so get the inputs right.
     * @warning Throws (not an error code) on any setup failure — cert file missing, key mismatch,
     * whatever. No partial/degraded context ever gets handed back; it's all-or-nothing. Also:
     * this hard-requires TLS 1.3 with `SSL_VERIFY_NONE` for client certs — that's a deliberate
     * choice for server-only auth, not a placeholder to "fix later." Don't loosen it without
     * knowing why it's set that way.
     * @param cert_pem path to the PEM-encoded certificate file.
     * @param key_pem path to the PEM-encoded private key file.
     * @return a fully-configured TlsContext ready to spawn TlsSessions from.
     * @throws std::runtime_error if `SSL_CTX_new` fails, or if the cert/key can't be loaded or
     * don't match.
     */
    static TlsContext from_files(std::string_view cert_pem, std::string_view key_pem) {
        // Stand up the raw SSL_CTX first — nothing else here matters if this fails.
        SSL_CTX *ctx = ::SSL_CTX_new(OSSL_QUIC_server_method());
        if (ctx == nullptr) {
            throw std::runtime_error("SSL_CTX_new(OSSL_QUIC_server_method) failed");
        }

        // Certificate + key
        // Load both and verify they actually match, cleaning up the ctx on any failure so
        // nothing half-configured leaks out.
        // Copied into null-terminated strings — string_view::data() is not guaranteed to be
        // null-terminated, and these OpenSSL calls require a C string.
        const std::string CERT_PEM_PATH{cert_pem};
        const std::string KEY_PEM_PATH{key_pem};
        if (::SSL_CTX_use_certificate_file(ctx, CERT_PEM_PATH.c_str(), SSL_FILETYPE_PEM) != 1 ||
            ::SSL_CTX_use_PrivateKey_file(ctx, KEY_PEM_PATH.c_str(), SSL_FILETYPE_PEM) != 1 ||
            ::SSL_CTX_check_private_key(ctx) != 1) {
            ::SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to load cert/key");
        }

        // ALPN: advertise h3 only
        // Wire format: length-prefixed string, e.g. "\x02h3"
        static constexpr unsigned char ALPN_PROTOCOLS[] = {2, 'h', '3'};
        ::SSL_CTX_set_alpn_select_cb(
            ctx,
            [](SSL *, const unsigned char **out, unsigned char *outlen, const unsigned char *input,
               unsigned int inlen, void *) -> int {
                // Only accept the negotiation if the client actually offered "h3" — anything
                // else gets NOACK, no fallback protocol.
                if (::SSL_select_next_proto(const_cast<unsigned char **>(out), outlen, ALPN_PROTOCOLS,  // NOLINT(cppcoreguidelines-pro-type-const-cast) — OpenSSL's ALPN callback signature fixes `out` as `const unsigned char **`, but SSL_select_next_proto() requires a non-const `unsigned char **`; array-to-pointer decay on ALPN_PROTOCOLS is a separate finding out of scope here
                                            sizeof(ALPN_PROTOCOLS), input, inlen) == OPENSSL_NPN_NEGOTIATED) {
                    return SSL_TLSEXT_ERR_OK;
                }
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

    /**
     * @brief Builds an empty, invalid context — no `SSL_CTX` yet. Use from_files() to get a real
     * one.
     */
    TlsContext() = default;

    /**
     * @brief Frees the underlying `SSL_CTX` if this context ever got a real one from from_files().
     */
    ~TlsContext() {
        if (m_ctx != nullptr) {
            ::SSL_CTX_free(m_ctx);
        }
    }

    /**
     * @brief Deleted — `SSL_CTX*` ownership can't be shared, copying would double-free on teardown.
     */
    TlsContext(const TlsContext &) = delete;
    /** @brief Deleted for the same reason as the copy ctor — no shared `SSL_CTX*` ownership. */
    TlsContext &operator=(const TlsContext &) = delete;

    /**
     * @brief Steals `other`'s `SSL_CTX*`, leaving `other` empty/invalid.
     * @param other the context being moved from.
     */
    TlsContext(TlsContext &&other) noexcept : m_ctx(std::exchange(other.m_ctx, nullptr)) {}

    /**
     * @brief Frees this context's existing `SSL_CTX*` (if any), then steals `other`'s.
     * @param other the context being moved from; left empty/invalid after this runs.
     * @return `*this`, now holding `other`'s former `SSL_CTX*`.
     */
    TlsContext &operator=(TlsContext &&other) noexcept {
        // Self-move is a no-op — skip straight to returning if `other` is really `*this`.
        if (this != &other) {
            // Free whatever this context already owned before taking `other`'s.
            if (m_ctx != nullptr) {
                ::SSL_CTX_free(m_ctx);
            }
            m_ctx = std::exchange(other.m_ctx, nullptr);
        }
        return *this;
    }

    /**
     * @brief Gets the raw `SSL_CTX*` to hand to `SSL_new` when spinning up a session.
     * @warning Null if this context was default-constructed and never went through from_files() —
     * check valid() first, or you're handing OpenSSL a null ctx.
     * @return the underlying `SSL_CTX*`, possibly null.
     */
    [[nodiscard]] SSL_CTX *get() const noexcept { return m_ctx; }
    /**
     * @brief Checks whether this context actually holds a live `SSL_CTX*` — lowkey the one gate
     * you should always clear before handing this off to `SSL_new`.
     * @return true if get() would be non-null.
     */
    [[nodiscard]] bool valid() const noexcept { return m_ctx != nullptr; }

  private:
    /**
     * @brief Private wrapping ctor — takes ownership of an already-configured `SSL_CTX*`, used
     * internally by from_files() to hand back the finished product.
     * @param ctx a fully-configured `SSL_CTX*`; ownership transfers here.
     */
    explicit TlsContext(SSL_CTX *ctx) : m_ctx(ctx) {}
    SSL_CTX *m_ctx{nullptr};
};

// ── TlsSession ────────────────────────────────────────────────────────────────

export class TlsSession {
  public:
    /**
     * @brief Spins up a server-side QUIC TLS session on top of `ctx`, binding it to a datagram
     * BIO over `udp_fd` and putting it in accept state — ready to start the handshake.
     * @note Uses a local RAII guard around the freshly-allocated SSL* so a throw mid-construction
     * (e.g. `BIO_new_dgram` failing) still frees it instead of leaking — that guard gets disarmed
     * right before the constructor body finishes, once ownership's safely landed in `m_ssl`.
     * @warning `udp_fd` isn't owned by this session — `BIO_NOCLOSE` means the BIO won't close it,
     * so lifetime management of the fd itself is entirely on the caller. Free the SSL first, close
     * the fd whenever you're actually done with it.
     * @param ctx the TLS context to pull the underlying `SSL_CTX*` from; must be valid() or this
     * throws.
     * @param udp_fd the UDP socket fd this session reads/writes datagrams through.
     * @throws std::runtime_error if `SSL_new` or `BIO_new_dgram` fails.
     */
    TlsSession(TlsContext &ctx, int udp_fd) {
        // Allocate the SSL* off the shared context first — nothing downstream matters without it.
        SSL *ssl = ::SSL_new(ctx.get());
        if (ssl == nullptr) {
            throw std::runtime_error("SSL_new failed");
        }
        // RAII guard in case BIO setup throws
        class Guard {
          public:
            explicit Guard(SSL *ssl) noexcept : m_ssl(ssl) {}
            ~Guard() {
                if (m_ssl != nullptr) {
                    ::SSL_free(m_ssl);
                }
            }
            Guard(const Guard &) = delete;
            Guard &operator=(const Guard &) = delete;
            Guard(Guard &&) = delete;
            Guard &operator=(Guard &&) = delete;

            void disarm() noexcept { m_ssl = nullptr; }

          private:
            SSL *m_ssl;
        } guard{ssl};

        // Datagram BIO — BIO_new_dgram is the correct OpenSSL 3 path for UDP
#ifdef _WIN32
        BIO *bio = ::BIO_new_dgram(static_cast<SOCKET>(udp_fd), BIO_NOCLOSE);
#else
        BIO *bio = ::BIO_new_dgram(udp_fd, BIO_NOCLOSE);
#endif
        if (bio == nullptr) {
            throw std::runtime_error("BIO_new_dgram failed");
        }
        // Same BIO for both read and write sides — that's the datagram-socket contract.
        ::SSL_set_bio(ssl, bio, bio);

        // Server-side session, waiting on the client's ClientHello.
        ::SSL_set_accept_state(ssl);

        // Everything succeeded — ownership lands in m_ssl, so the RAII guard backs off.
        m_ssl = ssl;
        guard.disarm();
    }

    /** @brief Frees the underlying SSL* if this session still owns one — teardown, no cap. */
    ~TlsSession() {
        if (m_ssl != nullptr) {
            ::SSL_free(m_ssl);
        }
    }

    /** @brief Deleted — SSL* ownership can't be shared, copying would double-free on teardown. */
    TlsSession(const TlsSession &) = delete;
    /** @brief Deleted for the same reason as the copy ctor — no shared SSL* ownership. */
    TlsSession &operator=(const TlsSession &) = delete;

    /**
     * @brief Steals `other`'s SSL*, leaving `other` empty.
     * @param other the session being moved from.
     */
    TlsSession(TlsSession &&other) noexcept : m_ssl(std::exchange(other.m_ssl, nullptr)) {}

    /**
     * @brief Frees this session's existing SSL* (if any), then steals `other`'s.
     * @param other the session being moved from; left empty after this runs.
     * @return `*this`, now holding `other`'s former SSL*.
     */
    TlsSession &operator=(TlsSession &&other) noexcept {
        // Same self-move guard as TlsContext's move-assign — free ours, then steal `other`'s.
        if (this != &other) {
            if (m_ssl != nullptr) {
                ::SSL_free(m_ssl);
            }
            m_ssl = std::exchange(other.m_ssl, nullptr);
        }
        return *this;
    }

    /**
     * @brief Gets the raw underlying SSL* for passing to lower-level OpenSSL calls.
     * @return the session's native SSL* handle.
     */
    [[nodiscard]] SSL *get() const noexcept { return m_ssl; }

    // Drive OpenSSL's internal event loop (retransmits, key updates, etc.)
    /**
     * @brief Drives OpenSSL's internal event loop — retransmits, key updates, timers, all the
     * background upkeep QUIC needs even when nobody's actively reading or writing.
     * @note Call this regularly (same cadence as Connection::tick()) or the session stalls —
     * missed retransmits and key updates are silent failures, not loud ones.
     */
    void process() noexcept { ::SSL_handle_events(m_ssl); }

    // Returns true while the handshake is in progress (WANT_READ/WRITE),
    // true on completion, false on fatal error.
    /**
     * @brief Drives the TLS handshake forward one step.
     * @warning The return value conflates two different states — true means either "still going,
     * come back later" (WANT_READ/WANT_WRITE) or "fully done." It does NOT distinguish between
     * them. Don't treat a `true` as "handshake complete" without checking connection/session
     * state separately, or you'll think you're ready to send app data when you're actually still
     * mid-flight.
     * @return true if the handshake is still in progress or just completed; false only on a fatal,
     * non-retryable error.
     */
    bool do_handshake() noexcept {
        int rc = ::SSL_do_handshake(m_ssl);
        // rc == 1 is a clean "fully done" — short-circuit straight to true.
        if (rc == 1) {
            return true;
        }
        // Otherwise the only non-fatal outcomes are "still negotiating" — anything else falls
        // through to false.
        int err = ::SSL_get_error(m_ssl, rc);
        return (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
    }

    // Open a new server-initiated stream.
    // flags: 0 = bidirectional, SSL_STREAM_FLAG_UNI = unidirectional.
    /**
     * @brief Opens a new server-initiated stream on this session — bet, this is the low-level
     * primitive Connection::open_stream() builds its bookkeeping on top of.
     * @param flags 0 for bidirectional (default), `SSL_STREAM_FLAG_UNI` for unidirectional.
     * @return the new stream's SSL*, or null if `SSL_new_stream` failed — caller owns it from
     * here, no auto-tracking like Connection does.
     */
    [[nodiscard]] SSL *create_stream(std::uint64_t flags = 0) noexcept { return ::SSL_new_stream(m_ssl, flags); }

  private:
    SSL *m_ssl{nullptr};
};

} // namespace quic::tls

// TlsSession needs a valid TlsContext (real cert/key files) plus a live UDP fd to construct at
// all — no pure-logic surface separable from that. TlsContext itself is testable at its edges
// without real certs: the default (invalid) state, and from_files()'s failure path when the
// cert/key can't be loaded.
#ifdef CONGELADO_TEST
namespace quic::tls::tests {
using namespace boost::ut;

suite<"TlsContext"> tls_context_suite = [] {
    "default-constructed context is invalid"_test = [] {
        TlsContext ctx;
        expect(not ctx.valid());
        expect(ctx.get() == nullptr);
    };
    "from_files throws when the cert/key can't be loaded"_test = [] {
        expect(throws<std::runtime_error>([] {
            [[maybe_unused]] auto ctx =
                TlsContext::from_files("/nonexistent/cert.pem", "/nonexistent/key.pem");
        }));
    };
};

} // namespace quic::tls::tests
#endif
