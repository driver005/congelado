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

export namespace io::base::leverage {

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
    /**
     * @brief Stands up the IOCP completion port and resolves the `ConnectEx`/`AcceptEx`
     * extension function pointers right away — by the time this ctor returns the context is
     * fully live.
     * @param entries concurrency hint forwarded to `CreateIoCompletionPort` as the number of
     * threads allowed to run concurrently (0 lets the OS pick based on CPU count).
     * @throws std::system_error via panic() if `CreateIoCompletionPort` fails.
     */
    explicit Context(int entries)
        : m_iocp_handle{nullptr}, m_pending_ops{0}, m_connect_ex_ptr{nullptr}, m_accept_ex_ptr{nullptr} {
        init(entries);
    };
    /** @brief Closes the IOCP handle via cleanup(). */
    ~Context() noexcept { cleanup(); };

    /** @brief Deleted — copying an owning IOCP handle would double-close it. */
    Context(const Context &) = delete;
    /** @brief Deleted — same reasoning as the copy ctor. */
    Context &operator=(const Context &) = delete;
    /** @brief Deleted — no move either, in-flight `OVERLAPPED` ops reference this instance. */
    Context(Context &&) = delete;
    /** @brief Deleted — mirrors the move ctor. */
    Context &operator=(Context &&) = delete;

    /**
     * @brief Creates the IOCP handle and loads the `ConnectEx`/`AcceptEx` function pointers.
     * Called from the ctor.
     * @param entries concurrency hint forwarded to `CreateIoCompletionPort`.
     * @throws std::system_error via panic() if `CreateIoCompletionPort` fails.
     */
    void init(int entries) {
        // WinSock has to be up before anything else here can touch sockets
        init_wsa();

        // the completion port itself — everything async gets associated with this handle
        m_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, static_cast<DWORD>(entries));
        if (!m_iocp_handle) {
            panic("CreateIoCompletionPort", static_cast<int>(GetLastError()));
        }

        // resolve ConnectEx/AcceptEx now so async_connect()/async_accept() have them ready
        load_extension_functions();
    };

    /** @brief Closes `m_iocp_handle` if it's live and nulls it out — safe to call more than once. */
    void cleanup() noexcept {
        // bet — this guard makes double-calling (dtor + manual cleanup) harmless
        if (m_iocp_handle) {
            CloseHandle(m_iocp_handle);
            m_iocp_handle = nullptr;
        }
    };

    /**
     * @brief Stashes `op`'s address in its own `OVERLAPPED.hEvent` (so the completion handler can
     * find its way back to the `pending_op`) and hands ownership off to the OS.
     * @warning `op.release()` deliberately leaks the smart pointer — the raw `pending_op *` only
     * gets reclaimed with `delete` inside process_completions() once the matching completion
     * packet shows up. If the op never completes, that allocation leaks. Same footgun shape as
     * the posix backend's `submit_async`.
     * @param op the pending operation to hand off — ownership transfers to the OS/completion
     * port for the lifetime of the in-flight I/O.
     */
    void submit_async(std::unique_ptr<pending_op> op) {
        // stash the op's own address in its OVERLAPPED so process_completions() can find its
        // way back from the completion packet alone
        op->overlapped.hEvent = reinterpret_cast<HANDLE>(op.get());
        m_pending_ops.fetch_add(1);

        // hand ownership off to the OS/completion port — reclaimed manually later
        // NOLINTNEXTLINE(bugprone-unused-return-value)
        op.release();
    };

    /**
     * @brief Pulls completion packets off the IOCP queue in a loop, reclaiming each `pending_op`
     * and firing its callback with the result.
     * @param timeout_ms how long `GetQueuedCompletionStatus` waits per iteration
     * (`INFINITE` to block forever). When 0, this drains exactly one iteration non-blocking and
     * returns — matches poll()'s "don't block" contract.
     */
    void process_completions(DWORD timeout_ms) {
        DWORD bytes_transferred;
        ULONG_PTR completion_key;
        LPOVERLAPPED overlapped;

        while (true) {
            BOOL result =
                GetQueuedCompletionStatus(m_iocp_handle, &bytes_transferred, &completion_key, &overlapped, timeout_ms);

            // no completion and no overlapped pointer means the wait itself timed out/failed
            // with nothing pending — nothing to reclaim, stop looping
            if (!result && !overlapped) {
                break;
            }

            // walk back from the OVERLAPPED to the pending_op that submit_async() stashed it in
            auto *op = reinterpret_cast<pending_op *>(overlapped->hEvent);

            // success carries the byte count; failure carries a negative GetLastError()
            int op_result;
            if (result) {
                op_result = static_cast<int>(bytes_transferred);
            } else {
                op_result = -static_cast<int>(GetLastError());
            }

            // reclaim ownership of the op and fire its callback with the result
            auto callback = std::move(op->callback);
            delete op;

            m_pending_ops.fetch_sub(1);

            if (callback) {
                callback(op_result);
            }

            // timeout_ms == 0 is the poll() contract — one non-blocking pass and done
            if (timeout_ms == 0)
                break;
        }
    };

    /**
     * @brief Grabs the raw IOCP handle for `CreateIoCompletionPort`/`PostQueuedCompletionStatus`
     * calls elsewhere.
     * @return the completion port handle.
     */
    [[nodiscard]] HANDLE get_iocp_handle() const noexcept { return m_iocp_handle; }
    /**
     * @brief Grabs the resolved `ConnectEx` extension function pointer.
     * @return the `ConnectEx` pointer, or null if load_extension_functions() never ran/failed.
     */
    [[nodiscard]] LPFN_CONNECTEX get_connect_ex() const noexcept { return m_connect_ex_ptr; }
    /**
     * @brief Grabs the resolved `AcceptEx` extension function pointer.
     * @return the `AcceptEx` pointer, or null if load_extension_functions() never ran/failed.
     */
    [[nodiscard]] LPFN_ACCEPTEX get_accept_ex() const noexcept { return m_accept_ex_ptr; }

    /**
     * @brief Reinterprets a posix-style int fd as a Windows `HANDLE` — pure bit-cast, no real
     * conversion happening (this codebase stores real `HANDLE`s in an `int`-sized slot upstream).
     * @param fd the descriptor to reinterpret.
     * @return `fd` reinterpreted as a `HANDLE`.
     */
    static HANDLE fd_to_handle(int fd) { return reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(fd)); };

  private:
    HANDLE m_iocp_handle;
    std::atomic<int> m_pending_ops;
    LPFN_CONNECTEX m_connect_ex_ptr;
    LPFN_ACCEPTEX m_accept_ex_ptr;

    static inline bool m_wsa_initialized = false;

    /**
     * @brief Lazily runs `WSAStartup` exactly once process-wide, guarded by `m_wsa_initialized`.
     * @warning Not thread-safe — the guard is a plain `bool` with no atomicity, so racing this
     * from two threads on first use is a real data race. Low blast radius since `WSAStartup`
     * itself is idempotent/refcounted by the OS, but it's still UB on the flag itself.
     */
    static void init_wsa() {
        // one-shot guard — WSAStartup only needs to run once per process
        if (!m_wsa_initialized) {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            m_wsa_initialized = true;
        }
    };

    /**
     * @brief Opens a throwaway TCP socket purely to query the `ConnectEx`/`AcceptEx` extension
     * function pointers via `WSAIoctl`, then closes it. Called from init().
     * @note Silently no-ops (returns early) if the dummy socket can't be created, and silently
     * leaves `m_connect_ex_ptr`/`m_accept_ex_ptr` null if either `WSAIoctl` call fails — no error
     * surfaced here, so a broken lookup only shows up later when async_connect()/async_accept()
     * dereference a null function pointer. Sus failure mode to debug blind.
     */
    void load_extension_functions() {
        // ConnectEx/AcceptEx aren't linkable directly — need a live socket handle just to ask
        // WSAIoctl for their addresses
        SOCKET dummy = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (dummy == INVALID_SOCKET)
            return;

        // resolve ConnectEx's pointer via its well-known GUID
        GUID guid_connect_ex = WSAID_CONNECTEX;
        DWORD bytes;
        WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connect_ex, sizeof(guid_connect_ex),
                 &m_connect_ex_ptr, sizeof(m_connect_ex_ptr), &bytes, nullptr, nullptr);

        // same motion for AcceptEx
        GUID guid_accept_ex = WSAID_ACCEPTEX;
        WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_accept_ex, sizeof(guid_accept_ex), &m_accept_ex_ptr,
                 sizeof(m_accept_ex_ptr), &bytes, nullptr, nullptr);

        // the dummy socket was only ever a lookup vehicle, done with it now
        closesocket(dummy);
    }
};

/**
 * @brief IOCP specialization ctor — `flags`/`wq_fd` are io_uring-only concepts, so they're
 * marked `[[maybe_unused]]` and dropped on the floor here; only `entries` (the IOCP concurrency
 * hint) actually gets threaded through to `Context`. Full param contract lives on the primary
 * declaration in types.cppm.
 */
template <>
Leverager<Context>::Leverager(int entries, [[maybe_unused]] std::uint32_t flags, [[maybe_unused]] std::uint32_t wq_fd)
    : m_context{Context{entries}} {};

/** @brief IOCP specialization dtor — manually invokes `m_context.~Context()`, which does the
 * real handle cleanup (unlike the posix `Context` dtor, this one isn't a no-op). */
template <>
Leverager<Context>::~Leverager() noexcept {
    m_context.~Context();
}

/**
 * @brief IOCP-only helper with no posix equivalent — blocks on a single
 * `process_completions(INFINITE)` pass. run() below is just this in a loop.
 */
template <>
void Leverager<Context>::run_once() {
    m_context.process_completions(INFINITE);
}

/**
 * @brief IOCP specialization of run() — flips `m_running` true and pumps run_once() in a loop
 * until stop() flips it back off.
 * @warning Unlike the posix specialization (one blocking submit-and-wait per call), this run()
 * itself never returns until stop() is called from another thread/callback — don't call this
 * expecting a single pump cycle, that's a behavioral mismatch with the io_uring backend worth
 * knowing about if you're writing backend-agnostic code.
 */
template <>
void Leverager<Context>::run() {
    m_running = true;
    while (m_running) {
        run_once();
    }
}

/**
 * @brief IOCP specialization of poll() — single non-blocking `process_completions(0)` pass.
 */
template <>
void Leverager<Context>::poll() {
    m_context.process_completions(0);
}

/**
 * @brief IOCP-only helper with no direct posix equivalent — issues a real overlapped `ReadFile`
 * and either completes inline (non-pending error) or hands the op off to submit_async() to ride
 * the completion port. The public readv()/read()/etc. surface all funnel through this and
 * async_write() underneath.
 * @param fd descriptor to read from (translated to a `HANDLE` via `Context::fd_to_handle`).
 * @param buf destination buffer — caller-owned till `cb` fires.
 * @param nbytes max bytes to read into `buf`.
 * @param offset file offset to read from.
 * @param cb completion callback, invoked with the syscall result (bytes read, or negative
 * `GetLastError()`).
 * @param iflags unused on this backend — `[[maybe_unused]]`.
 */
template <>
void Leverager<Context>::async_read(int fd, void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                                    [[maybe_unused]] std::uint8_t iflags) {
    // stash everything the completion handler will need to reconstruct this read later
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->buffer = buf;
    op->buffer_size = nbytes;
    op->offset = offset;
    op->op_type = static_cast<int>(op_type::read);
    op->handle = m_context.fd_to_handle(fd);

    // OVERLAPPED wants the 64-bit offset split into two DWORDs
    op->overlapped.Offset = static_cast<DWORD>(offset);
    op->overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

    // wire this handle into the completion port so its completions land on our IOCP
    CreateIoCompletionPort(op->handle, m_context.get_iocp_handle(), 0, 0);

    // fire the overlapped read — a non-pending failure completes inline right here
    DWORD bytes_read;
    BOOL result = ReadFile(op->handle, buf, nbytes, &bytes_read, &op->overlapped);

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    // pending — hand the op off to ride the completion port
    m_context.submit_async(std::move(op));
}

/**
 * @brief IOCP-only write counterpart to async_read() — overlapped `WriteFile`, inline-completes
 * on a non-pending error, otherwise rides the completion port.
 * @param fd descriptor to write to.
 * @param buf source buffer — caller-owned till `cb` fires.
 * @param nbytes bytes to write from `buf`.
 * @param offset file offset to write at.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend — `[[maybe_unused]]`.
 */
template <>
void Leverager<Context>::async_write(int fd, const void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                                     [[maybe_unused]] std::uint8_t iflags) {
    // same setup dance as async_read() but tagged as a write
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

    // fire the overlapped write — a non-pending failure completes inline right here
    DWORD bytes_written;
    BOOL result = WriteFile(op->handle, buf, nbytes, &bytes_written, &op->overlapped);

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    // pending — hand the op off to ride the completion port
    m_context.submit_async(std::move(op));
}

/**
 * @brief IOCP scatter-read shim — Windows overlapped I/O has no native vectored read here, so
 * this just forwards to async_read() using the *first* iovec only.
 * @warning Any iovec past index 0 is silently dropped — this is not a real `readv`. If a caller
 * passes multiple buffers expecting them all filled, only the first one gets touched. Straight
 * L if you're porting posix-vectored-I/O-reliant code to this backend without checking.
 * @param fd descriptor to read from.
 * @param iovecs scatter buffers — only `iovecs[0]` is actually used.
 * @param nr_vecs vector count; if 0, completes immediately with `cb(0)` and touches nothing.
 * @param offset file offset to read from.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                     completion_callback cb, std::uint8_t iflags) {
    // nothing to read into — complete immediately rather than touch iovecs[0]
    if (nr_vecs == 0) {
        cb(0);
        return;
    }

    // no real vectored read here, just forward the first buffer
    async_read(fd, iovecs[0].iov_base, static_cast<unsigned>(iovecs[0].iov_len), offset, std::move(cb), iflags);
}

/**
 * @brief Write counterpart to async_readv() — same single-iovec-only caveat applies.
 * @warning See async_readv()'s warning — only `iovecs[0]` gets written, the rest are dropped.
 * @param fd descriptor to write to.
 * @param iovecs scatter buffers — only `iovecs[0]` is actually used.
 * @param nr_vecs vector count; if 0, completes immediately with `cb(0)`.
 * @param offset file offset to write at.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                      completion_callback cb, std::uint8_t iflags) {
    // nothing to write from — complete immediately rather than touch iovecs[0]
    if (nr_vecs == 0) {
        cb(0);
        return;
    }

    // no real vectored write here, just forward the first buffer
    async_write(fd, iovecs[0].iov_base, static_cast<unsigned>(iovecs[0].iov_len), offset, std::move(cb), iflags);
}

/**
 * @brief `readv2`-style shim — the extra `flags` word (posix `RWF_*`) has no Windows equivalent
 * here, so it's dropped (`[[maybe_unused]]`) and this just forwards to async_readv().
 * @param fd descriptor to read from.
 * @param iovecs scatter buffers — only `iovecs[0]` is used, see async_readv().
 * @param nr_vecs vector count.
 * @param offset file offset to read from.
 * @param flags ignored on this backend.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_readv2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                      [[maybe_unused]] int flags, completion_callback cb, std::uint8_t iflags) {
    async_readv(fd, iovecs, nr_vecs, offset, std::move(cb), iflags);
}

/**
 * @brief `writev2`-style shim — mirrors async_readv2(), `flags` dropped, forwards to
 * async_writev().
 * @param fd descriptor to write to.
 * @param iovecs scatter buffers — only `iovecs[0]` is used, see async_writev().
 * @param nr_vecs vector count.
 * @param offset file offset to write at.
 * @param flags ignored on this backend.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_writev2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                       [[maybe_unused]] int flags, completion_callback cb, std::uint8_t iflags) {
    async_writev(fd, iovecs, nr_vecs, offset, std::move(cb), iflags);
}

/**
 * @brief Fixed-buffer read shim — Windows overlapped I/O has no registered-buffer concept, so
 * `buf_index` is dropped (`[[maybe_unused]]`) and this is just a plain async_read() underneath.
 * @param fd descriptor to read from.
 * @param buf destination buffer.
 * @param nbytes max bytes to read.
 * @param offset file offset to read from.
 * @param buf_index ignored on this backend.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_read_fixed(int fd, void *buf, unsigned nbytes, off_t offset,
                                          [[maybe_unused]] int buf_index, completion_callback cb, std::uint8_t iflags) {
    async_read(fd, buf, nbytes, offset, std::move(cb), iflags);
}

/**
 * @brief Fixed-buffer write shim — mirrors async_read_fixed(), forwards to async_write().
 * @param fd descriptor to write to.
 * @param buf source buffer.
 * @param nbytes bytes to write.
 * @param offset file offset to write at.
 * @param buf_index ignored on this backend.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_write_fixed(int fd, const void *buf, unsigned nbytes, off_t offset,
                                           [[maybe_unused]] int buf_index, completion_callback cb,
                                           std::uint8_t iflags) {
    async_write(fd, buf, nbytes, offset, std::move(cb), iflags);
}

/**
 * @brief Flush shim — calls `FlushFileBuffers` synchronously (there's no overlapped flush API)
 * and fires `cb` inline before returning; nothing actually goes through the completion port here.
 * @param fd descriptor to flush.
 * @param fsync_flags ignored on this backend — posix `sync_file_range`-style flags don't map.
 * @param cb completion callback, invoked synchronously before this call returns.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_fsync(int fd, [[maybe_unused]] unsigned fsync_flags, completion_callback cb,
                                     [[maybe_unused]] std::uint8_t iflags) {
    HANDLE h = m_context.fd_to_handle(fd);
    BOOL result = FlushFileBuffers(h);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Partial-range sync shim — Windows has no partial flush primitive, so this just forwards
 * to a full async_fsync(), dropping the range/flag params entirely.
 * @param fd descriptor to flush.
 * @param offset ignored on this backend.
 * @param nbytes ignored on this backend.
 * @param sync_range_flags ignored on this backend.
 * @param cb completion callback, invoked synchronously (see async_fsync()).
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_sync_file_range(int fd, [[maybe_unused]] off64_t offset, [[maybe_unused]] off64_t nbytes,
                                               [[maybe_unused]] unsigned sync_range_flags, completion_callback cb,
                                               std::uint8_t iflags) {
    async_fsync(fd, 0, std::move(cb), iflags);
}

/**
 * @brief Overlapped `WSARecv` shim — one-buffer `WSABUF`, inline-completes on a non-pending
 * error, otherwise rides the completion port.
 * @param sockfd socket descriptor to receive on.
 * @param buf destination buffer — caller-owned till `cb` fires.
 * @param nbytes max bytes to receive into `buf`.
 * @param flags `WSARecv` flags, threaded through as an in/out `DWORD`.
 * @param cb completion callback, invoked with the syscall result (or negative `WSAGetLastError`).
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_recv(int sockfd, void *buf, unsigned nbytes, std::uint32_t flags, completion_callback cb,
                                    [[maybe_unused]] std::uint8_t iflags) {
    // stash everything the completion handler needs, same shape as async_read()
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

    // fire the overlapped receive — a non-pending failure completes inline right here
    int result = WSARecv(op->socket, &op->wsabuf, 1, &bytes_received, &wsa_flags, &op->overlapped, nullptr);

    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        cb(-WSAGetLastError());
        return;
    }

    // pending — hand the op off to ride the completion port
    m_context.submit_async(std::move(op));
}

/**
 * @brief Overlapped `WSASend` shim — send counterpart to async_recv().
 * @param sockfd socket descriptor to send on.
 * @param buf source buffer — caller-owned till `cb` fires.
 * @param nbytes bytes to send from `buf`.
 * @param flags `WSASend` flags.
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_send(int sockfd, const void *buf, unsigned nbytes, std::uint32_t flags,
                                    completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    // same setup dance as async_recv() but tagged as a send
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->buffer = const_cast<void *>(buf);
    op->buffer_size = nbytes;
    op->op_type = static_cast<int>(op_type::send);
    op->socket = static_cast<SOCKET>(sockfd);

    op->wsabuf.buf = const_cast<CHAR *>(static_cast<const CHAR *>(buf));
    op->wsabuf.len = nbytes;

    DWORD bytes_sent;

    // fire the overlapped send — a non-pending failure completes inline right here
    int result = WSASend(op->socket, &op->wsabuf, 1, &bytes_sent, flags, &op->overlapped, nullptr);

    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        cb(-WSAGetLastError());
        return;
    }

    // pending — hand the op off to ride the completion port
    m_context.submit_async(std::move(op));
}

/**
 * @brief `recvmsg`-style shim — like async_readv(), only touches the first iovec in `msg`, no
 * real scatter or ancillary-data handling here.
 * @warning Anything beyond `msg->msg_iov[0]` is ignored — same single-buffer caveat as
 * async_readv(), plus ancillary/control data isn't handled at all on this backend.
 * @param sockfd socket descriptor to receive on.
 * @param msg message header — only `msg_iov[0]` is used; completes immediately with `cb(0)` if
 * `msg_iovlen` is 0.
 * @param flags forwarded to async_recv().
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_recvmsg(int sockfd, msghdr *msg, std::uint32_t flags, completion_callback cb,
                                       std::uint8_t iflags) {
    // only the first iovec is real here — forward to async_recv() if there's one, otherwise
    // there's nothing to receive into
    if (msg->msg_iovlen > 0) {
        async_recv(sockfd, msg->msg_iov[0].iov_base, static_cast<unsigned>(msg->msg_iov[0].iov_len), flags,
                   std::move(cb), iflags);
    } else {
        cb(0);
    }
}

/**
 * @brief `sendmsg`-style shim — send counterpart to async_recvmsg(), same first-iovec-only
 * limitation.
 * @param sockfd socket descriptor to send on.
 * @param msg message header — only `msg_iov[0]` is used; completes immediately with `cb(0)` if
 * `msg_iovlen` is 0.
 * @param flags forwarded to async_send().
 * @param cb completion callback, invoked with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_sendmsg(int sockfd, const msghdr *msg, std::uint32_t flags, completion_callback cb,
                                       std::uint8_t iflags) {
    // only the first iovec is real here — forward to async_send() if there's one, otherwise
    // there's nothing to send
    if (msg->msg_iovlen > 0) {
        async_send(sockfd, msg->msg_iov[0].iov_base, static_cast<unsigned>(msg->msg_iov[0].iov_len), flags,
                   std::move(cb), iflags);
    } else {
        cb(0);
    }
}

/**
 * @brief Stub — IOCP has no native readiness-poll primitive, so this just fires `cb(0)`
 * immediately without watching anything.
 * @warning This is a straight no-op. If calling code treats a poll() completion as "the fd is
 * actually ready now," it's cooked on this backend — the callback fires unconditionally and
 * instantly, not when `fd` becomes ready. Matches the note already on the primary declaration
 * in types.cppm.
 * @param fd ignored on this backend.
 * @param poll_mask ignored on this backend.
 * @param cb completion callback, invoked immediately with `0`.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_poll([[maybe_unused]] int fd, [[maybe_unused]] short poll_mask, completion_callback cb,
                                    [[maybe_unused]] std::uint8_t iflags) {
    cb(0);
}

/**
 * @brief IOCP specialization of yield() — posts a zero-byte completion packet via
 * `PostQueuedCompletionStatus` purely to get a round-trip through the completion port.
 * @warning Actual bug, not just a footgun: `op->callback = std::move(cb)` moves `cb` out on line
 * one, but the `PostQueuedCompletionStatus` failure branch then calls the now-moved-from `cb`
 * directly instead of `op->callback`. Invoking a moved-from `std::move_only_function` throws
 * `std::bad_function_call` instead of running the intended error callback — that's an L on the
 * failure path specifically, the success path (going through `op.release()` +
 * process_completions()) is fine.
 * @param cb completion callback — moved into the pending op; see the warning for the busted
 * failure-path reference to it.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_yield(completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    // no cap, this is just a round-trip: stash the callback, then post a zero-byte packet through IOCP
    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);

    BOOL result = PostQueuedCompletionStatus(m_context.get_iocp_handle(), 0, 0, &op->overlapped);
    if (!result) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    // posted successfully — process_completions() will reclaim and delete this op later
    // NOLINTNEXTLINE(bugprone-unused-return-value)
    op.release();
}

/**
 * @brief Overlapped `AcceptEx` shim — pre-creates the accept socket, kicks off `AcceptEx`, and
 * either completes inline on a non-pending failure or rides the completion port.
 * @warning Same use-after-move bug as async_yield(): once `op->callback = std::move(cb)` runs,
 * the later `AcceptEx`-failed branch still calls the original (now moved-from) `cb` instead of
 * `op->callback`. That path throws `std::bad_function_call` instead of delivering the error to
 * the caller. The early `accept_socket == INVALID_SOCKET` bail-out is fine since it returns
 * before `cb` ever gets moved.
 * @note `addr`/`addrlen` are accepted for interface parity with the posix signature but go
 * completely unused here — `AcceptEx` fills a fixed local `accept_buffer` instead, so the
 * accepted peer's address never makes it back out through these params. Mind that if calling
 * code expects the peer address populated the way the posix backend does.
 * @param fd listening socket descriptor.
 * @param addr unused on this backend — see the note.
 * @param addrlen unused on this backend.
 * @param flags unused on this backend.
 * @param cb completion callback — see the warning for the busted failure-path reference to it.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_accept(int fd, [[maybe_unused]] sockaddr *addr, [[maybe_unused]] socklen_t *addrlen,
                                      [[maybe_unused]] int flags, completion_callback cb,
                                      [[maybe_unused]] std::uint8_t iflags) {
    SOCKET listen_socket = static_cast<SOCKET>(fd);

    // lowkey annoying but AcceptEx needs a pre-created socket to accept into, unlike posix accept()
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

    // fire the overlapped accept — a non-pending failure completes inline right here
    BOOL result = m_context.get_accept_ex()(listen_socket, accept_socket, accept_buffer, 0, sizeof(sockaddr_in) + 16,
                                            sizeof(sockaddr_in) + 16, &bytes_received, &op->overlapped);

    if (!result && WSAGetLastError() != WSA_IO_PENDING) {
        // no cap, don't leak the pre-created accept socket on a failed kickoff
        closesocket(accept_socket);
        cb(-WSAGetLastError());
        return;
    }

    // pending — hand the op off to ride the completion port
    m_context.submit_async(std::move(op));
}

/**
 * @brief Overlapped `ConnectEx` shim — `ConnectEx` requires the socket be locally bound first, so
 * this does an explicit zero-port `bind()` before kicking off the connect.
 * @warning Same use-after-move bug pattern as async_accept()/async_yield(): after
 * `op->callback = std::move(cb)`, both the `bind()`-failure branch and the `ConnectEx`-failure
 * branch call the moved-from `cb` instead of `op->callback`, throwing `std::bad_function_call`
 * on either error path instead of delivering the error.
 * @param fd socket descriptor to connect (gets bound to `INADDR_ANY`/port 0 first).
 * @param addr peer address to connect to.
 * @param addrlen length of `addr`.
 * @param cb completion callback — see the warning for the busted failure-path references to it.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_connect(int fd, sockaddr *addr, socklen_t addrlen, completion_callback cb,
                                       [[maybe_unused]] std::uint8_t iflags) {
    SOCKET sock = static_cast<SOCKET>(fd);

    auto op = std::make_unique<pending_op>();
    op->callback = std::move(cb);
    op->op_type = static_cast<int>(op_type::connect);
    op->socket = sock;

    // ConnectEx requires the socket be locally bound first — bind to any address/port 0
    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = 0;
    if (::bind(sock, reinterpret_cast<sockaddr *>(&local_addr), sizeof(local_addr)) == SOCKET_ERROR) {
        cb(-::WSAGetLastError());
        return;
    }

    // fire the overlapped connect — a non-pending failure completes inline right here
    DWORD bytes_sent;
    BOOL result = m_context.get_connect_ex()(sock, addr, addrlen, nullptr, 0, &bytes_sent, &op->overlapped);

    if (!result && WSAGetLastError() != WSA_IO_PENDING) {
        cb(-WSAGetLastError());
        return;
    }

    // pending — hand the op off to ride the completion port
    m_context.submit_async(std::move(op));
}

/**
 * @brief Timer shim — arms a Windows waitable timer and closes it right away, without actually
 * waiting on it.
 * @warning This does not behave like the posix specialization. It calls `SetWaitableTimer` then
 * immediately `CloseHandle`s the timer and fires `cb(0)` unconditionally — the timer never gets
 * a chance to fire since closing its handle cancels it. Net effect: `cb` runs essentially
 * immediately regardless of `ts`, not after the requested delay. Don't rely on this for actual
 * timeout semantics on this backend.
 * @param ts requested delay — computed into `due_time` but effectively ignored given the
 * immediate-close behavior above.
 * @param cb completion callback, invoked immediately with `0`.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_timeout(__kernel_timespec *ts, completion_callback cb,
                                       [[maybe_unused]] std::uint8_t iflags) {
    HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (!timer) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    // convert seconds+nanoseconds into the 100ns units SetWaitableTimer wants, negated for a
    // relative (rather than absolute) due time
    LARGE_INTEGER due_time;
    due_time.QuadPart = -(ts->tv_sec * 10000000LL + ts->tv_nsec / 100);

    // arm it, then immediately tear it down — see the doxygen warning, this cancels the wait
    SetWaitableTimer(timer, &due_time, 0, nullptr, nullptr, FALSE);
    CloseHandle(timer);
    cb(0);
}

/**
 * @brief Open shim — `CreateFileA` is inherently synchronous for the open itself (only the
 * subsequent I/O is overlapped), so this maps posix `open`-style flags to Windows access/
 * disposition, opens with `FILE_FLAG_OVERLAPPED`, associates the handle with the completion
 * port, and fires `cb` inline with the new handle.
 * @note `dfd` (the `AT_FDCWD`-relative directory fd) is unused — `path` is always resolved as-is,
 * no `*at`-style relative-to-directory-handle behavior on this backend.
 * @param dfd unused on this backend.
 * @param path path to open.
 * @param flags posix `open`-style flags (e.g. `O_RDWR | O_CREAT | O_TRUNC`).
 * @param mode unused on this backend — Windows ACLs aren't touched here.
 * @param cb completion callback, invoked synchronously with the new handle (reinterpreted as an
 * int) or a negative `GetLastError()`.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_openat([[maybe_unused]] int dfd, const char *path, int flags,
                                      [[maybe_unused]] mode_t mode, completion_callback cb,
                                      [[maybe_unused]] std::uint8_t iflags) {
    DWORD access = 0;
    DWORD disposition = 0;

    // map posix access mode flags onto the Windows GENERIC_* access mask
    if (flags & O_RDWR) {
        access = GENERIC_READ | GENERIC_WRITE;
    } else if (flags & O_WRONLY) {
        access = GENERIC_WRITE;
    } else {
        access = GENERIC_READ;
    }

    // map posix create/truncate flags onto the Windows creation-disposition enum
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

    // the open itself is synchronous — FILE_FLAG_OVERLAPPED only affects subsequent I/O on
    // the handle, not this call
    HANDLE h = CreateFileA(path, access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, disposition, FILE_FLAG_OVERLAPPED,
                           nullptr);

    if (h == INVALID_HANDLE_VALUE) {
        cb(-static_cast<int>(GetLastError()));
        return;
    }

    // associate the new handle with our completion port so future async reads/writes on it work
    CreateIoCompletionPort(h, m_context.get_iocp_handle(), 0, 0);

    cb(static_cast<int>(reinterpret_cast<std::uintptr_t>(h)));
}

/**
 * @brief Close shim — `CloseHandle` is synchronous, so this fires `cb` inline with the result.
 * @param fd descriptor to close (translated via `Context::fd_to_handle`).
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_close(int fd, completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    HANDLE h = m_context.fd_to_handle(fd);
    BOOL result = CloseHandle(h);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Stat shim — synchronous `GetFileAttributesExA` mapped onto a subset of the posix
 * `statx` struct.
 * @warning Only fills `stx_mask`, `stx_size`, and `stx_mode` (with a hardcoded 0755-dir/0644-file
 * permission guess) — every other `statx` field (inode, timestamps, link count, etc.) stays
 * zeroed regardless of `mask`. Don't read anything but those three fields out of `statxbuf` on
 * this backend, the rest is silently unpopulated, not an error.
 * @param dfd unused on this backend — no `*at`-relative resolution.
 * @param path path to stat.
 * @param flags unused on this backend.
 * @param mask requested stat fields — only echoed back into `statxbuf->stx_mask`, doesn't
 * actually gate which fields get filled (see the warning).
 * @param statxbuf out-param the (partial) result gets written into, if non-null.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
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

    // zero first so every field this backend doesn't populate (inode, timestamps, etc.) reads
    // as 0 rather than garbage, then fill in the handful this backend actually supports
    if (statxbuf) {
        std::memset(statxbuf, 0, sizeof(*statxbuf));
        statxbuf->stx_mask = mask;
        statxbuf->stx_size = (static_cast<std::uint64_t>(attr_data.nFileSizeHigh) << 32) | attr_data.nFileSizeLow;
        statxbuf->stx_mode = (attr_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 040755 : 0100644;
    }

    cb(0);
}

/**
 * @brief Unsupported stub — Windows has no zero-copy pipe-splice primitive, so this immediately
 * fails every call with `cb(-ERROR_NOT_SUPPORTED)`. All params are dropped (`[[maybe_unused]]`).
 * @param fd_in unused, this backend doesn't implement splice.
 * @param off_in unused.
 * @param fd_out unused.
 * @param off_out unused.
 * @param nbytes unused.
 * @param flags unused.
 * @param cb completion callback, invoked immediately with `-ERROR_NOT_SUPPORTED`.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_splice([[maybe_unused]] int fd_in, [[maybe_unused]] loff_t off_in,
                                      [[maybe_unused]] int fd_out, [[maybe_unused]] loff_t off_out,
                                      [[maybe_unused]] size_t nbytes, [[maybe_unused]] unsigned flags,
                                      completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    cb(-ERROR_NOT_SUPPORTED);
}

/**
 * @brief Unsupported stub — mirrors async_splice(), no `tee` equivalent on this backend.
 * @param fd_in unused, this backend doesn't implement tee.
 * @param fd_out unused.
 * @param nbytes unused.
 * @param flags unused.
 * @param cb completion callback, invoked immediately with `-ERROR_NOT_SUPPORTED`.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_tee([[maybe_unused]] int fd_in, [[maybe_unused]] int fd_out,
                                   [[maybe_unused]] size_t nbytes, [[maybe_unused]] unsigned flags,
                                   completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    cb(-ERROR_NOT_SUPPORTED);
}

/**
 * @brief Socket shutdown shim — maps posix `SHUT_RD`/`SHUT_WR`/anything-else onto
 * `SD_RECEIVE`/`SD_SEND`/`SD_BOTH` and calls the synchronous winsock `shutdown`.
 * @param fd socket descriptor to shut down.
 * @param how which direction(s) — `SHUT_RD`/`SHUT_WR` map exactly, anything else (including
 * `SHUT_RDWR`) falls through to `SD_BOTH`.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_shutdown(int fd, int how, completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    SOCKET sock = static_cast<SOCKET>(fd);
    // map posix SHUT_RD/SHUT_WR/SHUT_RDWR onto the equivalent SD_* constant — anything not
    // explicitly RD or WR (including SHUT_RDWR) falls through to SD_BOTH
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

/**
 * @brief Rename shim — synchronous `MoveFileExA`.
 * @warning Odd flag mapping: this passes `MOVEFILE_REPLACE_EXISTING` when `RENAME_NOREPLACE` *is*
 * set — that reads backwards from what the flag name implies (posix `RENAME_NOREPLACE` means
 * "fail if the destination exists," which is the opposite of "replace existing"). Worth
 * double-checking against actual call sites before trusting rename semantics here.
 * @param olddfd unused on this backend — no `*at`-relative resolution.
 * @param oldpath current path.
 * @param newdfd unused on this backend.
 * @param newpath destination path.
 * @param flags `RENAME_NOREPLACE` — see the warning about the inverted mapping.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_renameat([[maybe_unused]] int olddfd, const char *oldpath, [[maybe_unused]] int newdfd,
                                        const char *newpath, unsigned flags, completion_callback cb,
                                        [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = MoveFileExA(oldpath, newpath, (flags & RENAME_NOREPLACE) ? MOVEFILE_REPLACE_EXISTING : 0);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Mkdir shim — synchronous `CreateDirectoryA`, `mode` is dropped since Windows
 * directories don't take posix permission bits at creation.
 * @param dirfd unused on this backend.
 * @param pathname directory path to create.
 * @param mode unused on this backend.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_mkdirat([[maybe_unused]] int dirfd, const char *pathname, [[maybe_unused]] mode_t mode,
                                       completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = CreateDirectoryA(pathname, nullptr);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Symlink shim — synchronous `CreateSymbolicLinkA` targeting a file (flag `0`, i.e. not a
 * directory symlink).
 * @warning Always creates a *file*-type symlink (flags `0`) — pointing this at a directory target
 * will make a broken/wrong-type link on Windows, which distinguishes file vs. directory symlinks
 * unlike posix. No detection/handling of that case here.
 * @param target text the symlink points at.
 * @param newdirfd unused on this backend.
 * @param linkpath where the new symlink gets created.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_symlinkat(const char *target, [[maybe_unused]] int newdirfd, const char *linkpath,
                                         completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = CreateSymbolicLinkA(linkpath, target, 0);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Hard link shim — synchronous `CreateHardLinkA`.
 * @param olddirfd unused on this backend.
 * @param oldpath existing path to link from.
 * @param newdirfd unused on this backend.
 * @param newpath new hard link path.
 * @param flags unused on this backend — `AT_SYMLINK_FOLLOW` etc. don't map.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_linkat([[maybe_unused]] int olddirfd, const char *oldpath, [[maybe_unused]] int newdirfd,
                                      const char *newpath, [[maybe_unused]] int flags, completion_callback cb,
                                      [[maybe_unused]] std::uint8_t iflags) {
    BOOL result = CreateHardLinkA(newpath, oldpath, nullptr);
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Unlink shim — picks `RemoveDirectoryA` vs `DeleteFileA` based on `AT_REMOVEDIR`.
 * @param dfd unused on this backend.
 * @param path path to remove.
 * @param flags `AT_REMOVEDIR` selects directory removal instead of file deletion.
 * @param cb completion callback, invoked synchronously with the syscall result.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_unlinkat([[maybe_unused]] int dfd, const char *path, unsigned flags,
                                        completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    // AT_REMOVEDIR picks directory removal, otherwise treat it as a plain file delete
    BOOL result;
    if (flags & AT_REMOVEDIR) {
        result = RemoveDirectoryA(path);
    } else {
        result = DeleteFileA(path);
    }
    cb(result ? 0 : -static_cast<int>(GetLastError()));
}

/**
 * @brief Unsupported stub — `msg_ring` is an io_uring-specific ring-to-ring messaging primitive
 * with no IOCP equivalent, so this immediately fails every call.
 * @param fd unused, this backend doesn't implement msg_ring.
 * @param len unused.
 * @param data unused.
 * @param flags unused.
 * @param cb completion callback, invoked immediately with `-ERROR_NOT_SUPPORTED`.
 * @param iflags unused on this backend.
 */
template <>
void Leverager<Context>::async_msg_ring([[maybe_unused]] int fd, [[maybe_unused]] unsigned len,
                                        [[maybe_unused]] std::uint64_t data, [[maybe_unused]] unsigned flags,
                                        completion_callback cb, [[maybe_unused]] std::uint8_t iflags) {
    cb(-ERROR_NOT_SUPPORTED);
}

/**
 * @brief Blocking convenience overload with no counterpart on the primary declaration in
 * types.cppm (that one only declares a void-returning, callback-taking readv()) — fires
 * async_readv() with a capturing lambda that stashes the result locally, then pumps exactly one
 * run_once() and returns whatever landed.
 * @warning Only correct if the op completes within that single run_once() pass. If it doesn't
 * (still pending after one IOCP wait), this returns the sentinel `-1` even though the real
 * result hasn't arrived yet — and the dangling stack-captured lambda is still registered with
 * the completion port, so a later completion will write through a reference to a destroyed
 * `result` local. That's a real use-after-return footgun baked into this whole family of
 * blocking wrappers.
 * @param fd descriptor to read from.
 * @param iovecs scatter buffers — see async_readv()'s single-iovec caveat.
 * @param nr_vecs vector count.
 * @param offset file offset to read from.
 * @return the syscall result if it landed within one run_once() pass, otherwise the `-1`
 * sentinel — see the warning.
 */
template <>
int Leverager<Context>::readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset) {
    // kick off the async op with a lambda that captures the result locally, pump exactly one
    // completion pass, then hand back whatever landed (see the doxygen warning re: timing)
    int result = -1;
    async_readv(fd, iovecs, nr_vecs, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking write counterpart to readv() — same one-pump-and-hope pattern and the same
 * dangling-capture risk described there.
 * @param fd descriptor to write to.
 * @param iovecs scatter buffers — see async_writev()'s single-iovec caveat.
 * @param nr_vecs vector count.
 * @param offset file offset to write at.
 * @return the syscall result if it landed within one run_once() pass, otherwise `-1` — see
 * readv()'s warning.
 */
template <>
int Leverager<Context>::writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset) {
    // same one-pump-and-hope pattern as readv()
    int result = -1;
    async_writev(fd, iovecs, nr_vecs, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking single-buffer read — same one-pump-and-hope pattern as readv(), see its
 * warning for the dangling-capture risk if the op doesn't finish in one pass.
 * @param fd descriptor to read from.
 * @param buf destination buffer.
 * @param nbytes max bytes to read.
 * @param offset file offset to read from.
 * @return the syscall result if it landed within one run_once() pass, otherwise `-1`.
 */
template <>
int Leverager<Context>::read(int fd, void *buf, unsigned nbytes, off_t offset) {
    // same one-pump-and-hope pattern as readv()
    int result = -1;
    async_read(fd, buf, nbytes, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking single-buffer write — write counterpart to read(), same caveats.
 * @param fd descriptor to write to.
 * @param buf source buffer.
 * @param nbytes bytes to write.
 * @param offset file offset to write at.
 * @return the syscall result if it landed within one run_once() pass, otherwise `-1`.
 */
template <>
int Leverager<Context>::write(int fd, const void *buf, unsigned nbytes, off_t offset) {
    // same one-pump-and-hope pattern as readv()
    int result = -1;
    async_write(fd, buf, nbytes, offset, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking flush — same one-pump-and-hope pattern; harmless in practice here since
 * async_fsync() always completes synchronously/inline anyway (no real overlapped flush op), so
 * the dangling-capture risk from readv()'s warning doesn't actually bite on this one.
 * @param fd descriptor to flush.
 * @param fsync_flags forwarded to async_fsync() (effectively ignored there).
 * @return the syscall result.
 */
template <>
int Leverager<Context>::fsync(int fd, unsigned fsync_flags) {
    // same pump-and-return shape as readv(), but harmless here since async_fsync() always
    // completes inline (no real overlapped flush op to dangle-capture against)
    int result = -1;
    async_fsync(fd, fsync_flags, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking close — same pattern as fsync(), and equally safe since async_close() always
 * completes inline.
 * @param fd descriptor to close.
 * @return the syscall result.
 */
template <>
int Leverager<Context>::close(int fd) {
    // same pattern as fsync(), equally safe since async_close() always completes inline
    int result = -1;
    async_close(fd, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking open — same pattern as close(), safe since async_openat() always completes
 * inline.
 * @param dfd unused, forwarded to async_openat().
 * @param path path to open.
 * @param flags posix `open`-style flags.
 * @param mode unused on this backend.
 * @return the new handle (reinterpreted as an int), or a negative error code.
 */
template <>
int Leverager<Context>::openat(int dfd, const char *path, int flags, mode_t mode) {
    // same pattern as close(), safe since async_openat() always completes inline
    int result = -1;
    async_openat(dfd, path, flags, mode, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking accept — this one *does* genuinely go through the completion port
 * (`AcceptEx` is a real overlapped op), so it inherits readv()'s dangling-capture risk if the
 * accept doesn't land within a single run_once() pass, plus async_accept()'s use-after-move `cb`
 * bug on its own failure paths.
 * @param fd listening socket descriptor.
 * @param addr unused — see async_accept()'s note, `AcceptEx` doesn't surface the peer address
 * through this param.
 * @param addrlen unused.
 * @param flags unused.
 * @return the accepted socket descriptor if it landed within one run_once() pass, otherwise the
 * `-1` sentinel — see the warnings on readv() and async_accept().
 */
template <>
int Leverager<Context>::accept(int fd, sockaddr *addr, socklen_t *addrlen, int flags) {
    // this one straight up goes through the completion port (AcceptEx is a real overlapped op),
    // so it inherits readv()'s dangling-capture risk if it doesn't land in one run_once() pass
    int result = -1;
    async_accept(fd, addr, addrlen, flags, [&](int res) { result = res; }, 0);
    run_once();
    return result;
}

/**
 * @brief Blocking connect — same real-overlapped-op caveats as accept(), plus it forwards
 * `iflags` as its default (unlike the others it doesn't even pass one through explicitly to
 * async_connect() beyond the default parameter).
 * @param fd socket descriptor to connect.
 * @param addr peer address to connect to.
 * @param addrlen length of `addr`.
 * @return the syscall result if it landed within one run_once() pass, otherwise `-1` — see the
 * warnings on readv() and async_connect().
 */
template <>
int Leverager<Context>::connect(int fd, sockaddr *addr, socklen_t addrlen) {
    // same real-overlapped-op caveats as accept()
    int result = -1;
    async_connect(fd, addr, addrlen, [&](int res) { result = res; });
    run_once();
    return result;
}

/**
 * @brief IOCP specialization of stop() — flips `m_running` false *and* posts a null completion
 * packet to wake a `run()` that's currently blocked inside `GetQueuedCompletionStatus`. Unlike
 * the posix specialization, this one has to actively wake the wait, since `m_running` alone
 * can't interrupt a blocked syscall.
 */
template <>
void Leverager<Context>::stop() {
    // W move — flip the flag first, then wake a possibly-blocked run() with a null completion
    // so it actually notices the flag changed
    m_running = false;
    PostQueuedCompletionStatus(m_context.get_iocp_handle(), 0, 0, nullptr);
}

/**
 * @brief IOCP specialization of register_files() — no-op. Fixed-file registration is an
 * io_uring-only optimization; there's nothing to register on this backend.
 * @param fds ignored on this backend.
 */
template <>
void Leverager<Context>::register_files([[maybe_unused]] std::span<const int> fds) {}

/**
 * @brief IOCP specialization of register_files_update() — no-op, mirrors register_files().
 * @param off ignored on this backend.
 * @param files ignored on this backend.
 */
template <>
void Leverager<Context>::register_files_update([[maybe_unused]] unsigned off, [[maybe_unused]] std::span<int> files) {}

/**
 * @brief IOCP specialization of unregister_files() — no-op.
 * @return always `0`.
 */
template <>
int Leverager<Context>::unregister_files() noexcept {
    return 0;
}

/**
 * @brief IOCP specialization of register_buffers() — no-op, mirrors register_files().
 * @param iovecs ignored on this backend.
 */
template <>
void Leverager<Context>::register_buffers([[maybe_unused]] std::span<const iovec> iovecs) {}

/**
 * @brief IOCP specialization of unregister_buffers() — no-op.
 * @return always `0`.
 */
template <>
int Leverager<Context>::unregister_buffers() noexcept {
    return 0;
}

} // namespace io::base::leverage
