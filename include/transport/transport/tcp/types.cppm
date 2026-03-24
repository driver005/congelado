export module tcp:types;

import std;
import socket;

namespace transport::tcp {

export class Connection {
  public:
    Connection() = default;
    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;
    Connection(Connection &&) noexcept = default;
    Connection &operator=(Connection &&) noexcept = default;
    ~Connection() = default;

    // I/O
    [[nodiscard]] std::ptrdiff_t send(std::span<const std::byte> buf, int flags = 0);
    [[nodiscard]] std::ptrdiff_t send(const void *data, std::size_t len, int flags = 0);
    [[nodiscard]] std::ptrdiff_t recv(std::span<std::byte> buf, int flags = 0);
    [[nodiscard]] std::ptrdiff_t recv(void *data, std::size_t len, int flags = 0);

    // Options
    void set_nonblocking(bool on);
    void set_nodelay(bool on);
    void set_recv_timeout(base::timeout_t t) noexcept;
    void set_send_timeout(base::timeout_t t) noexcept;

    // Accessors
    [[nodiscard]] bool valid() const noexcept { return m_sock.valid(); }
    [[nodiscard]] std::string peer() const { return m_peer.to_string(); }
    [[nodiscard]] const base::Endpoint &peer_endpoint() const noexcept { return m_peer; }
    [[nodiscard]] std::uintptr_t native_fd() const noexcept { return m_sock.native_fd(); }

    void close() noexcept;

  private:
    friend class Server;
    Connection(base::Socket sock, base::Endpoint peer) : m_sock(std::move(sock)), m_peer(std::move(peer)) {}

    base::Socket m_sock{};
    base::Endpoint m_peer{};
};

export class Server {
  public:
    Server() = default;
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) noexcept = default;
    Server &operator=(Server &&) noexcept = default;
    ~Server() = default;

    static Server listen(std::string_view ip, std::uint16_t port, int backlog = 128);
    static Server listen(std::uint16_t port, int backlog = 128);

    [[nodiscard]] Connection accept();

    void set_nonblocking(bool on);

    [[nodiscard]] bool valid() const noexcept { return m_sock.valid(); }

    void close() noexcept;

  private:
    base::Socket m_sock{};
};

} // namespace transport::tcp
