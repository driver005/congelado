module;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

export module tcp:win32;

import std;
import socket;
import :types;

namespace transport::tcp {

std::ptrdiff_t Connection::send(std::span<const std::byte> buf, int flags) { return m_sock.send(buf, flags); }

std::ptrdiff_t Connection::send(const void *data, std::size_t len, int flags) {
    return send(std::span{static_cast<const std::byte *>(data), len}, flags);
}

std::ptrdiff_t Connection::recv(std::span<std::byte> buf, int flags) { return m_sock.recv(buf, flags); }

std::ptrdiff_t Connection::recv(void *data, std::size_t len, int flags) {
    return recv(std::span{static_cast<std::byte *>(data), len}, flags);
}

void Connection::close() noexcept { m_sock.close(); }
void Connection::set_nonblocking(bool on) { m_sock.set_nonblocking(on); }
void Connection::set_recv_timeout(base::timeout_t t) noexcept { m_sock.set_recv_timeout(t); }
void Connection::set_send_timeout(base::timeout_t t) noexcept { m_sock.set_send_timeout(t); }

void Connection::set_nodelay(bool on) {
    BOOL v = on ? TRUE : FALSE;
    ::setsockopt(static_cast<SOCKET>(m_sock.native_fd()), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&v),
                 sizeof(v));
}

Server Server::listen(std::string_view ip, std::uint16_t port, int backlog) {
    Server s;
    s.m_sock.create_tcp();
    s.m_sock.set_reuseaddr(true);
    s.m_sock.bind(base::Endpoint(ip, port, SOCK_STREAM));
    s.m_sock.listen(backlog);
    return s;
}

Server Server::listen(std::uint16_t port, int backlog) { return listen("0.0.0.0", port, backlog); }

Connection Server::accept() {
    auto [client, peer] = m_sock.accept_with_peer();
    return Connection(std::move(client), std::move(peer));
}

void Server::close() noexcept { m_sock.close(); }
void Server::set_nonblocking(bool on) { m_sock.set_nonblocking(on); }

} // namespace transport::tcp
