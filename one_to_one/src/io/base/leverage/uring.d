module io.base.leverage.uring;
@nogc nothrow:

// PORT-NOTE: This module re-exports liburing symbols needed by posix.d.
// C++ used `export module io_base_leverage:uring;` which pulled in <liburing.h>.
// In D, we use extern(C) declarations for the liburing API.
// The ImportC shim at one_to_one/c/uring.c exposes the core types but not all
// prep helpers; those are declared here as extern(C) until the shim is extended.

// PORT-NOTE: io_uring struct fields — see c/uring.c shim.
// These are forward-declared as extern(C) structs; full layout is in the C shim.
extern(C) struct io_uring;
extern(C) struct io_uring_sqe;
extern(C) struct io_uring_cqe;
extern(C) struct io_uring_params;

// PORT-NOTE: io_uring constant values from liburing headers
enum int  enomem            = 12;            // ENOMEM
enum int  OP_SYNC_FILE_RANGE = 9;            // IORING_OP_SYNC_FILE_RANGE
enum int  OP_READ           = 22;            // IORING_OP_READ
enum int  OP_WRITE          = 23;            // IORING_OP_WRITE

// Core liburing functions — extern(C) bindings
extern(C) int  io_uring_queue_init(uint entries, io_uring* ring, uint flags) @nogc nothrow;
extern(C) int  io_uring_queue_init_params(uint entries, io_uring* ring, io_uring_params* p) @nogc nothrow;
extern(C) void io_uring_queue_exit(io_uring* ring) @nogc nothrow;

extern(C) io_uring_sqe* io_uring_get_sqe(io_uring* ring) @nogc nothrow;
extern(C) int           io_uring_submit(io_uring* ring) @nogc nothrow;
extern(C) int           io_uring_submit_and_wait(io_uring* ring, uint wait_nr) @nogc nothrow;

extern(C) void io_uring_cq_advance(io_uring* ring, uint nr) @nogc nothrow;
extern(C) int  io_uring_peek_cqe(io_uring* ring, io_uring_cqe** cqe_ptr) @nogc nothrow;
extern(C) int  io_uring_wait_cqe(io_uring* ring, io_uring_cqe** cqe_ptr) @nogc nothrow;
extern(C) void io_uring_cqe_seen(io_uring* ring, io_uring_cqe* cqe) @nogc nothrow;

extern(C) void* io_uring_cqe_get_data(const(io_uring_cqe)* cqe) @nogc nothrow;
extern(C) void  io_uring_sqe_set_data(io_uring_sqe* sqe, void* data) @nogc nothrow;
extern(C) void  io_uring_sqe_set_flags(io_uring_sqe* sqe, uint flags) @nogc nothrow;

// Prep functions
import io.base.leverage.types : iovec, kernel_timespec_t, statx_t, msghdr, sockaddr, socklen_t,
                                  mode_t, off_t, loff_t;

extern(C) void io_uring_prep_readv(io_uring_sqe* sqe, int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset) @nogc nothrow;
extern(C) void io_uring_prep_readv2(io_uring_sqe* sqe, int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags) @nogc nothrow;
extern(C) void io_uring_prep_writev(io_uring_sqe* sqe, int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset) @nogc nothrow;
extern(C) void io_uring_prep_writev2(io_uring_sqe* sqe, int fd, const(iovec)* iovecs, uint nr_vecs, off_t offset, int flags) @nogc nothrow;
extern(C) void io_uring_prep_read(io_uring_sqe* sqe, int fd, void* buf, uint nbytes, off_t offset) @nogc nothrow;
extern(C) void io_uring_prep_write(io_uring_sqe* sqe, int fd, const(void)* buf, uint nbytes, off_t offset) @nogc nothrow;
extern(C) void io_uring_prep_read_fixed(io_uring_sqe* sqe, int fd, void* buf, uint nbytes, off_t offset, int buf_index) @nogc nothrow;
extern(C) void io_uring_prep_write_fixed(io_uring_sqe* sqe, int fd, const(void)* buf, uint nbytes, off_t offset, int buf_index) @nogc nothrow;
extern(C) void io_uring_prep_fsync(io_uring_sqe* sqe, int fd, uint fsync_flags) @nogc nothrow;
extern(C) void io_uring_prep_rw(int op, io_uring_sqe* sqe, int fd, const(void)* addr, uint len, ulong offset) @nogc nothrow;
extern(C) void io_uring_prep_recvmsg(io_uring_sqe* sqe, int fd, msghdr* msg, uint flags) @nogc nothrow;
extern(C) void io_uring_prep_sendmsg(io_uring_sqe* sqe, int fd, const(msghdr)* msg, uint flags) @nogc nothrow;
extern(C) void io_uring_prep_recv(io_uring_sqe* sqe, int sockfd, void* buf, size_t len, int flags) @nogc nothrow;
extern(C) void io_uring_prep_send(io_uring_sqe* sqe, int sockfd, const(void)* buf, size_t len, int flags) @nogc nothrow;
extern(C) void io_uring_prep_poll_add(io_uring_sqe* sqe, int fd, uint poll_mask) @nogc nothrow;
extern(C) void io_uring_prep_nop(io_uring_sqe* sqe) @nogc nothrow;
extern(C) void io_uring_prep_accept(io_uring_sqe* sqe, int fd, sockaddr* addr, socklen_t* addrlen, int flags) @nogc nothrow;
extern(C) void io_uring_prep_connect(io_uring_sqe* sqe, int fd, const(sockaddr)* addr, socklen_t addrlen) @nogc nothrow;
extern(C) void io_uring_prep_timeout(io_uring_sqe* sqe, kernel_timespec_t* ts, uint count, uint flags) @nogc nothrow;
extern(C) void io_uring_prep_openat(io_uring_sqe* sqe, int dfd, const(char)* path, int flags, mode_t mode) @nogc nothrow;
extern(C) void io_uring_prep_close(io_uring_sqe* sqe, int fd) @nogc nothrow;
extern(C) void io_uring_prep_statx(io_uring_sqe* sqe, int dfd, const(char)* path, int flags, uint mask, statx_t* statxbuf) @nogc nothrow;
extern(C) void io_uring_prep_splice(io_uring_sqe* sqe, int fd_in, long off_in, int fd_out, long off_out, uint nbytes, uint splice_flags) @nogc nothrow;
extern(C) void io_uring_prep_tee(io_uring_sqe* sqe, int fd_in, int fd_out, uint nbytes, uint splice_flags) @nogc nothrow;
extern(C) void io_uring_prep_shutdown(io_uring_sqe* sqe, int fd, int how) @nogc nothrow;
extern(C) void io_uring_prep_renameat(io_uring_sqe* sqe, int olddfd, const(char)* oldpath, int newdfd, const(char)* newpath, uint flags) @nogc nothrow;
extern(C) void io_uring_prep_mkdirat(io_uring_sqe* sqe, int dirfd, const(char)* pathname, mode_t mode) @nogc nothrow;
extern(C) void io_uring_prep_symlinkat(io_uring_sqe* sqe, const(char)* target, int newdirfd, const(char)* linkpath) @nogc nothrow;
extern(C) void io_uring_prep_linkat(io_uring_sqe* sqe, int olddirfd, const(char)* oldpath, int newdirfd, const(char)* newpath, int flags) @nogc nothrow;
extern(C) void io_uring_prep_unlinkat(io_uring_sqe* sqe, int dfd, const(char)* path, int flags) @nogc nothrow;
extern(C) void io_uring_prep_msg_ring(io_uring_sqe* sqe, int fd, uint len, ulong data, uint flags) @nogc nothrow;

// Registration
extern(C) int io_uring_register_files(io_uring* ring, const(int)* files, uint nr_files) @nogc nothrow;
extern(C) int io_uring_register_files_update(io_uring* ring, uint off, int* files, uint nr_files) @nogc nothrow;
extern(C) int io_uring_unregister_files(io_uring* ring) @nogc nothrow;
extern(C) int io_uring_register_buffers(io_uring* ring, const(iovec)* iovecs, uint nr_iovecs) @nogc nothrow;
extern(C) int io_uring_unregister_buffers(io_uring* ring) @nogc nothrow;

// PORT-NOTE: io_uring_for_each_cqe is a macro in C; replaced by a D template function.
// The callback receives a non-const cqe pointer matching the C++ lambda form.
// cqe.res and cqe.user_data are accessed via the opaque pointer helper below.

// PORT-NOTE: io_uring_cqe fields not in shim — access via helper functions
extern(C) int   io_uring_cqe_get_res(const(io_uring_cqe)* cqe) @nogc nothrow;  // returns cqe->res
extern(C) ulong io_uring_cqe_get_user_data(const(io_uring_cqe)* cqe) @nogc nothrow;  // returns cqe->user_data

void for_each_cqe(Func)(io_uring* ring, scope Func func) @nogc nothrow {
    io_uring_cqe* cqe;
    // PORT-NOTE: io_uring_for_each_cqe macro — simulate with peek loop
    while (io_uring_peek_cqe(ring, &cqe) == 0 && cqe !is null) {
        func(cqe);
        io_uring_cqe_seen(ring, cqe);
    }
}
