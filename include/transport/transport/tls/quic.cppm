module;

#include <openssl/err.h>
#include <openssl/ssl.h>

export module tls:quic;

import std;
import udp;
import :types;

export namespace transport::tls::quic {
/**
 * @brief UDP-specific QUIC Connection
 */
class Connection {
  public:
    explicit Connection(::SSL *ssl) : m_ssl(ssl) {}

    std::ptrdiff_t send(std::span<const std::byte> buf) {
        size_t written = 0;
        if (::SSL_write_ex(m_ssl, buf.data(), buf.size(), &written) <= 0)
            return -1;
        return static_cast<std::ptrdiff_t>(written);
    }

    std::ptrdiff_t recv(std::span<std::byte> buf) {
        size_t read_bytes = 0;
        if (::SSL_read_ex(m_ssl, buf.data(), buf.size(), &read_bytes) <= 0)
            return -1;
        return static_cast<std::ptrdiff_t>(read_bytes);
    }

    void close() noexcept {
        if (m_ssl) {
            ::SSL_free(m_ssl);
            m_ssl = nullptr;
        }
    }

    [[nodiscard]] std::string_view alpn() const noexcept {
        const unsigned char *data{};
        unsigned int len{};
        ::SSL_get0_alpn_selected(m_ssl, &data, &len);
        return {reinterpret_cast<const char *>(data), len};
    }

    ::SSL *get_ssl() const noexcept { return m_ssl; }

  private:
    ::SSL *m_ssl{nullptr};
};

/**
 * @brief QUIC Server implementation
 */
class Server {
  public:
    Server(udp::Server &&udp_srv, ::SSL_CTX *ctx) : m_udp(std::move(udp_srv)), m_ctx(ctx) {}

    static Server listen(std::string_view ip, std::uint16_t port, SslCtx &ctx) {
        auto udp_srv = udp::Server::bind(ip, port);
        udp_srv.set_nonblocking(true);
        return Server{std::move(udp_srv), ctx.get()};
    }

    std::optional<Connection> accept() {
        if (!m_udp.valid())
            return std::nullopt;

        ::SSL *ssl = ::SSL_new(m_ctx);
        if (!ssl)
            return std::nullopt;

        BIO *bio = BIO_new_dgram(static_cast<int>(m_udp.native_fd()), BIO_NOCLOSE);
        ::SSL_set_bio(ssl, bio, bio);

        if (::SSL_accept(ssl) <= 0) {
            ::SSL_free(ssl);
            return std::nullopt;
        }
        return Connection{ssl};
    }

    void close() noexcept { m_udp.close(); }
    [[nodiscard]] bool valid() const noexcept { return m_udp.valid(); }

  private:
    udp::Server m_udp;
    ::SSL_CTX *m_ctx;
};
} // namespace transport::tls::quic
