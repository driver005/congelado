module;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <mswsock.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

export module io_base_leverage:win32;

import std;
import :types;

export namespace transport::base::leverage {

using LPFN_CONNECTEX = BOOL(PASCAL FAR *)(SOCKET s, const struct sockaddr FAR *name, int namelen, PVOID lpSendBuffer,
                                          DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped);

struct pending_op {
    completion_callback callback;
    void *buffer = nullptr;
    unsigned buffer_size = 0;
    off_t offset = 0;
    int op_type = 0;
    OVERLAPPED overlapped{};
    HANDLE handle = nullptr;
    socket_t socket = invalid_socket;
    WSABUF wsabuf{};
};

class Context {
  public:
    explicit Context(int entries)
        : m_iocp_handle{nullptr}, m_pending_ops{0}, m_connect_ex_ptr{nullptr}, m_accept_ex_ptr{nullptr} {
        init(entries);
    };
    ~Context() noexcept { cleanup(); };

    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;
    Context(Context &&) = delete;
    Context &operator=(Context &&) = delete;

    void init(int entries) {
        init_wsa();

        m_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, static_cast<DWORD>(entries));
        if (!m_iocp_handle) {
            panic("CreateIoCompletionPort", static_cast<int>(GetLastError()));
        }

        load_extension_functions();
    };

    void cleanup() noexcept {
        if (m_iocp_handle) {
            CloseHandle(m_iocp_handle);
            m_iocp_handle = nullptr;
        }
    };

    void submit_async(std::unique_ptr<pending_op> op) {
        op->overlapped.hEvent = reinterpret_cast<HANDLE>(op.get());
        m_pending_ops.fetch_add(1);

        // NOLINTNEXTLINE(bugprone-unused-return-value)
        op.release();
    };

    void process_completions(DWORD timeout_ms) {
        DWORD bytes_transferred;
        ULONG_PTR completion_key;
        LPOVERLAPPED overlapped;

        while (true) {
            BOOL result =
                GetQueuedCompletionStatus(m_iocp_handle, &bytes_transferred, &completion_key, &overlapped, timeout_ms);

            if (!result && !overlapped) {
                break;
            }

            auto *op = reinterpret_cast<pending_op *>(overlapped->hEvent);

            int op_result;
            if (result) {
                op_result = static_cast<int>(bytes_transferred);
            } else {
                op_result = -static_cast<int>(GetLastError());
            }

            auto callback = std::move(op->callback);
            delete op;

            m_pending_ops.fetch_sub(1);

            if (callback) {
                callback(op_result);
            }

            if (timeout_ms == 0)
                break;
        }
    };

    [[nodiscard]] HANDLE get_iocp_handle() const noexcept { return m_iocp_handle; }
    [[nodiscard]] LPFN_CONNECTEX get_connect_ex() const noexcept { return m_connect_ex_ptr; }
    [[nodiscard]] LPFN_ACCEPTEX get_accept_ex() const noexcept { return m_accept_ex_ptr; }

    static HANDLE fd_to_handle(int fd) { return reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(fd)); };

  private:
    HANDLE m_iocp_handle;
    std::atomic<int> m_pending_ops;
    LPFN_CONNECTEX m_connect_ex_ptr;
    LPFN_ACCEPTEX m_accept_ex_ptr;

    static inline bool m_wsa_initialized = false;

    static void init_wsa() {
        if (!m_wsa_initialized) {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            m_wsa_initialized = true;
        }
    };

    void load_extension_functions() {
        SOCKET dummy = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (dummy == INVALID_SOCKET)
            return;

        GUID guid_connect_ex = WSAID_CONNECTEX;
        DWORD bytes;
        WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connect_ex, sizeof(guid_connect_ex),
                 &m_connect_ex_ptr, sizeof(m_connect_ex_ptr), &bytes, nullptr, nullptr);

        GUID guid_accept_ex = WSAID_ACCEPTEX;
        WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_accept_ex, sizeof(guid_accept_ex), &m_accept_ex_ptr,
                 sizeof(m_accept_ex_ptr), &bytes, nullptr, nullptr);

        closesocket(dummy);
    }
};

template <>
Leverager<Context>::Leverager(int entries, [[maybe_unused]] std::uint32_t flags, [[maybe_unused]] std::uint32_t wq_fd)
    : m_context{Context{entries}}, m_running{false} {};

template <>
Leverager<Context>::~Leverager() noexcept {
    m_context.~Context();
}

template <>
void Leverager<Context>::run_once() {
    m_context.process_completions(INFINITE);
}

template <>
void Leverager<Context>::run() {
    m_running = true;
    while (m_running) {
        run_once();
    }
}

template <>
void Leverager<Context>::poll() {
    m_context.process_completions(0);
}

template <>
void Leverager<Context>::async_read(int fd, void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                                    [[maybe_unused]] std::uint8_t iflags) {
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->buffer = buf;
    op->buffer_size = nbytes;
    op->offset = offset;
    op->op_type = static_cast<int>(op_type::read);
    op->handle = m_context.fd_to_handle(fd);

    op->overlapped.Offset = static_cast<DWORD>(offset);
    op->overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

    CreateIoCompletionPort(op->handle, m_context.get_iocp_handle(), 0, 0);

    DWORD bytes_read;
    BOOL result = ReadFile(op->handle, buf, nbytes, &bytes_read, &op->overlapped);

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    m_context.submit_async(std::move(op));
}

template <>
void Leverager<Context>::async_write(int fd, const void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                                     [[maybe_unused]] std::uint8_t iflags) {
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->buffer = const_cast<void *>(buf);
    op->buffer_size = nbytes;
    op->offset = offset;
    op->op_type = static_cast<int>(op_type::write);
    op->handle = m_context.fd_to_handle(fd);

    op->overlapped.Offset = static_cast<DWORD>(offset);
    op->overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

    CreateIoCompletionPort(op->handle, m_context.get_iocp_handle(), 0, 0);

    DWORD bytes_written;
    BOOL result = WriteFile(op->handle, buf, nbytes, &bytes_written, &op->overlapped);

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    m_context.submit_async(std::move(op));
}

template <>
void Leverager<Context>::async_readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                     completion_callback cb, std::uint8_t iflags) {
    if (nr_vecs == 0) {
        cb(0);
        return;
    }

    async_read(fd, iovecs[0].iov_base, static_cast<unsigned>(iovecs[0].iov_len), offset, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                      completion_callback cb, std::uint8_t iflags) {
    if (nr_vecs == 0) {
        cb(0);
        return;
    }

    async_write(fd, iovecs[0].iov_base, static_cast<unsigned>(iovecs[0].iov_len), offset, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_readv2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                      [[maybe_unused]] int flags, completion_callback cb, std::uint8_t iflags) {
    async_readv(fd, iovecs, nr_vecs, offset, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_writev2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                       [[maybe_unused]] int flags, completion_callback cb, std::uint8_t iflags) {
    async_writev(fd, iovecs, nr_vecs, offset, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_read_fixed(int fd, void *buf, unsigned nbytes, off_t offset,
                                          [[maybe_unused]] int buf_index, completion_callback cb, std::uint8_t iflags) {
    async_read(fd, buf, nbytes, offset, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_write_fixed(int fd, const void *buf, unsigned nbytes, off_t offset,
                                           [[maybe_unused]] int buf_index, completion_callback cb,
                                           std::uint8_t iflags) {
    async_write(fd, buf, nbytes, offset, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_fsync(int fd, [[maybe_unused]] unsigned fsync_flags, completion_callback cb,
                                     [[maybe_unused]] std::uint8_t iflags) {
    HANDLE h = m_context.fd_to_handle(fd);
    BOOL result = FlushFileBuffers(h);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_sync_file_range(int fd, [[maybe_unused]] off64_t offset, [[maybe_unused]] off64_t nbytes,
                                               [[maybe_unused]] unsigned sync_range_flags, completion_callback cb,
                                               std::uint8_t iflags) {
    async_fsync(fd, 0, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_recv(int sockfd, void *buf, unsigned nbytes, std::uint32_t flags, completion_callback cb,
                                    [[maybe_unused]] std::uint8_t iflags) {
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->buffer = buf;
    op->buffer_size = nbytes;
    op->op_type = static_cast<int>(op_type::recv);
    op->socket = static_cast<SOCKET>(sockfd);

    op->wsabuf.buf = static_cast<CHAR *>(buf);
    op->wsabuf.len = nbytes;

    DWORD bytes_received;
    DWORD wsa_flags = flags;

    int result = WSARecv(op->socket, &op->wsabuf, 1, &bytes_received, &wsa_flags, &op->overlapped, nullptr);

    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        cb(-WSAGetLastError());
        return;
    }

    m_context.submit_async(std::move(op));
}

template <>
void Leverager<Context>::async_send(int sockfd, const void *buf, unsigned nbytes, std::uint32_t flags,
                                    completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->buffer = const_cast<void *>(buf);
    op->buffer_size = nbytes;
    op->op_type = static_cast<int>(op_type::send);
    op->socket = static_cast<SOCKET>(sockfd);

    op->wsabuf.buf = const_cast<CHAR *>(static_cast<const CHAR *>(buf));
    op->wsabuf.len = nbytes;

    DWORD bytes_sent;

    int result = WSASend(op->socket, &op->wsabuf, 1, &bytes_sent, flags, &op->overlapped, nullptr);

    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        cb(-WSAGetLastError());
        return;
    }

    m_context.submit_async(std::move(op));
}

template <>
void Leverager<Context>::async_recvmsg(int sockfd, msghdr *msg, std::uint32_t flags, completion_callback cb,
                                       std::uint8_t iflags) {
    if (msg->msg_iovlen > 0) {
        async_recv(sockfd, msg->msg_iov[0].iov_base, static_cast<unsigned>(msg->msg_iov[0].iov_len), flags,
                   std::move(cb), iflags);
    } else {
        cb(0);
    }
}

template <>
void Leverager<Context>::async_sendmsg(int sockfd, const msghdr *msg, std::uint32_t flags, completion_callback cb,
                                       std::uint8_t iflags) {
    if (msg->msg_iovlen > 0) {
        async_send(sockfd, msg->msg_iov[0].iov_base, static_cast<unsigned>(msg->msg_iov[0].iov_len), flags,
                   std::move(cb), iflags);
    } else {
        cb(0);
    }
}

template <>
void Leverager<Context>::async_poll([[maybe_unused]] int fd, [[maybe_unused]] short poll_mask, completion_callback cb,
                                    [[maybe_unused]] std::uint8_t iflags) {
    cb(0);
}

template <>
void Leverager<Context>::async_yield(completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);

    BOOL result = PostQueuedCompletionStatus(m_context.get_iocp_handle(), 0, 0, &op->overlapped);
    if (!result) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    // NOLINTNEXTLINE(bugprone-unused-return-value)
    op.release();
}

template <>
void Leverager<Context>::async_accept(int fd, [[maybe_unused]] sockaddr *addr, [[maybe_unused]] socklen_t *addrlen,
                                      [[maybe_unused]] int flags, completion_callback cb,
                                      [[maybe_unused]] std::uint8_t iflags) {
    SOCKET listen_socket = static_cast<SOCKET>(fd);

    SOCKET accept_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (accept_socket == INVALID_SOCKET) {
        cb(-WSAGetLastError());
        return;
    }

    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->op_type = static_cast<int>(op_type::accept);
    op->socket = accept_socket;

    static char accept_buffer[2 * (sizeof(sockaddr_in) + 16)];
    DWORD bytes_received;

    BOOL result = m_context.get_accept_ex()(listen_socket, accept_socket, accept_buffer, 0, sizeof(sockaddr_in) + 16,
                                            sizeof(sockaddr_in) + 16, &bytes_received, &op->overlapped);

    if (!result && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(accept_socket);
        cb(-WSAGetLastError());
        return;
    }

    m_context.submit_async(std::move(op));
}

template <>
void Leverager<Context>::async_connect(int fd, sockaddr *addr, socklen_t addrlen, completion_callback cb,
                                       [[maybe_unused]] std::uint8_t iflags) {
    SOCKET sock = static_cast<SOCKET>(fd);

    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->op_type = static_cast<int>(op_type::connect);
    op->socket = sock;

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = 0;
    if (::bind(sock, reinterpret_cast<sockaddr *>(&local_addr), sizeof(local_addr)) == SOCKET_ERROR) {
        cb(-::WSAGetLastError());
        return;
    }

    DWORD bytes_sent;
    BOOL result = m_context.get_connect_ex()(sock, addr, addrlen, nullptr, 0, &bytes_sent, &op->overlapped);

    if (!result && WSAGetLastError() != WSA_IO_PENDING) {
        cb(-WSAGetLastError());
        return;
    }

    m_context.submit_async(std::move(op));
}

template <>
void Leverager<Context>::async_timeout(__kernel_timespec *ts, completion_callback cb,
                                       [[maybe_unused]] std::uint8_t iflags) {
    HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (!timer) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    LARGE_INTEGER due_time;
    due_time.QuadPart = -(ts->tv_sec * 10000000LL + ts->tv_nsec / 100);

    SetWaitableTimer(timer, &due_time, 0, nullptr, nullptr, FALSE);
    CloseHandle(timer);
    cb(0);
}

template <>
void Leverager<Context>::async_openat([[maybe_unused]] int dfd, const char *path, int flags,
                                      [[maybe_unused]] mode_t mode, completion_callback cb,
                                      [[maybe_unused]] std::uint8_t iflags) {
    DWORD access = 0;
    DWORD disposition = 0;

    if (flags & O_RDWR) {
        access = GENERIC_READ | GENERIC_WRITE;
    } else if (flags & O_WRONLY) {
        access = GENERIC_WRITE;
    } else {
        access = GENERIC_READ;
    }

    if (flags & O_CREAT) {
        if (flags & O_TRUNC) {
            disposition = CREATE_ALWAYS;
        } else {
            disposition = OPEN_ALWAYS;
        }
    } else {
        if (flags & O_TRUNC) {
            disposition = TRUNCATE_EXISTING;
        } else {
            disposition = OPEN_EXISTING;
        }
    }

    HANDLE h = CreateFileA(path, access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, disposition, FILE_FLAG_OVERLAPPED,
                           nullptr);

    if (h == INVALID_HANDLE_VALUE) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    CreateIoCompletionPort(h, m_context.get_iocp_handle(), 0, 0);

    cb(static_cast<int>(reinterpret_cast<std::uintptr_t>(h)));
}

template <>
void Leverager<Context>::async_close(int fd, completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    HANDLE h = m_context.fd_to_handle(fd);
    BOOL result = CloseHandle(h);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_statx([[maybe_unused]] int dfd, const char *path, [[maybe_unused]] int flags,
                                     unsigned mask, struct statx *statxbuf, completion_callback cb,
                                     [[maybe_unused]] std::uint8_t iflags) {
    WIN32_FILE_ATTRIBUTE_DATA attr_data;
    BOOL result = GetFileAttributesExA(path, GetFileExInfoStandard, &attr_data);

    if (!result) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    if (statxbuf) {
        std::memset(statxbuf, 0, sizeof(*statxbuf));
        statxbuf->stx_mask = mask;
        statxbuf->stx_size = (static_cast<std::uint64_t>(attr_data.nFileSizeHigh) << 32) | attr_data.nFileSizeLow;
        statxbuf->stx_mode = (attr_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 040755 : 0100644;
    }

    cb(0);
}

template <>
void Leverager<Context>::async_splice([[maybe_unused]] int fd_in, [[maybe_unused]] loff_t off_in,
                                      [[maybe_unused]] int fd_out, [[maybe_unused]] loff_t off_out,
                                      [[maybe_unused]] size_t nbytes, [[maybe_unused]] unsigned flags,
                                      completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    cb(-ERROR_NOT_SUPPORTED);
}

template <>
void Leverager<Context>::async_tee([[maybe_unused]] int fd_in, [[maybe_unused]] int fd_out,
                                   [[maybe_unused]] size_t nbytes, [[maybe_unused]] unsigned flags,
                                   completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    cb(-ERROR_NOT_SUPPORTED);
}

template <>
void Leverager<Context>::async_shutdown(int fd, int how, completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    SOCKET sock = static_cast<SOCKET>(fd);
    int wsa_how;
    switch (how) {
    case SHUT_RD:
        wsa_how = SD_RECEIVE;
        break;
    case SHUT_WR:
        wsa_how = SD_SEND;
        break;
    default:
        wsa_how = SD_BOTH;
        break;
    }
    int result = shutdown(sock, wsa_how);
    cb(result == 0 ? 0 : -WSAGetLastError());
}

template <>
void Leverager<Context>::async_renameat([[maybe_unused]] int olddfd, const char *oldpath, [[maybe_unused]] int newdfd,
                                        const char *newpath, unsigned flags, completion_callback cb,
                                        [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = MoveFileExA(oldpath, newpath, (flags & RENAME_NOREPLACE) ? MOVEFILE_REPLACE_EXISTING : 0);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_mkdirat([[maybe_unused]] int dirfd, const char *pathname, [[maybe_unused]] mode_t mode,
                                       completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = CreateDirectoryA(pathname, nullptr);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_symlinkat(const char *target, [[maybe_unused]] int newdirfd, const char *linkpath,
                                         completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = CreateSymbolicLinkA(linkpath, target, 0);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_linkat([[maybe_unused]] int olddirfd, const char *oldpath, [[maybe_unused]] int newdirfd,
                                      const char *newpath, [[maybe_unused]] int flags, completion_callback cb,
                                      [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = CreateHardLinkA(newpath, oldpath, nullptr);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_unlinkat([[maybe_unused]] int dfd, const char *path, unsigned flags,
                                        completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    BOOL result;
    if (flags & AT_REMOVEDIR) {
        result = RemoveDirectoryA(path);
    } else {
        result = DeleteFileA(path);
    }
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

template <>
void Leverager<Context>::async_msg_ring([[maybe_unused]] int fd, [[maybe_unused]] unsigned len,
                                        [[maybe_unused]] std::uint64_t data, [[maybe_unused]] unsigned flags,
                                        completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    cb(-ERROR_NOT_SUPPORTED);
}

template <>
int Leverager<Context>::readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset) {
    int result = -1;
    async_readv(fd, iovecs, nr_vecs, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset) {
    int result = -1;
    async_writev(fd, iovecs, nr_vecs, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::read(int fd, void *buf, unsigned nbytes, off_t offset) {
    int result = -1;
    async_read(fd, buf, nbytes, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::write(int fd, const void *buf, unsigned nbytes, off_t offset) {
    int result = -1;
    async_write(fd, buf, nbytes, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::fsync(int fd, unsigned fsync_flags) {
    int result = -1;
    async_fsync(fd, fsync_flags, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::close(int fd) {
    int result = -1;
    async_close(fd, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::openat(int dfd, const char *path, int flags, mode_t mode) {
    int result = -1;
    async_openat(dfd, path, flags, mode, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::accept(int fd, sockaddr *addr, socklen_t *addrlen, int flags) {
    int result = -1;
    async_accept(fd, addr, addrlen, flags, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

template <>
int Leverager<Context>::connect(int fd, sockaddr *addr, socklen_t addrlen) {
    int result = -1;
    async_connect(fd, addr, addrlen, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
void Leverager<Context>::stop() {
    m_running = false;
    PostQueuedCompletionStatus(m_context.get_iocp_handle(), 0, 0, nullptr);
}

template <>
void Leverager<Context>::register_files([[maybe_unused]] std::span<const int> fds) {}

template <>
void Leverager<Context>::register_files_update([[maybe_unused]] unsigned off, [[maybe_unused]] std::span<int> files) {}

template <>
int Leverager<Context>::unregister_files() noexcept {
    return 0;
}

template <>
void Leverager<Context>::register_buffers([[maybe_unused]] std::span<const iovec> iovecs) {}

template <>
int Leverager<Context>::unregister_buffers() noexcept {
    return 0;
}

} // namespace transport::base::leverage
