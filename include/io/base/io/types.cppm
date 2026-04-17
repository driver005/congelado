module;

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h> // sockaddr_storage, socklen_t
#include <ws2tcpip.h>
#else
#include <sys/socket.h> // sockaddr_storage, socklen_t
#endif

export module io_io:types;

import std;
import :buffer;
import :consts;

export namespace io::base::io {

// ─── Op tag encoding ──────────────────────────────────────────────────────────
//
//  63      56 | 55     32 | 31       0
//  [ OpCode ] [  fd(24b) ] [ count   ]

enum class OpCode : std::uint8_t {
    READ = 0,
    WRITE = 1,
    POLL = 2,
    RECV = 3,
    SEND = 4,
    ACCEPT = 5,
    CONNECT = 6,
};

[[nodiscard]] constexpr std::uint64_t encode_tag(OpCode kind, int fd, std::uint32_t count) noexcept {
    return (static_cast<std::uint64_t>(kind) << 56) |
           (static_cast<std::uint64_t>(static_cast<std::uint32_t>(fd) & 0x00FF'FFFFu) << 32) |
           static_cast<std::uint64_t>(count);
}

[[nodiscard]] constexpr OpCode tag_kind(std::uint64_t t) noexcept { return static_cast<OpCode>(t >> 56); }
[[nodiscard]] constexpr int tag_fd(std::uint64_t t) noexcept { return static_cast<int>((t >> 32) & 0x00FF'FFFFu); }
[[nodiscard]] constexpr std::uint32_t tag_count(std::uint64_t t) noexcept {
    return static_cast<std::uint32_t>(t & 0xFFFF'FFFFu);
}

// ─── Completion event ─────────────────────────────────────────────────────────

struct CompletionEvent {
    std::uint64_t tag;   // encoded via encode_tag()
    std::int32_t result; // bytes transferred (>0), 0 = EOF, <0 = -errno / WSA error
    std::uint32_t flags; // platform-specific (e.g. IORING_CQE_F_MORE on Linux)
};

// ─── IO<State> ────────────────────────────────────────────────────────────────
//
// State is the platform-specific storage struct, owned by value inside IO —
// zero heap allocation, zero pointer indirection.
//
// The platform partition defines:
//   struct PosixState / Win32State    — all platform fields
//   all IO<ThatState> method bodies   — explicit instantiation in that TU
//   using PlatformIO = IO<PosixState> — the alias consumers import
//
// The build system links exactly one platform TU; the other is never compiled.

template <typename State>
class IO {
  public:
    explicit IO(RingBuffer &buffer, unsigned entries = 128);
    ~IO();

    IO(const IO &) = delete;
    IO &operator=(const IO &) = delete;
    IO(IO &&) = delete;
    IO &operator=(IO &&) = delete;

    // ── Submission ────────────────────────────────────────────────────────────
    // Linux:   queued into the SQ, flushed by submit().
    // Windows: live on post; submit() is a no-op returning 0.

    void submit_read(int fd, std::size_t count);
    void submit_write(int fd, std::size_t count);
    void submit_recv(int fd, std::size_t max_datagram);
    void submit_send(int fd, const sockaddr_storage &peer, socklen_t peer_len, std::size_t count);
    void submit_poll(int fd, short events);
    void submit_accept(int fd);
    void submit_connect(int fd, const sockaddr_storage &addr, socklen_t alen);

    // Flush the SQ. Returns submitted count (Linux) or 0 (Windows).
    int submit();

    // ── Completion ────────────────────────────────────────────────────────────
    // Blocks until at least `min` events arrive. Automatically advances the
    // RingBuffer: commit_write for READ/RECV, advance_read for WRITE/SEND.

    std::vector<CompletionEvent> wait_completions(unsigned min = 1);

    // UDP peer address captured from the most recent successful RECV.
    [[nodiscard]] const sockaddr_storage &last_peer() const noexcept;
    [[nodiscard]] socklen_t last_peer_len() const noexcept;

    // Raw platform handle as uintptr_t.
    // Linux:   reinterpret_cast<io_uring*>(h)
    // Windows: reinterpret_cast<HANDLE>(h)
    [[nodiscard]] std::uintptr_t native_handle() const noexcept;

    // Direct access to platform storage for advanced / setsockopt callers.
    [[nodiscard]] State &platform_state() noexcept { return state_; }
    [[nodiscard]] const State &platform_state() const noexcept { return state_; }

  private:
    State state_;
};

} // namespace io::base::io
