export module server.udp:types;

import std;
import socket;

namespace server::udp {

export class Server {
  public:
    struct RecvResult {
        std::ptrdiff_t bytes{};
        base::Endpoint from{};
    };

    Server() = default;
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) noexcept = default;
    Server &operator=(Server &&) noexcept = default;
    ~Server() = default;

    static Server bind(std::string_view ip, std::uint16_t port);
    static Server bind(std::uint16_t port);

    // I/O
    [[nodiscard]] RecvResult recvfrom(std::span<std::byte> buf, int flags = 0);
    [[nodiscard]] RecvResult recvfrom(void *data, std::size_t len, int flags = 0);
    [[nodiscard]] std::ptrdiff_t sendto(std::span<const std::byte> buf, const base::Endpoint &to, int flags = 0);
    [[nodiscard]] std::ptrdiff_t sendto(const void *data, std::size_t len, const base::Endpoint &to, int flags = 0);

    // Multicast
    void join_multicast(std::string_view group, std::string_view iface = "0.0.0.0");
    void leave_multicast(std::string_view group, std::string_view iface = "0.0.0.0");
    void set_multicast_ttl(std::uint8_t ttl);

    // Options
    void set_nonblocking(bool on);
    void set_recv_buf(int bytes);
    void set_recv_timeout(base::timeout_t t) noexcept;

    [[nodiscard]] bool valid() const noexcept { return m_sock.valid(); }

    void close() noexcept;

  private:
    base::Socket m_sock{};
};

} // namespace server::udp
