module;

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

export module io_tls:basic;

import std;
import io_tcp;
import io_io;
import io_error;
import :types;

export namespace io::base::tls::basic {

class Connection {
  public:
    Connection(tcp::Connection &&tcp_conn, ::SSL *ssl, ::BIO *rbio, ::BIO *wbio)
        : m_tcp(std::move(tcp_conn)), m_ssl(ssl), m_rbio(rbio), m_wbio(wbio) {}

    ~Connection() { close(); }

    Connection(Connection &&) noexcept = default;
    Connection &operator=(Connection &&) noexcept = default;
    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    // ── Upgrade (server-side TLS handshake) ──────────────────────────────────

    static Connection upgrade(tcp::Connection &&tcp_conn, ::SSL_CTX *ctx, ::io::base::io::PlatformIO &io) {
        ::BIO *rbio = ::BIO_new(::BIO_s_mem());
        ::BIO *wbio = ::BIO_new(::BIO_s_mem());
        if (!rbio || !wbio) {
            if (rbio)
                ::BIO_free(rbio);
            if (wbio)
                ::BIO_free(wbio);
            throw error::TlsError{"BIO_new failed"};
        }
        ::SSL *ssl = ::SSL_new(ctx);
        if (!ssl) {
            ::BIO_free(rbio);
            ::BIO_free(wbio);
            throw error::TlsError{"SSL_new failed"};
        }
        ::SSL_set_bio(ssl, rbio, wbio); // SSL takes ownership of both BIOs
        ::SSL_set_accept_state(ssl);

        Connection conn{std::move(tcp_conn), ssl, rbio, wbio};
        conn.pump_handshake(io);
        return conn;
    }

    // ── Plaintext send ────────────────────────────────────────────────────────

    std::ptrdiff_t send(std::span<const std::byte> buf, io::PlatformIO &io) {
        const int n = ::SSL_write(m_ssl, buf.data(), static_cast<int>(buf.size()));
        if (n <= 0)
            throw error::TlsError{"SSL_write failed"};
        flush_wbio(io);
        return n;
    }

    // ── Plaintext recv ────────────────────────────────────────────────────────

    std::ptrdiff_t recv(std::span<std::byte> buf, io::PlatformIO &io) {
        int n = ::SSL_read(m_ssl, buf.data(), static_cast<int>(buf.size()));
        if (n > 0)
            return n;

        const int err = ::SSL_get_error(m_ssl, n);
        if (err == SSL_ERROR_ZERO_RETURN)
            return 0;
        if (err != SSL_ERROR_WANT_READ)
            throw error::TlsError{"SSL_read failed"};

        feed_rbio(io);

        n = ::SSL_read(m_ssl, buf.data(), static_cast<int>(buf.size()));
        if (n > 0)
            return n;
        if (::SSL_get_error(m_ssl, n) == SSL_ERROR_ZERO_RETURN)
            return 0;
        throw error::TlsError{"SSL_read failed after feed"};
    }

    void close() noexcept {
        if (m_ssl) {
            ::SSL_shutdown(m_ssl);
            ::SSL_free(m_ssl); // also frees both BIOs
            m_ssl = nullptr;
            m_rbio = nullptr;
            m_wbio = nullptr;
        }
        m_tcp.close();
    }

    [[nodiscard]] std::string_view alpn() const noexcept {
        const unsigned char *data{};
        unsigned int len{};
        ::SSL_get0_alpn_selected(m_ssl, &data, &len);
        return {reinterpret_cast<const char *>(data), len};
    }

    [[nodiscard]] ::SSL *get_ssl() const noexcept { return m_ssl; }
    [[nodiscard]] bool valid() const noexcept { return m_tcp.valid(); }

  private:
    void pump_handshake(io::PlatformIO &io) {
        while (true) {
            const int rc = ::SSL_do_handshake(m_ssl);
            const int err = ::SSL_get_error(m_ssl, rc);
            flush_wbio(io);
            if (rc == 1)
                return;
            if (err == SSL_ERROR_WANT_READ) {
                feed_rbio(io);
                continue;
            }
            throw error::TlsError{"SSL_do_handshake failed"};
        }
    }

    void flush_wbio(io::PlatformIO &io) {
        const int fd = static_cast<int>(m_tcp.native_fd());
        char tmp[16384];
        int n;
        while ((n = ::BIO_read(m_wbio, tmp, sizeof(tmp))) > 0) {
            auto &state = io.platform_state();
            if (!state.buffer) [[unlikely]]
                throw error::TlsError{"IO buffer not initialized"};

            auto &rb = *state.buffer;
            auto span = rb.get_writable_span();
            const auto bytes = static_cast<std::size_t>(n);
            const auto to_copy = std::min(bytes, span.size());
            std::memcpy(span.data(), tmp, to_copy);
            rb.commit_write(to_copy);
            io.submit_write(fd, bytes);
            io.submit();
            io.wait_completions(1);
        }
    }

    void feed_rbio(io::PlatformIO &io) {
        const int fd = static_cast<int>(m_tcp.native_fd());
        io.submit_recv(fd, 16384);
        io.submit();
        for (const auto &ev : io.wait_completions(1)) {
            if (io::tag_kind(ev.tag) != io::OpCode::RECV || ev.result <= 0)
                continue;
            auto &state = io.platform_state();
            if (!state.buffer) [[unlikely]]
                throw error::TlsError{"IO buffer not initialized"};

            auto &rb = *state.buffer;
            auto span = rb.get_readable_span();
            ::BIO_write(m_rbio, span.data(),
                        static_cast<int>(std::min(span.size(), static_cast<std::size_t>(ev.result))));
            rb.advance_read(static_cast<std::size_t>(ev.result));
        }
    }

    tcp::Connection m_tcp;
    ::SSL *m_ssl;
    ::BIO *m_rbio;
    ::BIO *m_wbio;
};

class Server {
  public:
    Server(tcp::Server &&server, ::SSL_CTX *ctx) : m_server(std::move(server)), m_ctx(ctx) {}

    static Server listen(std::string_view ip, std::uint16_t port, SslCtx &ctx) {
        return Server{tcp::Server::listen(ip, port), ctx.get()};
    }

    std::optional<Connection> accept(io::PlatformIO &io) {
        auto tcp_conn = m_server.accept();

        try {
            return Connection::upgrade(std::move(tcp_conn), m_ctx, io);
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

} // namespace io::base::tls::basic
