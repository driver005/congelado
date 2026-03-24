module;

#include <netinet/in.h>
#include <netinet/tcp.h>

export module client.tcp:posix;

import std;
import socket;
import :types;

namespace client::tcp {

Client Client::connect(std::string_view host, std::uint16_t port) {
    Client c;
    c.m_peer = base::Endpoint(host, port, SOCK_STREAM);
    c.m_sock.create_tcp();
    c.m_sock.connect(c.m_peer);
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

void Client::set_nonblocking(bool on) { m_sock.set_nonblocking(on); }
void Client::set_recv_timeout(base::timeout_t t) noexcept { m_sock.set_recv_timeout(t); }
void Client::set_send_timeout(base::timeout_t t) noexcept { m_sock.set_send_timeout(t); }

void Client::set_nodelay(bool on) {
    int v = on ? 1 : 0;
    ::setsockopt(static_cast<int>(m_sock.native_fd()), IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
}

} // namespace client::tcp
