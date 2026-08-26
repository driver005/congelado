module;

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <winsock2.h>
#    include <ws2tcpip.h>
// break
#    include <cerrno>
#    include <ctime>
#    include <mswsock.h>
#    include <windows.h>
#    pragma comment(lib, "ws2_32.lib")
#    pragma comment(lib, "mswsock.lib")

#else
#    include <cerrno>
#    include <linux/time_types.h>
#    include <sys/socket.h>
#    include <sys/stat.h>
#    include <sys/uio.h>
#    include <unistd.h>
#endif

export module io_base_leverage:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import shared;

export namespace io::base::leverage {

enum class op_type : std::uint8_t
{
    READ,
    WRITE,
    READV,
    WRITEV,
    FSYNC,
    CLOSE,
    OPENAT,
    ACCEPT,
    CONNECT,
    RECV,
    SEND,
    RECVMSG,
    SENDMSG,
    POLL,
    TIMEOUT,
    NOP,
    STATX,
    SPLICE,
    TEE,
    SHUTDOWN,
    RENAMEAT,
    MKDIRAT,
    SYMLINKAT,
    LINKAT,
    UNLINKAT,
    MSG_RING,
};

#ifdef _WIN32
using native_fd_t = HANDLE;
using socket_t = SOCKET;
constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
using native_fd_t = int;
using socket_t = int;
constexpr socket_t INVALID_SOCKET_VALUE = -1;
#endif

#ifdef _WIN32

struct iovec
{
    void* iov_base;
    size_t iov_len;
};

// statx compatibility
struct statx
{
    std::uint32_t stx_mask;
    std::uint32_t stx_blksize;
    std::uint64_t stx_attributes;
    std::uint32_t stx_nlink;
    std::uint32_t stx_uid;
    std::uint32_t stx_gid;
    std::uint16_t stx_mode;
    std::uint16_t __spare0[1];
    std::uint64_t stx_ino;
    std::uint64_t stx_size;
    std::uint64_t stx_blocks;
    std::uint64_t stx_attributes_mask;
    struct ::timespec stx_atime;
    struct ::timespec stx_btime;
    struct ::timespec stx_ctime;
    struct ::timespec stx_mtime;
    std::uint32_t stx_rdev_major;
    std::uint32_t stx_rdev_minor;
    std::uint32_t stx_dev_major;
    std::uint32_t stx_dev_minor;
    std::uint64_t __spare2[14];
};

// timespec for Windows
struct __kernel_timespec
{
    long long tv_sec;
    long long tv_nsec;
};

// sockaddr compatibility (already in winsock)
struct msghdr
{
    void* msg_name;           // Optional address
    unsigned int msg_namelen; // Size of address
    struct iovec* msg_iov;    // Scatter/gather array
    size_t msg_iovlen;        // # elements in msg_iov
    void* msg_control;        // Ancillary data
    size_t msg_controllen;    // Ancillary data buffer len
    int msg_flags;            // Flags on received message
};

using ::sockaddr;
using ::socklen_t;

// File flags
constexpr int AT_FDCWD = -100;
constexpr int O_RDONLY = 0x00'00;
constexpr int O_WRONLY = 0x00'01;
constexpr int O_RDWR = 0x00'02;
constexpr int O_CREAT = 0x01'00;
constexpr int O_TRUNC = 0x02'00;
constexpr int O_APPEND = 0x04'00;
constexpr int O_NONBLOCK = 0x10'00;
constexpr int O_CLOEXEC = 0x20'00;

// Mode flags
using mode_t = unsigned int;
constexpr mode_t S_IRUSR = 0400;
constexpr mode_t S_IWUSR = 0200;
constexpr mode_t S_IXUSR = 0100;
constexpr mode_t S_IRGRP = 0040;
constexpr mode_t S_IWGRP = 0020;
constexpr mode_t S_IXGRP = 0010;
constexpr mode_t S_IROTH = 0004;
constexpr mode_t S_IWOTH = 0002;
constexpr mode_t S_IXOTH = 0001;
constexpr mode_t S_IRWXU = 0700;
constexpr mode_t S_IRWXG = 0070;
constexpr mode_t S_IRWXO = 0007;
constexpr mode_t S_ISUID = 04000;
constexpr mode_t S_ISGID = 02000;
constexpr mode_t S_ISVTX = 01000;
constexpr mode_t ACCESSPERMS = 0777;

// Poll flags
// constexpr short POLLIN = 0x0001;
// constexpr short POLLOUT = 0x0004;
// constexpr short POLLERR = 0x0008;
// constexpr short POLLHUP = 0x0010;
// constexpr short POLLNVAL = 0x0020;

// Shutdown flags
constexpr int SHUT_RD = 0;
constexpr int SHUT_WR = 1;
constexpr int SHUT_RDWR = 2;

// Sync flags
constexpr unsigned SYNC_FILE_RANGE_WAIT_BEFORE = 1;
constexpr unsigned SYNC_FILE_RANGE_WRITE = 2;
constexpr unsigned SYNC_FILE_RANGE_WAIT_AFTER = 4;

// Statx flags
constexpr int AT_SYMLINK_NOFOLLOW = 0x1'00;
constexpr int AT_NO_AUTOMOUNT = 0x2'00;
constexpr int AT_EMPTY_PATH = 0x4'00;
constexpr unsigned STATX_TYPE = 0x00'01;
constexpr unsigned STATX_MODE = 0x00'02;
constexpr unsigned STATX_NLINK = 0x00'04;
constexpr unsigned STATX_UID = 0x00'08;
constexpr unsigned STATX_GID = 0x00'10;
constexpr unsigned STATX_ATIME = 0x00'20;
constexpr unsigned STATX_MTIME = 0x00'40;
constexpr unsigned STATX_CTIME = 0x00'80;
constexpr unsigned STATX_INO = 0x01'00;
constexpr unsigned STATX_SIZE = 0x02'00;
constexpr unsigned STATX_BLOCKS = 0x04'00;
constexpr unsigned STATX_BASIC_STATS = 0x07'FF;
constexpr unsigned STATX_BTIME = 0x08'00;
constexpr unsigned STATX_ALL = 0x0F'FF;

// Splice flags
constexpr unsigned SPLICE_F_MOVE = 1;
constexpr unsigned SPLICE_F_NONBLOCK = 2;
constexpr unsigned SPLICE_F_MORE = 4;
constexpr unsigned SPLICE_F_GIFT = 8;

// Rename flags
constexpr unsigned RENAME_NOREPLACE = 1;
constexpr unsigned RENAME_EXCHANGE = 2;
constexpr unsigned RENAME_WHITEOUT = 4;

// Unlink flags
constexpr unsigned AT_REMOVEDIR = 0x2'00;

// Link flags
constexpr int AT_SYMLINK_FOLLOW = 0x4'00;

// Socket flags
// constexpr int SOCK_STREAM = 1;
// constexpr int SOCK_DGRAM = 2;
// constexpr int SOCK_NONBLOCK = 0x800;
// constexpr int SOCK_CLOEXEC = 0x1000;
// constexpr int AF_UNSPEC = 0;
// constexpr int AF_UNIX = 1;
// constexpr int AF_INET = 2;
// constexpr int AF_INET6 = 23;
// constexpr int IPPROTO_TCP = 6;
// constexpr int IPPROTO_UDP = 17;

// // Send/Recv flags
// constexpr int MSG_OOB = 0x1;
// constexpr int MSG_PEEK = 0x2;
// constexpr int MSG_DONTROUTE = 0x4;
// constexpr int MSG_CTRUNC = 0x8;
// constexpr int MSG_TRUNC = 0x10;
// constexpr int MSG_DONTWAIT = 0x20;
// constexpr int MSG_EOR = 0x40;
// constexpr int MSG_WAITALL = 0x100;
// constexpr int MSG_NOSIGNAL = 0x4000;

// Fsync flags
constexpr unsigned FSYNC_DATASYNC = 1;

// Type aliases
using off_t = long long;
using off64_t = long long;
using loff_t = long long;
using ssize_t = long long;

// timespec
struct timespec
{
    long long tv_sec;
    long long tv_nsec;
};

#endif // _WIN32

using completion_callback = std::move_only_function<void(int)>;

consteval bool verbose_enabled()
{
    return false;
}

template<typename... Args>
constexpr void verbose_print(std::string_view fmt, Args&&... args)
{
    if constexpr (verbose_enabled()) {
        std::println(fmt, std::forward<Args>(args)...);
    }
}

// Error handling
[[noreturn]] inline void
panic(std::string_view msg, int err, std::source_location loc = std::source_location::current())
{
    throw std::system_error(
        err, std::system_category(), std::format("{} at {}:{}", msg, loc.file_name(), loc.line())
    );
}

inline void panic_on_err(std::string_view msg, int ret, bool ignore_eagain = false)
{
    // negative ret means a syscall failure — unless the caller opted to shrug off EAGAIN/
    // EWOULDBLOCK specifically (the "not ready yet, not a real error" case), panic
    if (ret < 0 && (!ignore_eagain ||
#ifdef _WIN32
                    ret != -WSAEWOULDBLOCK
#else
                    ret != -EAGAIN
#endif
                    ))
        {
        panic(msg, -ret);
    }
}

template<typename SharedContext>
class Leverager : public shared::HandlerBase
{
public:
    /**
     * @brief Spins up the backing async I/O context — io_uring on posix, IOCP on win32,
     * whichever `SharedContext` specialization gets linked in. The primary template has no
     * body; every real implementation lives in an explicit specialization over on `Context`
     * (`leverage/posix.cppm` or `leverage/win32.cppm`), so this signature is a contract, not
     * code.
     * @param entries submission queue depth to reserve (posix io_uring) — ignored on win32.
     * @param flags io_uring setup flags — ignored on win32.
     * @param wq_fd shared async worker-queue descriptor to attach to (io_uring
     * `IORING_SETUP_ATTACH_WQ`) — ignored on win32.
     */
    Leverager(int entries = 64, std::uint32_t flags = 0, std::uint32_t wq_fd = 0);

    /**
     * @brief Tears down `m_context` — actual ring/IOCP teardown is on the platform
     * specialization.
     */
    ~Leverager() noexcept override;

    /** @brief Deleted — copying an owning ring/IOCP handle would double-close it, straight
     * cooked. */
    Leverager(const Leverager&) = delete;
    /** @brief Deleted — same reasoning as the copy ctor, no aliasing the underlying handle. */
    Leverager& operator=(const Leverager&) = delete;
    /** @brief Deleted — no move either, downstream code holds refs/pointers into this instance.
     */
    Leverager(Leverager&&) = delete;
    /** @brief Deleted — mirrors the move ctor, this thing stays put once constructed. */
    Leverager& operator=(Leverager&&) = delete;

    // Async operation methods
    /**
     * @brief Queues a scatter-gather read (`preadv`-style) and fires `callback` once it lands.
     * @param descriptor file/socket descriptor to read from.
     * @param iovecs scatter buffers to fill — caller owns this memory till `callback` fires,
     * don't free it early or you're asking for a UAF.
     * @param nr_vecs number of entries in `iovecs`.
     * @param offset file offset to read from.
     * @param callback completion callback, invoked with the syscall result (bytes read, or
     * `-errno`).
     * @param iflags io_uring SQE flags (e.g. `IOSQE_IO_LINK`) — no-op on the win32 backend.
     */
    void readv(
        int descriptor,
        const iovec* iovecs,
        unsigned nr_vecs,
        off_t offset,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Same motion as readv() but with an extra `flags` word (`preadv2`-style, e.g.
     * `RWF_NOWAIT`).
     * @param descriptor file/socket descriptor to read from.
     * @param iovecs scatter buffers to fill — caller-owned till `callback` fires.
     * @param nr_vecs number of entries in `iovecs`.
     * @param offset file offset to read from.
     * @param flags `preadv2`-style read flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void readv2(
        int descriptor,
        const iovec* iovecs,
        unsigned nr_vecs,
        off_t offset,
        int flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a scatter-gather write (`pwritev`-style) and fires `callback` once it
     * lands.
     * @param descriptor file/socket descriptor to write to.
     * @param iovecs scatter buffers to write out — caller-owned till `callback` fires.
     * @param nr_vecs number of entries in `iovecs`.
     * @param offset file offset to write at.
     * @param callback completion callback, invoked with the syscall result (bytes written, or
     * `-errno`).
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void writev(
        int descriptor,
        const iovec* iovecs,
        unsigned nr_vecs,
        off_t offset,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Same motion as writev() but with an extra `flags` word (`pwritev2`-style).
     * @param descriptor file/socket descriptor to write to.
     * @param iovecs scatter buffers to write out — caller-owned till `callback` fires.
     * @param nr_vecs number of entries in `iovecs`.
     * @param offset file offset to write at.
     * @param flags `pwritev2`-style write flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void writev2(
        int descriptor,
        const iovec* iovecs,
        unsigned nr_vecs,
        off_t offset,
        int flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a single-buffer read and fires `callback` once it lands.
     * @param descriptor file/socket descriptor to read from.
     * @param buf destination buffer — caller-owned till `callback` fires.
     * @param nbytes max bytes to read into `buf`.
     * @param offset file offset to read from.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void read(
        int descriptor,
        void* buf,
        unsigned nbytes,
        off_t offset,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a single-buffer write and fires `callback` once it lands.
     * @param descriptor file/socket descriptor to write to.
     * @param buf source buffer — caller-owned till `callback` fires.
     * @param nbytes bytes to write from `buf`.
     * @param offset file offset to write at.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void write(
        int descriptor,
        const void* buf,
        unsigned nbytes,
        off_t offset,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a read against a pre-registered fixed buffer (skips the usual pin/unpin
     * per-op, cheaper on posix; on win32 this just falls back to a regular async read).
     * @param descriptor file/socket descriptor to read from.
     * @param buf destination — must fall inside a buffer registered via register_buffers() on
     * the posix backend, or the kernel will bounce the op.
     * @param nbytes max bytes to read into `buf`.
     * @param offset file offset to read from.
     * @param buf_index index of the registered buffer `buf` lives in.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void read_fixed(
        int descriptor,
        void* buf,
        unsigned nbytes,
        off_t offset,
        int buf_index,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Fixed-buffer counterpart to write() — see read_fixed() for the registration
     * caveat.
     * @param descriptor file/socket descriptor to write to.
     * @param buf source — must fall inside a buffer registered via register_buffers() on posix.
     * @param nbytes bytes to write from `buf`.
     * @param offset file offset to write at.
     * @param buf_index index of the registered buffer `buf` lives in.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void write_fixed(
        int descriptor,
        const void* buf,
        unsigned nbytes,
        off_t offset,
        int buf_index,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an `fsync`/`FlushFileBuffers`-style flush for `descriptor`.
     * @param descriptor file descriptor to sync.
     * @param fsync_flags posix fsync flags (e.g. `FSYNC_DATASYNC`) — ignored on win32, which
     * just calls `FlushFileBuffers`.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void fsync(
        int descriptor, unsigned fsync_flags, completion_callback callback, std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a `sync_file_range`-style partial flush. No cap, this one's posix-only in
     * spirit — the win32 backend just proxies it to a full fsync().
     * @param descriptor file descriptor to sync a range of.
     * @param offset byte offset the range starts at.
     * @param nbytes byte length of the range (0 means "to EOF").
     * @param sync_range_flags `sync_file_range`-style flags (e.g. `SYNC_FILE_RANGE_WRITE`).
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void sync_file_range(
        int descriptor,
        off64_t offset,
        off64_t nbytes,
        unsigned sync_range_flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a `recvmsg`-style receive for scatter buffers + ancillary data.
     * @param sockfd socket descriptor to receive on.
     * @param msg message header describing the destination iovecs/control buffer — caller-owned
     * till `callback` fires.
     * @param flags `recvmsg` flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void recvmsg(
        int sockfd,
        msghdr* msg,
        std::uint32_t flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a `sendmsg`-style send for scatter buffers + ancillary data.
     * @param sockfd socket descriptor to send on.
     * @param msg message header describing the source iovecs/control buffer — caller-owned till
     * `callback` fires.
     * @param flags `sendmsg` flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void sendmsg(
        int sockfd,
        const msghdr* msg,
        std::uint32_t flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a single-buffer `recv`.
     * @param sockfd socket descriptor to receive on.
     * @param buf destination buffer — caller-owned till `callback` fires.
     * @param nbytes max bytes to receive into `buf`.
     * @param flags `recv` flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void recv(
        int sockfd,
        void* buf,
        unsigned nbytes,
        std::uint32_t flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a single-buffer `send`.
     * @param sockfd socket descriptor to send on.
     * @param buf source buffer — caller-owned till `callback` fires.
     * @param nbytes bytes to send from `buf`.
     * @param flags `send` flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void send(
        int sockfd,
        const void* buf,
        unsigned nbytes,
        std::uint32_t flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a `poll`-style readiness watch on `descriptor` — fires `callback` once the
     * requested event mask is ready, no data movement here.
     * @warning The win32 IOCP backend has no real poll primitive, so this specialization is a
     * straight no-op stub that just calls `callback(0)` immediately — don't rely on it actually
     * watching anything there. That's an L waiting to happen if you assume parity across
     * backends.
     * @param descriptor file/socket descriptor to poll.
     * @param poll_mask event mask to wait for (e.g. `POLLIN`).
     * @param callback completion callback, invoked once the mask is satisfied (or immediately
     * on win32).
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void poll(
        int descriptor, short poll_mask, completion_callback callback, std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a no-op SQE purely to get a completion round-trip — bet, this is the
     * cheapest way to yield back into the ring/loop without doing real I/O.
     * @param callback completion callback, invoked once the nop lands.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void yield(completion_callback callback, std::uint8_t iflags = 0);

    /**
     * @brief Queues an async `accept` on a listening socket.
     * @param descriptor listening socket descriptor.
     * @param addr out-param for the accepted peer's address — caller-owned till `callback`
     * fires.
     * @param addrlen[in,out] in: size of `addr`'s storage; out: actual peer address length.
     * @param flags `accept4`-style flags (e.g. `SOCK_NONBLOCK`).
     * @param callback completion callback, invoked with the new socket descriptor (or
     * `-errno`).
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void accept(
        int descriptor,
        sockaddr* addr,
        socklen_t* addrlen,
        int flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `connect`.
     * @param descriptor socket descriptor to connect.
     * @param addr peer address to connect to — caller-owned till `callback` fires.
     * @param addrlen length of `addr`.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void connect(
        int descriptor,
        sockaddr* addr,
        socklen_t addrlen,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues a one-shot timer, `callback` fires once it expires.
     * @param timeout_spec absolute/relative timeout spec (backend-dependent) — caller-owned
     * till `callback` fires.
     * @param callback completion callback, invoked once the timer fires.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void timeout(
        __kernel_timespec* timeout_spec, completion_callback callback, std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `openat`.
     * @param dfd directory descriptor `path` is relative to (or `AT_FDCWD`).
     * @param path path to open — caller-owned till `callback` fires.
     * @param flags `open`-style flags (e.g. `O_RDWR | O_CREAT`).
     * @param mode file mode bits used when creating a new file.
     * @param callback completion callback, invoked with the new descriptor (or `-errno`).
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void openat(
        int dfd,
        const char* path,
        int flags,
        mode_t mode,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `close` for `descriptor`.
     * @param descriptor descriptor to close.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void close(int descriptor, completion_callback callback, std::uint8_t iflags = 0);

    /**
     * @brief Queues an async `statx`.
     * @param dfd directory descriptor `path` is relative to (or `AT_FDCWD`).
     * @param path path to stat — caller-owned till `callback` fires.
     * @param flags `statx`-style flags (e.g. `AT_SYMLINK_NOFOLLOW`).
     * @param mask which stat fields to actually fill in.
     * @param statxbuf out-param the result gets written into — caller-owned till `callback`
     * fires.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void statx(
        int dfd,
        const char* path,
        int flags,
        unsigned mask,
        struct statx* statxbuf,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `splice` between two fds (zero-copy pipe move).
     * @param fd_in source descriptor.
     * @param off_in source offset, or `-1` to use/advance the descriptor's current position.
     * @param fd_out destination descriptor.
     * @param off_out destination offset, or `-1` to use/advance the descriptor's current
     * position.
     * @param nbytes max bytes to move.
     * @param flags `splice`-style flags (e.g. `SPLICE_F_MOVE`).
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void splice(
        int fd_in,
        loff_t off_in,
        int fd_out,
        loff_t off_out,
        size_t nbytes,
        unsigned flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `tee` — like splice() but doesn't drain the source pipe.
     * @param fd_in source descriptor (must be a pipe).
     * @param fd_out destination descriptor (must be a pipe).
     * @param nbytes max bytes to duplicate.
     * @param flags `tee`-style flags.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void tee(
        int fd_in,
        int fd_out,
        size_t nbytes,
        unsigned flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async socket `shutdown`.
     * @param descriptor socket descriptor to shut down.
     * @param how which directions to shut down (`SHUT_RD`/`SHUT_WR`/`SHUT_RDWR`).
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void shutdown(int descriptor, int how, completion_callback callback, std::uint8_t iflags = 0);

    /**
     * @brief Queues an async `renameat2`.
     * @param olddfd directory descriptor `oldpath` is relative to.
     * @param oldpath current path — caller-owned till `callback` fires.
     * @param newdfd directory descriptor `newpath` is relative to.
     * @param newpath destination path — caller-owned till `callback` fires.
     * @param flags `renameat2`-style flags (e.g. `RENAME_NOREPLACE`).
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void renameat(
        int olddfd,
        const char* oldpath,
        int newdfd,
        const char* newpath,
        unsigned flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `mkdirat`.
     * @param dirfd directory descriptor `pathname` is relative to.
     * @param pathname directory path to create — caller-owned till `callback` fires.
     * @param mode permission bits for the new directory.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void mkdirat(
        int dirfd,
        const char* pathname,
        mode_t mode,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `symlinkat`.
     * @param target the text the symlink will point at — caller-owned till `callback` fires.
     * @param newdirfd directory descriptor `linkpath` is relative to.
     * @param linkpath where the new symlink gets created — caller-owned till `callback` fires.
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void symlinkat(
        const char* target,
        int newdirfd,
        const char* linkpath,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `linkat` (hard link).
     * @param olddirfd directory descriptor `oldpath` is relative to.
     * @param oldpath existing path to link from — caller-owned till `callback` fires.
     * @param newdirfd directory descriptor `newpath` is relative to.
     * @param newpath new hard link path — caller-owned till `callback` fires.
     * @param flags `linkat`-style flags (e.g. `AT_SYMLINK_FOLLOW`).
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void linkat(
        int olddirfd,
        const char* oldpath,
        int newdirfd,
        const char* newpath,
        int flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an async `unlinkat`.
     * @param dfd directory descriptor `path` is relative to.
     * @param path path to remove — caller-owned till `callback` fires.
     * @param flags `unlinkat`-style flags (e.g. `AT_REMOVEDIR` to rmdir instead of unlink).
     * @param callback completion callback, invoked with the syscall result.
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void unlinkat(
        int dfd,
        const char* path,
        unsigned flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Queues an io_uring `msg_ring` — pokes a message/descriptor across into another
     * ring's completion queue. Posix-only motion, the win32 backend just no-ops it out with an
     * error.
     * @param descriptor the target ring's descriptor to message.
     * @param len value threaded through as the target CQE's `res` field.
     * @param data value threaded through as the target CQE's `user_data`.
     * @param flags `msg_ring`-style flags.
     * @param callback completion callback, invoked with the syscall result (on the *sending*
     * ring).
     * @param iflags io_uring SQE flags — no-op on win32.
     */
    void msg_ring(
        int descriptor,
        unsigned len,
        std::uint64_t data,
        unsigned flags,
        completion_callback callback,
        std::uint8_t iflags = 0
    );

    /**
     * @brief Blocks submitting whatever's queued and waits for at least one completion, then
     * drains the completion queue. This is the core pump — call it in a loop to actually make
     * queued ops progress.
     */
    void run();
    /**
     * @brief Non-blocking completion drain — peeks the queue and processes whatever's already
     * landed without waiting. Overloaded against run(): this one never blocks.
     */
    void poll();
    /** @brief Flags the run loop to stop — posix backend just flips a bool; win32 also wakes
     * the IOCP with a null completion so a blocked run() actually notices. */
    void stop();

    /**
     * @brief Registers a single descriptor for fixed-file ops (cheaper repeated I/O, skips
     * descriptor lookup per op on posix). Thin wrapper around register_files() with a
     * one-element span.
     * @param descriptor descriptor to register.
     */
    void register_file(int descriptor);
    /**
     * @brief Unregisters a single fixed file by slot, replacing it with a sentinel. Thin
     * wrapper around register_files_update().
     * @param descriptor the *slot index*, not a raw descriptor — mind the naming, it's a
     * leftover from the single-descriptor convenience wrapper.
     */
    void unregister_file(unsigned int descriptor) noexcept;

    /**
     * @brief Registers a whole batch of fds for fixed-file I/O in one shot.
     * @param fds descriptors to register — caller-owned for the duration of the call, not
     * retained after it returns.
     * @throws std::system_error via panic_on_err() if the underlying registration syscall
     * fails.
     */
    void register_files(std::span<const int> fds);
    /**
     * @brief Swaps a subset of already-registered fixed files starting at `off`.
     * @param off slot offset to start updating at.
     * @param files replacement descriptors — caller-owned for the duration of the call.
     * @throws std::system_error via panic_on_err() if the underlying update syscall fails.
     */
    void register_files_update(unsigned off, std::span<int> files);
    /**
     * @brief Drops the entire fixed-file registration table.
     * @return the raw syscall result — 0 on success, negative errno on failure. Unlike its
     * register_* siblings this one doesn't panic, it just hands the code back, so check it W.
     */
    int unregister_files() noexcept;

    /**
     * @brief Registers fixed buffers for read_fixed()/write_fixed() to reference by index.
     * @param iovecs buffer descriptors to register — caller-owned for the duration of the call.
     * @throws std::system_error via panic_on_err() if the underlying registration syscall
     * fails.
     */
    void register_buffers(std::span<const iovec> iovecs);
    /**
     * @brief Drops the entire fixed-buffer registration table.
     * @return the raw syscall result — 0 on success, negative errno on failure.
     */
    int unregister_buffers() noexcept;

    /**
     * @brief Grabs the backing platform context (the io_uring ring or the IOCP wrapper).
     * @return a mutable reference to `m_context`.
     */
    [[nodiscard]] SharedContext& context() noexcept
    {
        return m_context;
    }

    /**
     * @brief Const overload of context() — same deal, read-only view.
     * @return a const reference to `m_context`.
     */
    [[nodiscard]] const SharedContext& context() const noexcept
    {
        return m_context;
    }

    /**
     * @brief HandlerBase override — this handler's registered name is always the literal string
     * `"Leverager"`.
     * @return `"Leverager"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "Leverager";
    }

    /**
     * @brief HandlerBase override — the work this handler does when scheduled is a single run()
     * pump followed by re-scheduling itself, so it keeps pumping completions forever once wired
     * into a controller.
     * @return the callable the controller invokes to do one pump-and-reschedule cycle.
     */
    shared::WorkerFunction on_execute() override
    {
        // pump one run() cycle, then re-queue this handler so it keeps getting scheduled
        // forever
        return [this]() {
            run();
            shared::this_handler::shedule();
        };
    }

    /**
     * @brief HandlerBase override — cleanup hook that calls stop() when this handler gets
     * released from its controller.
     * @return the release callback.
     */
    shared::ReleaseFunction on_released() noexcept override
    {
        return [this]() noexcept {
            stop();
        };
    }

private:
    SharedContext m_context;

    bool m_running{false};
};

} // namespace io::base::leverage

#ifdef CONGELADO_TEST
namespace io::base::leverage::tests {
using namespace boost::ut;

// Leverager<SharedContext> itself needs a live io_uring ring (or IOCP on win32) to do anything
// meaningful, so it's not unit-testable in isolation — skipped here. panic()/panic_on_err()/
// verbose_enabled() are pure logic with no syscalls involved, so those are covered for real.

suite<"leverage_panic"> panic_suite = [] {
    "panic_on_err does not throw on a non-negative result"_test = [] {
        expect(nothrow([] {
            panic_on_err("op", 0);
        }));
        expect(nothrow([] {
            panic_on_err("op", 42);
        }));
    };

    "panic_on_err throws std::system_error on a negative result"_test = [] {
        expect(throws<std::system_error>([] {
            panic_on_err("op", -1);
        }));
    };

    "panic_on_err swallows EAGAIN when ignore_eagain is set"_test = [] {
        expect(nothrow([] {
            panic_on_err("op", -EAGAIN, true);
        }));
    };

    "panic_on_err still throws EAGAIN when ignore_eagain is not set"_test = [] {
        expect(throws<std::system_error>([] {
            panic_on_err("op", -EAGAIN, false);
        }));
    };

    "panic() itself always throws std::system_error"_test = [] {
        expect(throws<std::system_error>([] {
            panic("boom", EINVAL);
        }));
    };
};

suite<"leverage_verbose"> verbose_suite = [] {
    "verbose_enabled is compiled off by default"_test = [] {
        expect(not verbose_enabled());
    };
};

} // namespace io::base::leverage::tests
#endif
