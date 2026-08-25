module;

#include <liburing.h>

export module io_base_leverage:uring;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace liburing {

// FIXME(clang-tidy): readability-identifier-naming — 'enomem' can't be uppercased to 'ENOMEM',
// that would collide with the <errno.h> macro of the same name.
inline constexpr int enomem = ENOMEM;  // NOLINT(readability-identifier-naming) — ENOMEM is an errno.h macro, can't rename to match
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
void for_each_cqe(io_uring *ring, Func &&func) {  // NOLINT(cppcoreguidelines-missing-std-forward) — func is called once per cqe in a loop; forwarding it would use-after-move on the second call
    io_uring_cqe *cqe = nullptr;
    unsigned head = 0;

    // walk every cqe that's currently ready on the ring and hand each one off to func — head
    // is just the macro's internal iterator slot, nothing callers need to touch. func is
    // invoked once per cqe, so it must be called as an lvalue each time rather than forwarded
    // (forwarding a forwarding-reference repeatedly in a loop is a genuine use-after-move risk).
    io_uring_for_each_cqe(ring, head, cqe) { func(cqe); }
}

} // namespace liburing

#ifdef CONGELADO_TEST
namespace liburing::leverage_uring_tests {
using namespace boost::ut;

suite<"liburing constants"> constants_suite = [] {
    "enomem mirrors the ENOMEM errno macro"_test = [] { expect(liburing::enomem == ENOMEM); };

    "opcode constants mirror their IORING_OP_* macros"_test = [] {
        expect(liburing::OP_READ == IORING_OP_READ);
        expect(liburing::OP_WRITE == IORING_OP_WRITE);
        expect(liburing::OP_SYNC_FILE_RANGE == IORING_OP_SYNC_FILE_RANGE);
    };
};

// for_each_cqe() drives the io_uring_for_each_cqe macro purely off ring->cq's head/tail/mask/
// cqes pointers — none of that needs a live kernel ring, so a hand-built io_uring struct with
// stack-backed cq storage exercises the real iteration logic without a single syscall (a live
// ring, per Leverager's own test-skip note in include/io/base/leverage/types.cppm, is off-limits
// in this shared test binary).
suite<"liburing for_each_cqe"> for_each_cqe_suite = [] {
    "walks every ready cqe from head to tail"_test = [] {
        std::array<io_uring_cqe, 4> cqes{};
        cqes[0].res = 10;
        cqes[1].res = 20;
        cqes[2].res = 30;

        unsigned head = 0;
        unsigned tail = 3; // three cqes ready
        io_uring ring{};
        ring.cq.khead = &head;
        ring.cq.ktail = &tail;
        ring.cq.ring_mask = 3; // 4-entry ring, mask = size - 1
        ring.cq.cqes = cqes.data();

        std::vector<int> seen;
        liburing::for_each_cqe(&ring, [&](io_uring_cqe *cqe) { seen.push_back(cqe->res); });

        expect(seen.size() == 3) << fatal;
        expect(seen[0] == 10);
        expect(seen[1] == 20);
        expect(seen[2] == 30);
    };

    "an empty ring (head == tail) never invokes func"_test = [] {
        std::array<io_uring_cqe, 4> cqes{};
        unsigned head = 5;
        unsigned tail = 5; // nothing ready
        io_uring ring{};
        ring.cq.khead = &head;
        ring.cq.ktail = &tail;
        ring.cq.ring_mask = 3;
        ring.cq.cqes = cqes.data();

        int calls = 0;
        liburing::for_each_cqe(&ring, [&](io_uring_cqe *) { ++calls; });

        expect(calls == 0);
    };

    "head wraps around the ring mask once it exceeds the buffer size"_test = [] {
        std::array<io_uring_cqe, 4> cqes{};
        cqes[2].res = 77; // slot (6 & 3) == 2

        unsigned head = 6;
        unsigned tail = 7;
        io_uring ring{};
        ring.cq.khead = &head;
        ring.cq.ktail = &tail;
        ring.cq.ring_mask = 3;
        ring.cq.cqes = cqes.data();

        std::vector<int> seen;
        liburing::for_each_cqe(&ring, [&](io_uring_cqe *cqe) { seen.push_back(cqe->res); });

        expect(seen.size() == 1) << fatal;
        expect(seen[0] == 77);
    };
};

} // namespace liburing::leverage_uring_tests
#endif
