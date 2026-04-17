module;

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
// break
#include <cerrno>
#include <ctime>
#include <mswsock.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#else
#include <errno.h>
#include <linux/time_types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#endif

export module io_base_leverage:types;

import std;
import shared;

export namespace io::base::leverage {

enum class op_type : int {
    read,
    write,
    readv,
    writev,
    fsync,
    close,
    openat,
    accept,
    connect,
    recv,
    send,
    recvmsg,
    sendmsg,
    poll,
    timeout,
    nop,
    statx,
    splice,
    tee,
    shutdown,
    renameat,
    mkdirat,
    symlinkat,
    linkat,
    unlinkat,
    msg_ring,
};

#ifdef _WIN32
using native_fd_t = HANDLE;
using socket_t = SOCKET;
constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
using native_fd_t = int;
using socket_t = int;
constexpr socket_t invalid_socket = -1;
#endif

#ifdef _WIN32
struct iovec {
    void *iov_base;
    size_t iov_len;
};

// statx compatibility
struct statx {
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
struct __kernel_timespec {
    long long tv_sec;
    long long tv_nsec;
};

// sockaddr compatibility (already in winsock)
struct msghdr {
    void *msg_name;           // Optional address
    unsigned int msg_namelen; // Size of address
    struct iovec *msg_iov;    // Scatter/gather array
    size_t msg_iovlen;        // # elements in msg_iov
    void *msg_control;        // Ancillary data
    size_t msg_controllen;    // Ancillary data buffer len
    int msg_flags;            // Flags on received message
};

using ::sockaddr;
using ::socklen_t;

// File flags
constexpr int AT_FDCWD = -100;
constexpr int O_RDONLY = 0x0000;
constexpr int O_WRONLY = 0x0001;
constexpr int O_RDWR = 0x0002;
constexpr int O_CREAT = 0x0100;
constexpr int O_TRUNC = 0x0200;
constexpr int O_APPEND = 0x0400;
constexpr int O_NONBLOCK = 0x1000;
constexpr int O_CLOEXEC = 0x2000;

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
constexpr int AT_SYMLINK_NOFOLLOW = 0x100;
constexpr int AT_NO_AUTOMOUNT = 0x200;
constexpr int AT_EMPTY_PATH = 0x400;
constexpr unsigned STATX_TYPE = 0x0001;
constexpr unsigned STATX_MODE = 0x0002;
constexpr unsigned STATX_NLINK = 0x0004;
constexpr unsigned STATX_UID = 0x0008;
constexpr unsigned STATX_GID = 0x0010;
constexpr unsigned STATX_ATIME = 0x0020;
constexpr unsigned STATX_MTIME = 0x0040;
constexpr unsigned STATX_CTIME = 0x0080;
constexpr unsigned STATX_INO = 0x0100;
constexpr unsigned STATX_SIZE = 0x0200;
constexpr unsigned STATX_BLOCKS = 0x0400;
constexpr unsigned STATX_BASIC_STATS = 0x07ff;
constexpr unsigned STATX_BTIME = 0x0800;
constexpr unsigned STATX_ALL = 0x0fff;

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
constexpr unsigned AT_REMOVEDIR = 0x200;

// Link flags
constexpr int AT_SYMLINK_FOLLOW = 0x400;

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
struct timespec {
    long long tv_sec;
    long long tv_nsec;
};

#endif // _WIN32

using completion_callback = std::move_only_function<void(int)>;

consteval bool verbose_enabled() { return false; }

template <typename... Args>
constexpr void verbose_print(std::string_view fmt, Args &&...args) {
    if constexpr (verbose_enabled()) {
        std::println(fmt, std::forward<Args>(args)...);
    }
}

// Error handling
[[noreturn]] inline void panic(std::string_view msg, int err,
                               std::source_location loc = std::source_location::current()) {
    throw std::system_error(err, std::system_category(), std::format("{} at {}:{}", msg, loc.file_name(), loc.line()));
}

inline void panic_on_err(std::string_view msg, int ret, bool ignore_eagain = false) {
    if (ret < 0 && (!ignore_eagain ||
#ifdef _WIN32
                    ret != -WSAEWOULDBLOCK
#else
                    ret != -EAGAIN
#endif
                    )) {
        panic(msg, -ret);
    }
}

template <typename SharedContext>
class Leverager : public shared::HandlerBase {
  public:
    /** Init io_service
     * @param entries Maximum pending operations (Linux: io_uring queue size, Windows: IOCP concurrency hint)
     * @param flags Platform-specific flags (Linux: io_uring flags, Windows: ignored)
     * @param wq_fd Platform-specific (Linux: attach to existing wq, Windows: ignored)
     */
    Leverager(int entries = 64, std::uint32_t flags = 0, std::uint32_t wq_fd = 0);

    /** Destroy io_service */
    ~Leverager() noexcept;

    // Non-copyable, non-movable
    Leverager(const Leverager &) = delete;
    Leverager &operator=(const Leverager &) = delete;
    Leverager(Leverager &&) = delete;
    Leverager &operator=(Leverager &&) = delete;

    // Async operation methods
    void async_readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset, completion_callback cb,
                     std::uint8_t iflags = 0);

    void async_readv2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset, int flags, completion_callback cb,
                      std::uint8_t iflags = 0);

    void async_writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset, completion_callback cb,
                      std::uint8_t iflags = 0);

    void async_writev2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset, int flags, completion_callback cb,
                       std::uint8_t iflags = 0);

    void async_read(int fd, void *buf, unsigned nbytes, off_t offset, completion_callback cb, std::uint8_t iflags = 0);

    void async_write(int fd, const void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                     std::uint8_t iflags = 0);

    void async_read_fixed(int fd, void *buf, unsigned nbytes, off_t offset, int buf_index, completion_callback cb,
                          std::uint8_t iflags = 0);

    void async_write_fixed(int fd, const void *buf, unsigned nbytes, off_t offset, int buf_index,
                           completion_callback cb, std::uint8_t iflags = 0);

    void async_fsync(int fd, unsigned fsync_flags, completion_callback cb, std::uint8_t iflags = 0);

    void async_sync_file_range(int fd, off64_t offset, off64_t nbytes, unsigned sync_range_flags,
                               completion_callback cb, std::uint8_t iflags = 0);

    void async_recvmsg(int sockfd, msghdr *msg, std::uint32_t flags, completion_callback cb, std::uint8_t iflags = 0);

    void async_sendmsg(int sockfd, const msghdr *msg, std::uint32_t flags, completion_callback cb,
                       std::uint8_t iflags = 0);

    void async_recv(int sockfd, void *buf, unsigned nbytes, std::uint32_t flags, completion_callback cb,
                    std::uint8_t iflags = 0);

    void async_send(int sockfd, const void *buf, unsigned nbytes, std::uint32_t flags, completion_callback cb,
                    std::uint8_t iflags = 0);

    void async_poll(int fd, short poll_mask, completion_callback cb, std::uint8_t iflags = 0);

    void async_yield(completion_callback cb, std::uint8_t iflags = 0);

    void async_accept(int fd, sockaddr *addr, socklen_t *addrlen, int flags, completion_callback cb,
                      std::uint8_t iflags = 0);

    void async_connect(int fd, sockaddr *addr, socklen_t addrlen, completion_callback cb, std::uint8_t iflags = 0);

    void async_timeout(__kernel_timespec *ts, completion_callback cb, std::uint8_t iflags = 0);

    void async_openat(int dfd, const char *path, int flags, mode_t mode, completion_callback cb,
                      std::uint8_t iflags = 0);

    void async_close(int fd, completion_callback cb, std::uint8_t iflags = 0);

    void async_statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf,
                     completion_callback cb, std::uint8_t iflags = 0);

    void async_splice(int fd_in, loff_t off_in, int fd_out, loff_t off_out, size_t nbytes, unsigned flags,
                      completion_callback cb, std::uint8_t iflags = 0);

    void async_tee(int fd_in, int fd_out, size_t nbytes, unsigned flags, completion_callback cb,
                   std::uint8_t iflags = 0);

    void async_shutdown(int fd, int how, completion_callback cb, std::uint8_t iflags = 0);

    void async_renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath, unsigned flags,
                        completion_callback cb, std::uint8_t iflags = 0);

    void async_mkdirat(int dirfd, const char *pathname, mode_t mode, completion_callback cb, std::uint8_t iflags = 0);

    void async_symlinkat(const char *target, int newdirfd, const char *linkpath, completion_callback cb,
                         std::uint8_t iflags = 0);

    void async_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags,
                      completion_callback cb, std::uint8_t iflags = 0);

    void async_unlinkat(int dfd, const char *path, unsigned flags, completion_callback cb, std::uint8_t iflags = 0);

    void async_msg_ring(int fd, unsigned len, std::uint64_t data, unsigned flags, completion_callback cb,
                        std::uint8_t iflags = 0);

    int readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset);
    int writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset);
    int read(int fd, void *buf, unsigned nbytes, off_t offset);
    int write(int fd, const void *buf, unsigned nbytes, off_t offset);
    int fsync(int fd, unsigned fsync_flags);
    int close(int fd);
    int openat(int dfd, const char *path, int flags, mode_t mode);
    int accept(int fd, sockaddr *addr, socklen_t *addrlen, int flags = 0);
    int connect(int fd, sockaddr *addr, socklen_t addrlen);

    void run();
    void run_once();
    void poll();
    void stop();

    void register_file(int fd);
    void unregister_file(unsigned int fd) noexcept;

    void register_files(std::span<const int> fds);
    void register_files_update(unsigned off, std::span<int> files);
    int unregister_files() noexcept;

    void register_buffers(std::span<const iovec> iovecs);
    int unregister_buffers() noexcept;

    [[nodiscard]] SharedContext &context() noexcept { return m_context; }
    [[nodiscard]] const SharedContext &context() const noexcept { return m_context; }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            run_once();
            shared::this_handler::shedule();
        };
    }

    shared::ReleaseFunction on_released() noexcept override {
        return [this]() noexcept { stop(); };
    }

  private:
    SharedContext m_context;

    bool m_running;
};

} // namespace io::base::leverage
