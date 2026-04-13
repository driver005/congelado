module;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <mswsock.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

export module io_io:win32;

import std;
import :types;
import :buffer;
import :consts;

export namespace transport::base::io {

// ─── WsaGuard ────────────────────────────────────────────────────────────────

class WsaGuard {
  public:
    WsaGuard() {
        WSADATA wd{};
        if (const int rc = WSAStartup(MAKEWORD(2, 2), &wd); rc != 0)
            throw std::system_error(rc, std::system_category(), "WSAStartup");
    }
    ~WsaGuard() { WSACleanup(); }

    WsaGuard(const WsaGuard &) = delete;
    WsaGuard &operator=(const WsaGuard &) = delete;
};

// ─── Win32Overlapped ─────────────────────────────────────────────────────────
// Per-operation context threaded through OVERLAPPED. Must be first member.

struct Win32Overlapped {
    OVERLAPPED ov{}; // ← first — aliased via LPOVERLAPPED cast
    std::uint64_t tag{};
    WSABUF wsabuf{};
    sockaddr_storage peer{};
    INT peer_len{sizeof(sockaddr_storage)};

    static constexpr DWORD addr_buf_len = sizeof(sockaddr_storage) + 16;
    std::array<char, addr_buf_len * 2> accept_buf{};
    SOCKET accept_socket{INVALID_SOCKET};
};

// ─── Win32State ───────────────────────────────────────────────────────────────
//
// All IOCP-specific fields. Owned by value inside IO<Win32State>.

struct Win32State {
    HANDLE iocp{nullptr};
    RingBuffer *buffer{};
    sockaddr_storage last_peer{};
    socklen_t last_peer_len{};
    LPFN_CONNECTEX fn_connectex{nullptr};

    Win32State() = default;

    void init(RingBuffer &buf) {
        buffer = &buf;
        iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        if (!iocp)
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "CreateIoCompletionPort");
    }

    void destroy() noexcept {
        if (iocp) {
            CloseHandle(iocp);
            iocp = nullptr;
        }
    }

    // Must be called once per socket before posting operations.
    void associate(SOCKET s, std::uintptr_t key = 0) {
        if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(s), iocp, key, 0))
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "associate");
    }

    [[nodiscard]] Win32Overlapped *alloc(OpCode op, int fd) {
        auto *ov = new Win32Overlapped{};
        ov->tag = encode_tag(op, fd, 0);
        return ov;
    }

    static void free(Win32Overlapped *ov) noexcept { delete ov; }

    static void check_pending(const char *ctx) {
        if (const int err = WSAGetLastError(); err != WSA_IO_PENDING)
            throw std::system_error(err, std::system_category(), ctx);
    }

    LPFN_CONNECTEX get_connectex(SOCKET s) {
        if (fn_connectex)
            return fn_connectex;
        GUID guid = WSAID_CONNECTEX;
        DWORD bytes{};
        if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &fn_connectex, sizeof(fn_connectex),
                     &bytes, nullptr, nullptr) == SOCKET_ERROR)
            throw std::system_error(WSAGetLastError(), std::system_category(), "WSAIoctl ConnectEx");
        return fn_connectex;
    }
};

// ─── Platform alias ───────────────────────────────────────────────────────────

using PlatformIO = IO<Win32State>;

} // namespace transport::base::io

// ─── IO<Win32State> method bodies ─────────────────────────────────────────────

namespace transport::base::io {

template <>
IO<Win32State>::IO(RingBuffer &buffer, unsigned /*entries — IOCP queue is unbounded*/) {
    state_.init(buffer);
}

template <>
IO<Win32State>::~IO() {
    state_.destroy();
}

template <>
void IO<Win32State>::submit_read(int fd, std::size_t count) {
    auto span = state_.buffer->get_writable_span();
    if (span.empty())
        throw BufferOverflowException{};
    auto *ov = state_.alloc(OpCode::READ, fd);
    ov->wsabuf = {static_cast<ULONG>(std::min(count, span.size())), reinterpret_cast<char *>(span.data())};
    DWORD flags{}, bytes{};
    if (WSARecv(static_cast<SOCKET>(fd), &ov->wsabuf, 1, &bytes, &flags, &ov->ov, nullptr) == SOCKET_ERROR)
        Win32State::check_pending("WSARecv");
}

template <>
void IO<Win32State>::submit_write(int fd, std::size_t count) {
    auto span = state_.buffer->get_readable_span();
    if (span.empty())
        throw std::logic_error("submit_write: empty buffer");
    auto *ov = state_.alloc(OpCode::WRITE, fd);
    ov->wsabuf = {static_cast<ULONG>(std::min(count, span.size())),
                  reinterpret_cast<char *>(const_cast<unsigned char *>(span.data()))};
    DWORD bytes{};
    if (WSASend(static_cast<SOCKET>(fd), &ov->wsabuf, 1, &bytes, 0, &ov->ov, nullptr) == SOCKET_ERROR)
        Win32State::check_pending("WSASend");
}

template <>
void IO<Win32State>::submit_recv(int fd, std::size_t max_datagram) {
    auto span = state_.buffer->get_writable_span();
    if (span.empty())
        throw BufferOverflowException{};
    auto *ov = state_.alloc(OpCode::RECV, fd);
    ov->wsabuf = {static_cast<ULONG>(std::min(max_datagram, span.size())), reinterpret_cast<char *>(span.data())};
    ov->peer_len = sizeof(sockaddr_storage);
    DWORD flags{}, bytes{};
    if (WSARecvFrom(static_cast<SOCKET>(fd), &ov->wsabuf, 1, &bytes, &flags, reinterpret_cast<sockaddr *>(&ov->peer),
                    &ov->peer_len, &ov->ov, nullptr) == SOCKET_ERROR)
        Win32State::check_pending("WSARecvFrom");
}

template <>
void IO<Win32State>::submit_send(int fd, const sockaddr_storage &peer, socklen_t peer_len, std::size_t count) {
    auto span = state_.buffer->get_readable_span();
    if (span.empty())
        throw std::logic_error("submit_send: empty buffer");
    auto *ov = state_.alloc(OpCode::SEND, fd);
    ov->wsabuf = {static_cast<ULONG>(std::min(count, span.size())),
                  reinterpret_cast<char *>(const_cast<unsigned char *>(span.data()))};
    DWORD bytes{};
    if (WSASendTo(static_cast<SOCKET>(fd), &ov->wsabuf, 1, &bytes, 0, reinterpret_cast<const sockaddr *>(&peer),
                  peer_len, &ov->ov, nullptr) == SOCKET_ERROR)
        Win32State::check_pending("WSASendTo");
}

template <>
void IO<Win32State>::submit_poll(int fd, short events) {
    if (!(events & POLLIN))
        return;
    auto *ov = state_.alloc(OpCode::POLL, fd);
    ov->wsabuf = {0, nullptr};
    DWORD flags{}, bytes{};
    if (WSARecv(static_cast<SOCKET>(fd), &ov->wsabuf, 1, &bytes, &flags, &ov->ov, nullptr) == SOCKET_ERROR)
        Win32State::check_pending("WSARecv (poll)");
}

template <>
void IO<Win32State>::submit_accept(int fd) {
    SOCKET accept_sock = WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (accept_sock == INVALID_SOCKET)
        throw std::system_error(WSAGetLastError(), std::system_category(), "WSASocketW");
    auto *ov = state_.alloc(OpCode::ACCEPT, fd);
    ov->accept_socket = accept_sock;
    DWORD bytes{};
    if (!AcceptEx(static_cast<SOCKET>(fd), accept_sock, ov->accept_buf.data(), 0, Win32Overlapped::addr_buf_len,
                  Win32Overlapped::addr_buf_len, &bytes, &ov->ov))
        Win32State::check_pending("AcceptEx");
}

template <>
void IO<Win32State>::submit_connect(int fd, const sockaddr_storage &addr, socklen_t alen) {
    auto *fn = state_.get_connectex(static_cast<SOCKET>(fd));
    auto *ov = state_.alloc(OpCode::CONNECT, fd);
    DWORD bytes{};
    if (!fn(static_cast<SOCKET>(fd), reinterpret_cast<const sockaddr *>(&addr), alen, nullptr, 0, &bytes, &ov->ov))
        Win32State::check_pending("ConnectEx");
}

template <>
int IO<Win32State>::submit() {
    return 0;
}

template <>
std::vector<CompletionEvent> IO<Win32State>::wait_completions(unsigned min) {
    constexpr ULONG max_batch = 64;
    std::array<OVERLAPPED_ENTRY, max_batch> entries{};
    ULONG count{};
    std::vector<CompletionEvent> out;

    while (out.size() < min) {
        const DWORD timeout = (out.empty() && min == 1) ? INFINITE : 50;
        if (!GetQueuedCompletionStatusEx(state_.iocp, entries.data(), max_batch, &count, timeout, FALSE)) {
            const DWORD err = GetLastError();
            if (err == WAIT_TIMEOUT)
                continue;
            throw std::system_error(static_cast<int>(err), std::system_category(), "GetQueuedCompletionStatusEx");
        }

        for (ULONG i = 0; i < count; ++i) {
            auto *ov = reinterpret_cast<Win32Overlapped *>(entries[i].lpOverlapped);
            const auto res = static_cast<std::int32_t>(entries[i].dwNumberOfBytesTransferred);

            if (res > 0) {
                const auto bytes = static_cast<std::size_t>(res);
                switch (tag_kind(ov->tag)) {
                case OpCode::READ:
                case OpCode::RECV:
                    state_.buffer->commit_write(bytes);
                    break;
                case OpCode::WRITE:
                case OpCode::SEND:
                    state_.buffer->advance_read(bytes);
                    break;
                default:
                    break;
                }
                if (tag_kind(ov->tag) == OpCode::RECV) {
                    state_.last_peer = ov->peer;
                    state_.last_peer_len = static_cast<socklen_t>(ov->peer_len);
                }
            }

            out.push_back({ov->tag, res, 0u});
            Win32State::free(ov);
        }
    }
    return out;
}

template <>
const sockaddr_storage &IO<Win32State>::last_peer() const noexcept {
    return state_.last_peer;
}

template <>
socklen_t IO<Win32State>::last_peer_len() const noexcept {
    return state_.last_peer_len;
}

template <>
std::uintptr_t IO<Win32State>::native_handle() const noexcept {
    return reinterpret_cast<std::uintptr_t>(state_.iocp);
}

} // namespace transport::base::io
