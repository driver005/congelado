module;

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif
#include <openssl/err.h>
#include <openssl/ssl.h>

export module tls:tls;

import std;
import tcp;
import :types;

export namespace transport::tls {

SslCtx SslCtx::from_files(std::string_view cert_pem, std::string_view key_pem, const SSL_METHOD *method,
                          SSL_CTX_alpn_select_cb_func alpn_cb) {
    ::SSL_CTX *ctx = ::SSL_CTX_new(method);
    if (!ctx)
        throw TlsError("SSL_CTX_new");

    auto release = [&]() noexcept { ::SSL_CTX_free(ctx); };
    auto check = [&](int ok, std::string_view fn) {
        if (ok != 1) {
            release();
            throw TlsError(fn);
        }
    };

    check(::SSL_CTX_use_certificate_file(ctx, cert_pem.data(), SSL_FILETYPE_PEM), "SSL_CTX_use_certificate_file");
    check(::SSL_CTX_use_PrivateKey_file(ctx, key_pem.data(), SSL_FILETYPE_PEM), "SSL_CTX_use_PrivateKey_file");
    check(::SSL_CTX_check_private_key(ctx), "SSL_CTX_check_private_key");

    ::SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    if (alpn_cb)
        ::SSL_CTX_set_alpn_select_cb(ctx, alpn_cb, nullptr);

    return SslCtx{ctx};
}

Connection Connection::upgrade(transport::tcp::Connection &&tcp, SSL_CTX *ctx) {
    SSL *ssl = ::SSL_new(ctx);
    if (!ssl)
        throw TlsError("SSL_new");

    // native_fd() is int on POSIX, SOCKET (UINT_PTR) on Win32.
    // SSL_set_fd takes int; cast is safe for all real socket values.
    if (::SSL_set_fd(ssl, static_cast<int>(tcp.native_fd())) != 1) {
        ::SSL_free(ssl);
        throw TlsError("SSL_set_fd");
    }
    if (::SSL_accept(ssl) != 1) {
        ::SSL_free(ssl);
        throw TlsError("SSL_accept");
    }
    return Connection{std::move(tcp), ssl};
}

std::ptrdiff_t Connection::send(std::span<const std::byte> buf) {
    int n = ::SSL_write(m_ssl, reinterpret_cast<const char *>(buf.data()), static_cast<int>(buf.size()));
    if (n <= 0)
        throw TlsError("SSL_write");
    return static_cast<std::ptrdiff_t>(n);
}

std::ptrdiff_t Connection::recv(std::span<std::byte> buf) {
    int n = ::SSL_read(m_ssl, reinterpret_cast<char *>(buf.data()), static_cast<int>(buf.size()));
    if (n <= 0)
        throw TlsError("SSL_read");
    return static_cast<std::ptrdiff_t>(n);
}

void Connection::close() noexcept {
    if (m_ssl) {
        ::SSL_shutdown(m_ssl);
        ::SSL_free(m_ssl);
        m_ssl = nullptr;
    }
    m_tcp.close();
}

Server Server::listen(std::string_view ip, std::uint16_t port, SslCtx &ctx, int backlog) {
    Server s;
    s.m_tcp = transport::tcp::Server::listen(ip, port, backlog);
    s.m_ctx = ctx.get();
    return s;
}
Server Server::listen(std::uint16_t port, SslCtx &ctx, int backlog) { return listen("0.0.0.0", port, ctx, backlog); }
Connection Server::accept() {
    auto tcp = m_tcp.accept();
    tcp.set_nodelay(true);
    return Connection::upgrade(std::move(tcp), m_ctx);
}
void Server::set_nonblocking(bool on) { m_tcp.set_nonblocking(on); }

} // namespace transport::tls
