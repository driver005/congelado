module io.base.leverage.types;
@nogc nothrow:

import shared_.handler : HandlerBase, WorkerFunction, ReleaseFunction;

enum op_type : int {
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
}

version (Windows) {
    // PORT-NOTE: Windows HANDLE/SOCKET types — kept as opaque size_t placeholders.
    // Full Win32 binding belongs in a dedicated win32.d binding layer.
    alias native_fd_t = size_t;    // HANDLE
    alias socket_t    = size_t;    // SOCKET
    static immutable socket_t invalid_socket = size_t.max;

    // PORT-NOTE: ABI POD structs for Windows compatibility layer.
    // These are only used in the win32 leverage specialisation.
    extern(C) struct iovec {
        void*  iov_base;
        size_t iov_len;
    }

    // statx compatibility
    extern(C) struct statx_t {  // PORT-NOTE: renamed from statx to avoid conflict with posix statx
        uint   stx_mask;
        uint   stx_blksize;
        ulong  stx_attributes;
        uint   stx_nlink;
        uint   stx_uid;
        uint   stx_gid;
        ushort stx_mode;
        ushort[1] spare0;
        ulong  stx_ino;
        ulong  stx_size;
        ulong  stx_blocks;
        ulong  stx_attributes_mask;
        // PORT-NOTE: timespec fields omitted — see c/uring.c shim for __kernel_timespec
        uint   stx_rdev_major;
        uint   stx_rdev_minor;
        uint   stx_dev_major;
        uint   stx_dev_minor;
        ulong[14] spare2;
    }

    // timespec for Windows
    extern(C) struct kernel_timespec_t {
        long tv_sec;
        long tv_nsec;
    }

    // sockaddr compatibility (already in winsock)
    extern(C) struct msghdr {
        void*   msg_name;        // Optional address
        uint    msg_namelen;     // Size of address
        iovec*  msg_iov;         // Scatter/gather array
        size_t  msg_iovlen;      // # elements in msg_iov
        void*   msg_control;     // Ancillary data
        size_t  msg_controllen;  // Ancillary data buffer len
        int     msg_flags;       // Flags on received message
    }

    // File flags
    enum int AT_FDCWD    = -100;
    enum int O_RDONLY    = 0x0000;
    enum int O_WRONLY    = 0x0001;
    enum int O_RDWR      = 0x0002;
    enum int O_CREAT     = 0x0100;
    enum int O_TRUNC     = 0x0200;
    enum int O_APPEND    = 0x0400;
    enum int O_NONBLOCK  = 0x1000;
    enum int O_CLOEXEC   = 0x2000;

    // Mode flags
    alias mode_t = uint;
    enum mode_t S_IRUSR    = 0400;
    enum mode_t S_IWUSR    = 0200;
    enum mode_t S_IXUSR    = 0100;
    enum mode_t S_IRGRP    = 0040;
    enum mode_t S_IWGRP    = 0020;
    enum mode_t S_IXGRP    = 0010;
    enum mode_t S_IROTH    = 0004;
    enum mode_t S_IWOTH    = 0002;
    enum mode_t S_IXOTH    = 0001;
    enum mode_t S_IRWXU    = 0700;
    enum mode_t S_IRWXG    = 0070;
    enum mode_t S_IRWXO    = 0007;
    enum mode_t S_ISUID    = 04000;
    enum mode_t S_ISGID    = 02000;
    enum mode_t S_ISVTX    = 01000;
    enum mode_t ACCESSPERMS = 0777;

    // Shutdown flags
    enum int SHUT_RD   = 0;
    enum int SHUT_WR   = 1;
    enum int SHUT_RDWR = 2;

    // Sync flags
    enum uint SYNC_FILE_RANGE_WAIT_BEFORE = 1;
    enum uint SYNC_FILE_RANGE_WRITE       = 2;
    enum uint SYNC_FILE_RANGE_WAIT_AFTER  = 4;

    // Statx flags
    enum int AT_SYMLINK_NOFOLLOW = 0x100;
    enum int AT_NO_AUTOMOUNT     = 0x200;
    enum int AT_EMPTY_PATH       = 0x400;
    enum uint STATX_TYPE         = 0x0001;
    enum uint STATX_MODE         = 0x0002;
    enum uint STATX_NLINK        = 0x0004;
    enum uint STATX_UID          = 0x0008;
    enum uint STATX_GID          = 0x0010;
    enum uint STATX_ATIME        = 0x0020;
    enum uint STATX_MTIME        = 0x0040;
    enum uint STATX_CTIME        = 0x0080;
    enum uint STATX_INO          = 0x0100;
    enum uint STATX_SIZE         = 0x0200;
    enum uint STATX_BLOCKS       = 0x0400;
    enum uint STATX_BASIC_STATS  = 0x07ff;
    enum uint STATX_BTIME        = 0x0800;
    enum uint STATX_ALL          = 0x0fff;

    // Splice flags
    enum uint SPLICE_F_MOVE     = 1;
    enum uint SPLICE_F_NONBLOCK = 2;
    enum uint SPLICE_F_MORE     = 4;
    enum uint SPLICE_F_GIFT     = 8;

    // Rename flags
    enum uint RENAME_NOREPLACE = 1;
    enum uint RENAME_EXCHANGE  = 2;
    enum uint RENAME_WHITEOUT  = 4;

    // Unlink flags
    enum uint AT_REMOVEDIR = 0x200;

    // Link flags
    enum int AT_SYMLINK_FOLLOW = 0x400;

    // Fsync flags
    enum uint FSYNC_DATASYNC = 1;

    // Type aliases
    alias off_t   = long;
    alias off64_t = long;
    alias loff_t  = long;
    alias ssize_t = long;

    // PORT-NOTE: ABI POD struct
    extern(C) struct timespec_t {
        long tv_sec;
        long tv_nsec;
    }

} else {
    // POSIX — these come from the C runtime / liburing headers
    alias native_fd_t = int;
    alias socket_t    = int;
    static immutable socket_t invalid_socket = -1;

    // Import POSIX types from C runtime
    import core.sys.posix.sys.uio   : iovec;
    import core.sys.posix.sys.types : off_t, mode_t, ssize_t;
    import core.stdc.config         : c_long;

    alias off64_t = long;
    alias loff_t  = long;

    // PORT-NOTE: __kernel_timespec is not in druntime; declare extern(C)
    extern(C) struct kernel_timespec_t {  // maps to __kernel_timespec
        long tv_sec;
        long tv_nsec;
    }

    // PORT-NOTE: msghdr comes from core.sys.posix
    import core.sys.posix.sys.socket : sockaddr, socklen_t, msghdr;
    // PORT-NOTE: statx not in druntime; declare minimal extern(C) struct
    extern(C) struct statx_t {
        uint   stx_mask;
        uint   stx_blksize;
        ulong  stx_attributes;
        uint   stx_nlink;
        uint   stx_uid;
        uint   stx_gid;
        ushort stx_mode;
        ushort[1] spare0;
        ulong  stx_ino;
        ulong  stx_size;
        ulong  stx_blocks;
        ulong  stx_attributes_mask;
        // PORT-NOTE: io_uring struct field, see c/uring.c shim (timespec sub-fields)
        uint   stx_rdev_major;
        uint   stx_rdev_minor;
        uint   stx_dev_major;
        uint   stx_dev_minor;
        ulong[14] spare2;
    }
}

// PORT-NOTE: std::move_only_function<void(int)> → @nogc nothrow function pointer + void* context.
// completion_callback is a pair so callers remain @nogc.
struct completion_callback {
    void function(int, void*) @nogc nothrow fn;
    void* ctx;

    bool opCast(T : bool)() const { return fn !is null; }
    void opCall(int result) { if (fn) fn(result, ctx); }
}

bool verbose_enabled() { return false; }

// PORT-NOTE: verbose_print is a no-op when verbose_enabled() == false.
void verbose_print(const(char)[] fmt, ...) {
    // no-op: verbose_enabled() == false
}

// Error handling
// PORT-NOTE: [[noreturn]] panic() threw std::system_error; D port calls abort().
import core.stdc.stdlib : abort;
import core.stdc.stdio  : fprintf, stderr;

void panic(const(char)[] msg, int err) {
    fprintf(stderr, "panic: %.*s (errno=%d)\n", cast(int) msg.length, msg.ptr, err);
    abort();
}

void panic_on_err(const(char)[] msg, int ret, bool ignore_eagain = false) {
    if (ret < 0) {
        version (Windows) {
            enum int EAGAIN_EQUIV = 10035;  // WSAEWOULDBLOCK
        } else {
            import core.stdc.errno : EAGAIN;
            enum int EAGAIN_EQUIV = EAGAIN;
        }
        if (ignore_eagain && ret == -EAGAIN_EQUIV) return;
        panic(msg, -ret);
    }
}

// Forward declaration of Leverager.
// The concrete implementation lives in posix.d / win32.d.
// PORT-NOTE: C++ Leverager<SharedContext> is a template; D uses a class with a
//            SharedContext type parameter via alias.
class Leverager(SharedContext) : HandlerBase {
  public:
    this(int entries = 64, uint flags = 0, uint wq_fd = 0);
    ~this();

    // Async operation methods
    void readv(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, completion_callback cb, ubyte iflags = 0);
    void readv2(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags, completion_callback cb, ubyte iflags = 0);
    void writev(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, completion_callback cb, ubyte iflags = 0);
    void writev2(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags, completion_callback cb, ubyte iflags = 0);
    void read(int fd, void* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0);
    void write(int fd, const(void)* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0);
    void read_fixed(int fd, void* buf, uint nbytes, off_t offset, int buf_index, completion_callback cb, ubyte iflags = 0);
    void write_fixed(int fd, const(void)* buf, uint nbytes, off_t offset, int buf_index, completion_callback cb, ubyte iflags = 0);
    void fsync(int fd, uint fsync_flags, completion_callback cb, ubyte iflags = 0);
    void sync_file_range(int fd, off64_t offset, off64_t nbytes, uint sync_range_flags, completion_callback cb, ubyte iflags = 0);
    void recvmsg(int sockfd, msghdr* msg, uint flags, completion_callback cb, ubyte iflags = 0);
    void sendmsg(int sockfd, const(msghdr)* msg, uint flags, completion_callback cb, ubyte iflags = 0);
    void recv(int sockfd, void* buf, uint nbytes, uint flags, completion_callback cb, ubyte iflags = 0);
    void send(int sockfd, const(void)* buf, uint nbytes, uint flags, completion_callback cb, ubyte iflags = 0);
    void poll(int fd, short poll_mask, completion_callback cb, ubyte iflags = 0);
    void yield_(completion_callback cb, ubyte iflags = 0);  // PORT-NOTE: renamed from yield (D keyword)
    void accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags, completion_callback cb, ubyte iflags = 0);
    void connect(int fd, sockaddr* addr, socklen_t addrlen, completion_callback cb, ubyte iflags = 0);
    void timeout(kernel_timespec_t* ts, completion_callback cb, ubyte iflags = 0);
    void openat(int dfd, const(char)* path, int flags, mode_t mode, completion_callback cb, ubyte iflags = 0);
    void close(int fd, completion_callback cb, ubyte iflags = 0);
    void statx_(int dfd, const(char)* path, int flags, uint mask, statx_t* statxbuf, completion_callback cb, ubyte iflags = 0);  // PORT-NOTE: renamed from statx
    void splice(int fd_in, loff_t off_in, int fd_out, loff_t off_out, size_t nbytes, uint flags, completion_callback cb, ubyte iflags = 0);
    void tee(int fd_in, int fd_out, size_t nbytes, uint flags, completion_callback cb, ubyte iflags = 0);
    void shutdown(int fd, int how, completion_callback cb, ubyte iflags = 0);
    void renameat(int olddfd, const(char)* oldpath, int newdfd, const(char)* newpath, uint flags, completion_callback cb, ubyte iflags = 0);
    void mkdirat(int dirfd, const(char)* pathname, mode_t mode, completion_callback cb, ubyte iflags = 0);
    void symlinkat(const(char)* target, int newdirfd, const(char)* linkpath, completion_callback cb, ubyte iflags = 0);
    void linkat(int olddirfd, const(char)* oldpath, int newdirfd, const(char)* newpath, int flags, completion_callback cb, ubyte iflags = 0);
    void unlinkat(int dfd, const(char)* path, uint flags, completion_callback cb, ubyte iflags = 0);
    void msg_ring(int fd, uint len, ulong data, uint flags, completion_callback cb, ubyte iflags = 0);

    void run();
    void poll_ring();  // PORT-NOTE: overloaded poll() renamed to poll_ring() to avoid collision
    void stop();

    void register_file(int fd);
    void unregister_file(uint fd);

    void register_files(const(int)[] fds);
    void register_files_update(uint off, int[] files);
    int unregister_files();

    void register_buffers(const(iovec)[] iovecs);
    int unregister_buffers();

    ref SharedContext context() { return m_context; }
    ref const(SharedContext) context() const { return m_context; }

    override const(char)[] get_name() const { return "Leverager"; }

    override WorkerFunction on_execute() {
        return WorkerFunction(&this._do_run, cast(void*) this);
    }

    override ReleaseFunction on_released() {
        return ReleaseFunction(&this._do_stop, cast(void*) this);
    }

  private:
    SharedContext m_context;
    bool          m_running;

    static void _do_run(void* self) @nogc nothrow {
        auto lev = cast(Leverager!SharedContext) self;
        lev.run();
        // PORT-NOTE: shared_.this_handler.shedule() call omitted — see shared/handler.d
    }

    static void _do_stop(void* self) @nogc nothrow {
        auto lev = cast(Leverager!SharedContext) self;
        lev.stop();
    }
}
