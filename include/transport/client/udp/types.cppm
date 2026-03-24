export module client.udp:types;

import std;
import socket;

namespace client::udp {

export class Client {
  public:
    Client() = default;
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&) noexcept = default;
    Client &operator=(Client &&) noexcept = default;
    ~Client() = default;

    static Client connect(std::string_view host, std::uint16_t port);
    static Client create();

    // I/O — connected
    [[nodiscard]] std::ptrdiff_t send(std::span<const std::byte> buf, int flags = 0);
    [[nodiscard]] std::ptrdiff_t send(const void *data, std::size_t len, int flags = 0);
    [[nodiscard]] std::ptrdiff_t recv(std::span<std::byte> buf, int flags = 0);
    [[nodiscard]] std::ptrdiff_t recv(void *data, std::size_t len, int flags = 0);

    // I/O — unconnected
    [[nodiscard]] std::ptrdiff_t sendto(std::span<const std::byte> buf, const base::Endpoint &to, int flags = 0);
    [[nodiscard]] std::ptrdiff_t sendto(const void *data, std::size_t len, const base::Endpoint &to, int flags = 0);

    // Options
    void set_nonblocking(bool on);
    void set_broadcast(bool on);
    void set_send_buf(int bytes);
    void set_recv_timeout(base::timeout_t t) noexcept;

    [[nodiscard]] bool valid() const noexcept { return m_sock.valid(); }
    [[nodiscard]] std::string peer_addr() const { return m_peer.to_string(); }
    void close() noexcept { m_sock.close(); }

  private:
    base::Socket m_sock{};
    base::Endpoint m_peer{};
};

} // namespace client::udp
