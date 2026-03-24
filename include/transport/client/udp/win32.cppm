module;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

export module server.udp:win32;

import std;
import socket;
import :types;

namespace server::udp {

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
    ::InetPtonA(AF_INET, group.data(), &mreq.imr_multiaddr);
    ::InetPtonA(AF_INET, iface.data(), &mreq.imr_interface);
    if (::setsockopt(static_cast<SOCKET>(m_sock.native_fd()), IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     reinterpret_cast<const char *>(&mreq), sizeof(mreq)) == SOCKET_ERROR)
        throw base::SocketError(::WSAGetLastError(), "setsockopt(IP_ADD_MEMBERSHIP)");
}

void Server::leave_multicast(std::string_view group, std::string_view iface) {
    ip_mreq mreq{};
    ::InetPtonA(AF_INET, group.data(), &mreq.imr_multiaddr);
    ::InetPtonA(AF_INET, iface.data(), &mreq.imr_interface);
    if (::setsockopt(static_cast<SOCKET>(m_sock.native_fd()), IPPROTO_IP, IP_DROP_MEMBERSHIP,
                     reinterpret_cast<const char *>(&mreq), sizeof(mreq)) == SOCKET_ERROR)
        throw base::SocketError(::WSAGetLastError(), "setsockopt(IP_DROP_MEMBERSHIP)");
}

void Server::set_multicast_ttl(std::uint8_t ttl) {
    ::setsockopt(static_cast<SOCKET>(m_sock.native_fd()), IPPROTO_IP, IP_MULTICAST_TTL,
                 reinterpret_cast<const char *>(&ttl), sizeof(ttl));
}

void Server::set_recv_buf(int bytes) {
    ::setsockopt(static_cast<SOCKET>(m_sock.native_fd()), SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&bytes),
                 sizeof(bytes));
}

void Server::close() noexcept { m_sock.close(); }
void Server::set_nonblocking(bool on) { m_sock.set_nonblocking(on); }
void Server::set_recv_timeout(base::timeout_t t) noexcept { m_sock.set_recv_timeout(t); }

} // namespace server::udp
