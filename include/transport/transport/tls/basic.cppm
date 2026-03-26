module;

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
#include <openssl/err.h>
#include <openssl/ssl.h>

export module tls:basic;

import std;
import tcp;
import :types;

export namespace transport::tls::basic {
/**
 * @brief TCP-specific TLS Connection
 */
class Connection {
  public:
    Connection(tcp::Connection &&tcp_conn, ::SSL *ssl) : m_tcp(std::move(tcp_conn)), m_ssl(ssl) {}

    static Connection upgrade(tcp::Connection &&tcp_conn, ::SSL_CTX *ctx) {
        ::SSL *ssl = ::SSL_new(ctx);
        if (!ssl)
            throw TlsError("SSL_new failed");

        ::SSL_set_fd(ssl, static_cast<int>(tcp_conn.native_fd()));

        if (::SSL_accept(ssl) <= 0) {
            ::SSL_free(ssl);
            throw TlsError("SSL_accept failed");
        }
        return Connection{std::move(tcp_conn), ssl};
    }

    std::ptrdiff_t send(std::span<const std::byte> buf) {
        int n = ::SSL_write(m_ssl, buf.data(), static_cast<int>(buf.size()));
        if (n <= 0)
            throw TlsError("SSL_write failed");
        return n;
    }

    std::ptrdiff_t recv(std::span<std::byte> buf) {
        int n = ::SSL_read(m_ssl, buf.data(), static_cast<int>(buf.size()));
        if (n <= 0) {
            if (::SSL_get_error(m_ssl, n) == SSL_ERROR_ZERO_RETURN)
                return 0;
            throw TlsError("SSL_read failed");
        }
        return n;
    }

    void close() noexcept {
        if (m_ssl) {
            ::SSL_shutdown(m_ssl);
            ::SSL_free(m_ssl);
            m_ssl = nullptr;
        }
        m_tcp.close();
    }

    [[nodiscard]] std::string_view alpn() const noexcept {
        const unsigned char *data{};
        unsigned int len{};
        ::SSL_get0_alpn_selected(m_ssl, &data, &len);
        return {reinterpret_cast<const char *>(data), len};
    }

    ::SSL *get_ssl() const noexcept { return m_ssl; }

  private:
    tcp::Connection m_tcp;
    ::SSL *m_ssl{nullptr};
};

/**
 * @brief TCP TLS Server implementation
 */
class Server {
  public:
    Server(tcp::Server &&server, ::SSL_CTX *ctx) : m_server(std::move(server)), m_ctx(ctx) {}

    static Server listen(std::string_view ip, std::uint16_t port, SslCtx &ctx) {
        return Server{tcp::Server::listen(ip, port), ctx.get()};
    }

    std::optional<Connection> accept() {
        std::optional<tcp::Connection> tcp_conn = m_server.accept();
        if (!tcp_conn.has_value())
            return std::nullopt;

        try {
            return Connection::upgrade(std::move(tcp_conn.value()), m_ctx);
        } catch (...) {
            return std::nullopt;
        }
    }

    void close() noexcept { m_server.close(); }
    [[nodiscard]] bool valid() const noexcept { return m_server.valid(); }

  private:
    tcp::Server m_server;
    ::SSL_CTX *m_ctx;
};
} // namespace transport::tls::basic
