module;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

export module socket:win32;

import std;
import :types;

namespace base {

namespace detail {
struct WsaGuard {
    WsaGuard() {
        WSADATA d{};
        if (::WSAStartup(MAKEWORD(2, 2), &d) != 0)
            throw SocketError(::WSAGetLastError(), "WSAStartup");
    }
    ~WsaGuard() { ::WSACleanup(); }
    WsaGuard(const WsaGuard &) = delete;
    WsaGuard &operator=(const WsaGuard &) = delete;
};
static WsaGuard wsa_init;
} // namespace detail


void Endpoint::fill_from_sockaddr() {
    const auto *sa = reinterpret_cast<const sockaddr_in *>(storage);
    char buf[INET_ADDRSTRLEN];
    ::InetNtopA(AF_INET, const_cast<IN_ADDR *>(&sa->sin_addr), buf, sizeof(buf));
    ip = buf;
    port = ntohs(sa->sin_port);
}


Endpoint::Endpoint(std::string_view host, std::uint16_t port) {
    auto *sa = reinterpret_cast<sockaddr_in *>(storage);
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    if (host.empty() || host == "0.0.0.0") {
        sa->sin_addr.s_addr = INADDR_ANY;
        ip = "0.0.0.0";
    } else {
        if (::InetPtonA(AF_INET, host.data(), &sa->sin_addr) != 1)
            throw SocketError(::WSAGetLastError(), "InetPton");
        ip = std::string(host);
    }
    this->port = port;
}

Endpoint::Endpoint(std::string_view host, std::uint16_t port, int socktype) {
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    const std::string ps = std::to_string(port);

    int rc = ::getaddrinfo(host.data(), ps.c_str(), &hints, &res);
    if (rc != 0)
        throw std::runtime_error("getaddrinfo error: " + std::to_string(rc));

    std::memcpy(storage, res->ai_addr, sizeof(sockaddr_in));
    ::freeaddrinfo(res);
    fill_from_sockaddr();
}


struct Socket::Impl {
    SOCKET fd{INVALID_SOCKET};

    [[noreturn]] static void throw_last(std::string_view ctx) { throw SocketError(::WSAGetLastError(), ctx); }
};


Socket::Socket() : impl(new Impl{}) {}
Socket::Socket(Socket &&o) noexcept : impl(o.impl) { o.impl = nullptr; }
Socket::~Socket() {
    close();
    delete impl;
}

Socket &Socket::operator=(Socket &&o) noexcept {
    if (this != &o) {
        close();
        delete impl;
        impl = o.impl;
        o.impl = nullptr;
    }
    return *this;
}

void Socket::close() noexcept {
    if (impl && impl->fd != INVALID_SOCKET) {
        ::closesocket(impl->fd);
        impl->fd = INVALID_SOCKET;
    }
}

void Socket::create_tcp() {
    impl->fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (impl->fd == INVALID_SOCKET)
        impl->throw_last("socket(TCP)");
}

void Socket::create_udp() {
    impl->fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (impl->fd == INVALID_SOCKET)
        impl->throw_last("socket(UDP)");
}

void Socket::bind(const Endpoint &ep) {
    if (::bind(impl->fd, reinterpret_cast<const sockaddr *>(ep.storage), sizeof(sockaddr_in)) == SOCKET_ERROR)
        impl->throw_last("bind");
}

void Socket::listen(int backlog) {
    if (::listen(impl->fd, backlog) == SOCKET_ERROR)
        impl->throw_last("listen");
}

void Socket::connect(const Endpoint &ep) {
    if (::connect(impl->fd, reinterpret_cast<const sockaddr *>(ep.storage), sizeof(sockaddr_in)) == SOCKET_ERROR)
        impl->throw_last("connect");
}

Socket Socket::accept() {
    sockaddr_in peer{};
    int len = sizeof(peer);
    SOCKET client = ::accept(impl->fd, reinterpret_cast<sockaddr *>(&peer), &len);
    if (client == INVALID_SOCKET)
        impl->throw_last("accept");
    Socket s{};
    s.impl->fd = client;
    return s;
}

std::pair<Socket, Endpoint> Socket::accept_with_peer() {
    sockaddr_in peer{};
    int len = sizeof(peer);
    SOCKET client = ::accept(impl->fd, reinterpret_cast<sockaddr *>(&peer), &len);
    if (client == INVALID_SOCKET)
        impl->throw_last("accept");
    Socket s{};
    s.impl->fd = client;
    Endpoint ep{};
    std::memcpy(ep.storage, &peer, sizeof(peer));
    ep.fill_from_sockaddr();
    return {std::move(s), std::move(ep)};
}

std::ptrdiff_t Socket::send(std::span<const std::byte> buf, int flags) {
    int n = ::send(impl->fd, reinterpret_cast<const char *>(buf.data()), static_cast<int>(buf.size()), flags);
    if (n == SOCKET_ERROR)
        impl->throw_last("send");
    return static_cast<std::ptrdiff_t>(n);
}

std::ptrdiff_t Socket::recv(std::span<std::byte> buf, int flags) {
    int n = ::recv(impl->fd, reinterpret_cast<char *>(buf.data()), static_cast<int>(buf.size()), flags);
    if (n == SOCKET_ERROR)
        impl->throw_last("recv");
    return static_cast<std::ptrdiff_t>(n);
}

std::ptrdiff_t Socket::sendto(std::span<const std::byte> buf, const Endpoint &to, int flags) {
    int n = ::sendto(impl->fd, reinterpret_cast<const char *>(buf.data()), static_cast<int>(buf.size()), flags,
                     reinterpret_cast<const sockaddr *>(to.storage), sizeof(sockaddr_in));
    if (n == SOCKET_ERROR)
        impl->throw_last("sendto");
    return static_cast<std::ptrdiff_t>(n);
}

RecvFromResult Socket::recvfrom(std::span<std::byte> buf, int flags) {
    sockaddr_in peer{};
    int len = sizeof(peer);
    int n = ::recvfrom(impl->fd, reinterpret_cast<char *>(buf.data()), static_cast<int>(buf.size()), flags,
                       reinterpret_cast<sockaddr *>(&peer), &len);
    if (n == SOCKET_ERROR)
        impl->throw_last("recvfrom");
    Endpoint ep{};
    std::memcpy(ep.storage, &peer, sizeof(peer));
    ep.fill_from_sockaddr();
    return {static_cast<std::ptrdiff_t>(n), std::move(ep)};
}

void Socket::set_reuseaddr(bool on) noexcept {
    BOOL v = on ? TRUE : FALSE;
    ::setsockopt(impl->fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&v), sizeof(v));
}

void Socket::set_nonblocking(bool on) {
    u_long mode = on ? 1UL : 0UL;
    if (::ioctlsocket(impl->fd, FIONBIO, &mode) != 0)
        impl->throw_last("ioctlsocket(FIONBIO)");
}

void Socket::set_recv_timeout(timeout_t t) noexcept {
    DWORD ms = static_cast<DWORD>(t.sec * 1000 + t.usec / 1000);
    ::setsockopt(impl->fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&ms), sizeof(ms));
}

void Socket::set_send_timeout(timeout_t t) noexcept {
    DWORD ms = static_cast<DWORD>(t.sec * 1000 + t.usec / 1000);
    ::setsockopt(impl->fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&ms), sizeof(ms));
}

bool Socket::valid() const noexcept { return impl && impl->fd != INVALID_SOCKET; }

std::uintptr_t Socket::native_fd() const noexcept {
    return impl ? static_cast<std::uintptr_t>(impl->fd) : static_cast<std::uintptr_t>(INVALID_SOCKET);
}

} // namespace base
