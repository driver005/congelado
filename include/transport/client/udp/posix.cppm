module;

#include <cerrno>
#include <sys/socket.h>

export module client.udp:posix;

import std;
import socket;
import :types;

namespace client::udp {

Client Client::connect(std::string_view host, std::uint16_t port) {
    Client c;
    c.m_peer = base::Endpoint(host, port, SOCK_DGRAM);
    c.m_sock.create_udp();
    c.m_sock.connect(c.m_peer);
    return c;
}

Client Client::create() {
    Client c;
    c.m_sock.create_udp();
    return c;
}

std::ptrdiff_t Client::send(std::span<const std::byte> buf, int flags) { return m_sock.send(buf, flags); }

std::ptrdiff_t Client::send(const void *data, std::size_t len, int flags) {
    return send(std::span{static_cast<const std::byte *>(data), len}, flags);
}

std::ptrdiff_t Client::recv(std::span<std::byte> buf, int flags) { return m_sock.recv(buf, flags); }

std::ptrdiff_t Client::recv(void *data, std::size_t len, int flags) {
    return recv(std::span{static_cast<std::byte *>(data), len}, flags);
}

std::ptrdiff_t Client::sendto(std::span<const std::byte> buf, const base::Endpoint &to, int flags) {
    return m_sock.sendto(buf, to, flags);
}

std::ptrdiff_t Client::sendto(const void *data, std::size_t len, const base::Endpoint &to, int flags) {
    return sendto(std::span{static_cast<const std::byte *>(data), len}, to, flags);
}

void Client::set_nonblocking(bool on) { m_sock.set_nonblocking(on); }
void Client::set_recv_timeout(base::timeout_t t) noexcept { m_sock.set_recv_timeout(t); }

void Client::set_send_buf(int bytes) {
    ::setsockopt(static_cast<int>(m_sock.native_fd()), SOL_SOCKET, SO_SNDBUF, &bytes, sizeof(bytes));
}

void Client::set_broadcast(bool on) {
    int v = on ? 1 : 0;
    if (::setsockopt(static_cast<int>(m_sock.native_fd()), SOL_SOCKET, SO_BROADCAST, &v, sizeof(v)) < 0)
        throw base::SocketError(errno, "setsockopt(SO_BROADCAST)");
}

} // namespace client::udp
