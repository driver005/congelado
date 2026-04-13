module;

#include <errno.h>
#include <linux/time_types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

export module io_base_leverage:posix;

import std;
import :types;
import :uring;

export namespace transport::base::leverage {


// Pending operation structure
struct pending_op {
    completion_callback callback;
    void *buffer = nullptr;
    unsigned buffer_size = 0;
    off_t offset = 0;
    int op_type = 0;
};

class Context {
  public:
    Context(int entries = 64, std::uint32_t flags = 0, std::uint32_t wq_fd = 0) : m_ring{}, m_cqe_count{0} {
        // m_probe_ops{}
        init(entries, flags, wq_fd);
    };

    ~Context() noexcept {};

    // Non-copyable, non-movable
    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;
    Context(Context &&) = delete;
    Context &operator=(Context &&) = delete;

    void init(int entries, std::uint32_t flags, std::uint32_t wq_fd) {
        liburing::io_uring_params p = {
            .sq_entries = 0,
            .cq_entries = 0,
            .flags = flags,
            .sq_thread_cpu = 0,
            .sq_thread_idle = 0,
            .features = 0,
            .wq_fd = wq_fd,
            .resv = {},
            .sq_off = {},
            .cq_off = {},
        };

        int ret = liburing::io_uring_queue_init_params(entries, &m_ring, &p);
        if (ret < 0) {
            panic_on_err("queue_init_params", ret, false);
        }

        verbose_print("Linux io_uring initialized");
    };

    void cleanup() noexcept { liburing::io_uring_queue_exit(&m_ring); };
    liburing::io_uring_sqe *get_sqe_safe() {
        auto *sqe = liburing::io_uring_get_sqe(&m_ring);
        if (__builtin_expect(!!sqe, true)) {
            return sqe;
        }

        verbose_print("{}: SQ is full, flushing {} cqe(s)\n", __FILE__, m_cqe_count);

        liburing::io_uring_cq_advance(&m_ring, m_cqe_count);
        m_cqe_count = 0;
        liburing::io_uring_submit(&m_ring);

        sqe = liburing::io_uring_get_sqe(&m_ring);
        if (__builtin_expect(!!sqe, true)) {
            return sqe;
        }
        panic("liburing::io_uring_get_sqe", liburing::enomem);
    };

    void submit_async(liburing::io_uring_sqe *sqe, completion_callback cb, std::uint8_t iflags) {
        liburing::io_uring_sqe_set_flags(sqe, iflags);

        auto op = std::make_unique<pending_op>();
        op->callback = std::move(cb);
        liburing::io_uring_sqe_set_data(sqe, op.release());
    };

    void process_completions() {
        liburing::for_each_cqe(&m_ring, [&](auto cqe) {
            ++m_cqe_count;

            auto *op = static_cast<pending_op *>(liburing::io_uring_cqe_get_data(cqe));
            if (op) {
                int result = cqe->res;
                auto callback = std::move(op->callback);
                delete op;

                if (callback) {
                    callback(result);
                }
            }
        });

        verbose_print("{}: Found {} cqe(s), looping...\n", __FILE__, m_cqe_count);

        liburing::io_uring_cq_advance(&m_ring, m_cqe_count);
        m_cqe_count = 0;
    };

    void set_cqe_count(unsigned count) { m_cqe_count = count; }

    liburing::io_uring *get_ring() noexcept { return &m_ring; }
    const liburing::io_uring *get_ring() const noexcept { return &m_ring; }


  private:
    liburing::io_uring m_ring;
    unsigned m_cqe_count = 0;
    // std::array<bool, 128> m_probe_ops;
};


template <>
Leverager<Context>::Leverager(int entries, std::uint32_t flags, std::uint32_t wq_fd)
    : m_context{Context{entries, flags, wq_fd}}, m_running{false} {};

template <>
Leverager<Context>::~Leverager() noexcept {
    m_context.~Context();
}


template <>
void Leverager<Context>::run_once() {
    liburing::io_uring_submit_and_wait(m_context.get_ring(), 1);
    m_context.process_completions();
}

template <>
void Leverager<Context>::run() {
    m_running = true;
    while (m_running) {
        run_once();
    }
}


template <>
void Leverager<Context>::async_readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                     completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_readv(sqe, fd, iovecs, nr_vecs, offset);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_readv2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset, int flags,
                                      completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_readv2(sqe, fd, iovecs, nr_vecs, offset, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                      completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_writev(sqe, fd, iovecs, nr_vecs, offset);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_writev2(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset, int flags,
                                       completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_writev2(sqe, fd, iovecs, nr_vecs, offset, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_read(int fd, void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                                    std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_read(sqe, fd, buf, nbytes, offset);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_write(int fd, const void *buf, unsigned nbytes, off_t offset, completion_callback cb,
                                     std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_write(sqe, fd, buf, nbytes, offset);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_read_fixed(int fd, void *buf, unsigned nbytes, off_t offset, int buf_index,
                                          completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_read_fixed(sqe, fd, buf, nbytes, offset, buf_index);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_write_fixed(int fd, const void *buf, unsigned nbytes, off_t offset, int buf_index,
                                           completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_write_fixed(sqe, fd, buf, nbytes, offset, buf_index);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_fsync(int fd, unsigned fsync_flags, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_fsync(sqe, fd, fsync_flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_sync_file_range(int fd, off64_t offset, off64_t nbytes, unsigned sync_range_flags,
                                               completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_rw(liburing::OP_SYNC_FILE_RANGE, sqe, fd, nullptr, nbytes, offset);
    sqe->sync_range_flags = sync_range_flags;
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_recvmsg(int sockfd, msghdr *msg, std::uint32_t flags, completion_callback cb,
                                       std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_recvmsg(sqe, sockfd, msg, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_sendmsg(int sockfd, const msghdr *msg, std::uint32_t flags, completion_callback cb,
                                       std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_sendmsg(sqe, sockfd, msg, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_recv(int sockfd, void *buf, unsigned nbytes, std::uint32_t flags, completion_callback cb,
                                    std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_recv(sqe, sockfd, buf, nbytes, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_send(int sockfd, const void *buf, unsigned nbytes, std::uint32_t flags,
                                    completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_send(sqe, sockfd, buf, nbytes, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_poll(int fd, short poll_mask, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_poll_add(sqe, fd, poll_mask);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_yield(completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_nop(sqe);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_accept(int fd, sockaddr *addr, socklen_t *addrlen, int flags, completion_callback cb,
                                      std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_connect(int fd, sockaddr *addr, socklen_t addrlen, completion_callback cb,
                                       std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_connect(sqe, fd, addr, addrlen);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_timeout(__kernel_timespec *ts, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_timeout(sqe, ts, 0, 0);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_openat(int dfd, const char *path, int flags, mode_t mode, completion_callback cb,
                                      std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_openat(sqe, dfd, path, flags, mode);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_close(int fd, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_close(sqe, fd);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf,
                                     completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_statx(sqe, dfd, path, flags, mask, statxbuf);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_splice(int fd_in, off_t off_in, int fd_out, off_t off_out, std::size_t nbytes,
                                      unsigned flags, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_splice(sqe, fd_in, off_in, fd_out, off_out, nbytes, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_tee(int fd_in, int fd_out, std::size_t nbytes, unsigned flags, completion_callback cb,
                                   std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_tee(sqe, fd_in, fd_out, nbytes, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_shutdown(int fd, int how, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_shutdown(sqe, fd, how);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath,
                                        unsigned flags, completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_renameat(sqe, olddfd, oldpath, newdfd, newpath, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_mkdirat(int dirfd, const char *pathname, mode_t mode, completion_callback cb,
                                       std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_mkdirat(sqe, dirfd, pathname, mode);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_symlinkat(const char *target, int newdirfd, const char *linkpath, completion_callback cb,
                                         std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_symlinkat(sqe, target, newdirfd, linkpath);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags,
                                      completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_linkat(sqe, olddirfd, oldpath, newdirfd, newpath, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_unlinkat(int dfd, const char *path, unsigned flags, completion_callback cb,
                                        std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_unlinkat(sqe, dfd, path, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
void Leverager<Context>::async_msg_ring(int fd, unsigned len, std::uint64_t data, unsigned flags,
                                        completion_callback cb, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_msg_ring(sqe, fd, len, data, flags);
    m_context.submit_async(sqe, std::move(cb), iflags);
}

template <>
int Leverager<Context>::readv(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset) {
    int result = -1;
    async_readv(fd, iovecs, nr_vecs, offset, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::writev(int fd, const iovec *iovecs, unsigned nr_vecs, off_t offset) {
    int result = -1;
    async_writev(fd, iovecs, nr_vecs, offset, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::read(int fd, void *buf, unsigned nbytes, off_t offset) {
    int result = -1;
    async_read(fd, buf, nbytes, offset, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::write(int fd, const void *buf, unsigned nbytes, off_t offset) {
    int result = -1;
    async_write(fd, buf, nbytes, offset, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::fsync(int fd, unsigned fsync_flags) {
    int result = -1;
    async_fsync(fd, fsync_flags, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::close(int fd) {
    int result = -1;
    async_close(fd, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::openat(int dfd, const char *path, int flags, mode_t mode) {
    int result = -1;
    async_openat(dfd, path, flags, mode, [&](int res) { result = res; });
    run_once();
    return result;
}

template <>
int Leverager<Context>::accept(int fd, sockaddr *addr, socklen_t *addrlen, int flags) {
    int result = -1;
    async_accept(fd, addr, addrlen, flags, [&](int res) { result = res; });
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
void Leverager<Context>::poll() {
    liburing::io_uring_cqe *cqe;
    if (liburing::io_uring_peek_cqe(m_context.get_ring(), &cqe) == 0) {
        m_context.process_completions();
    }
}

template <>
void Leverager<Context>::stop() {
    m_running = false;
}

template <>
void Leverager<Context>::register_files(std::span<const int> fds) {
    int ret = liburing::io_uring_register_files(m_context.get_ring(), fds.data(), static_cast<unsigned>(fds.size()));
    panic_on_err("liburing::io_uring_register_files", ret, false);
}

template <>
void Leverager<Context>::register_files_update(unsigned off, std::span<int> files) {
    int ret = liburing::io_uring_register_files_update(m_context.get_ring(), off, files.data(),
                                                       static_cast<unsigned>(files.size()));
    panic_on_err("liburing::io_uring_register_files_update", ret, false);
}

template <>
int Leverager<Context>::unregister_files() noexcept {
    return liburing::io_uring_unregister_files(m_context.get_ring());
}

template <>
void Leverager<Context>::register_buffers(std::span<const iovec> iovecs) {
    int ret =
        liburing::io_uring_register_buffers(m_context.get_ring(), iovecs.data(), static_cast<unsigned>(iovecs.size()));
    panic_on_err("liburing::io_uring_register_buffers", ret, false);
}

template <>
int Leverager<Context>::unregister_buffers() noexcept {
    return liburing::io_uring_unregister_buffers(m_context.get_ring());
}

// io_uring &template <> Leverager<Context>::get_handle() noexcept { return m_ring; }


} // namespace transport::base::leverage
