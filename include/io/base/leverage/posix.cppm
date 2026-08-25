module;

#include <cerrno>
#include <linux/time_types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

export module io_base_leverage:posix;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import :types;
import :uring;

export namespace io::base::leverage {


// Pending operation structure
class PendingOp {
  public:
    void set_callback(completion_callback callback) { m_callback = std::move(callback); }
    void set_buffer(void *buffer) { m_buffer = buffer; }
    void set_buffer_size(unsigned buffer_size) { m_buffer_size = buffer_size; }
    void set_offset(off_t offset) { m_offset = offset; }
    void set_op_type(int op_type) { m_op_type = op_type; }

    [[nodiscard]] completion_callback &get_callback() { return m_callback; }
    [[nodiscard]] void *get_buffer() const { return m_buffer; }
    [[nodiscard]] unsigned get_buffer_size() const { return m_buffer_size; }
    [[nodiscard]] off_t get_offset() const { return m_offset; }
    [[nodiscard]] int get_op_type() const { return m_op_type; }

  private:
    completion_callback m_callback;
    void *m_buffer = nullptr;
    unsigned m_buffer_size = 0;
    off_t m_offset = 0;
    int m_op_type = 0;
};

class Context {
  public:
    /**
     * @brief Builds the io_uring ring right away — no lazy init here, by the time this ctor
     * returns the ring is live and ready to take SQEs.
     * @param entries submission queue depth to reserve.
     * @param flags io_uring setup flags (e.g. `IORING_SETUP_SQPOLL`).
     * @param wq_fd shared async worker-queue fd to attach to, or 0 for a fresh one.
     * @throws std::system_error via panic_on_err() if `io_uring_queue_init_params` fails.
     */
    Context(int entries = 64, std::uint32_t flags = 0, std::uint32_t wq_fd = 0) : m_ring{} {
        // m_probe_ops{}
        init(entries, flags, wq_fd);
    };

    /**
     * @brief Empty by design — the ring itself gets torn down by Leverager<Context>'s
     * explicit-specialization dtor calling `m_context.~Context()` manually, not by this dtor
     * running cleanup(). Kinda sus double-teardown pattern but that's how it's wired.
     */
    ~Context() noexcept = default;

    // Non-copyable, non-movable
    /** @brief Deleted — copying an owning `io_uring` ring would double-own kernel resources. */
    Context(const Context &) = delete;
    /** @brief Deleted — same reasoning as the copy ctor. */
    Context &operator=(const Context &) = delete;
    /** @brief Deleted — no move either, the ring's address is baked into in-flight SQEs. */
    Context(Context &&) = delete;
    /** @brief Deleted — mirrors the move ctor. */
    Context &operator=(Context &&) = delete;

    /**
     * @brief Actually stands up the io_uring ring via `io_uring_queue_init_params`. Called from
     * the ctor — you shouldn't need to call this yourself unless you're doing something unusual.
     * @param entries submission queue depth to reserve.
     * @param flags io_uring setup flags.
     * @param wq_fd shared async worker-queue fd to attach to, or 0 for a fresh one.
     * @throws std::system_error via panic_on_err() if the underlying init call fails.
     */
    void init(int entries, std::uint32_t flags, std::uint32_t wq_fd) {
        // build the params struct the kernel wants — sq/cq_entries stay 0 so the kernel picks
        // its own sizing off of `entries`
        liburing::io_uring_params params = {
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

        // stand up the ring for real — any failure here is fatal, panic_on_err throws
        int ret = liburing::io_uring_queue_init_params(entries, &m_ring, &params);
        if (ret < 0) {
            panic_on_err("queue_init_params", ret, false);
        }

        verbose_print("Linux io_uring initialized");
    };

    /** @brief Tears down the io_uring ring via `io_uring_queue_exit`. Not called automatically —
     * see the dtor's note, this thing is invoked manually elsewhere in the teardown path. */
    void cleanup() noexcept { liburing::io_uring_queue_exit(&m_ring); };

    /**
     * @brief Grabs a fresh submission queue entry, force-flushing pending completions and
     * resubmitting if the SQ is full so it can retry. This is the gate every async op method
     * goes through before filling in its `io_uring_prep_*` call.
     * @return a ready-to-fill SQE pointer. Never null on return — panic() fires instead of
     * handing back nullptr, so callers don't need to check.
     * @throws std::system_error via panic() if the ring is out of memory (`ENOMEM`) even after
     * a flush-and-retry.
     */
    liburing::io_uring_sqe *get_sqe_safe() {
        // fast path, bet — SQ isn't full, hand back the SQE straight away
        auto *sqe = liburing::io_uring_get_sqe(&m_ring);
        if (__builtin_expect(static_cast<long>(sqe != nullptr), 1L) != 0) {
            return sqe;
        }

        // SQ is full: advance past whatever CQEs we already counted, submit to free up ring
        // space, and try again
        verbose_print("{}: SQ is full, flushing {} cqe(s)\n", __FILE__, m_cqe_count);

        liburing::io_uring_cq_advance(&m_ring, m_cqe_count);
        m_cqe_count = 0;
        liburing::io_uring_submit(&m_ring);

        // second attempt — if it's still null the ring is genuinely out of memory, straight cooked
        sqe = liburing::io_uring_get_sqe(&m_ring);
        if (__builtin_expect(static_cast<long>(sqe != nullptr), 1L) != 0) {
            return sqe;
        }
        panic("liburing::io_uring_get_sqe", liburing::enomem);
    };

    /**
     * @brief Stamps SQE flags, wraps the callback in a heap-allocated `pending_op`, and hangs
     * it off the SQE's user_data so process_completions() can find it later.
     * @warning `op.release()` deliberately leaks ownership into the SQE — that raw pointer only
     * gets reclaimed and `delete`d inside process_completions() when the matching CQE shows up.
     * If the op never completes (ring torn down mid-flight, etc.) that `pending_op` leaks. Real
     * footgun if you tear down the ring with ops still in flight.
     * @param sqe the submission queue entry to finish preparing — must already have its
     * `io_uring_prep_*` call filled in by the caller.
     * @param callback completion callback to run once the matching CQE lands.
     * @param iflags io_uring SQE flags (e.g. `IOSQE_IO_LINK`).
     */
    static void submit_async(liburing::io_uring_sqe *sqe, completion_callback callback, std::uint8_t iflags) {
        // stamp the requested SQE flags onto the already-prepped entry
        liburing::io_uring_sqe_set_flags(sqe, iflags);

        // wrap the callback in a heap PendingOp and hang it off the SQE's user_data — release()
        // hands ownership to the kernel, process_completions() reclaims it later
        auto op = std::make_unique<PendingOp>();
        op->set_callback(std::move(callback));
        liburing::io_uring_sqe_set_data(sqe, op.release());
    };

    /**
     * @brief Drains every ready CQE, reclaims its `PendingOp`, and fires the stashed callback
     * with the syscall result — the other half of submit_async()'s handoff.
     */
    void process_completions() {
        // walk every ready cqe, reclaim its PendingOp, and fire the stashed callback with the
        // syscall result — this is the other half of submit_async()'s handoff
        liburing::for_each_cqe(&m_ring, [&](auto cqe) {
            ++m_cqe_count;

            // a null op means this cqe wasn't tagged with user_data — no cap, nothing to reclaim/call
            auto *op = static_cast<PendingOp *>(liburing::io_uring_cqe_get_data(cqe));
            if (op) {
                int result = cqe->res;
                auto callback = std::move(op->get_callback());
                delete op;  // NOLINT(cppcoreguidelines-owning-memory) — would need gsl::owner<> annotation; no GSL dependency in this codebase

                if (callback) {
                    callback(result);
                }
            }
        });

        verbose_print("{}: Found {} cqe(s), looping...\n", __FILE__, m_cqe_count);

        // tell the kernel we're done with everything we just walked, then reset the counter
        liburing::io_uring_cq_advance(&m_ring, m_cqe_count);
        m_cqe_count = 0;
    };

    /**
     * @brief Overwrites the pending-CQE counter directly.
     * @param count the new count to set — mostly here for tests/edge cases, normal flow just lets
     * process_completions() manage this itself.
     */
    void set_cqe_count(unsigned count) { m_cqe_count = count; }

    /**
     * @brief Grabs the raw ring pointer for handing to `liburing::io_uring_prep_*` calls.
     * @return a mutable pointer to `m_ring`.
     */
    [[nodiscard]] liburing::io_uring *get_ring() noexcept { return &m_ring; }
    /**
     * @brief Const overload of get_ring().
     * @return a const pointer to `m_ring`.
     */
    [[nodiscard]] const liburing::io_uring *get_ring() const noexcept { return &m_ring; }


  private:
    liburing::io_uring m_ring;
    unsigned m_cqe_count = 0;
    // std::array<bool, 128> m_probe_ops;
};


/**
 * @brief io_uring specialization ctor — builds `m_context` as a live `Context{entries, flags,
 * wq_fd}` right away. Full param contract lives on the primary declaration in types.cppm.
 */
template <>
Leverager<Context>::Leverager(int entries, std::uint32_t flags, std::uint32_t wq_fd)
    : m_context{Context{entries, flags, wq_fd}} {};

/**
 * @brief io_uring specialization dtor — defaulted, so `m_context` is torn down exactly once via
 * ordinary member destruction (equivalent to the previous manual `m_context.~Context()` call,
 * since `Context::~Context()` is an empty no-op — see its own note). Don't copy this pattern onto
 * a `Context` with a non-trivial dtor without re-checking that equivalence.
 */
template <>
Leverager<Context>::~Leverager() noexcept = default;


/**
 * @brief io_uring specialization of run() — submits whatever's queued and blocks for at least
 * one completion, then drains the completion queue.
 */
template <>
void Leverager<Context>::run() {
    liburing::io_uring_submit_and_wait(m_context.get_ring(), 1);
    m_context.process_completions();
}


/**
 * @brief io_uring specialization of readv() — preps via `io_uring_prep_readv`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::readv(int file_descriptor, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                               completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_readv(sqe, file_descriptor, iovecs, nr_vecs, offset);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of readv2() — preps via `io_uring_prep_readv2`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::readv2(int file_descriptor, const iovec *iovecs, unsigned nr_vecs, off_t offset, int flags,
                                completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_readv2(sqe, file_descriptor, iovecs, nr_vecs, offset, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of writev() — preps via `io_uring_prep_writev`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::writev(int file_descriptor, const iovec *iovecs, unsigned nr_vecs, off_t offset,
                                completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_writev(sqe, file_descriptor, iovecs, nr_vecs, offset);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of writev2() — preps via `io_uring_prep_writev2`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::writev2(int file_descriptor, const iovec *iovecs, unsigned nr_vecs, off_t offset, int flags,
                                 completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_writev2(sqe, file_descriptor, iovecs, nr_vecs, offset, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of read() — preps via `io_uring_prep_read`. Full param contract
 * lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::read(int file_descriptor, void *buf, unsigned nbytes, off_t offset,
                              completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_read(sqe, file_descriptor, buf, nbytes, offset);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of write() — preps via `io_uring_prep_write`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::write(int file_descriptor, const void *buf, unsigned nbytes, off_t offset,
                               completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_write(sqe, file_descriptor, buf, nbytes, offset);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of read_fixed() — preps via `io_uring_prep_read_fixed`. Full
 * param contract (including the buffer-registration requirement) lives on the primary
 * declaration in types.cppm.
 */
template <>
void Leverager<Context>::read_fixed(int file_descriptor, void *buf, unsigned nbytes, off_t offset, int buf_index,
                                    completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_read_fixed(sqe, file_descriptor, buf, nbytes, offset, buf_index);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of write_fixed() — preps via `io_uring_prep_write_fixed`. Full
 * param contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::write_fixed(int file_descriptor, const void *buf, unsigned nbytes, off_t offset,
                                     int buf_index, completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_write_fixed(sqe, file_descriptor, buf, nbytes, offset, buf_index);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of fsync() — preps via `io_uring_prep_fsync`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::fsync(int file_descriptor, unsigned fsync_flags, completion_callback callback,
                               std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_fsync(sqe, file_descriptor, fsync_flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of sync_file_range() — preps via the generic `io_uring_prep_rw`
 * entry point using `liburing::OP_SYNC_FILE_RANGE`, then patches `sync_range_flags` onto the
 * SQE directly since there's no dedicated `io_uring_prep_sync_file_range` helper here. Full
 * param contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::sync_file_range(int file_descriptor, off64_t offset, off64_t nbytes,
                                         unsigned sync_range_flags, completion_callback callback,
                                         std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_rw(liburing::OP_SYNC_FILE_RANGE, sqe, file_descriptor, nullptr, nbytes, offset);
    sqe->sync_range_flags = sync_range_flags;  // NOLINT(cppcoreguidelines-pro-type-union-access) — sqe is a
                                                // kernel io_uring_sqe C struct with a union layout; no encapsulation
                                                // possible without wrapping the liburing ABI type
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of recvmsg() — preps via `io_uring_prep_recvmsg`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::recvmsg(int sockfd, msghdr *msg, std::uint32_t flags, completion_callback callback,
                                 std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_recvmsg(sqe, sockfd, msg, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of sendmsg() — preps via `io_uring_prep_sendmsg`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::sendmsg(int sockfd, const msghdr *msg, std::uint32_t flags, completion_callback callback,
                                 std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_sendmsg(sqe, sockfd, msg, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of recv() — preps via `io_uring_prep_recv`. Full param contract
 * lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::recv(int sockfd, void *buf, unsigned nbytes, std::uint32_t flags,
                              completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_recv(sqe, sockfd, buf, nbytes, static_cast<int>(flags));
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of send() — preps via `io_uring_prep_send`. Full param contract
 * lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::send(int sockfd, const void *buf, unsigned nbytes, std::uint32_t flags,
                              completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_send(sqe, sockfd, buf, nbytes, static_cast<int>(flags));
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of poll() — preps via `io_uring_prep_poll_add`, a real readiness
 * watch (unlike the win32 backend's stub). Full param contract lives on the primary declaration
 * in types.cppm.
 */
template <>
void Leverager<Context>::poll(int file_descriptor, short poll_mask, completion_callback callback,
                              std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_poll_add(sqe, file_descriptor, poll_mask);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of yield() — preps via `io_uring_prep_nop`. Full param contract
 * lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::yield(completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_nop(sqe);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of accept() — preps via `io_uring_prep_accept`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::accept(int file_descriptor, sockaddr *addr, socklen_t *addrlen, int flags,
                                completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_accept(sqe, file_descriptor, addr, addrlen, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of connect() — preps via `io_uring_prep_connect`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::connect(int file_descriptor, sockaddr *addr, socklen_t addrlen,
                                 completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_connect(sqe, file_descriptor, addr, addrlen);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of timeout() — preps via `io_uring_prep_timeout` with a fixed
 * count/flags of 0. Full param contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::timeout(__kernel_timespec *timeout_spec, completion_callback callback,
                                 std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_timeout(sqe, timeout_spec, 0, 0);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of openat() — preps via `io_uring_prep_openat`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::openat(int dfd, const char *path, int flags, mode_t mode,
                                completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_openat(sqe, dfd, path, flags, mode);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of close() — preps via `io_uring_prep_close`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::close(int file_descriptor, completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_close(sqe, file_descriptor);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of statx() — preps via `io_uring_prep_statx`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf,
                               completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_statx(sqe, dfd, path, flags, mask, statxbuf);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of splice() — preps via `io_uring_prep_splice`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::splice(int fd_in, off_t off_in, int fd_out, off_t off_out, std::size_t nbytes, unsigned flags,
                                completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_splice(sqe, fd_in, off_in, fd_out, off_out, nbytes, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of tee() — preps via `io_uring_prep_tee`. Full param contract
 * lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::tee(int fd_in, int fd_out, std::size_t nbytes, unsigned flags,
                             completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_tee(sqe, fd_in, fd_out, nbytes, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of shutdown() — preps via `io_uring_prep_shutdown`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::shutdown(int file_descriptor, int how, completion_callback callback,
                                  std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_shutdown(sqe, file_descriptor, how);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of renameat() — preps via `io_uring_prep_renameat`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath, unsigned flags,
                                  completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_renameat(sqe, olddfd, oldpath, newdfd, newpath, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of mkdirat() — preps via `io_uring_prep_mkdirat`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::mkdirat(int dirfd, const char *pathname, mode_t mode, completion_callback callback,
                                 std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_mkdirat(sqe, dirfd, pathname, mode);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of symlinkat() — preps via `io_uring_prep_symlinkat`. Full
 * param contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::symlinkat(const char *target, int newdirfd, const char *linkpath,
                                   completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_symlinkat(sqe, target, newdirfd, linkpath);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of linkat() — preps via `io_uring_prep_linkat`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags,
                                completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_linkat(sqe, olddirfd, oldpath, newdirfd, newpath, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of unlinkat() — preps via `io_uring_prep_unlinkat`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::unlinkat(int dfd, const char *path, unsigned flags, completion_callback callback,
                                  std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_unlinkat(sqe, dfd, path, static_cast<int>(flags));
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of msg_ring() — preps via `io_uring_prep_msg_ring`. Full param
 * contract lives on the primary declaration in types.cppm.
 */
template <>
void Leverager<Context>::msg_ring(int file_descriptor, unsigned len, std::uint64_t data, unsigned flags,
                                  completion_callback callback, std::uint8_t iflags) {
    auto *sqe = m_context.get_sqe_safe();
    liburing::io_uring_prep_msg_ring(sqe, file_descriptor, len, data, flags);
    Context::submit_async(sqe, std::move(callback), iflags);
}

/**
 * @brief io_uring specialization of poll() — peeks a single CQE without blocking, and only
 * drains completions if one's actually ready. Full contract lives on the primary declaration
 * in types.cppm.
 */
template <>
void Leverager<Context>::poll() {
    // non-blocking peek — only drain the completion queue if something's actually ready
    liburing::io_uring_cqe *cqe = nullptr;
    if (liburing::io_uring_peek_cqe(m_context.get_ring(), &cqe) == 0) {
        m_context.process_completions();
    }
}

/**
 * @brief io_uring specialization of stop() — just flips `m_running` false, no wake-up needed
 * since a blocked `io_uring_submit_and_wait` call isn't parked on this flag anyway (unlike the
 * win32 IOCP backend, which has to explicitly wake the completion port).
 */
template <>
void Leverager<Context>::stop() {
    m_running = false;
}


/**
 * @brief io_uring specialization of register_files() — wraps `io_uring_register_files`.
 * @throws std::system_error via panic_on_err() if the registration syscall fails.
 */
template <>
void Leverager<Context>::register_files(std::span<const int> fds) {
    int ret = liburing::io_uring_register_files(m_context.get_ring(), fds.data(), static_cast<unsigned>(fds.size()));
    panic_on_err("liburing::io_uring_register_files", ret, false);
}

/**
 * @brief io_uring specialization of register_files_update() — wraps
 * `io_uring_register_files_update`.
 * @throws std::system_error via panic_on_err() if the update syscall fails.
 */
template <>
void Leverager<Context>::register_files_update(unsigned off, std::span<int> files) {
    int ret = liburing::io_uring_register_files_update(m_context.get_ring(), off, files.data(),
                                                       static_cast<unsigned>(files.size()));
    panic_on_err("liburing::io_uring_register_files_update", ret, false);
}

/**
 * @brief io_uring specialization of unregister_files() — wraps `io_uring_unregister_files`.
 * @return the raw syscall result.
 */
template <>
int Leverager<Context>::unregister_files() noexcept {
    return liburing::io_uring_unregister_files(m_context.get_ring());
}

/**
 * @brief io_uring specialization of register_buffers() — wraps `io_uring_register_buffers`.
 * @throws std::system_error via panic_on_err() if the registration syscall fails.
 */
template <>
void Leverager<Context>::register_buffers(std::span<const iovec> iovecs) {
    int ret =
        liburing::io_uring_register_buffers(m_context.get_ring(), iovecs.data(), static_cast<unsigned>(iovecs.size()));
    panic_on_err("liburing::io_uring_register_buffers", ret, false);
}

/**
 * @brief io_uring specialization of unregister_buffers() — wraps `io_uring_unregister_buffers`.
 * @return the raw syscall result.
 */
template <>
int Leverager<Context>::unregister_buffers() noexcept {
    return liburing::io_uring_unregister_buffers(m_context.get_ring());
}


/**
 * @brief io_uring specialization of register_file() — forwards to register_files() with a
 * one-element span over `fd`.
 */
template <>
void Leverager<Context>::register_file(int file_descriptor) {
    register_files({&file_descriptor, 1});
}

/**
 * @brief io_uring specialization of unregister_file() — forwards to register_files_update() with
 * a single `-1` sentinel to clear the slot.
 */
// FIXME(clang-tidy): bugprone-exception-escape — register_files_update() calls panic_on_err(),
// which throws std::system_error on syscall failure; this function is declared noexcept in the
// primary template (types.cppm:693), so an explicit specialization here can't drop noexcept
// without also changing that shared declaration, which every backend (posix/uring/win32)
// specializes against. Needs a cross-backend audit, not a local guess.
template <>
void Leverager<Context>::unregister_file(unsigned int file_descriptor) noexcept {
    int sentinel = -1;
    register_files_update(file_descriptor, {&sentinel, 1});
}

} // namespace io::base::leverage

#ifdef CONGELADO_TEST
namespace io::base::leverage::tests {
using namespace boost::ut;

// Context's instance methods (init/get_sqe_safe/process_completions/cleanup/...) and every
// Leverager<Context> specialization in this file need a live io_uring ring to do anything
// meaningful — not unit-testable in isolation, skipped here. PendingOp is a plain value type with
// no syscalls involved, so that's covered for real. Context::submit_async() is the one exception:
// it's `static` and only ever touches the `io_uring_sqe*` argument it's handed (stamps flags,
// calls `io_uring_sqe_set_data()`) — both real liburing calls, but pure struct-field writes with
// no ring/kernel interaction (verified against /usr/include/liburing.h: `io_uring_sqe_set_data()`
// is just `sqe->user_data = (unsigned long)data`). A stack-allocated `io_uring_sqe{}` is enough to
// exercise it for real, no live ring required.

suite<"PendingOp"> pending_op_suite = [] {
    "fields round-trip through their setters/getters"_test = [] {
        PendingOp op;
        int buffer = 0;
        op.set_buffer(&buffer);
        op.set_buffer_size(128);
        op.set_offset(64);
        op.set_op_type(3);

        expect(op.get_buffer() == &buffer);
        expect(op.get_buffer_size() == 128);
        expect(op.get_offset() == 64);
        expect(op.get_op_type() == 3);
    };

    "starts with a null buffer, zeroed size/offset/op_type"_test = [] {
        PendingOp op;

        expect(op.get_buffer() == nullptr);
        expect(op.get_buffer_size() == 0);
        expect(op.get_offset() == 0);
        expect(op.get_op_type() == 0);
    };

    "stashed callback fires with the value it's invoked with"_test = [] {
        PendingOp op;
        int seen = 0;
        op.set_callback([&seen](int result) { seen = result; });

        op.get_callback()(42);

        expect(seen == 42);
    };
};

suite<"Context::submit_async"> submit_async_suite = [] {
    "stamps the requested SQE flags onto the entry"_test = [] {
        liburing::io_uring_sqe sqe{};
        Context::submit_async(&sqe, [](int) {}, 0x04);

        expect(sqe.flags == 0x04);

        // Reclaim the PendingOp submit_async() heap-allocated so this test doesn't leak it —
        // nothing else will, since no ring ever saw this SQE.
        delete reinterpret_cast<PendingOp *>(static_cast<std::uintptr_t>(sqe.user_data));  // NOLINT(cppcoreguidelines-owning-memory) — reclaiming what submit_async() deliberately leaked into user_data, see below
    };

    "hangs a heap PendingOp carrying the callback off user_data, unconditionally"_test = [] {
        liburing::io_uring_sqe sqe{};
        int seen = -1;
        Context::submit_async(&sqe, [&seen](int result) { seen = result; }, 0);

        // user_data now holds the raw pointer submit_async() released ownership of — exactly the
        // handoff process_completions() expects to reclaim later via io_uring_cqe_get_data().
        expect(sqe.user_data != 0) << fatal;
        auto *op = reinterpret_cast<PendingOp *>(static_cast<std::uintptr_t>(sqe.user_data));

        op->get_callback()(7);
        expect(seen == 7);

        delete op;  // NOLINT(cppcoreguidelines-owning-memory) — see above
    };

    // Regression/design-gap marker, NOT a fix (matches submit_async()'s own @warning): this test
    // proves the missing "reject submissions once the ring is tearing down" guard structurally,
    // through the function's own signature, rather than by actually tearing down a live ring
    // mid-flight (unsafe — real ops could still be in flight against kernel memory). submit_async()
    // is `static` and takes no `Context&`/`this` at all — its only inputs are the caller-prepped
    // `io_uring_sqe*`, the callback, and the iflags byte. A function that cannot observe `this`
    // cannot consult any instance state (a "ring is shutting down" flag, an op counter, anything)
    // to decide whether to refuse the submission — there is structurally nowhere for such a guard
    // to live short of adding a new parameter, which is exactly what's missing. Every call
    // unconditionally heap-allocates and hands off ownership, confirmed by the two tests above.
    "submit_async() takes no Context/ring reference, so no ring-liveness guard is structurally "
    "possible"_test = [] {
        // A plain (non-member) function pointer type — no `Context::*` receiver at all — proves
        // this static method has zero access to any per-instance "ring is tearing down" state.
        using SubmitAsyncPtr = decltype(&Context::submit_async);
        expect(std::is_same_v<SubmitAsyncPtr,
                              void (*)(liburing::io_uring_sqe *, completion_callback, std::uint8_t)>);
    };
};

} // namespace io::base::leverage::tests
#endif
