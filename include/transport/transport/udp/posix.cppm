module;

#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>

export module udp:posix;

import std;
import socket;
import modules.net;
import :types;

namespace transport::udp {

Server Server::bind(std::string_view ip, std::uint16_t port) {
    Server s;
    s.m_sock.create_udp();
    s.m_sock.set_reuseaddr(true);
    s.m_sock.bind(base::Endpoint(ip, port, SOCK_DGRAM));
    return s;
}

Server Server::bind(std::uint16_t port) { return bind("0.0.0.0", port); }

Server::RecvResult Server::recvfrom(std::span<std::byte> buf, int flags) {
    auto [n, from] = m_sock.recvfrom(buf, flags);
    return {n, std::move(from)};
}

Server::RecvResult Server::recvfrom(void *data, std::size_t len, int flags) {
    return recvfrom(std::span{static_cast<std::byte *>(data), len}, flags);
}

std::ptrdiff_t Server::sendto(std::span<const std::byte> buf, const base::Endpoint &to, int flags) {
    return m_sock.sendto(buf, to, flags);
}

std::ptrdiff_t Server::sendto(const void *data, std::size_t len, const base::Endpoint &to, int flags) {
    return sendto(std::span{static_cast<const std::byte *>(data), len}, to, flags);
}

void Server::join_multicast(std::string_view group, std::string_view iface) {
    ip_mreq mreq{};
    ::inet_pton(AF_INET, group.data(), &mreq.imr_multiaddr);
    ::inet_pton(AF_INET, iface.data(), &mreq.imr_interface);
    if (::setsockopt(static_cast<int>(m_sock.native_fd()), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        throw base::SocketError(errno, "setsockopt(IP_ADD_MEMBERSHIP)");
}

void Server::leave_multicast(std::string_view group, std::string_view iface) {
    ip_mreq mreq{};
    ::inet_pton(AF_INET, group.data(), &mreq.imr_multiaddr);
    ::inet_pton(AF_INET, iface.data(), &mreq.imr_interface);
    if (::setsockopt(static_cast<int>(m_sock.native_fd()), IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        throw base::SocketError(errno, "setsockopt(IP_DROP_MEMBERSHIP)");
}

void Server::set_multicast_ttl(std::uint8_t ttl) {
    ::setsockopt(static_cast<int>(m_sock.native_fd()), IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

void Server::set_recv_buf(int bytes) {
    ::setsockopt(static_cast<int>(m_sock.native_fd()), SOL_SOCKET, SO_RCVBUF, &bytes, sizeof(bytes));
}

void Server::close() noexcept { m_sock.close(); }
void Server::set_nonblocking(bool on) { m_sock.set_nonblocking(on); }
void Server::set_recv_timeout(base::timeout_t t) noexcept { m_sock.set_recv_timeout(t); }

} // namespace transport::udp
