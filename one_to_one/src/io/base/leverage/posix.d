module io.base.leverage.posix;
@nogc nothrow:

import io.base.leverage.types;
import io.base.leverage.uring;
import util.alloc : make, dispose;

// Pending operation structure
struct pending_op {
    completion_callback callback;
    void*  buffer       = null;
    uint   buffer_size  = 0;
    off_t  offset       = 0;
    int    op_type_val  = 0;  // PORT-NOTE: renamed from op_type to avoid shadowing the enum
}

class Context {
  public:
    this(int entries = 64, uint flags = 0, uint wq_fd = 0) {
        // m_probe_ops{}
        init(entries, flags, wq_fd);
    }

    ~this() {}  // cleanup is explicit via cleanup()

    void init(int entries, uint flags, uint wq_fd) {
        io_uring_params p;
        // PORT-NOTE: io_uring_params fields — see c/uring.c shim
        // p.flags = flags; p.wq_fd = wq_fd; (zeroed by default in D)

        int ret = io_uring_queue_init_params(entries, &m_ring, &p);
        if (ret < 0) {
            panic_on_err("queue_init_params", ret, false);
        }

        verbose_print("Linux io_uring initialized");
    }

    void cleanup() { io_uring_queue_exit(&m_ring); }

    io_uring_sqe* get_sqe_safe() {
        auto sqe = io_uring_get_sqe(&m_ring);
        if (sqe !is null) {
            return sqe;
        }

        verbose_print("{}: SQ is full, flushing {} cqe(s)\n", __FILE__, m_cqe_count);

        io_uring_cq_advance(&m_ring, m_cqe_count);
        m_cqe_count = 0;
        io_uring_submit(&m_ring);

        sqe = io_uring_get_sqe(&m_ring);
        if (sqe !is null) {
            return sqe;
        }
        panic("io_uring_get_sqe", enomem);
        return null;  // unreachable
    }

    void submit_async(io_uring_sqe* sqe, completion_callback cb, ubyte iflags) {
        io_uring_sqe_set_flags(sqe, iflags);

        // PORT-NOTE: std::make_unique<pending_op>() → make!pending_op from util.alloc
        auto op = make!pending_op();
        op.callback = cb;
        io_uring_sqe_set_data(sqe, cast(void*) op);
    }

    void process_completions() {
        for_each_cqe(&m_ring, (io_uring_cqe* cqe) @nogc nothrow {
            ++m_cqe_count;

            auto op = cast(pending_op*) io_uring_cqe_get_data(cqe);
            if (op !is null) {
                int result   = io_uring_cqe_get_res(cqe);
                auto callback = op.callback;
                dispose(op);  // PORT-NOTE: delete op → dispose

                if (callback) {
                    callback(result);
                }
            }
        });

        verbose_print("{}: Found {} cqe(s), looping...\n", __FILE__, m_cqe_count);

        io_uring_cq_advance(&m_ring, m_cqe_count);
        m_cqe_count = 0;
    }

    void set_cqe_count(uint count) { m_cqe_count = count; }

    io_uring*       get_ring()       { return &m_ring; }
    const(io_uring)* get_ring() const { return &m_ring; }

  private:
    io_uring m_ring;
    uint     m_cqe_count = 0;
    // std::array<bool, 128> m_probe_ops;
}


// Leverager!(Context) specialisation for Linux / io_uring
// PORT-NOTE: C++ used explicit template specialisations; D overrides the base class methods.
class PosixLeverager : Leverager!Context {
  public:
    this(int entries = 64, uint flags = 0, uint wq_fd = 0) {
        // PORT-NOTE: m_context constructed in-line in C++; call super
        m_context = make!Context(entries, flags, wq_fd);
        m_running = false;
    }

    ~this() {
        m_context.cleanup();
        dispose(m_context);
    }

    override void run() {
        io_uring_submit_and_wait(m_context.get_ring(), 1);
        m_context.process_completions();
    }

    override void readv(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_readv(sqe, fd, iovecs, nr_vecs, offset);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void readv2(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_readv2(sqe, fd, iovecs, nr_vecs, offset, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void writev(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_writev(sqe, fd, iovecs, nr_vecs, offset);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void writev2(int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_writev2(sqe, fd, iovecs, nr_vecs, offset, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void read(int fd, void* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_read(sqe, fd, buf, nbytes, offset);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void write(int fd, const(void)* buf, uint nbytes, off_t offset, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_write(sqe, fd, buf, nbytes, offset);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void read_fixed(int fd, void* buf, uint nbytes, off_t offset, int buf_index, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_read_fixed(sqe, fd, buf, nbytes, offset, buf_index);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void write_fixed(int fd, const(void)* buf, uint nbytes, off_t offset, int buf_index, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_write_fixed(sqe, fd, buf, nbytes, offset, buf_index);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void fsync(int fd, uint fsync_flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_fsync(sqe, fd, fsync_flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void sync_file_range(int fd, off64_t offset, off64_t nbytes, uint sync_range_flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_rw(OP_SYNC_FILE_RANGE, sqe, fd, null, cast(uint) nbytes, cast(ulong) offset);
        // sqe->sync_range_flags = sync_range_flags;
        // PORT-NOTE: io_uring_sqe field sync_range_flags not in shim; access via cast if needed in Run 2
        m_context.submit_async(sqe, cb, iflags);
    }

    override void recvmsg(int sockfd, msghdr* msg, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_recvmsg(sqe, sockfd, msg, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void sendmsg(int sockfd, const(msghdr)* msg, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_sendmsg(sqe, sockfd, msg, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void recv(int sockfd, void* buf, uint nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_recv(sqe, sockfd, buf, nbytes, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void send(int sockfd, const(void)* buf, uint nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_send(sqe, sockfd, buf, nbytes, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void poll(int fd, short poll_mask, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_poll_add(sqe, fd, poll_mask);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void yield_(completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_nop(sqe);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void connect(int fd, sockaddr* addr, socklen_t addrlen, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_connect(sqe, fd, addr, addrlen);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void timeout(kernel_timespec_t* ts, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_timeout(sqe, ts, 0, 0);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void openat(int dfd, const(char)* path, int flags, mode_t mode, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_openat(sqe, dfd, path, flags, mode);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void close(int fd, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_close(sqe, fd);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void statx_(int dfd, const(char)* path, int flags, uint mask, statx_t* statxbuf, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_statx(sqe, dfd, path, flags, mask, statxbuf);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void splice(int fd_in, loff_t off_in, int fd_out, loff_t off_out, size_t nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_splice(sqe, fd_in, off_in, fd_out, off_out, cast(uint) nbytes, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void tee(int fd_in, int fd_out, size_t nbytes, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_tee(sqe, fd_in, fd_out, cast(uint) nbytes, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void shutdown(int fd, int how, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_shutdown(sqe, fd, how);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void renameat(int olddfd, const(char)* oldpath, int newdfd, const(char)* newpath, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_renameat(sqe, olddfd, oldpath, newdfd, newpath, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void mkdirat(int dirfd, const(char)* pathname, mode_t mode, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_mkdirat(sqe, dirfd, pathname, mode);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void symlinkat(const(char)* target, int newdirfd, const(char)* linkpath, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_symlinkat(sqe, target, newdirfd, linkpath);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void linkat(int olddirfd, const(char)* oldpath, int newdirfd, const(char)* newpath, int flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_linkat(sqe, olddirfd, oldpath, newdirfd, newpath, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void unlinkat(int dfd, const(char)* path, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_unlinkat(sqe, dfd, path, cast(int) flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void msg_ring(int fd, uint len, ulong data, uint flags, completion_callback cb, ubyte iflags = 0) {
        auto sqe = m_context.get_sqe_safe();
        io_uring_prep_msg_ring(sqe, fd, len, data, flags);
        m_context.submit_async(sqe, cb, iflags);
    }

    override void poll_ring() {
        io_uring_cqe* cqe;
        if (io_uring_peek_cqe(m_context.get_ring(), &cqe) == 0) {
            m_context.process_completions();
        }
    }

    override void stop() {
        m_running = false;
    }

    override void register_files(const(int)[] fds) {
        int ret = io_uring_register_files(m_context.get_ring(), fds.ptr, cast(uint) fds.length);
        panic_on_err("io_uring_register_files", ret, false);
    }

    override void register_files_update(uint off, int[] files) {
        int ret = io_uring_register_files_update(m_context.get_ring(), off, files.ptr, cast(uint) files.length);
        panic_on_err("io_uring_register_files_update", ret, false);
    }

    override int unregister_files() {
        return io_uring_unregister_files(m_context.get_ring());
    }

    override void register_buffers(const(iovec)[] iovecs) {
        int ret = io_uring_register_buffers(m_context.get_ring(), iovecs.ptr, cast(uint) iovecs.length);
        panic_on_err("io_uring_register_buffers", ret, false);
    }

    override int unregister_buffers() {
        return io_uring_unregister_buffers(m_context.get_ring());
    }

    override void register_file(int fd) {
        int[1] arr = [fd];
        register_files(arr[]);
    }

    override void unregister_file(uint fd) {
        int[1] sentinel = [-1];
        register_files_update(fd, sentinel[]);
    }
}
