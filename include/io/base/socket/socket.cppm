module;

#include <cassert>
#include <cstdio>
#include <openssl/err.h>
#include <openssl/quic.h>
#include <openssl/ssl.h>

#if defined(_WIN32)

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#endif

export module io_base_socket;

import std;
import io_error;
import :consts;

#if defined(_WIN32)
export import :win32;
#else
export import :posix;
#endif

export namespace transport::base::socket {

enum class Protocol { TCP = 0, UDP = 1, TLS = 2, QUIC = 3 };

class AddressInfo {
  public:
    AddressInfo() : m_adrinf{}, m_sock_size{0} {}

    void set(const sockaddr_storage &adrinf, socklen_t &sock_size) {
        m_adrinf = adrinf;
        m_sock_size = sock_size;
    }

    void set_data(const sockaddr_storage &adrinf) { m_adrinf = adrinf; }
    void set_size(const socklen_t &sock_size) { m_sock_size = sock_size; }

    const sockaddr_storage &get_data() const noexcept { return m_adrinf; }
    const socklen_t &get_size() const noexcept { return m_sock_size; }

  private:
    sockaddr_storage m_adrinf;
    socklen_t m_sock_size;
};

enum class Event : int { READ = 0x1, WRITE = 0x2, EXCEPT = 0x4 };

class Endpoint {
  public:
    Endpoint() : m_address{}, m_port{0} {}

    Endpoint(std::string_view address, std::uint16_t port) : m_address(std::move(address)), m_port(port) {}

    Endpoint(std::string_view address) {
        const auto separator = address.find_last_of(':');

        if (separator == std::string_view::npos)
            error::handle_error<>("Endpoint: invalid address format");
        if (separator == address.size() - 1)
            error::handle_error<>("Endpoint: missing port number");

        m_address = address.substr(0, separator);

        std::string_view port_view = address.substr(separator + 1);
        std::uint16_t parsed_port = 0;

        auto [ptr, ec] = std::from_chars(port_view.data(), port_view.data() + port_view.size(), parsed_port);

        if (ec != std::errc{}) {
            if (ec == std::errc::invalid_argument) {
                error::handle_error("Invalid port format");
            } else if (ec == std::errc::result_out_of_range) {
                error::handle_error("Port number too large (max 65535)");
            }

            error::handle_error("Failed to parse port number");
        }

        m_port = parsed_port;
    }

    Endpoint(const SOCKADDR *address) {
        if (!address) {
            error::handle_error("Null address passed to Endpoint");
            return;
        }

        switch (address->sa_family) {
        case AF_INET: {
            auto addr = (SOCKADDR_IN *)address;
            m_address = inet_ntoa(addr->sin_addr);
            m_port = ntohs(addr->sin_port);
            break;
        }
        case AF_INET6: {
            auto addr = (sockaddr_in6 *)address;
            char buf[INET6_ADDRSTRLEN];
            m_address = inet_ntop(AF_INET6, &addr->sin6_addr, buf, sizeof(buf));
            m_port = ntohs(addr->sin6_port);
            break;
        }
        default: {
            error::handle_error("Unsupported address family");
        }
        }

        if (m_address.empty()) {
            error::handle_error("Failed to convert address to string");
        }
    }

    const std::string &get_address() const noexcept { return m_address; }
    const std::uint16_t &get_port() const noexcept { return m_port; }

  private:
    std::string m_address;
    std::uint16_t m_port;
};

enum class VALUES : std::int8_t {
    ERRORED = 0x0,
    VALID = 0x1,
    CLEANLY_DISCONNECTED = 0x2,
    NON_BLOCKING_WOULD_HAVE_BLOCKED = 0x3,
    TIMED_OUT = 0x4
};

class SocketStatus {
  public:
    SocketStatus() : m_value(VALUES::ERRORED) {}
    SocketStatus(bool is_valid) : m_value(is_valid ? VALUES::VALID : VALUES::ERRORED) {}
    SocketStatus(VALUES value) : m_value(value) {}

    SocketStatus(const SocketStatus &) = default;
    SocketStatus(SocketStatus &&) = default;

    operator bool() const noexcept { return std::to_underlying(m_value) > 0; }
    std::int8_t get_value() const noexcept { return std::to_underlying(m_value); }
    bool operator==(VALUES val) const noexcept { return m_value == val; }

    bool is_valid() const noexcept { return m_value == VALUES::VALID; }
    bool is_errored() const noexcept { return m_value == VALUES::ERRORED; }
    bool is_cleanly_disconnected() const noexcept { return m_value == VALUES::CLEANLY_DISCONNECTED; }
    bool would_have_blocked() const noexcept { return m_value == VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED; }
    bool is_timed_out() const noexcept { return m_value == VALUES::TIMED_OUT; }

  private:
    VALUES m_value;
};


class InitializeSSL {
  public:
    InitializeSSL() {
        OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
        OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CONFIG | OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS,
                            nullptr);
    };
};

template <Protocol protocol, bool RootSocket = true>
class Socket {
  public:
    Socket()
        : m_socket{INVALID_SOCKET}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0} {}

    Socket(const Endpoint &endpoint)
        : m_socket{INVALID_SOCKET}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{std::move(endpoint)},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_os{} {

        init_address_info();

        // Don't use AI_ADDRCONFIG if connecting to loopback
        // See https://fedoraproject.org/wiki/QA/Networking/NameResolution/ADDRCONFIG
        const std::string_view address = m_endpoint.get_address();
        if (address == "localhost" || address == "localhost.localdomain" || address == "localhost6" ||
            address == "localhost6.localdomain6" || address == "127.0.0.1" || address == "::1") {
            m_address_info_hint.ai_flags = 0;
        }

        if (getaddrinfo(address.data(), std::to_string(m_endpoint.get_port()).data(), &m_address_info_hint,
                        &m_address_info_result) != 0) {
            error::handle_error("Failed to resolve address");
        }

        for (addrinfo *addr = m_address_info_result; addr != nullptr; addr = addr->ai_next) {
            m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (m_socket != INVALID_SOCKET) {
                m_socket_address_info = addr;
                break;
            }
        }

        if (m_socket == INVALID_SOCKET) {
            error::handle_error("Failed to create socket");
        }
    }

    Socket(SOCKET nativ, Endpoint endpoint)
        : m_socket{nativ}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{std::move(endpoint)},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_os{} {
        init_address_info();
    }

    Socket(SOCKET nativ, SSL *ssl, Endpoint endpoint)
        : m_socket{nativ}, m_ssl{ssl}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{std::move(endpoint)},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_os{} {
        init_address_info();
    }

    Socket(SOCKET nativ, SSL *ssl)
        : m_socket{nativ}, m_ssl{ssl}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{nullptr}, m_address_info_hint{},
          m_address_info_result{nullptr}, m_socket_address_info{nullptr}, m_socket_input_buffer{},
          m_socket_input_buffer_length{0}, m_os{} {
        init_address_info();
    }

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;

    Socket(Socket &&other) noexcept
        : m_socket{std::move(other.m_socket)}, m_ssl{std::move(other.m_ssl)}, m_ssl_ctx{std::move(other.m_ssl_ctx)},
          m_bio{std::move(other.m_bio)}, m_endpoint{std::move(other.m_endpoint)},
          m_address_info_hint{std::move(other.m_address_info_hint)},
          m_address_info_result{std::move(other.m_address_info_result)},
          m_socket_address_info{std::move(other.m_socket_address_info)},
          m_socket_input_buffer{std::move(other.m_socket_input_buffer)},
          m_socket_input_buffer_length{std::move(other.m_socket_input_buffer_length)}, m_os{std::move(other.m_os)} {
        other.m_socket = INVALID_SOCKET;
        other.m_ssl = nullptr;
        other.m_ssl_ctx = nullptr;
        other.m_bio = nullptr;
        other.m_address_info_result = nullptr;
        other.m_socket_address_info = nullptr;
    }

    Socket &operator=(Socket &&other) noexcept {
        if (this != &other) {
            m_socket = std::move(other.m_socket);
            m_ssl = std::move(other.m_ssl);
            m_ssl_ctx = std::move(other.m_ssl_ctx);
            m_bio = std::move(other.m_bio);
            m_endpoint = std::move(other.m_endpoint);
            m_address_info_hint = std::move(other.m_address_info_hint);
            m_address_info_result = std::move(other.m_address_info_result);
            m_socket_address_info = std::move(other.m_socket_address_info);
            m_socket_input_buffer = std::move(other.m_socket_input_buffer);
            m_socket_input_buffer_length = std::move(other.m_socket_input_buffer_length);
            m_os = std::move(other.m_os);

            other.m_socket = INVALID_SOCKET;
            other.m_ssl = nullptr;
            other.m_ssl_ctx = nullptr;
            other.m_bio = nullptr;
            other.m_address_info_result = nullptr;
            other.m_socket_address_info = nullptr;
        }
        return *this;
    }

    ~Socket() {
        if constexpr (protocol != Protocol::QUIC || RootSocket) {
            close();
            if (m_address_info_result) {
                freeaddrinfo(m_address_info_result);
            }
        }
    }

    void set_non_blocking(bool non_blocking = true) const { set_non_blocking_impl(m_socket, non_blocking); }

    void set_reuse_address(bool reuse = true) const {
        int optval = reuse ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&optval), sizeof(optval)) != 0) {
            error::handle_error("Failed to set SO_REUSEADDR");
        }
    }


    void set_broadcast(bool broadcast = true) const {
        int optval = broadcast ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&optval), sizeof(optval)) != 0) {
            error::handle_error("Failed to set SO_BROADCAST");
        }
    }

    void set_tcp_no_delay(bool no_delay = true) const {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            int optval = no_delay ? 1 : 0;
            if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&optval), sizeof(optval)) !=
                0) {
                error::handle_error("Failed to set TCP_NODELAY");
            }
        }
    }

    SocketStatus get_status() const {
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &len) != 0) {
            error::handle_error("Failed to get socket status");
        }
    }

    bool load_certificate(const char *cert_file, const char *key_file) {
        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (!m_ssl_ctx)
                return false;
            if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx, cert_file) != 1) {
                error::handle_error("Failed to load certificate chain from file");
                return false;
            }
            if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key_file, SSL_FILETYPE_PEM) != 1) {
                error::handle_error("Failed to load private key from file");
                return false;
            }
            if (SSL_CTX_check_private_key(m_ssl_ctx) != 1) {
                error::handle_error("Private key does not match the certificate public key");
                return false;
            }
            return true;
        }
        return false;
    }

    void bind() {
        if (m_socket_address_info == nullptr) {
            error::handle_error("No valid address info to bind to");
        }

        if (::bind(m_socket, m_socket_address_info->ai_addr,
                   static_cast<socklen_t>(m_socket_address_info->ai_addrlen)) == SOCKET_ERROR) {
            error::handle_error("Failed to bind socket");
        }

        if constexpr (protocol == Protocol::TLS) {
            m_ssl_ctx = SSL_CTX_new(TLS_server_method());
            if (!m_ssl_ctx) {
                error::handle_error("Failed to create SSL context");
            }

            SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
            SSL_CTX_set_default_verify_paths(m_ssl_ctx);
        } else if constexpr (protocol == Protocol::QUIC) {
            add_quic_bio();
        }
    }

    void join(const Endpoint &endpoint, std::string_view group = "") {
        if constexpr (protocol != Protocol::UDP) {
            error::handle_error("Joining multicast groups is only supported for UDP protocol");
        }

        addrinfo *multicast_addr_info;
        addrinfo *local_addr_info;
        addrinfo hints = {};
        hints.ai_family = PF_UNSPEC;
        hints.ai_flags = AI_NUMERICHOST;
        if (getaddrinfo(endpoint.get_address().data(), nullptr, &hints, &multicast_addr_info) != 0) {
            error::handle_error("Failed to resolve multicast group address");
        }

        hints.ai_family = multicast_addr_info->ai_family;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;
        if (getaddrinfo(nullptr, std::to_string(endpoint.get_port()).data(), &hints, &local_addr_info) != 0) {
            error::handle_error("Failed to resolve local address for multicast");
        }

        m_socket = ::socket(local_addr_info->ai_family, local_addr_info->ai_socktype, local_addr_info->ai_protocol);
        if (m_socket == INVALID_SOCKET) {
            error::handle_error("Failed to create socket for multicast");
        } else {
            m_socket_address_info = local_addr_info;
        }

        bind();

        if (multicast_addr_info->ai_family == AF_INET &&
            multicast_addr_info->ai_addrlen == sizeof(struct sockaddr_in)) {
            struct ip_mreq mreq;

            std::memcpy(&mreq.imr_multiaddr, &((struct sockaddr_in *)multicast_addr_info->ai_addr)->sin_addr,
                        sizeof(mreq.imr_multiaddr));

            if (group.empty()) {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            } else {
                mreq.imr_interface.s_addr = inet_addr(group.data());
            }

            if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char *>(&mreq), sizeof(mreq)) !=
                0) {
                error::handle_error("Failed to join multicast group");
            }
        } else if (multicast_addr_info->ai_family == AF_INET6 &&
                   multicast_addr_info->ai_addrlen == sizeof(struct sockaddr_in6)) {
            struct ipv6_mreq mreq6;

            std::memcpy(&mreq6.ipv6mr_multiaddr, &((struct sockaddr_in6 *)multicast_addr_info->ai_addr)->sin6_addr,
                        sizeof(mreq6.ipv6mr_multiaddr));

            if (group.empty()) {
                mreq6.ipv6mr_interface = 0;
            } else {
                struct addrinfo *group_addr_info;
                if (getaddrinfo(group.data(), nullptr, nullptr, &group_addr_info) != 0) {
                    error::handle_error("Failed to resolve group address for multicast");
                }

                mreq6.ipv6mr_interface = ((sockaddr_in6 *)group_addr_info->ai_addr)->sin6_scope_id;
                freeaddrinfo(group_addr_info);
            }

            if (setsockopt(m_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char *>(&mreq6), sizeof(mreq6)) !=
                0) {
                error::handle_error("Failed to join multicast group");
            }
        } else {
            error::handle_error("Unsupported address family for multicast");
        }

        freeaddrinfo(multicast_addr_info);
    }

    SocketStatus connect(std::uint64_t timeout = 0) {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            auto current = m_socket_address_info;
            if (connect<false>(current, timeout) != SocketStatus(VALUES::VALID)) {
                for (auto addr = m_address_info_result->ai_next; addr != nullptr; addr = addr->ai_next) {
                    // Already checked this one, skip it
                    if (addr == current) {
                        continue;
                    }
                    if (connect<true>(addr, timeout) == SocketStatus(VALUES::VALID)) {
                        break;
                    }
                }
            }

            if (m_socket == INVALID_SOCKET) {
                error::handle_error("Failed to connect to any resolved address");
            }

            if constexpr (protocol == Protocol::TLS) {
                m_ssl_ctx = SSL_CTX_new(TLS_client_method());
                if (!m_ssl_ctx) {
                    return SocketStatus(VALUES::ERRORED);
                }

                SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, nullptr);
                SSL_CTX_set_default_verify_paths(m_ssl_ctx);

                m_ssl = SSL_new(m_ssl_ctx);
                if (!m_ssl) {
                    return SocketStatus(VALUES::ERRORED);
                }

                if (!SSL_set_fd(m_ssl, m_socket)) {
                    return SocketStatus(VALUES::ERRORED);
                }

                if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.get_address().data()) != 1) {
                    return SocketStatus(VALUES::ERRORED);
                }

                SSL_set_connect_state(m_ssl);
            } else if constexpr (protocol == Protocol::QUIC) {
                add_quic_bio();

                m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_client_method());
                if (!m_ssl_ctx) {
                    return SocketStatus(VALUES::ERRORED);
                }

                SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, nullptr);
                SSL_CTX_set_default_verify_paths(m_ssl_ctx);


                m_ssl = SSL_new(m_ssl_ctx);
                if (!m_ssl) {
                    return SocketStatus(VALUES::ERRORED);
                }

                SSL_set_bio(m_ssl, m_bio, m_bio);

                if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.get_address().data()) != 1) {
                    return SocketStatus(VALUES::ERRORED);
                }

                SSL_set_connect_state(m_ssl);
            }

            return SocketStatus(VALUES::VALID);
        } else {
            error::handle_error("Connect is only supported for TCP, TLS, QUIC protocols");
            return SocketStatus(VALUES::ERRORED);
        }
    }

    void listen() {
        if constexpr (protocol == Protocol::TCP && protocol == Protocol::TLS) {
            if (::listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
                error::handle_error("Failed to listen on socket");
            } else {
                error::handle_error("Listen is only supported for TCP and TLS protocols");
            }
        } else if constexpr (protocol == Protocol::QUIC) {
            if (!m_bio) {
                error::handle_error("BIO must be initialized before listening for QUIC");
            }

            m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_server_method());
            if (!m_ssl_ctx) {
                error::handle_error("Failed to create SSL context for QUIC");
            }

            m_ssl = SSL_new(m_ssl_ctx);
            if (!m_ssl) {
                error::handle_error("Failed to create SSL object for QUIC");
            }

            SSL_set_bio(m_ssl, m_bio, m_bio);
            SSL_set_accept_state(m_ssl);
        }
    }

    /// Call this after connect() or after accept() returns a socket, each time
    /// the completion queue signals the fd is ready. Returns:
    ///   socket_status::valid                       — handshake complete, ready to send/recv
    ///   socket_status::non_blocking_would_have_blocked — not done yet; re-arm the fd
    ///     *wait_for_write is set to tell you which direction to wait on:
    ///       false → wait for the fd to be readable  (EPOLLIN / IOCP recv)
    ///       true  → wait for the fd to be writable  (EPOLLOUT / IOCP send)
    ///   socket_status::errored                     — handshake failed, close the socket
    ///
    /// For plain tcp sockets this is a no-op and always returns valid.
    SocketStatus handshake(bool *wait_for_write = nullptr) noexcept {
        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (!m_ssl) {
                return SocketStatus(VALUES::ERRORED);
            }

            int ret = SSL_do_handshake(m_ssl);
            if (ret == 1) {
                return SocketStatus(VALUES::VALID);
            }

            int err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_READ) {
                if (wait_for_write) {
                    *wait_for_write = false;
                }
                return SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            if (err == SSL_ERROR_WANT_WRITE) {
                if (wait_for_write) {
                    *wait_for_write = true;
                }
                return SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            return SocketStatus(VALUES::ERRORED);
        } else {
            return SocketStatus(VALUES::VALID);
        }
    }

    bool is_handshake_done() const noexcept {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::QUIC) {
            return m_ssl && SSL_is_init_finished(m_ssl);
        }
        return true;
    }

    Socket accept() const {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            sockaddr_storage client_addr{};
            socklen_t addr_len = sizeof(client_addr);

            SOCKET client_fd = ::accept(m_socket, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);
            if (client_fd == INVALID_SOCKET) {
                const auto err = get_error_code();

                if (err == EWOULDBLOCK || err == EAGAIN || err == EINTR) {
                    return Socket<protocol, false>{};
                }

                error::handle_error("Critical failure in accept() syscall");
            }

            if constexpr (protocol == Protocol::TLS) {
                SSL *client_ssl = SSL_new(m_ssl_ctx);
                if (!client_ssl) {
                    closesocket(client_fd);
                    error::handle_error("Failed to create SSL object for accepted connection");
                }

                if (!SSL_set_fd(client_ssl, client_fd)) {
                    SSL_free(client_ssl);
                    closesocket(client_fd);
                    error::handle_error("Failed to associate SSL object with accepted socket");
                }

                SSL_set_accept_state(client_ssl);

                return Socket<protocol, false>{client_fd, client_ssl,
                                               Endpoint(reinterpret_cast<sockaddr *>(&client_addr))};
            }

            return Socket<protocol, true>{client_fd, Endpoint(reinterpret_cast<sockaddr *>(&client_addr))};
        } else if constexpr (protocol == Protocol::QUIC) {
            if (!m_ssl) {
                return Socket<protocol, false>{INVALID_SOCKET};
            }

            SSL *client_ssl = SSL_accept_connection(m_ssl, 0);
            if (!client_ssl) {
                return Socket<protocol, false>{INVALID_SOCKET};
            }

            return Socket<protocol, false>{m_socket, client_ssl};

        } else {
            error::handle_error("Accept is only supported for TCP, TLS and QUIC protocols");
            return Socket<protocol, false>{INVALID_SOCKET};
        }
    }


    SocketStatus select(int fd, std::uint64_t timeout_ms) const {
        fd_set readfds, writefds, exceptfds;
        fd_set *p_read, *p_write, *p_except = nullptr;

        if (fd & std::to_underlying(Event::READ)) {
            FD_ZERO(&readfds);
            FD_SET(m_socket, &readfds);
            p_read = &readfds;
        }
        if (fd & std::to_underlying(Event::WRITE)) {
            FD_ZERO(&writefds);
            FD_SET(m_socket, &writefds);
            p_write = &writefds;
        }
        if (fd & std::to_underlying(Event::EXCEPT)) {
            FD_ZERO(&exceptfds);
            FD_SET(m_socket, &exceptfds);
            p_except = &exceptfds;
        }

        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int result = ::select(static_cast<int>(m_socket + 1), p_read, p_write, p_except, &tv);
        if (result < 0) {
            const auto err = get_error_code();
            if (err == EINTR || err == EAGAIN || err == EWOULDBLOCK) {
                return SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            error::handle_error("Critical failure in select() syscall");
            return SocketStatus(VALUES::ERRORED);
        } else if (result == 0) {
            return SocketStatus(VALUES::TIMED_OUT);
        }

        return SocketStatus(VALUES::VALID);
    }

    template <std::size_t BufferSize>
    std::pair<std::size_t, SocketStatus> send(const std::array<std::byte, BufferSize> &buffer,
                                              const std::size_t length = BufferSize,
                                              AddressInfo *addr = nullptr) const {
        assert(length <= BufferSize);
        return send(buffer.data(), length, addr);
    }

    std::pair<std::size_t, SocketStatus> send(const std::byte *buffer, const std::size_t length,
                                              AddressInfo *addr = nullptr) const {
        std::size_t sent_bytes{0};
        int ssl_call_result{0};

        if constexpr (protocol == Protocol::TCP) {
            sent_bytes = ::send(m_socket, reinterpret_cast<const char *>(buffer), static_cast<buffsize_t>(length), 0);
        } else if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            ssl_call_result = SSL_write_ex(m_ssl, reinterpret_cast<const void *>(buffer), length, &sent_bytes);
        } else if constexpr (protocol == Protocol::UDP) {
            if (addr) {
                sent_bytes = ::sendto(m_socket, reinterpret_cast<const char *>(buffer), static_cast<buffsize_t>(length),
                                      0, reinterpret_cast<const sockaddr *>(&addr->get_data()), addr->get_size());
            } else {
                sent_bytes = ::sendto(m_socket, reinterpret_cast<const char *>(buffer), static_cast<buffsize_t>(length),
                                      0, static_cast<sockaddr *>(m_socket_address_info->ai_addr),
                                      static_cast<socklen_t>(m_socket_address_info->ai_addrlen));
            }
        }

        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (ssl_call_result <= 0) {
                const int ssl_err = SSL_get_error(m_ssl, ssl_call_result);

                switch (ssl_err) {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));

                case SSL_ERROR_ZERO_RETURN:
                    return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));

                case SSL_ERROR_SYSCALL: {
                    if (get_error_code() == EWOULDBLOCK) {
                        return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                    }

                    return std::make_pair(0, SocketStatus(VALUES::ERRORED));
                }

                default:
                    return std::make_pair(0, SocketStatus(VALUES::ERRORED));
                }
            }
        } else {
            if (sent_bytes < 0) {
                const auto err = get_error_code();
                if (err == EWOULDBLOCK) {
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                return std::make_pair(0, SocketStatus(VALUES::ERRORED));
            }
        }

        return std::make_pair(sent_bytes, SocketStatus(VALUES::VALID));
    }


    template <std::output_iterator<std::byte> Out, std::size_t BufferSize>
    std::pair<std::size_t, SocketStatus> receive(Out out, const std::size_t start_offset = 0,
                                                 AddressInfo *addr = nullptr) {
        std::advance(out, start_offset);
        const auto max_length = BufferSize - start_offset;

        std::size_t received_bytes{0};
        int ssl_call_result{0};

        if constexpr (protocol == Protocol::TCP) {
            received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),
                                    static_cast<buffsize_t>(max_length), 0);
        } else if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            ssl_call_result =
                SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)), max_length, &received_bytes);
        } else if constexpr (protocol == Protocol::UDP) {
            m_socket_input_buffer_length = sizeof(m_socket_input_buffer);

            received_bytes = ::recvfrom(
                m_socket, reinterpret_cast<char *>(std::to_address(out)), static_cast<buffsize_t>(max_length), 0,
                reinterpret_cast<sockaddr *>(&m_socket_input_buffer), &m_socket_input_buffer_length);
            if (addr) {
                addr->set(m_socket_input_buffer, m_socket_input_buffer_length);
            }
        }

        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (ssl_call_result <= 0) {
                const int ssl_err = SSL_get_error(m_ssl, ssl_call_result);

                switch (ssl_err) {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));

                case SSL_ERROR_ZERO_RETURN:
                    return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));

                case SSL_ERROR_SYSCALL: {
                    const auto err = get_error_code();
                    if (err == EWOULDBLOCK || err == EAGAIN) {
                        return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                    }

                    return std::make_pair(0, SocketStatus(VALUES::ERRORED));
                }

                default:
                    return std::make_pair(0, SocketStatus(VALUES::ERRORED));
                }
            }
        } else {
            if (received_bytes < 0) {
                const auto err = get_error_code();
                if (err == EWOULDBLOCK || err == EAGAIN) {
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                return std::make_pair(0, SocketStatus(VALUES::ERRORED));
            } else if (received_bytes == 0) {
                return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
            }
        }

        return std::make_pair(received_bytes, SocketStatus(VALUES::VALID));
    }

    template <std::output_iterator<std::byte> Out>
    std::pair<std::size_t, SocketStatus> receive(Out out, const std::size_t length, bool wait = true,
                                                 AddressInfo *addr = nullptr) {
        std::size_t received_bytes{0};
        int ssl_call_result{0};

        if constexpr (protocol == Protocol::TCP) {
            int flags = 0;

            if (wait) {
                flags = MSG_WAITALL;
            } else {
#if defined(_WIN32)
                set_non_blocking(true);
#else
                flags = MSG_DONTWAIT;
#endif
            }

            received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),
                                    static_cast<buffsize_t>(length), flags);

            if constexpr (is_windows) {
                set_non_blocking(false);
            }
        } else if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (!wait) {
                SSL_set_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
            }

            ssl_call_result =
                SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)), length, &received_bytes);

            if (!wait) {
                SSL_clear_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
            }
        } else if constexpr (protocol == Protocol::UDP) {
            m_socket_input_buffer_length = sizeof(m_socket_input_buffer);

            int flags = 0;

            if (!wait) {
#if defined(_WIN32)
                set_non_blocking(true);
#else
                flags = MSG_DONTWAIT;
#endif
            }

            received_bytes =
                ::recvfrom(m_socket, reinterpret_cast<char *>(std::to_address(out)), static_cast<buffsize_t>(length),
                           flags, reinterpret_cast<sockaddr *>(&m_socket_input_buffer), &m_socket_input_buffer_length);
            if (addr) {
                addr->set(m_socket_input_buffer, m_socket_input_buffer_length);
            }
        }

        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (ssl_call_result <= 0) {
                const int ssl_err = SSL_get_error(m_ssl, ssl_call_result);

                switch (ssl_err) {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));

                case SSL_ERROR_ZERO_RETURN:
                    return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));

                case SSL_ERROR_SYSCALL: {
                    const auto err = get_error_code();
                    if (err == EWOULDBLOCK || err == EAGAIN) {
                        return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                    }

                    return std::make_pair(0, SocketStatus(VALUES::ERRORED));
                }

                default:
                    return std::make_pair(0, SocketStatus(VALUES::ERRORED));
                }
            }
        } else {
            if (received_bytes < 0) {
                const auto err = get_error_code();
                if (err == EWOULDBLOCK || err == EAGAIN) {
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                return std::make_pair(0, SocketStatus(VALUES::ERRORED));
            } else if (received_bytes == 0) {
                return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
            }
        }

        return std::make_pair(received_bytes, SocketStatus(VALUES::VALID));
    }

    void close() {
        if (m_socket != INVALID_SOCKET) {
            if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
                if (m_ssl) {
                    SSL_set_shutdown(m_ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
                    SSL_shutdown(m_ssl);
                    SSL_free(m_ssl);
                }

                if (m_ssl_ctx) {
                    SSL_CTX_free(m_ssl_ctx);
                }

                if constexpr (protocol == Protocol::QUIC) {
                    if (m_bio) {
                        BIO_free(m_bio);
                    }
                }
            }
            if constexpr (protocol != Protocol::QUIC || RootSocket) {
                if (closesocket(m_socket) == SOCKET_ERROR) {
                    error::handle_error("Failed to close socket");
                }
            }
        }
        if constexpr (protocol != Protocol::QUIC || RootSocket) {
            m_socket = INVALID_SOCKET;
        }
    }

    void shutdown() {
        if constexpr (protocol != Protocol::QUIC || RootSocket) {
            if (m_socket != INVALID_SOCKET) {
                if (::shutdown(m_socket, SHUT_RDWR) == SOCKET_ERROR) {
                    error::handle_error("Failed to shutdown socket");
                }
            }
        }
    }

    Endpoint get_endpoint() const noexcept { return m_endpoint; }
    Endpoint get_recived_endpoint() const {
        if constexpr (protocol == Protocol::TCP) {
            return get_endpoint();
        } else if constexpr (protocol == Protocol::UDP) {
            const SOCKADDR *addr = reinterpret_cast<const SOCKADDR *>(&m_socket_input_buffer);
            return Endpoint{addr};
        }
    }

    std::size_t get_pending_bytes() const {
        ioctl_setting pending_bytes = 0;
        if (ioctlsocket(m_socket, FIONREAD, &pending_bytes) < 0) {
            error::handle_error("Failed to get pending bytes");
        }

        if (pending_bytes > 0) {
            return static_cast<std::size_t>(pending_bytes);
        } else {
            return 0;
        }
    }

    void set_alpn_protos(const std::vector<std::string> &protocols) {
        alpn_wire_format.clear();
        for (const auto &proto : protocols) {
            if (proto.length() > 255) {
                error::handle_error("ALPN protocol name invalid or too long");
            }
            alpn_wire_format.push_back(static_cast<unsigned char>(proto.length()));
            alpn_wire_format.insert(alpn_wire_format.end(), proto.begin(), proto.end());
        }
    }

    void add_alpn_proto(const std::string_view alpn) {
        if (alpn.empty() || alpn.length() > 255) {
            error::handle_error("ALPN protocol name invalid or too long");
            return;
        }

        alpn_wire_format.push_back(static_cast<unsigned char>(alpn.length()));
        alpn_wire_format.insert(alpn_wire_format.end(), alpn.begin(), alpn.end());
    }

    static Protocol get_protocol() noexcept { return protocol; }
    SOCKET get_native_handle() const noexcept { return m_socket; }

    constexpr bool operator==(const Socket &other) const noexcept { return m_socket == other.m_socket; }
    bool is_valid() const noexcept { return m_socket != INVALID_SOCKET; }
    inline operator bool() const noexcept { return is_valid(); }

  private:
    template <bool create_socket = true>
    SocketStatus connect(addrinfo *addr, std::uint64_t timeout) {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            if constexpr (create_socket) {
                close();
                m_socket_address_info = nullptr;
                m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            }

            if (m_socket == INVALID_SOCKET) {
                return SocketStatus(VALUES::ERRORED);
            }

            m_socket_address_info = addr;

            if (timeout > 0) {
                set_non_blocking(true);
            }

            int err = ::connect(m_socket, addr->ai_addr, static_cast<socklen_t>(addr->ai_addrlen));
            if (err == SOCKET_ERROR) {
                err = get_error_code();
                if (err == EINPROGRESS || err == EWOULDBLOCK || err == EAGAIN) {
                    struct timeval tv;
                    tv.tv_sec = timeout / 1000;
                    tv.tv_usec = (timeout % 1000) * 1000;

                    fd_set writefds, exceptfds;
                    FD_ZERO(&writefds);
                    FD_SET(m_socket, &writefds);
                    FD_ZERO(&exceptfds);
                    FD_SET(m_socket, &exceptfds);

                    int select_result = select(static_cast<int>(m_socket + 1), nullptr, &writefds, &exceptfds, &tv);
                    if (select_result > 0) {
                        socklen_t len = sizeof(err);
                        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&err), &len) < 0) {
                            error::handle_error("getsockopt failed after select");
                        }
                    } else if (select_result == 0) {
                        err = ETIMEDOUT;
                    } else {
                        err = get_error_code();
                    }
                }
            }

            if (timeout > 0) {
                set_non_blocking(false);
            }

            if (err != 0) {
                close();
                m_socket_address_info = nullptr;
                return SocketStatus(VALUES::ERRORED);
            }

            return SocketStatus(VALUES::VALID);
        } else {
            error::handle_error("Connect is only supported for TCP and TLS protocols");
        }
    }

    void init_address_info() {
        int sock_type{};
        int iprotocol{};

        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            sock_type = SOCK_STREAM;
            iprotocol = IPPROTO_TCP;
        } else if constexpr (protocol == Protocol::UDP || protocol == Protocol::QUIC) {
            sock_type = SOCK_DGRAM;
            iprotocol = IPPROTO_UDP;
        }

        m_address_info_hint = {};
        m_address_info_hint.ai_family = AF_UNSPEC;
        m_address_info_hint.ai_socktype = sock_type;
        m_address_info_hint.ai_protocol = iprotocol;
        m_address_info_hint.ai_flags = AI_ADDRCONFIG;
    }

    void add_quic_bio() {
        m_bio = BIO_new(BIO_s_datagram());
        if (!m_bio) {
            error::handle_error("Failed to create BIO for QUIC socket");
            return;
        }
        BIO_set_fd(m_bio, static_cast<SOCKET>(m_socket), BIO_NOCLOSE);
        BIO_ctrl(m_bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, nullptr);
    }

    SOCKET m_socket;
    SSL *m_ssl;
    SSL_CTX *m_ssl_ctx;
    BIO *m_bio;
    Endpoint m_endpoint;
    addrinfo m_address_info_hint;
    addrinfo *m_address_info_result;
    addrinfo *m_socket_address_info;
    sockaddr_storage m_socket_input_buffer;
    socklen_t m_socket_input_buffer_length;
    std::vector<unsigned char> alpn_wire_format;
    [[no_unique_address]] OsPayload m_os;
};

} // namespace transport::base::socket
