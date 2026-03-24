module;

#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

export module socket:posix;

import std;
import :types;
import modules.socket;
import modules.net;
import modules.netdb;
import modules.unistd;
import modules.fcntl;

namespace base {


// Reads the sockaddr_in already copied into storage and populates ip/port.

void Endpoint::fill_from_sockaddr() {
    const auto *sa = reinterpret_cast<const sockaddr_in *>(storage);
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
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
        if (::inet_pton(AF_INET, host.data(), &sa->sin_addr) != 1)
            throw SocketError(errno, "inet_pton");
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
        throw std::runtime_error(std::string("getaddrinfo: ") + ::gai_strerror(rc));

    std::memcpy(storage, res->ai_addr, sizeof(sockaddr_in));
    ::freeaddrinfo(res);
    fill_from_sockaddr();
}


struct Socket::Impl {
    int fd{-1};

    [[noreturn]] static void throw_last(std::string_view ctx) { throw SocketError(errno, ctx); }
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
    if (impl && impl->fd != -1) {
        ::close(impl->fd);
        impl->fd = -1;
    }
}

void Socket::create_tcp() {
    impl->fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (impl->fd < 0)
        impl->throw_last("socket(TCP)");
}

void Socket::create_udp() {
    impl->fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (impl->fd < 0)
        impl->throw_last("socket(UDP)");
}

void Socket::bind(const Endpoint &ep) {
    if (::bind(impl->fd, reinterpret_cast<const sockaddr *>(ep.storage), sizeof(sockaddr_in)) < 0)
        impl->throw_last("bind");
}

void Socket::listen(int backlog) {
    if (::listen(impl->fd, backlog) < 0)
        impl->throw_last("listen");
}

void Socket::connect(const Endpoint &ep) {
    if (::connect(impl->fd, reinterpret_cast<const sockaddr *>(ep.storage), sizeof(sockaddr_in)) < 0)
        impl->throw_last("connect");
}

Socket Socket::accept() {
    sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    int client = ::accept(impl->fd, reinterpret_cast<sockaddr *>(&peer), &len);
    if (client < 0)
        impl->throw_last("accept");
    Socket s{};
    s.impl->fd = client;
    return s;
}

std::pair<Socket, Endpoint> Socket::accept_with_peer() {
    sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    int client = ::accept(impl->fd, reinterpret_cast<sockaddr *>(&peer), &len);
    if (client < 0)
        impl->throw_last("accept");
    Socket s{};
    s.impl->fd = client;
    Endpoint ep{};
    std::memcpy(ep.storage, &peer, sizeof(peer));
    ep.fill_from_sockaddr();
    return {std::move(s), std::move(ep)};
}

std::ptrdiff_t Socket::send(std::span<const std::byte> buf, int flags) {
    ssize_t n = ::send(impl->fd, buf.data(), buf.size(), flags);
    if (n < 0)
        impl->throw_last("send");
    return static_cast<std::ptrdiff_t>(n);
}

std::ptrdiff_t Socket::recv(std::span<std::byte> buf, int flags) {
    ssize_t n = ::recv(impl->fd, buf.data(), buf.size(), flags);
    if (n < 0)
        impl->throw_last("recv");
    return static_cast<std::ptrdiff_t>(n);
}

std::ptrdiff_t Socket::sendto(std::span<const std::byte> buf, const Endpoint &to, int flags) {
    ssize_t n = ::sendto(impl->fd, buf.data(), buf.size(), flags, reinterpret_cast<const sockaddr *>(to.storage),
                         sizeof(sockaddr_in));
    if (n < 0)
        impl->throw_last("sendto");
    return static_cast<std::ptrdiff_t>(n);
}

RecvFromResult Socket::recvfrom(std::span<std::byte> buf, int flags) {
    sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    ssize_t n = ::recvfrom(impl->fd, buf.data(), buf.size(), flags, reinterpret_cast<sockaddr *>(&peer), &len);
    if (n < 0)
        impl->throw_last("recvfrom");
    Endpoint ep{};
    std::memcpy(ep.storage, &peer, sizeof(peer));
    ep.fill_from_sockaddr();
    return {static_cast<std::ptrdiff_t>(n), std::move(ep)};
}

void Socket::set_reuseaddr(bool on) noexcept {
    int v = on ? 1 : 0;
    ::setsockopt(impl->fd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
}

void Socket::set_nonblocking(bool on) {
    int flags = ::fcntl(impl->fd, F_GETFL, 0);
    if (flags < 0)
        impl->throw_last("fcntl(F_GETFL)");
    flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (::fcntl(impl->fd, F_SETFL, flags) < 0)
        impl->throw_last("fcntl(F_SETFL)");
}

void Socket::set_recv_timeout(timeout_t t) noexcept {
    struct timeval tv{t.sec, static_cast<suseconds_t>(t.usec)};
    ::setsockopt(impl->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void Socket::set_send_timeout(timeout_t t) noexcept {
    struct timeval tv{t.sec, static_cast<suseconds_t>(t.usec)};
    ::setsockopt(impl->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

bool Socket::valid() const noexcept { return impl && impl->fd != -1; }

std::uintptr_t Socket::native_fd() const noexcept {
    return impl ? static_cast<std::uintptr_t>(impl->fd) : static_cast<std::uintptr_t>(-1);
}

} // namespace base
