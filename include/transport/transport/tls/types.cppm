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

class SslCtx {
  public:
    SslCtx() = default;
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

    // Base factory — protocol layers supply the SSL_METHOD and optional ALPN callback.
    // Default method is TLS_server_method() (TLS 1.2+), which is compatible with both h2 and h3.
    static SslCtx from_files(std::string_view cert_pem, std::string_view key_pem,
                             const SSL_METHOD *method = TLS_server_method(),
                             SSL_CTX_alpn_select_cb_func alpn_cb = nullptr);

    [[nodiscard]] SSL_CTX *get() const noexcept { return m_ctx; }
    [[nodiscard]] bool valid() const noexcept { return m_ctx != nullptr; }

  protected:
    explicit SslCtx(SSL_CTX *ctx) : m_ctx(ctx) {}
    SSL_CTX *m_ctx{nullptr};
};

// ── Connection ────────────────────────────────────────────────────────────────
class Connection {
  public:
    static Connection upgrade(transport::tcp::Connection &&tcp, SSL_CTX *ctx);

    Connection() = default;
    ~Connection() { close(); }
    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;
    Connection(Connection &&o) noexcept : m_tcp(std::move(o.m_tcp)), m_ssl(std::exchange(o.m_ssl, nullptr)) {}
    Connection &operator=(Connection &&o) noexcept {
        if (this != &o) {
            close();
            m_tcp = std::move(o.m_tcp);
            m_ssl = std::exchange(o.m_ssl, nullptr);
        }
        return *this;
    }

    [[nodiscard]] std::ptrdiff_t send(std::span<const std::byte> buf);
    [[nodiscard]] std::ptrdiff_t recv(std::span<std::byte> buf);
    void close() noexcept;

    [[nodiscard]] bool valid() const noexcept { return m_ssl != nullptr; }
    [[nodiscard]] std::string peer_addr() const { return m_tcp.peer(); }
    [[nodiscard]] SSL *ssl() const noexcept { return m_ssl; }

    // Negotiated ALPN protocol ("h2", "http/1.1", "h3", …). Empty if none.
    [[nodiscard]] std::string_view alpn() const noexcept {
        const unsigned char *data{};
        unsigned int len{};
        ::SSL_get0_alpn_selected(m_ssl, &data, &len);
        return {reinterpret_cast<const char *>(data), len};
    }

  protected:
    Connection(transport::tcp::Connection &&tcp, SSL *ssl) : m_tcp(std::move(tcp)), m_ssl(ssl) {}

    transport::tcp::Connection m_tcp{};
    SSL *m_ssl{nullptr};
};

// ── Server ────────────────────────────────────────────────────────────────────
class Server {
  public:
    static Server listen(std::string_view ip, std::uint16_t port, SslCtx &ctx, int backlog = 128);
    static Server listen(std::uint16_t port, SslCtx &ctx, int backlog = 128);

    Server() = default;
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) noexcept = default;
    Server &operator=(Server &&) noexcept = default;
    ~Server() = default;

    [[nodiscard]] Connection accept();
    void set_nonblocking(bool on);
    [[nodiscard]] bool valid() const noexcept { return m_tcp.valid(); }
    void close() noexcept { m_tcp.close(); }

  private:
    transport::tcp::Server m_tcp{};
    SSL_CTX *m_ctx{nullptr};
};

} // namespace transport::tls
