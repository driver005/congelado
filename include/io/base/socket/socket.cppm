module;

#include <cassert>
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
import core_logger;
import io_base_leverage;
import shared;
import :consts;

#if defined(_WIN32)
export import :win32;
#else
export import :posix;
#endif

export namespace io::base::socket {

enum class Protocol { TCP = 0, UDP = 1, TLS = 2, QUIC = 3 };
enum class WaitMode { AS_SOON_AS_ARRIVED, WAIT_FOR_WHOLE_MESSAGE };
[[nodiscard]] constexpr bool is_waiting(WaitMode mode) noexcept { return mode == WaitMode::WAIT_FOR_WHOLE_MESSAGE; }

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
    sockaddr_storage &get_data() noexcept { return m_adrinf; }
    const socklen_t &get_size() const noexcept { return m_sock_size; }
    socklen_t &get_size() noexcept { return m_sock_size; }

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
            core::logger::fatal("SocketLib", "Endpoint: invalid address format");
        if (separator == address.size() - 1)
            core::logger::fatal("SocketLib", "Endpoint: missing port number");

        m_address = address.substr(0, separator);

        std::string_view port_view = address.substr(separator + 1);
        std::uint16_t parsed_port = 0;

        auto [ptr, ec] = std::from_chars(port_view.data(), port_view.data() + port_view.size(), parsed_port);

        if (ec != std::errc{}) {
            if (ec == std::errc::invalid_argument) {
                core::logger::fatal("SocketLib", "Invalid port format");
            } else if (ec == std::errc::result_out_of_range) {
                core::logger::fatal("SocketLib", "Port number too large (max 65535)");
            }

            core::logger::fatal("SocketLib", "Failed to parse port number");
        }

        m_port = parsed_port;
    }

    Endpoint(const SOCKADDR *address) {
        if (!address) {
            core::logger::fatal("SocketLib", "Null address passed to Endpoint");
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
            core::logger::fatal("SocketLib", "Unsupported address family");
        }
        }

        if (m_address.empty()) {
            core::logger::fatal("SocketLib", "Failed to convert address to string");
        }
    }

    std::string to_string() const { return std::format("{}:{}", m_address, m_port); }

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

    const VALUES &get_status() const noexcept { return m_value; }
    VALUES &get_status() noexcept { return m_value; }

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

template <Protocol protocol, bool RootSocket = false>
class Socket {
  public:
    Socket()
        : m_socket{INVALID_SOCKET}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_leverager{std::nullopt}, m_ktls_tx{false},
          m_ktls_rx{false}, m_os{} {}

    Socket(const Endpoint &endpoint,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager = std::nullopt)
        : m_socket{INVALID_SOCKET}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{std::move(endpoint)},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_leverager{leverager}, m_ktls_tx{false},
          m_ktls_rx{false}, m_os{} {
        core::logger::debug("SocketLib", "Creating socket from endpoint");

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
            core::logger::error("SocketLib", "Failed to resolve address");
        }

        for (addrinfo *addr = m_address_info_result; addr != nullptr; addr = addr->ai_next) {
            m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (m_socket != INVALID_SOCKET) {
                m_socket_address_info = addr;
                break;
            }
        }

        if (m_socket == INVALID_SOCKET) {
            core::logger::error("SocketLib", "Failed to create socket");
        } else {
            core::logger::info("SocketLib", "New socket created from endpoint {}:{}", m_endpoint.get_address(),
                               m_endpoint.get_port());
        }
    }

    Socket(SOCKET nativ, Endpoint endpoint,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager = std::nullopt)
        : m_socket{nativ}, m_ssl{nullptr}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{std::move(endpoint)},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_leverager{leverager}, m_ktls_tx{false},
          m_ktls_rx{false}, m_os{} {
        core::logger::debug("SocketLib", "Creating socket from native handle");
        init_address_info();
        core::logger::info("SocketLib", "Socket created from native handle `{}` for endpoint `{}:{}`", nativ,
                           m_endpoint.get_address(), m_endpoint.get_port());
    }

    Socket(SOCKET nativ, SSL *ssl, Endpoint endpoint,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager = std::nullopt)
        : m_socket{nativ}, m_ssl{ssl}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{std::move(endpoint)},
          m_address_info_hint{}, m_address_info_result{nullptr}, m_socket_address_info{nullptr},
          m_socket_input_buffer{}, m_socket_input_buffer_length{0}, m_leverager{leverager}, m_ktls_tx{false},
          m_ktls_rx{false}, m_os{} {
        core::logger::debug("SocketLib", "Creating socket from native handle with SSL");
        init_address_info();
        core::logger::info("SocketLib", "Socket created from native handle `{}` for endpoint `{}:{}`", nativ,
                           m_endpoint.get_address(), m_endpoint.get_port());
    }

    Socket(SOCKET nativ, SSL *ssl,
           std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> leverager = std::nullopt)
        : m_socket{nativ}, m_ssl{ssl}, m_ssl_ctx{nullptr}, m_bio{nullptr}, m_endpoint{nullptr}, m_address_info_hint{},
          m_address_info_result{nullptr}, m_socket_address_info{nullptr}, m_socket_input_buffer{},
          m_socket_input_buffer_length{0}, m_leverager{leverager}, m_ktls_tx{false}, m_ktls_rx{false}, m_os{} {
        core::logger::debug("SocketLib", "Creating socket from native handle `{}` without endpoint information", nativ);
        init_address_info();
        core::logger::info("SocketLib", "Socket created from native handle `{}` for QUIC connection", nativ);
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
          m_socket_input_buffer_length{std::move(other.m_socket_input_buffer_length)},
          m_leverager{std::move(other.m_leverager)}, m_ktls_tx{std::move(other.m_ktls_tx)},
          m_ktls_rx{std::move(other.m_ktls_rx)}, m_os{std::move(other.m_os)} {
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
            m_leverager = std::move(other.m_leverager);
            m_ktls_tx = std::move(other.m_ktls_tx);
            m_ktls_rx = std::move(other.m_ktls_rx);
            m_os = std::move(other.m_os);

            other.m_socket = INVALID_SOCKET;
            other.m_ssl = nullptr;
            other.m_ssl_ctx = nullptr;
            other.m_bio = nullptr;
            other.m_address_info_result = nullptr;
            other.m_socket_address_info = nullptr;
            other.m_leverager = std::nullopt;
        }
        return *this;
    }

    ~Socket() {
        core::logger::debug("SocketLib", "Destroying socket");
        if constexpr (protocol != Protocol::QUIC || RootSocket) {
            sync_close();
            if (m_address_info_result) {
                freeaddrinfo(m_address_info_result);
            }
        }
    }

    void set_non_blocking(bool non_blocking = true) const { set_non_blocking_impl(m_socket, non_blocking); }

    void set_reuse_address(bool reuse = true) const {
        int optval = reuse ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&optval), sizeof(optval)) != 0) {
            core::logger::error("SocketLib", "Failed to set SO_REUSEADDR");
        }

        core::logger::info("SocketLib", "Socket `{}` set SO_REUSEADDR to `{}` ", m_socket, reuse);
    }


    void set_broadcast(bool broadcast = true) const {
        int optval = broadcast ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&optval), sizeof(optval)) != 0) {
            core::logger::error("SocketLib", "Failed to set SO_BROADCAST");
        }

        core::logger::info("SocketLib", "Socket `{}` set SO_BROADCAST to `{}` ", m_socket, broadcast);
    }

    void set_tcp_no_delay(bool no_delay = true) const {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            int optval = no_delay ? 1 : 0;
            if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&optval), sizeof(optval)) !=
                0) {
                core::logger::error("SocketLib", "Failed to set TCP_NODELAY");
            }

            core::logger::info("SocketLib", "Socket `{}` set TCP_NODELAY to `{}` ", m_socket, no_delay);
        }
    }

    SocketStatus get_status() const {
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &len) != 0) {
            core::logger::error("SocketLib", "Failed to get socket status");
        }

        core::logger::info("SocketLib", "Socket `{}` status checked - error code `{}`", m_socket, error);
    }

    bool load_certificate(const char *cert_file, const char *key_file) {
        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (!m_ssl_ctx)
                return false;
            if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx, cert_file) != 1) {
                core::logger::error("SocketLib", "Failed to load certificate chain from file");
                return false;
            }
            if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key_file, SSL_FILETYPE_PEM) != 1) {
                core::logger::error("SocketLib", "Failed to load private key from file");
                return false;
            }
            if (SSL_CTX_check_private_key(m_ssl_ctx) != 1) {
                core::logger::error("SocketLib", "Private key does not match the certificate public key");
                return false;
            }

            core::logger::info("SocketLib", "Certificate and private key loaded successfully from files `{}` and `{}`",
                               cert_file, key_file);
            return true;
        }

        core::logger::fatal("SocketLib", "Loading certificates is only supported for TLS and QUIC protocols");
    }

    void bind() {
        if (m_socket_address_info == nullptr) {
            core::logger::error("SocketLib", "No valid address info to bind to");
        }

        if (::bind(m_socket, m_socket_address_info->ai_addr,
                   static_cast<socklen_t>(m_socket_address_info->ai_addrlen)) == SOCKET_ERROR) {
            core::logger::error("SocketLib", "Failed to bind socket");
        }

        if constexpr (protocol == Protocol::TLS) {
            m_ssl_ctx = SSL_CTX_new(TLS_server_method());
            if (!m_ssl_ctx) {
                core::logger::error("SocketLib", "Failed to create SSL context");
            }

            SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
            SSL_CTX_set_default_verify_paths(m_ssl_ctx);
        } else if constexpr (protocol == Protocol::QUIC) {
            add_quic_bio();
        }

        core::logger::info("SocketLib", "Socket `{}` bound to address `{}:{}`", m_socket, m_endpoint.get_address(),
                           m_endpoint.get_port());
    }

    void join(const Endpoint &endpoint, std::string_view group = "") {
        if constexpr (protocol != Protocol::UDP) {
            core::logger::fatal("SocketLib", "Joining multicast groups is only supported for UDP protocol");
        }

        addrinfo *multicast_addr_info;
        addrinfo *local_addr_info;
        addrinfo hints = {};
        hints.ai_family = PF_UNSPEC;
        hints.ai_flags = AI_NUMERICHOST;
        if (getaddrinfo(endpoint.get_address().data(), nullptr, &hints, &multicast_addr_info) != 0) {
            core::logger::error("SocketLib", "Failed to resolve multicast group address");
        }

        hints.ai_family = multicast_addr_info->ai_family;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;
        if (getaddrinfo(nullptr, std::to_string(endpoint.get_port()).data(), &hints, &local_addr_info) != 0) {
            core::logger::error("SocketLib", "Failed to resolve local address for multicast");
        }

        m_socket = ::socket(local_addr_info->ai_family, local_addr_info->ai_socktype, local_addr_info->ai_protocol);
        if (m_socket == INVALID_SOCKET) {
            core::logger::error("SocketLib", "Failed to create socket for multicast");
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
                core::logger::error("SocketLib", "Failed to join multicast group");
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
                    core::logger::error("SocketLib", "Failed to resolve group address for multicast");
                }

                mreq6.ipv6mr_interface = ((sockaddr_in6 *)group_addr_info->ai_addr)->sin6_scope_id;
                freeaddrinfo(group_addr_info);
            }

            if (setsockopt(m_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char *>(&mreq6), sizeof(mreq6)) !=
                0) {
                core::logger::error("SocketLib", "Failed to join multicast group");
            }
        } else {
            core::logger::error("SocketLib", "Unsupported address family for multicast");
        }

        core::logger::info("SocketLib", "Socket `{}` joined multicast group `{}` on endpoint `{}:{}`", m_socket,
                           group.empty() ? "(ip4/ip6 - default)" : group, endpoint.get_address(), endpoint.get_port());

        freeaddrinfo(multicast_addr_info);
    }

    void listen() {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            if (::listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
                core::logger::error("SocketLib", "Failed to listen on socket");
            }
            core::logger::info("SocketLib", "Socket `{}` is now listening for connections on `{}:{}`", m_socket,
                               m_endpoint.get_address(), m_endpoint.get_port());
        } else if constexpr (protocol == Protocol::QUIC) {
            if (!m_bio) {
                core::logger::error("SocketLib", "BIO must be initialized before listening for QUIC");
            }

            m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_server_method());
            if (!m_ssl_ctx) {
                core::logger::error("SocketLib", "Failed to create SSL context for QUIC");
            }

            m_ssl = SSL_new(m_ssl_ctx);
            if (!m_ssl) {
                core::logger::error("SocketLib", "Failed to create SSL object for QUIC");
            }

            SSL_set_bio(m_ssl, m_bio, m_bio);
            SSL_set_accept_state(m_ssl);

            core::logger::info("SocketLib", "Socket `{}` is now listening for QUIC connections on `{}:{}`", m_socket,
                               m_endpoint.get_address(), m_endpoint.get_port());
        } else {
            core::logger::fatal("SocketLib", "Listen is only supported for TCP, TLS, and QUIC protocols");
        }
    }

    SocketStatus select(int fd, std::uint64_t timeout_ms) const noexcept {
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
                core::logger::warning("SocketLib",
                                      "Select on socket `{}` would have blocked or was interrupted, no events ready",
                                      m_socket);
                return SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            core::logger::warning("SocketLib", "Critical failure in select() syscall with error code `{}`", err);
            return SocketStatus(VALUES::ERRORED);
        } else if (result == 0) {
            core::logger::warning("SocketLib", "Select on socket `{}` timed out after {} ms with no events ready",
                                  m_socket, timeout_ms);
            return SocketStatus(VALUES::TIMED_OUT);
        }

        core::logger::info("SocketLib", "Select on socket `{}` returned with events ready", m_socket);
        return SocketStatus(VALUES::VALID);
    }

    SocketStatus sync_connect(std::uint64_t timeout = 0) {
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
                core::logger::error("SocketLib", "Failed to connect to any resolved address");
            }

            if constexpr (protocol == Protocol::TLS) {
                if (!setup_tls()) {
                    return SocketStatus(VALUES::ERRORED);
                }
            } else if constexpr (protocol == Protocol::QUIC) {
                add_quic_bio();

                if (!setup_quic()) {
                    return SocketStatus(VALUES::ERRORED);
                }
            }

            core::logger::info("SocketLib", "Socket `{}` connected to `{}:{}`", m_socket, m_endpoint.get_address(),
                               m_endpoint.get_port());
            return SocketStatus(VALUES::VALID);
        } else {
            core::logger::fatal("SocketLib", "Connect is only supported for TCP, TLS, QUIC protocols");
        }
    }

    void async_connect(std::move_only_function<void(SocketStatus)> callback, std::uint8_t iflags = 0) noexcept {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (m_leverager) {
                auto current = m_socket_address_info ? m_socket_address_info : m_address_info_result;

                set_non_blocking(true);

                auto attempt = std::make_shared<std::function<void(int)>>();
                *attempt = [this, current, callback = std::move(callback), iflags, attempt](int res,
                                                                                            std::uint32_t) mutable {
                    if (res == 0) {
                        m_socket_address_info = current;

                        if constexpr (protocol == Protocol::TLS) {
                            if (!setup_tls()) {
                                callback(SocketStatus(VALUES::ERRORED));
                                return;
                            }
                        } else if constexpr (protocol == Protocol::QUIC) {
                            add_quic_bio();
                            if (!setup_quic()) {
                                callback(SocketStatus(VALUES::ERRORED));
                                return;
                            }
                        }

                        core::logger::info("SocketLib", "Socket `{}` connected to `{}:{}`", m_socket,
                                           m_endpoint.get_address(), m_endpoint.get_port());
                        callback(SocketStatus(VALUES::VALID));
                        return;
                    }

                    current = current->ai_next;
                    if (!current) {
                        core::logger::warning("SocketLib", "Socket `{}` no more addresses to try to connect to",
                                              m_socket);
                        if (res == -ETIMEDOUT) {
                            core::logger::warning("SocketLib", "Socket `{}` connection attempt timed out", m_socket);
                            callback(SocketStatus(VALUES::TIMED_OUT));
                        } else if (res == -EWOULDBLOCK || res == -EAGAIN || res == -EINPROGRESS) {
                            core::logger::warning(
                                "SocketLib",
                                "Socket `{}` connection attempt would have blocked, but socket is non-blocking",
                                m_socket);
                            callback(SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                        } else {
                            core::logger::warning("SocketLib",
                                                  "Socket `{}` connection attempt failed with error code `{}`",
                                                  m_socket, res);
                            callback(SocketStatus(VALUES::ERRORED));
                        }
                        return;
                    }

                    // close();

                    m_socket = ::socket(current->ai_family, current->ai_socktype, current->ai_protocol);
                    if (m_socket == INVALID_SOCKET) {
                        core::logger::warning("SocketLib", "Failed to create socket for connection attempt");
                        callback(SocketStatus(VALUES::ERRORED));
                        return;
                    }

                    set_non_blocking(true);

                    m_leverager->get().connect(m_socket, current->ai_addr, static_cast<socklen_t>(current->ai_addrlen),
                                               *attempt, iflags);
                };

                m_leverager->get().connect(m_socket, current->ai_addr, static_cast<socklen_t>(current->ai_addrlen),
                                           *attempt, iflags);
            } else {
                core::logger::fatal("SocketLib", "m_leverager is not set so async funtion calls cannot be used");
            }
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
    SocketStatus sync_handshake(bool *wait_for_write = nullptr) noexcept {
        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (!m_ssl) {
                core::logger::warning("SocketLib", "SSL object is not initialized for handshake");
                return SocketStatus(VALUES::ERRORED);
            }

            int ret = SSL_do_handshake(m_ssl);
            if (ret == 1) {
                if constexpr (protocol == Protocol::TLS) {
                    m_ktls_tx = BIO_get_ktls_send(SSL_get_wbio(m_ssl));
                    m_ktls_rx = BIO_get_ktls_recv(SSL_get_rbio(m_ssl));
                    if (m_ktls_tx) {
                        core::logger::info("SocketLib", "Socket `{}` kTLS TX active", m_socket);
                    }
                    if (m_ktls_rx) {
                        core::logger::info("SocketLib", "Socket `{}` kTLS RX active", m_socket);
                    }
                }
                core::logger::info("SocketLib", "Socket `{}` completed handshake successfully", m_socket);
                return SocketStatus(VALUES::VALID);
            }

            int err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_READ) {
                if (wait_for_write) {
                    *wait_for_write = false;
                }
                core::logger::info("SocketLib", "Socket `{}` handshake not complete, waiting for it to be readable",
                                   m_socket);
                return SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }
            if (err == SSL_ERROR_WANT_WRITE) {
                if (wait_for_write) {
                    *wait_for_write = true;
                }
                core::logger::info("SocketLib", "Socket `{}` handshake not complete, waiting for it to be writable",
                                   m_socket);
                return SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED);
            }

            core::logger::warning("SocketLib", "Socket `{}` handshake failed with error code `{}`", m_socket, err);
            return SocketStatus(VALUES::ERRORED);
        } else {
            core::logger::fatal("SocketLib", "Socket `{}` is a plain TCP socket, no handshake needed", m_socket);
        }
    }

    void async_handshake(std::move_only_function<void(SocketStatus)> callback, std::uint8_t iflags = 0) noexcept {
        if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
            if (m_leverager) {
                auto attempt = std::make_shared<std::function<void(int)>>();
                *attempt = [this, callback = std::move(callback), iflags, attempt](int res, std::uint32_t) mutable {
                    if (res < 0) {
                        core::logger::warning("SocketLib",
                                              "Socket `{}` async handshake attempt failed with error code `{}`",
                                              m_socket, res);
                        callback(SocketStatus(VALUES::ERRORED));
                        return;
                    }

                    bool wait_for_write = false;
                    SocketStatus status = sync_handshake(&wait_for_write);

                    if (status.would_have_blocked()) {
                        if (wait_for_write) {
                            m_leverager->get().send(m_socket, nullptr, 0, 0, *attempt, iflags);
                        } else {
                            m_leverager->get().recv(m_socket, nullptr, 0, 0, *attempt, iflags);
                        }
                    }

                    callback(status);
                };

                // Call to jumpstart the async handshake process, it will immediately call the callback which will then
                // re-arm itself until the handshake is complete or fails
                (*attempt)(0);
            } else {
                core::logger::fatal("SocketLib", "m_leverager is not set so async funtion calls cannot be used");
            }
        } else {
            core::logger::fatal("SocketLib", "Socket `{}` is a plain TCP socket, no handshake needed", m_socket);
        }
    }

    bool is_handshake_done() const noexcept {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::QUIC) {
            return m_ssl && SSL_is_init_finished(m_ssl);
        }
        return true;
    }

    Socket<protocol> sync_accept() const {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            sockaddr_storage client_addr{};
            socklen_t addr_len = sizeof(client_addr);

            SOCKET client_fd = ::accept(m_socket, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);
            if (client_fd == INVALID_SOCKET) {
                const auto err = get_error_code();

                if (err == EWOULDBLOCK || err == EAGAIN || err == EINTR) {
                    core::logger::info("SocketLib",
                                       "Accept on socket `{}` would have blocked, no incoming connections ready",
                                       m_socket);
                    return Socket<protocol>{};
                }

                core::logger::error("SocketLib", "Critical failure in accept() syscall with error code `{}`", err);
            }

            if constexpr (protocol == Protocol::TLS) {
                SSL *client_ssl = SSL_new(m_ssl_ctx);
                if (!client_ssl) {
                    closesocket(client_fd);
                    core::logger::error("SocketLib", "Failed to create SSL object for accepted connection");
                }

                if (!SSL_set_fd(client_ssl, client_fd)) {
                    SSL_free(client_ssl);
                    closesocket(client_fd);
                    core::logger::error("SocketLib", "Failed to associate SSL object with accepted socket");
                }

                SSL_set_accept_state(client_ssl);

                auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(&client_addr));
                core::logger::info("SocketLib", "Accepted new TLS connection on socket `{}` from `{}:{}`", m_socket,
                                   endpoint.get_address(), endpoint.get_port());

                return Socket<protocol>{client_fd, client_ssl, std::move(endpoint)};
            }

            auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(&client_addr));
            core::logger::info("SocketLib", "Accepted new TCP connection on socket `{}` from `{}:{}`", m_socket,
                               endpoint.get_address(), endpoint.get_port());

            return Socket<protocol>{client_fd, std::move(endpoint)};
        } else if constexpr (protocol == Protocol::QUIC) {
            if (!m_ssl) {
                core::logger::error("SocketLib", "Socket `{}` SSL object is not initialized for QUIC accept", m_socket);
            }

            SSL *client_ssl = SSL_accept_connection(m_ssl, 0);
            if (!client_ssl) {
                core::logger::error("SocketLib", "Socket `{}` failed to accept new QUIC connection ", m_socket);
            }

            core::logger::info("SocketLib", "Socket `{}` accepted new QUIC connection", m_socket);
            return Socket<protocol>{m_socket, client_ssl};

        } else {
            core::logger::fatal("SocketLib", "Accept is only supported for TCP, TLS and QUIC protocols");
        }
    }

    void async_accept(std::move_only_function<void(Socket<protocol>)> callback, std::uint8_t iflags = 0) noexcept {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            if (m_leverager) {
                auto client_addr = std::make_shared<sockaddr_storage>();
                auto addr_len = std::make_shared<socklen_t>(sizeof(sockaddr_storage));

                m_leverager->get().accept(
                    m_socket, reinterpret_cast<sockaddr *>(client_addr.get()), addr_len.get(), 0,
                    [this, client_addr, addr_len, callback = std::move(callback)](int res, std::uint32_t) mutable {
                        if (res < 0) {
                            const auto err = get_error_code();

                            if (err == EWOULDBLOCK || err == EAGAIN || err == EINTR) {
                                core::logger::info(
                                    "SocketLib",
                                    "Accept on socket `{}` would have blocked, no incoming connections ready",
                                    m_socket);
                                callback(Socket<protocol>{});
                                return;
                            }

                            core::logger::warning("SocketLib", "Socket `{}` critical failure in async accept",
                                                  m_socket);
                            callback(Socket<protocol>{});
                            return;
                        }

                        SOCKET client_fd = static_cast<SOCKET>(res);

                        if constexpr (protocol == Protocol::TLS) {
                            SSL *client_ssl = SSL_new(m_ssl_ctx);
                            if (!client_ssl) {
                                closesocket(client_fd);
                                core::logger::warning("SocketLib",
                                                      "Failed to create SSL object for accepted connection");
                                callback(Socket<protocol>{});
                                return;
                            }

                            if (!SSL_set_fd(client_ssl, client_fd)) {
                                SSL_free(client_ssl);
                                closesocket(client_fd);
                                core::logger::warning("SocketLib",
                                                      "Failed to associate SSL object with accepted socket");
                                callback(Socket<protocol>{});
                                return;
                            }

                            SSL_set_accept_state(client_ssl);

                            auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(client_addr.get()));
                            core::logger::info("SocketLib", "Accepted new TLS connection on socket `{}` from `{}:{}`",
                                               m_socket, endpoint.get_address(), endpoint.get_port());
                            callback(Socket<protocol>{client_fd, client_ssl, std::move(endpoint), m_leverager});
                            return;
                        }
                        auto endpoint = Endpoint(reinterpret_cast<sockaddr *>(client_addr.get()));
                        core::logger::info("SocketLib", "Accepted new TCP connection on socket `{}` from `{}:{}`",
                                           m_socket, endpoint.get_address(), endpoint.get_port());

                        callback(Socket<protocol>{client_fd, std::move(endpoint), m_leverager});
                    },
                    iflags);
            } else {
                core::logger::fatal("SocketLib", "m_leverager is not set so async funtion calls cannot be used");
            }
        } else if constexpr (protocol == Protocol::QUIC) {
            if (!m_ssl) {
                core::logger::error("SocketLib", "Socket `{}` SSL object is not initialized for QUIC accept", m_socket);
            }

            SSL *client_ssl = SSL_accept_connection(m_ssl, 0);
            if (!client_ssl) {
                core::logger::error("SocketLib", "Socket `{}` failed to accept new QUIC connection ", m_socket);
            }

            core::logger::info("SocketLib", "Socket `{}` accepted new QUIC connection", m_socket);
            return Socket<protocol>{m_socket, client_ssl};

        } else {
            core::logger::fatal("SocketLib", "Accept is only supported for TCP, TLS and QUIC protocols");
        }
    }


    std::pair<std::size_t, SocketStatus> sync_send(const std::byte *buffer, const std::size_t length,
                                                   AddressInfo *addr = nullptr) const {
        std::size_t sent_bytes{0};
        int ssl_call_result{0};

        if constexpr (protocol == Protocol::TCP) {
            sent_bytes = ::send(m_socket, reinterpret_cast<const char *>(buffer), static_cast<buffsize_t>(length), 0);
        } else if constexpr (protocol == Protocol::TLS) {
            if (m_ktls_tx) {
                sent_bytes =
                    ::send(m_socket, reinterpret_cast<const char *>(buffer), static_cast<buffsize_t>(length), 0);
            } else {
                ssl_call_result = SSL_write_ex(m_ssl, reinterpret_cast<const void *>(buffer), length, &sent_bytes);
            }
        } else if constexpr (protocol == Protocol::QUIC) {
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

        if (!m_ktls_tx) {
            if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
                if (ssl_call_result <= 0) {
                    const int ssl_err = SSL_get_error(m_ssl, ssl_call_result);

                    core::logger::warning("SocketLib", "SSL_write_ex on socket `{}` failed with error code `{}`",
                                          m_socket, ssl_err);

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
            }
        } else {
            if (sent_bytes < 0) {
                const auto err = get_error_code();
                if (err == EWOULDBLOCK) {
                    core::logger::warning(
                        "SocketLib", "Send on socket `{}` would have blocked, no buffer space available", m_socket);
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                core::logger::warning("SocketLib", "Socket `{}` critical failure in send() syscall", m_socket);
                return std::make_pair(0, SocketStatus(VALUES::ERRORED));
            }
        }

        core::logger::info("SocketLib", "Socket `{}` sent {} bytes ", m_socket, sent_bytes);
        return std::make_pair(sent_bytes, SocketStatus(VALUES::VALID));
    }

    void async_send(const std::byte *buffer, const std::size_t length,
                    std::move_only_function<void(std::size_t, SocketStatus)> callback, AddressInfo *addr = nullptr,
                    std::uint8_t iflags = 0) noexcept {
        if (m_leverager) {
            if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
                if constexpr (protocol == Protocol::TLS) {
                    if (!m_ktls_tx) {
                        core::logger::fatal("SocketLib",
                                            "Async send is not supported for TLS sockets without kTLS enabled due to "
                                            "the complexity of handling SSL_write's various return conditions in an "
                                            "async context. Please enable kTLS for this socket to use async send.");
                    }
                }
                m_leverager->get().send(
                    m_socket, buffer, static_cast<unsigned>(length), 0,
                    [this, callback = std::move(callback)](int sent_bytes, std::uint32_t) mutable {
                        if (sent_bytes < 0) {
                            const auto err = get_error_code();
                            if (err == EWOULDBLOCK) {
                                core::logger::warning(
                                    "SocketLib", "Send on socket `{}` would have blocked, no buffer space available",
                                    m_socket);
                                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                return;
                            }
                            core::logger::warning("SocketLib", "Socket `{}` critical failure when sending data async",
                                                  m_socket);
                            callback(0, SocketStatus(VALUES::ERRORED));
                            return;
                        }

                        core::logger::info("SocketLib", "Socket `{}` sent {} bytes ", m_socket, sent_bytes);
                        callback(sent_bytes, SocketStatus(VALUES::VALID));
                    },
                    iflags);
            } else if constexpr (protocol == Protocol::UDP) {
                if (addr) {
                    iovec iov{};
                    iov.iov_base = const_cast<std::byte *>(buffer);
                    iov.iov_len = length;

                    msghdr msg{};
                    msg.msg_name = reinterpret_cast<sockaddr *>(&addr->get_data());
                    msg.msg_namelen = static_cast<socklen_t>(addr->get_size());
                    msg.msg_iov = &iov;
                    msg.msg_iovlen = 1;

                    m_leverager->get().sendmsg(
                        m_socket, &msg, 0,
                        [this, callback = std::move(callback)](int sent_bytes, std::uint32_t) mutable {
                            if (sent_bytes < 0) {
                                const auto err = get_error_code();
                                if (err == EWOULDBLOCK) {
                                    core::logger::warning(
                                        "SocketLib",
                                        "Send on socket `{}` would have blocked, no buffer space available", m_socket);
                                    callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                    return;
                                }
                                core::logger::warning("SocketLib",
                                                      "Socket `{}` critical failure when sending data async", m_socket);
                                callback(0, SocketStatus(VALUES::ERRORED));
                                return;
                            }

                            core::logger::info("SocketLib", "Socket `{}` sent {} bytes ", m_socket, sent_bytes);
                            callback(sent_bytes, SocketStatus(VALUES::VALID));
                        },
                        iflags);
                } else {
                    iovec iov{};
                    iov.iov_base = const_cast<std::byte *>(buffer);
                    iov.iov_len = length;

                    msghdr msg{};
                    msg.msg_name = static_cast<sockaddr *>(m_socket_address_info->ai_addr);
                    msg.msg_namelen = static_cast<socklen_t>(m_socket_address_info->ai_addrlen);
                    msg.msg_iov = &iov;
                    msg.msg_iovlen = 1;

                    m_leverager->get().sendmsg(
                        m_socket, &msg, 0,
                        [this, callback = std::move(callback)](int sent_bytes, std::uint32_t) mutable {
                            if (sent_bytes < 0) {
                                const auto err = get_error_code();
                                if (err == EWOULDBLOCK) {
                                    core::logger::warning(
                                        "SocketLib",
                                        "Send on socket `{}` would have blocked, no buffer space available", m_socket);
                                    callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                    return;
                                }
                                core::logger::warning("SocketLib",
                                                      "Socket `{}` critical failure when sending data async", m_socket);
                                callback(0, SocketStatus(VALUES::ERRORED));
                                return;
                            }

                            core::logger::info("SocketLib", "Socket `{}` sent {} bytes ", m_socket, sent_bytes);
                            callback(sent_bytes, SocketStatus(VALUES::VALID));
                        },
                        iflags);
                }
            } else if constexpr (protocol == Protocol::QUIC) {
                auto attempt = std::make_shared<std::function<void(int)>>();
                *attempt = [this, buffer, length, callback = std::move(callback), iflags,
                            attempt](int res, std::uint32_t) mutable {
                    if (res < 0) {
                        core::logger::warning("SocketLib", "Socket `{}` async send attempt failed with error code `{}`",
                                              m_socket, res);
                        callback(0, SocketStatus(VALUES::ERRORED));
                        return;
                    }

                    std::size_t sent_bytes = 0;
                    int ret = SSL_write_ex(m_ssl, buffer, length, &sent_bytes);
                    if (ret > 0) {
                        core::logger::info("SocketLib", "Socket `{}` sent {} bytes", m_socket, sent_bytes);
                        callback(sent_bytes, SocketStatus(VALUES::VALID));
                        return;
                    }

                    if (sent_bytes <= 0) {
                        const int ssl_err = SSL_get_error(m_ssl, sent_bytes);

                        core::logger::warning("SocketLib", "SSL_write_ex on socket `{}` failed with error code `{}`",
                                              m_socket, ssl_err);

                        switch (ssl_err) {
                        case SSL_ERROR_WANT_READ:
                            m_leverager->get().recv(m_socket, nullptr, 0, 0, *attempt, iflags);
                            break;
                        case SSL_ERROR_WANT_WRITE:
                            m_leverager->get().send(m_socket, nullptr, 0, 0, *attempt, iflags);
                            break;
                        case SSL_ERROR_ZERO_RETURN:
                            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
                            break;

                        case SSL_ERROR_SYSCALL: {
                            if (get_error_code() == EWOULDBLOCK) {
                                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                break;
                            }

                            callback(0, SocketStatus(VALUES::ERRORED));
                            break;
                        }
                        default:
                            callback(0, SocketStatus(VALUES::ERRORED));
                        }
                    }
                };

                // Call to jumpstart the async send process
                (*attempt)(0);
            } else {
                core::logger::fatal("SocketLib", "Unsupported protocol for async send");
            }
        } else {
            core::logger::fatal("SocketLib", "m_leverager is not set so async funtion calls cannot be used");
        }
    }


    template <std::output_iterator<std::byte> Out>
    std::pair<std::size_t, SocketStatus> sync_receive(Out out, const std::size_t length,
                                                      const std::size_t start_offset = 0, AddressInfo *addr = nullptr) {
        std::advance(out, start_offset);
        const auto max_length = length - start_offset;
        if (max_length < 0) {
            core::logger::error("SocketLib", "Start offset {} is greater than the total buffer length {}", start_offset,
                                length);
            return std::make_pair(0, SocketStatus(VALUES::ERRORED));
        }

        std::size_t received_bytes{0};
        int ssl_call_result{0};

        if constexpr (protocol == Protocol::TCP) {
            received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),
                                    static_cast<buffsize_t>(max_length), 0);
        } else if constexpr (protocol == Protocol::TCP) {
            if (m_ktls_rx) {
                received_bytes = ::recv(m_socket, reinterpret_cast<char *>(std::to_address(out)),
                                        static_cast<buffsize_t>(max_length), 0);
            } else {
                ssl_call_result =
                    SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)), max_length, &received_bytes);
            }
        } else if constexpr (protocol == Protocol::QUIC) {
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

        if (!m_ktls_rx) {
            if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
                if (ssl_call_result <= 0) {
                    const int ssl_err = SSL_get_error(m_ssl, ssl_call_result);

                    core::logger::warning("SocketLib", "SSL_read_ex on socket `{}` failed with error code `{}`",
                                          m_socket, ssl_err);

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
            }
        } else {
            if (received_bytes < 0) {
                const auto err = get_error_code();
                if (err == EWOULDBLOCK || err == EAGAIN) {
                    core::logger::warning("SocketLib", "Receive on socket `{}` would have blocked, no data available",
                                          m_socket);
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall", m_socket);
                return std::make_pair(0, SocketStatus(VALUES::ERRORED));
            } else if (received_bytes == 0) {
                core::logger::info("SocketLib", "Socket `{}` has been cleanly disconnected by the peer", m_socket);
                return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
            }
        }

        core::logger::info("SocketLib", "Socket `{}` received {} bytes ", m_socket, received_bytes);
        return std::make_pair(received_bytes, SocketStatus(VALUES::VALID));
    }

    template <std::output_iterator<std::byte> Out>
    std::pair<std::size_t, SocketStatus> sync_receive(Out out, const std::size_t length,
                                                      WaitMode wait = WaitMode::AS_SOON_AS_ARRIVED,
                                                      AddressInfo *addr = nullptr) {
        std::size_t received_bytes{0};
        int ssl_call_result{0};

        if constexpr (protocol == Protocol::TCP) {
            int flags = 0;

            if (is_waiting(wait)) {
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
        } else if constexpr (protocol == Protocol::TLS) {
            if (m_ktls_rx) {
                int flags = 0;

                if (is_waiting(wait)) {
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
            } else {
                if (!is_waiting(wait)) {
                    SSL_set_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
                }

                ssl_call_result =
                    SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)), length, &received_bytes);

                if (!is_waiting(wait)) {
                    SSL_clear_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
                }
            }

        } else if constexpr (protocol == Protocol::QUIC) {
            if (!is_waiting(wait)) {
                SSL_set_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
            }

            ssl_call_result =
                SSL_read_ex(m_ssl, reinterpret_cast<void *>(std::to_address(out)), length, &received_bytes);

            if (!is_waiting(wait)) {
                SSL_clear_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
            }
        } else if constexpr (protocol == Protocol::UDP) {
            m_socket_input_buffer_length = sizeof(m_socket_input_buffer);

            int flags = 0;

            if (!is_waiting(wait)) {
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

        if (!m_ktls_rx) {
            if constexpr (protocol == Protocol::TLS || protocol == Protocol::QUIC) {
                if (ssl_call_result <= 0) {
                    const int ssl_err = SSL_get_error(m_ssl, ssl_call_result);

                    core::logger::warning("SocketLib", "SSL_read_ex on socket `{}` failed with error code `{}`",
                                          m_socket, ssl_err);

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
            }
        } else {
            if (received_bytes < 0) {
                const auto err = get_error_code();
                if (err == EWOULDBLOCK || err == EAGAIN) {
                    core::logger::warning("SocketLib", "Receive on socket `{}` would have blocked, no data available",
                                          m_socket);
                    return std::make_pair(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                }
                core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall", m_socket);
                return std::make_pair(0, SocketStatus(VALUES::ERRORED));
            } else if (received_bytes == 0) {
                core::logger::info("SocketLib", "Socket `{}` has been cleanly disconnected by the peer", m_socket);
                return std::make_pair(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
            }
        }

        core::logger::info("SocketLib", "Socket `{}` received {} bytes ", m_socket, received_bytes);
        return std::make_pair(received_bytes, SocketStatus(VALUES::VALID));
    }

    template <std::output_iterator<std::byte> Out>
    void async_receive(Out out, const std::size_t length,
                       std::move_only_function<void(std::size_t, SocketStatus)> callback,
                       std::uint8_t iflags = 0) noexcept {
        if (m_leverager) {
            if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
                if constexpr (protocol == Protocol::TLS) {
                    if (!m_ktls_rx) {
                        core::logger::fatal(
                            "SocketLib", "Async receive is not supported for TLS sockets without kTLS enabled due to "
                                         "the complexity of handling SSL_write's various return conditions in an "
                                         "async context. Please enable kTLS for this socket to use async receive.");
                    }
                }
                m_leverager->get().recv(
                    m_socket, reinterpret_cast<char *>(std::to_address(out)), static_cast<unsigned>(length), 0,
                    [this, callback = std::move(callback)](int received_bytes, std::uint32_t) mutable {
                        if (received_bytes < 0) {
                            const auto err = get_error_code();
                            if (err == EWOULDBLOCK || err == EAGAIN) {
                                core::logger::warning("SocketLib",
                                                      "Receive on socket `{}` would have blocked, no data available",
                                                      m_socket);
                                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                return;
                            }
                            core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall",
                                                  m_socket);
                            callback(0, SocketStatus(VALUES::ERRORED));
                        } else if (received_bytes == 0) {
                            core::logger::info("SocketLib", "Socket `{}` has been cleanly disconnected by the peer",
                                               m_socket);
                            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
                        }
                    },
                    iflags);
            } else if constexpr (protocol == Protocol::UDP) {
                iovec iov{};
                iov.iov_base = reinterpret_cast<char *>(std::to_address(out));
                iov.iov_len = length;

                msghdr msg{};
                msg.msg_iov = &iov;
                msg.msg_iovlen = 1;
                msg.msg_name = &m_socket_input_buffer;
                msg.msg_namelen = sizeof(m_socket_input_buffer);

                m_leverager->get().recvmsg(
                    m_socket, &msg, 0,
                    [this, callback = std::move(callback)](int received_bytes, std::uint32_t) mutable {
                        if (received_bytes < 0) {
                            const auto err = get_error_code();
                            if (err == EWOULDBLOCK || err == EAGAIN) {
                                core::logger::warning("SocketLib",
                                                      "Receive on socket `{}` would have blocked, no data available",
                                                      m_socket);
                                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                return;
                            }
                            core::logger::warning("SocketLib", "Socket `{}` critical failure in recv() syscall",
                                                  m_socket);
                            callback(0, SocketStatus(VALUES::ERRORED));
                        } else if (received_bytes == 0) {
                            core::logger::info("SocketLib", "Socket `{}` has been cleanly disconnected by the peer",
                                               m_socket);
                            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
                        }
                    },
                    iflags);
            } else if constexpr (protocol == Protocol::QUIC) {
                auto attempt = std::make_shared<std::function<void(int)>>();
                *attempt = [this, out, length, callback = std::move(callback), iflags, attempt](int res,
                                                                                                std::uint32_t) mutable {
                    if (res < 0) {
                        core::logger::warning("SocketLib", "Socket `{}` async read attempt failed with error code `{}`",
                                              m_socket, res);
                        callback(0, SocketStatus(VALUES::ERRORED));
                        return;
                    }

                    std::size_t received_bytes = 0;
                    int ret =
                        SSL_read_ex(m_ssl, reinterpret_cast<char *>(std::to_address(out)), length, &received_bytes);
                    if (ret > 0) {
                        core::logger::info("SocketLib", "Socket `{}` received {} bytes", m_socket, received_bytes);
                        callback(received_bytes, SocketStatus(VALUES::VALID));
                        return;
                    }

                    if (received_bytes <= 0) {
                        const int ssl_err = SSL_get_error(m_ssl, received_bytes);

                        core::logger::warning("SocketLib", "SSL_read_ex on socket `{}` failed with error code `{}`",
                                              m_socket, ssl_err);

                        switch (ssl_err) {
                        case SSL_ERROR_WANT_READ:
                            m_leverager->get().recv(m_socket, nullptr, 0, 0, *attempt, iflags);
                            break;
                        case SSL_ERROR_WANT_WRITE:
                            m_leverager->get().send(m_socket, nullptr, 0, 0, *attempt, iflags);
                            break;
                        case SSL_ERROR_ZERO_RETURN:
                            callback(0, SocketStatus(VALUES::CLEANLY_DISCONNECTED));
                            break;

                        case SSL_ERROR_SYSCALL: {
                            if (get_error_code() == EWOULDBLOCK) {
                                callback(0, SocketStatus(VALUES::NON_BLOCKING_WOULD_HAVE_BLOCKED));
                                break;
                            }

                            callback(0, SocketStatus(VALUES::ERRORED));
                            break;
                        }
                        default:
                            callback(0, SocketStatus(VALUES::ERRORED));
                        }
                    }
                };

                // Call to jumpstart the async receive process
                (*attempt)(0);
            } else {
                core::logger::fatal("SocketLib", "Unsupported protocol for async receive");
            }
        } else {
            core::logger::fatal("SocketLib", "m_leverager is not set so async funtion calls cannot be used");
        }
    }

    void sync_close() {
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
                    core::logger::error("SocketLib", "Socket `{}` failed to close", m_socket);
                }
            }

            m_socket = INVALID_SOCKET;

            core::logger::info("SocketLib", "Socket `{}` has been closed", m_socket);
        }
    }

    void async_close(std::function<void(bool)> callback, std::uint8_t iflags = 0) noexcept {
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
                m_leverager->get().close(
                    m_socket,
                    [this, callback = std::move(callback)](int res, std::uint32_t) mutable {
                        if (res < 0) {
                            core::logger::warning("SocketLib", "Socket `{}` failed to close", m_socket);
                            callback(false);
                            return;
                        }
                        core::logger::info("SocketLib", "Socket `{}` has been closed", m_socket);
                        m_socket = INVALID_SOCKET;
                        callback(true);
                    },
                    iflags);
            }
        }
    }

    void shutdown() {
        if constexpr (protocol != Protocol::QUIC || RootSocket) {
            if (m_socket != INVALID_SOCKET) {
                if (::shutdown(m_socket, SHUT_RDWR) == SOCKET_ERROR) {
                    core::logger::error("SocketLib", "Socket `{}` failed to shutdown", m_socket);
                }

                core::logger::info("SocketLib", "Socket `{}` has been shutdown", m_socket);
            }
        }

        core::logger::fatal("SocketLib", "Shutdown is not supported for QUIC or non-root sockets");
    }

    void async_shutdown(std::function<void(bool)> callback, std::uint8_t iflags = 0) {
        if constexpr (protocol != Protocol::QUIC || !RootSocket) {
            if (m_socket != INVALID_SOCKET) {
                m_leverager->get().shutdown(
                    m_socket, SHUT_RDWR,
                    [this, callback = std::move(callback)](int res, std::uint32_t) mutable {
                        if (res < 0) {
                            core::logger::warning("SocketLib", "Socket `{}` failed to shutdown", m_socket);
                            callback(false);
                            return;
                        }
                        core::logger::info("SocketLib", "Socket `{}` has been shutdown", m_socket);
                        callback(true);
                    },
                    iflags);
            }
        }

        core::logger::fatal("SocketLib", "Shutdown is not supported for QUIC or non-root sockets");
    }

    const Endpoint &get_endpoint() const noexcept { return m_endpoint; }
    Endpoint &get_endpoint() noexcept { return m_endpoint; }

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
            core::logger::error("SocketLib", "Failed to get pending bytes");
        }

        core::logger::info("SocketLib", "Socket `{}` has `{}` pending bytes available to read", m_socket,
                           pending_bytes);

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
                core::logger::error("SocketLib", "ALPN protocol name invalid or too long");
            }

            core::logger::info("SocketLib", "Socket `{}` adding ALPN protocol `{}`", m_socket, proto);
            alpn_wire_format.push_back(static_cast<unsigned char>(proto.length()));
            alpn_wire_format.insert(alpn_wire_format.end(), proto.begin(), proto.end());
        }
    }

    void add_alpn_proto(const std::string_view alpn) {
        if (alpn.empty() || alpn.length() > 255) {
            core::logger::error("SocketLib", "ALPN protocol name invalid or too long");
            return;
        }

        core::logger::info("SocketLib", "Socket `{}` adding ALPN protocol `{}`", m_socket, alpn);
        alpn_wire_format.push_back(static_cast<unsigned char>(alpn.length()));
        alpn_wire_format.insert(alpn_wire_format.end(), alpn.begin(), alpn.end());
    }

    static Protocol get_protocol() noexcept { return protocol; }
    SOCKET &get_fd() noexcept { return m_socket; }
    const SOCKET &get_fd() const noexcept { return m_socket; }

    constexpr bool operator==(const Socket &other) const noexcept { return m_socket == other.m_socket; }
    bool is_valid() const noexcept { return m_socket != INVALID_SOCKET; }
    inline operator bool() const noexcept { return is_valid(); }

  private:
    template <bool create_socket = true>
    SocketStatus connect(addrinfo *addr, std::uint64_t timeout) noexcept {
        if constexpr (protocol == Protocol::TCP || protocol == Protocol::TLS) {
            if constexpr (create_socket) {
                core::logger::info("SocketLib", "Closing existing socket `{}` to attempt connection to new address",
                                   m_socket);
                sync_close();
                m_socket_address_info = nullptr;
                m_socket = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            }

            if (m_socket == INVALID_SOCKET) {
                core::logger::warning("SocketLib", "Failed to create socket for connection attempt");
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
                    core::logger::info("SocketLib",
                                       "Connect on socket `{}` in progress, waiting for it to complete "
                                       "with timeout of {} ms",
                                       m_socket, timeout);

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
                            core::logger::error("SocketLib", "getsockopt failed after select");
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
                sync_close();
                m_socket_address_info = nullptr;
                core::logger::warning("SocketLib", "Connect attempt on socket `{}` failed with error code `{}`",
                                      m_socket, err);
                return SocketStatus(VALUES::ERRORED);
            }

            core::logger::info("SocketLib", "Socket `{}` successfully connected to `{}:{}`", m_socket,
                               m_endpoint.get_address(), m_endpoint.get_port());
            return SocketStatus(VALUES::VALID);
        } else {
            core::logger::fatal("SocketLib", "Connect is only supported for TCP and TLS protocols");
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
            core::logger::error("SocketLib", "Failed to create BIO for QUIC socket");
        }
        BIO_set_fd(m_bio, static_cast<SOCKET>(m_socket), BIO_NOCLOSE);
        BIO_ctrl(m_bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, nullptr);

        core::logger::info("SocketLib", "BIO for QUIC socket `{}` has been created and configured", m_socket);
    }

    bool setup_tls()
        requires(protocol == Protocol::TLS)
    {
        m_ssl_ctx = SSL_CTX_new(TLS_client_method());
        if (!m_ssl_ctx) {
            core::logger::warning("SocketLib", "Failed to create SSL context");
            return false;
        }

        SSL_CTX_set_options(m_ssl_ctx, SSL_OP_ENABLE_KTLS);

        SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, nullptr);
        SSL_CTX_set_default_verify_paths(m_ssl_ctx);

        m_ssl = SSL_new(m_ssl_ctx);
        if (!m_ssl) {
            core::logger::warning("SocketLib", "Failed to create SSL object");
            return false;
        }

        if (!SSL_set_fd(m_ssl, m_socket)) {
            core::logger::warning("SocketLib", "Failed to associate SSL object with socket");
            return false;
        }

        if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.get_address().data()) != 1) {
            core::logger::warning("SocketLib", "Failed to set SNI hostname");
            return false;
        }

        core::logger::info("SocketLib", "SSL object created and configured for TLS connection to SNI hostname `{}`",
                           m_endpoint.get_address());
        SSL_set_connect_state(m_ssl);
        return true;
    }

    bool setup_quic()
        requires(protocol == Protocol::QUIC)
    {
        add_quic_bio();

        m_ssl_ctx = SSL_CTX_new(OSSL_QUIC_client_method());
        if (!m_ssl_ctx) {
            core::logger::warning("SocketLib", "Failed to create SSL context for QUIC");
            return false;
        }

        SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, nullptr);
        SSL_CTX_set_default_verify_paths(m_ssl_ctx);

        m_ssl = SSL_new(m_ssl_ctx);
        if (!m_ssl) {
            core::logger::warning("SocketLib", "Failed to create SSL object for QUIC");
            return false;
        }

        SSL_set_bio(m_ssl, m_bio, m_bio);

        if (SSL_set_tlsext_host_name(m_ssl, m_endpoint.get_address().data()) != 1) {
            core::logger::warning("SocketLib", "Failed to set SNI hostname for QUIC");
            return false;
        }

        core::logger::info("SocketLib", "SSL object created and configured for QUIC connection to SNI hostname `{}`",
                           m_endpoint.get_address());
        SSL_set_connect_state(m_ssl);
        return true;
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
    std::optional<std::reference_wrapper<leverage::Leverager<leverage::Context>>> m_leverager;
    bool m_ktls_tx;
    bool m_ktls_rx;
    [[no_unique_address]] OsPayload m_os;
};

} // namespace io::base::socket
