module;

#include <liburing.h>

export module io_base_leverage:uring;

export namespace liburing {

inline constexpr int enomem = ENOMEM;
inline constexpr int OP_SYNC_FILE_RANGE = IORING_OP_SYNC_FILE_RANGE;
inline constexpr int OP_READ = IORING_OP_READ;
inline constexpr int OP_WRITE = IORING_OP_WRITE;

using ::io_uring;
using ::io_uring_cq_advance;
using ::io_uring_cqe;
using ::io_uring_cqe_get_data;
using ::io_uring_cqe_seen;
using ::io_uring_get_sqe;
using ::io_uring_params;
using ::io_uring_peek_cqe;
using ::io_uring_prep_accept;
using ::io_uring_prep_close;
using ::io_uring_prep_connect;
using ::io_uring_prep_fsync;
using ::io_uring_prep_linkat;
using ::io_uring_prep_mkdirat;
using ::io_uring_prep_msg_ring;
using ::io_uring_prep_nop;
using ::io_uring_prep_openat;
using ::io_uring_prep_poll_add;
using ::io_uring_prep_read;
using ::io_uring_prep_read_fixed;
using ::io_uring_prep_readv;
using ::io_uring_prep_readv2;
using ::io_uring_prep_recv;
using ::io_uring_prep_recvmsg;
using ::io_uring_prep_renameat;
using ::io_uring_prep_rw;
using ::io_uring_prep_send;
using ::io_uring_prep_sendmsg;
using ::io_uring_prep_shutdown;
using ::io_uring_prep_splice;
using ::io_uring_prep_statx;
using ::io_uring_prep_symlinkat;
using ::io_uring_prep_tee;
using ::io_uring_prep_timeout;
using ::io_uring_prep_unlinkat;
using ::io_uring_prep_write;
using ::io_uring_prep_write_fixed;
using ::io_uring_prep_writev;
using ::io_uring_prep_writev2;
using ::io_uring_queue_exit;
using ::io_uring_queue_init;
using ::io_uring_queue_init_params;
using ::io_uring_register_buffers;
using ::io_uring_register_files;
using ::io_uring_register_files_update;
using ::io_uring_sqe;
using ::io_uring_sqe_set_data;
using ::io_uring_sqe_set_flags;
using ::io_uring_submit;
using ::io_uring_submit_and_wait;
using ::io_uring_unregister_buffers;
using ::io_uring_unregister_files;
using ::io_uring_wait_cqe;

template <typename Func>
void for_each_cqe(io_uring *ring, Func &&func) {
    io_uring_cqe *cqe;
    unsigned head;

    io_uring_for_each_cqe(ring, head, cqe) { func(cqe); }
}

} // namespace liburing
