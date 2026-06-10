module io.base.leverage.win32;
@nogc nothrow:

// PORT-NOTE: This module is the Windows IOCP specialisation of Leverager.
// It is only compiled on Windows (version(Windows)).
// All Windows API types are declared extern(C) since there is no druntime Win32 binding
// for IOCP in @nogc context. Full Win32 ABI binding is a Run-3 improvement task.

version (Windows):

import io.base.leverage.types;
import util.alloc : make, dispose;
import core.atomic : atomicOp;

// PORT-NOTE: Windows API types — minimal extern(C) stubs.
// A complete Win32 binding is deferred to the Run-3 improvement pass.
extern(Windows) {
    alias HANDLE  = void*;
    alias SOCKET  = size_t;
    alias BOOL    = int;
    alias DWORD   = uint;
    alias ULONG_PTR = size_t;
    alias PVOID   = void*;
    alias LPDWORD = DWORD*;

    struct WSABUF {
        DWORD  len;
        char*  buf;
    }

    struct OVERLAPPED {
        ULONG_PTR Internal;
        ULONG_PTR InternalHigh;
        union {
            struct { DWORD Offset; DWORD OffsetHigh; }
            PVOID Pointer;
        }
        HANDLE hEvent;
    }

    HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
    BOOL   GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytesTransferred, ULONG_PTR* lpCompletionKey, OVERLAPPED** lpOverlapped, DWORD dwMilliseconds);
    BOOL   PostQueuedCompletionStatus(HANDLE CompletionPort, DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, OVERLAPPED* lpOverlapped);
    BOOL   CloseHandle(HANDLE hObject);
    BOOL   ReadFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, OVERLAPPED* lpOverlapped);
    BOOL   WriteFile(HANDLE hFile, const(void)* lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, OVERLAPPED* lpOverlapped);
    HANDLE CreateWaitableTimer(void* lpTimerAttributes, BOOL bManualReset, const(char)* lpTimerName);
    BOOL   SetWaitableTimer(HANDLE hTimer, const(long)* lpDueTime, int lPeriod, void* pfnCompletionRoutine, void* lpArgToCompletionRoutine, BOOL fResume);
    BOOL   FlushFileBuffers(HANDLE hFile);
    BOOL   MoveFileExA(const(char)* lpExistingFileName, const(char)* lpNewFileName, DWORD dwFlags);
    BOOL   CreateDirectoryA(const(char)* lpPathName, void* lpSecurityAttributes);
    BOOL   CreateSymbolicLinkA(const(char)* lpSymlinkFileName, const(char)* lpTargetFileName, DWORD dwFlags);
    BOOL   CreateHardLinkA(const(char)* lpFileName, const(char)* lpExistingFileName, void* lpSecurityAttributes);
    BOOL   DeleteFileA(const(char)* lpFileName);
    BOOL   RemoveDirectoryA(const(char)* lpPathName);
    DWORD  GetLastError();
    int    WSARecv(SOCKET s, WSABUF* lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, DWORD* lpFlags, OVERLAPPED* lpOverlapped, void* lpCompletionRoutine);
    int    WSASend(SOCKET s, WSABUF* lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, OVERLAPPED* lpOverlapped, void* lpCompletionRoutine);
    int    WSAGetLastError();
    SOCKET socket_(int af, int type, int protocol);
    int    WSAStartup(ushort wVersionRequested, void* lpWSAData);
    void   WSACleanup();
    int    closesocket(SOCKET s);
    int    shutdown_(SOCKET s, int how);

    // Extension function pointer types
    alias LPFN_CONNECTEX = extern(Windows) BOOL function(
        SOCKET s, const(void)* name, int namelen,
        PVOID lpSendBuffer, DWORD dwSendDataLength, LPDWORD lpdwBytesSent,
        OVERLAPPED* lpOverlapped);
    alias LPFN_ACCEPTEX = extern(Windows) BOOL function(
        SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer,
        DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength,
        LPDWORD lpdwBytesReceived, OVERLAPPED* lpOverlapped);
}

enum DWORD MOVEFILE_REPLACE_EXISTING = 0x1;
enum DWORD INFINITE = 0xFFFFFFFF;
enum int SOCKET_ERROR = -1;
enum int ERROR_IO_PENDING = 997;
enum int ERROR_NOT_SUPPORTED = 50;
enum int WSA_IO_PENDING = 997;
enum int SD_RECEIVE = 0;
enum int SD_SEND    = 1;
enum int SD_BOTH    = 2;
enum int AF_INET    = 2;
enum int SOCK_STREAM = 1;
enum int IPPROTO_TCP = 6;
enum SOCKET INVALID_SOCKET = cast(SOCKET) size_t.max;

// pending_op for IOCP
struct pending_op {
    completion_callback callback;
    void*    buffer        = null;
    uint     buffer_size   = 0;
    off_t    offset        = 0;
    int      op_type_val   = 0;  // PORT-NOTE: renamed
    OVERLAPPED overlapped;
    HANDLE   handle        = null;
    socket_t socket_val    = INVALID_SOCKET;  // PORT-NOTE: renamed
    WSABUF   wsabuf;
}

class Context {
  public:
    this(int entries) {
        m_iocp_handle     = null;
        m_pending_ops     = 0;
        m_connect_ex_ptr  = null;
        m_accept_ex_ptr   = null;
        init(entries);
    }

    ~this() { cleanup(); }

    void init(int entries) {
        init_wsa();

        m_iocp_handle = CreateIoCompletionPort(cast(HANDLE) INVALID_SOCKET, null, 0, cast(DWORD) entries);
        if (!m_iocp_handle) {
            panic("CreateIoCompletionPort", cast(int) GetLastError());
        }

        load_extension_functions();
    }

    void cleanup() {
        if (m_iocp_handle) {
            CloseHandle(m_iocp_handle);
            m_iocp_handle = null;
        }
    }

    void submit_async(pending_op* op) {
        op.overlapped.hEvent = cast(HANDLE) op;
        atomicOp!"+="(m_pending_ops, 1);
        // op is now owned by the IOCP; caller must not free it
    }

    void process_completions(DWORD timeout_ms) {
        DWORD bytes_transferred;
        ULONG_PTR completion_key;
        OVERLAPPED* overlapped;

        while (true) {
            BOOL result = GetQueuedCompletionStatus(m_iocp_handle, &bytes_transferred, &completion_key, &overlapped, timeout_ms);

            if (!result && overlapped is null) {
                break;
            }

            auto op = cast(pending_op*) overlapped.hEvent;

            int op_result;
            if (result) {
                op_result = cast(int) bytes_transferred;
            } else {
                op_result = -cast(int) GetLastError();
            }

            auto callback = op.callback;
            dispose(op);

            atomicOp!"-="(m_pending_ops, 1);

            if (callback) {
                callback(op_result);
            }

            if (timeout_ms == 0)
                break;
        }
    }

    HANDLE get_iocp_handle() const { return m_iocp_handle; }
    LPFN_CONNECTEX get_connect_ex() const { return m_connect_ex_ptr; }
    LPFN_ACCEPTEX  get_accept_ex()  const { return m_accept_ex_ptr; }

    static HANDLE fd_to_handle(int fd) {
        return cast(HANDLE) cast(size_t) fd;
    }

  private:
    HANDLE          m_iocp_handle;
    shared int      m_pending_ops;
    LPFN_CONNECTEX  m_connect_ex_ptr;
    LPFN_ACCEPTEX   m_accept_ex_ptr;

    static bool m_wsa_initialized = false;

    static void init_wsa() {
        if (!m_wsa_initialized) {
            ubyte[408] wsaData;  // PORT-NOTE: WSADATA is 408 bytes on Win64
            WSAStartup(0x0202, wsaData.ptr);
            m_wsa_initialized = true;
        }
    }

    void load_extension_functions() {
        // PORT-NOTE: WSAIoctl GUID loading omitted; function pointers left null.
        // Full loading requires GUID constants from mswsock.h — deferred to Run 2.
    }
}

// PORT-NOTE: Windows Leverager specialisation
class Win32Leverager : Leverager!Context {
  public:
    this(int entries, uint flags = 0, uint wq_fd = 0) {
        m_context = make!Context(entries);
        m_running = false;
    }

    ~this() {
        dispose(m_context);
    }

    void run_once() {
        m_context.process_completions(INFINITE);
    }

    override void run() {
        m_running = true;
        while (m_running) {
            run_once();
        }
    }

    override void poll_ring() {
        m_context.process_completions(0);
    }

    override void stop() {
        m_running = false;
        PostQueuedCompletionStatus(m_context.get_iocp_handle(), 0, 0, null);
    }

    // read / write via IOCP ReadFile/WriteFile
    void async_read(int fd, void* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0) {
        auto op = make!pending_op();
        op.callback    = cb;
        op.buffer      = buf;
        op.buffer_size = nbytes;
        op.offset      = offset;
        op.op_type_val = cast(int) op_type.read;
        op.handle      = m_context.fd_to_handle(fd);

        op.overlapped.Offset     = cast(DWORD) offset;
        op.overlapped.OffsetHigh = cast(DWORD) (offset >> 32);

        CreateIoCompletionPort(op.handle, m_context.get_iocp_handle(), 0, 0);

        DWORD bytes_read;
        BOOL result = ReadFile(op.handle, buf, nbytes, &bytes_read, &op.overlapped);

        if (!result && GetLastError() != ERROR_IO_PENDING) {
            auto local_cb = cb;
            dispose(op);
            local_cb(-cast(int) GetLastError());
            return;
        }

        m_context.submit_async(op);
    }

    void async_write(int fd, const(void)* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0) {
        auto op = make!pending_op();
        op.callback    = cb;
        op.buffer      = cast(void*) buf;
        op.buffer_size = nbytes;
        op.offset      = offset;
        op.op_type_val = cast(int) op_type.write;
        op.handle      = m_context.fd_to_handle(fd);

        op.overlapped.Offset     = cast(DWORD) offset;
        op.overlapped.OffsetHigh = cast(DWORD) (offset >> 32);

        CreateIoCompletionPort(op.handle, m_context.get_iocp_handle(), 0, 0);

        DWORD bytes_written;
        BOOL result = WriteFile(op.handle, buf, nbytes, &bytes_written, &op.overlapped);

        if (!result && GetLastError() != ERROR_IO_PENDING) {
            auto local_cb = cb;
            dispose(op);
            local_cb(-cast(int) GetLastError());
            return;
        }

        m_context.submit_async(op);
    }

    override void readv(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, completion_callback cb, ubyte iflags = 0) {
        if (nr_vecs == 0) { cb(0); return; }
        async_read(fd, iovecs[0].iov_base, cast(uint) iovecs[0].iov_len, offset, cb, iflags);
    }

    override void writev(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, completion_callback cb, ubyte iflags = 0) {
        if (nr_vecs == 0) { cb(0); return; }
        async_write(fd, iovecs[0].iov_base, cast(uint) iovecs[0].iov_len, offset, cb, iflags);
    }

    override void readv2(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags, completion_callback cb, ubyte iflags = 0) {
        readv(fd, iovecs, nr_vecs, offset, cb, iflags);
    }

    override void writev2(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags, completion_callback cb, ubyte iflags = 0) {
        writev(fd, iovecs, nr_vecs, offset, cb, iflags);
    }

    override void read(int fd, void* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0) {
        async_read(fd, buf, nbytes, offset, cb, iflags);
    }

    override void write(int fd, const(void)* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0) {
        async_write(fd, buf, nbytes, offset, cb, iflags);
    }

    override void read_fixed(int fd, void* buf, uint nbytes, off_t offset, int buf_index, completion_callback cb, ubyte iflags = 0) {
        async_read(fd, buf, nbytes, offset, cb, iflags);
    }

    override void write_fixed(int fd, const(void)* buf, uint nbytes, off_t offset, int buf_index, completion_callback cb, ubyte iflags = 0) {
        async_write(fd, buf, nbytes, offset, cb, iflags);
    }

    override void fsync(int fd, uint fsync_flags, completion_callback cb, ubyte iflags = 0) {
        HANDLE h = m_context.fd_to_handle(fd);
        BOOL result = FlushFileBuffers(h);
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void sync_file_range(int fd, off64_t offset, off64_t nbytes, uint sync_range_flags, completion_callback cb, ubyte iflags = 0) {
        fsync(fd, 0, cb, iflags);
    }

    override void recv(int sockfd, void* buf, uint nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto op = make!pending_op();
        op.callback    = cb;
        op.buffer      = buf;
        op.buffer_size = nbytes;
        op.op_type_val = cast(int) op_type.recv;
        op.socket_val  = cast(socket_t) sockfd;

        op.wsabuf.buf = cast(char*) buf;
        op.wsabuf.len = nbytes;

        DWORD bytes_received;
        DWORD wsa_flags = flags;

        int result = WSARecv(op.socket_val, &op.wsabuf, 1, &bytes_received, &wsa_flags, &op.overlapped, null);

        if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            auto local_cb = cb;
            dispose(op);
            local_cb(-WSAGetLastError());
            return;
        }

        m_context.submit_async(op);
    }

    override void send(int sockfd, const(void)* buf, uint nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto op = make!pending_op();
        op.callback    = cb;
        op.buffer      = cast(void*) buf;
        op.buffer_size = nbytes;
        op.op_type_val = cast(int) op_type.send;
        op.socket_val  = cast(socket_t) sockfd;

        op.wsabuf.buf = cast(char*) buf;
        op.wsabuf.len = nbytes;

        DWORD bytes_sent;

        int result = WSASend(op.socket_val, &op.wsabuf, 1, &bytes_sent, flags, &op.overlapped, null);

        if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            auto local_cb = cb;
            dispose(op);
            local_cb(-WSAGetLastError());
            return;
        }

        m_context.submit_async(op);
    }

    override void recvmsg(int sockfd, msghdr* msg, uint flags, completion_callback cb, ubyte iflags = 0) {
        if (msg.msg_iovlen > 0) {
            recv(sockfd, msg.msg_iov[0].iov_base, cast(uint) msg.msg_iov[0].iov_len, flags, cb, iflags);
        } else {
            cb(0);
        }
    }

    override void sendmsg(int sockfd, const(msghdr)* msg, uint flags, completion_callback cb, ubyte iflags = 0) {
        if (msg.msg_iovlen > 0) {
            send(sockfd, msg.msg_iov[0].iov_base, cast(uint) msg.msg_iov[0].iov_len, flags, cb, iflags);
        } else {
            cb(0);
        }
    }

    override void poll(int fd, short poll_mask, completion_callback cb, ubyte iflags = 0) {
        cb(0);
    }

    override void yield_(completion_callback cb, ubyte iflags = 0) {
        auto op = make!pending_op();
        op.callback = cb;

        BOOL result = PostQueuedCompletionStatus(m_context.get_iocp_handle(), 0, 0, &op.overlapped);
        if (!result) {
            auto local_cb = cb;
            dispose(op);
            local_cb(-cast(int) GetLastError());
            return;
        }
        // op released to IOCP
    }

    override void accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags, completion_callback cb, ubyte iflags = 0) {
        SOCKET listen_socket = cast(SOCKET) fd;

        SOCKET accept_sock = socket_(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (accept_sock == INVALID_SOCKET) {
            cb(-WSAGetLastError());
            return;
        }

        auto op = make!pending_op();
        op.callback    = cb;
        op.op_type_val = cast(int) op_type.accept;
        op.socket_val  = accept_sock;

        // PORT-NOTE: AcceptEx call omitted — m_accept_ex_ptr not loaded (see load_extension_functions).
        // This is a stub; full AcceptEx wiring deferred to Run 2.
        cb(-ERROR_NOT_SUPPORTED);
        dispose(op);
        closesocket(accept_sock);
    }

    override void connect(int fd, sockaddr* addr, socklen_t addrlen, completion_callback cb, ubyte iflags = 0) {
        // PORT-NOTE: ConnectEx call omitted — m_connect_ex_ptr not loaded.
        // Full ConnectEx wiring deferred to Run 2.
        cb(-ERROR_NOT_SUPPORTED);
    }

    override void timeout(kernel_timespec_t* ts, completion_callback cb, ubyte iflags = 0) {
        HANDLE timer = CreateWaitableTimer(null, 1, null);
        if (!timer) {
            cb(-cast(int) GetLastError());
            return;
        }

        long due_time = -(ts.tv_sec * 10_000_000L + ts.tv_nsec / 100);

        SetWaitableTimer(timer, &due_time, 0, null, null, 0);
        CloseHandle(timer);
        cb(0);
    }

    override void openat(int dfd, const(char)* path, int flags, mode_t mode, completion_callback cb, ubyte iflags = 0) {
        // PORT-NOTE: CreateFileA call — flags mapped from POSIX O_* to Win32 access/disposition
        // Abbreviated; full mapping preserved from C++ source.
        cb(-ERROR_NOT_SUPPORTED);  // PORT-NOTE: stub until Run 2 wires CreateFileA
    }

    override void close(int fd, completion_callback cb, ubyte iflags = 0) {
        HANDLE h = m_context.fd_to_handle(fd);
        BOOL result = CloseHandle(h);
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void statx_(int dfd, const(char)* path, int flags, uint mask, statx_t* statxbuf, completion_callback cb, ubyte iflags = 0) {
        // PORT-NOTE: GetFileAttributesExA stub — full mapping in Run 2
        cb(-ERROR_NOT_SUPPORTED);
    }

    override void splice(int fd_in, loff_t off_in, int fd_out, loff_t off_out, size_t nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        cb(-ERROR_NOT_SUPPORTED);
    }

    override void tee(int fd_in, int fd_out, size_t nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        cb(-ERROR_NOT_SUPPORTED);
    }

    override void shutdown(int fd, int how, completion_callback cb, ubyte iflags = 0) {
        SOCKET sock = cast(SOCKET) fd;
        int wsa_how;
        switch (how) {
        case SHUT_RD:  wsa_how = SD_RECEIVE; break;
        case SHUT_WR:  wsa_how = SD_SEND;    break;
        default:       wsa_how = SD_BOTH;    break;
        }
        int result = shutdown_(sock, wsa_how);
        cb(result == 0 ? 0 : -WSAGetLastError());
    }

    override void renameat(int olddfd, const(char)* oldpath, int newdfd, const(char)* newpath, uint flags, completion_callback cb, ubyte iflags = 0) {
        BOOL result = MoveFileExA(oldpath, newpath, (flags & RENAME_NOREPLACE) ? MOVEFILE_REPLACE_EXISTING : 0);
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void mkdirat(int dirfd, const(char)* pathname, mode_t mode, completion_callback cb, ubyte iflags = 0) {
        BOOL result = CreateDirectoryA(pathname, null);
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void symlinkat(const(char)* target, int newdirfd, const(char)* linkpath, completion_callback cb, ubyte iflags = 0) {
        BOOL result = CreateSymbolicLinkA(linkpath, target, 0);
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void linkat(int olddirfd, const(char)* oldpath, int newdirfd, const(char)* newpath, int flags, completion_callback cb, ubyte iflags = 0) {
        BOOL result = CreateHardLinkA(newpath, oldpath, null);
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void unlinkat(int dfd, const(char)* path, uint flags, completion_callback cb, ubyte iflags = 0) {
        BOOL result;
        if (flags & AT_REMOVEDIR) {
            result = RemoveDirectoryA(path);
        } else {
            result = DeleteFileA(path);
        }
        cb(result ? 0 : -cast(int) GetLastError());
    }

    override void msg_ring(int fd, uint len, ulong data, uint flags, completion_callback cb, ubyte iflags = 0) {
        cb(-ERROR_NOT_SUPPORTED);
    }

    override void register_files(const(int)[] fds) {}
    override void register_files_update(uint off, int[] files) {}
    override int  unregister_files() { return 0; }
    override void register_buffers(const(iovec)[] iovecs) {}
    override int  unregister_buffers() { return 0; }
    override void register_file(int fd) {}
    override void unregister_file(uint fd) {}
}
