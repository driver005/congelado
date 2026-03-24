module;
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
export module socket:types;

import std;

namespace base {

export struct SocketError : std::runtime_error {
    int native_code{};
    SocketError(int code, std::string_view ctx)
        : std::runtime_error(std::string(ctx) + " [" + std::to_string(code) + "]"), native_code(code) {}
};

export class Endpoint {
  public:
    Endpoint(std::string_view host, std::uint16_t port);
    Endpoint(std::string_view host, std::uint16_t port, int socktype = SOCK_STREAM);
    [[nodiscard]] std::string to_string() const { return ip + ":" + std::to_string(port); }

  public:
    // Default-constructs an empty/invalid sentinel endpoint.
    Endpoint() = default;

  private:
    friend class Socket;

    // Fills ip/port from an already-populated sockaddr_in stored in storage.
    // Called by platform Socket implementations after copying raw sockaddr data.
    void fill_from_sockaddr();

    alignas(8) std::byte storage[16]{};
    std::string ip{};
    std::uint16_t port{};
};

export struct timeout_t {
    long sec{};
    long usec{};
};

export struct RecvFromResult {
    std::ptrdiff_t bytes;
    Endpoint from;
};

export class Socket {
  public:
    Socket();
    ~Socket();

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;
    Socket(Socket &&) noexcept;
    Socket &operator=(Socket &&) noexcept;

    void close() noexcept;
    void create_tcp();
    void create_udp();

    void bind(const Endpoint &ep);
    void listen(int backlog);
    void connect(const Endpoint &ep);
    [[nodiscard]] Socket accept();
    [[nodiscard]] std::pair<Socket, Endpoint> accept_with_peer();

    [[nodiscard]] std::ptrdiff_t send(std::span<const std::byte> buf, int flags = 0);
    [[nodiscard]] std::ptrdiff_t recv(std::span<std::byte> buf, int flags = 0);

    [[nodiscard]] std::ptrdiff_t sendto(std::span<const std::byte> buf, const Endpoint &to, int flags = 0);
    [[nodiscard]] RecvFromResult recvfrom(std::span<std::byte> buf, int flags = 0);

    void set_reuseaddr(bool on) noexcept;
    void set_nonblocking(bool on);
    void set_recv_timeout(timeout_t t) noexcept;
    void set_send_timeout(timeout_t t) noexcept;

    [[nodiscard]] bool valid() const noexcept;

    // Raw fd — use only for platform setsockopt calls not covered by the API.
    // On POSIX: int. On Win32: SOCKET (UINT_PTR). Fits in uintptr_t on both.
    [[nodiscard]] std::uintptr_t native_fd() const noexcept;

  private:
    struct Impl;
    Impl *impl{nullptr};
};

} // namespace base
