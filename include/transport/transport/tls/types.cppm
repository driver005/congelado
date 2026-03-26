module;

#include <openssl/err.h>
#include <openssl/ssl.h>

export module tls:types;

import std;
import tcp;

export namespace transport::tls {

struct TlsError : std::runtime_error {
    explicit TlsError(std::string_view ctx) : std::runtime_error(make_msg(ctx)) {}

  private:
    static std::string make_msg(std::string_view ctx) {
        char buf[256];
        ::ERR_error_string_n(::ERR_get_error(), buf, sizeof(buf));
        return std::string(ctx) + ": " + buf;
    }
};


/**
 * @brief Managed OpenSSL Context
 */
class SslCtx {
  public:
    SslCtx() = default;
    explicit SslCtx(::SSL_CTX *ctx) : m_ctx(ctx) {}
    ~SslCtx() {
        if (m_ctx)
            ::SSL_CTX_free(m_ctx);
    }

    SslCtx(const SslCtx &) = delete;
    SslCtx &operator=(const SslCtx &) = delete;
    SslCtx(SslCtx &&o) noexcept : m_ctx(std::exchange(o.m_ctx, nullptr)) {}

    SslCtx &operator=(SslCtx &&o) noexcept {
        if (this != &o) {
            if (m_ctx)
                ::SSL_CTX_free(m_ctx);
            m_ctx = std::exchange(o.m_ctx, nullptr);
        }
        return *this;
    }

    static SslCtx from_files(std::string_view cert, std::string_view key,
                             const ::SSL_METHOD *method = ::TLS_server_method(),
                             ::SSL_CTX_alpn_select_cb_func alpn_cb = nullptr) {
        ::SSL_CTX *ctx = ::SSL_CTX_new(method);
        if (!ctx)
            throw TlsError("SSL_CTX_new");

        if (::SSL_CTX_use_certificate_chain_file(ctx, cert.data()) != 1) {
            ::SSL_CTX_free(ctx);
            throw TlsError("SSL_CTX_use_certificate_chain_file");
        }
        if (::SSL_CTX_use_PrivateKey_file(ctx, key.data(), SSL_FILETYPE_PEM) != 1) {
            ::SSL_CTX_free(ctx);
            throw TlsError("SSL_CTX_use_PrivateKey_file");
        }

        ::SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        if (alpn_cb)
            ::SSL_CTX_set_alpn_select_cb(ctx, alpn_cb, nullptr);

        return SslCtx{ctx};
    }

    [[nodiscard]] ::SSL_CTX *get() const {
        if (!m_ctx)
            throw TlsError("SslCtx::get() called on invalid context");
        return m_ctx;
    }

    [[nodiscard]] bool valid() const noexcept { return m_ctx != nullptr; }

  private:
    ::SSL_CTX *m_ctx{nullptr};
};

} // namespace transport::tls
