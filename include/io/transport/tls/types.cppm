module;

#include <openssl/err.h>
#include <openssl/ssl.h>

export module io_tls:types;

import std;
import io_tcp;
import io_error;

export namespace transport::base::tls {


/**
 * @brief Managed OpenSSL Context
 */
class SslCtx {
  public:
    explicit SslCtx(const ::SSL_METHOD *method = ::TLS_server_method(), std::filesystem::path dir = ".") {
        m_ctx = ::SSL_CTX_new(method);
        if (!m_ctx)
            throw error::TlsError{"SSL_CTX_new failed"};

        ::SSL_CTX_set_min_proto_version(m_ctx, TLS1_2_VERSION);

        ensure_credentials(dir);
    }

    ~SslCtx() {
        if (m_ctx)
            ::SSL_CTX_free(m_ctx);
    }

    // Logic to ensure certs exist using CLI
    void ensure_credentials(const std::filesystem::path &dir) {
        auto cert = dir / "server.crt";
        auto key = dir / "server.key";

        if (!std::filesystem::exists(cert) || !std::filesystem::exists(key)) {
            std::println(std::clog, "Credentials missing. Generating via openssl CLI...");

            // Modern C++: Use std::format to construct the shell command
            std::string cmd = std::format("openssl req -x509 -newkey rsa:2048 -nodes -keyout \"{}\" -out \"{}\" "
                                          "-days 365 -subj \"/CN=localhost\" 2>/dev/null",
                                          key.string(), cert.string());

            if (std::system(cmd.data()) != 0) {
                throw error::TlsError{"Failed to execute openssl command. Is it installed?"};
            }
        }

        load_file(cert, key);
    }

    void load_file(const std::filesystem::path &cert, const std::filesystem::path &key) {
        if (::SSL_CTX_use_certificate_chain_file(m_ctx, cert.string().data()) != 1)
            throw error::TlsError{"Failed to load generated certificate"};

        if (::SSL_CTX_use_PrivateKey_file(m_ctx, key.string().data(), SSL_FILETYPE_PEM) != 1)
            throw error::TlsError{"Failed to load generated private key"};
    }

    void set_alpn_callback(::SSL_CTX_alpn_select_cb_func cb, void *arg = nullptr) {
        if (!m_ctx)
            throw error::TlsError{"Cannot set ALPN on invalid context"};
        ::SSL_CTX_set_alpn_select_cb(m_ctx, cb, arg);
    }

    // Modern helper: if you want to set the protocols directly as a vector/span
    void set_alpn_protos(std::span<const unsigned char> next_protos) {
        // This is for the client-side or specific server setups
        ::SSL_CTX_set_alpn_protos(m_ctx, next_protos.data(), next_protos.size());
    }

    [[nodiscard]] ::SSL_CTX *get() const noexcept { return m_ctx; }

  private:
    ::SSL_CTX *m_ctx{nullptr};
};

} // namespace transport::base::tls
