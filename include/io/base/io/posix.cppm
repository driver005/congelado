module;

#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>

export module io_io:posix;

import std;
import :types;
import :buffer;
import :consts;

export namespace io::base::io {

struct PosixState {
    io_uring ring{};
    RingBuffer *buffer{};
    iovec iov{};
    msghdr msg{};
    sockaddr_storage peer{};
    socklen_t peer_len{sizeof(sockaddr_storage)};

    // Non-constructing default — IO<PosixState> calls init() explicitly.
    PosixState() = default;

    void init(RingBuffer &buf, unsigned entries) {
        buffer = &buf;
        io_uring_params params{};
        params.flags = IORING_SETUP_COOP_TASKRUN | IORING_SETUP_SINGLE_ISSUER;
        if (int rc = io_uring_queue_init_params(entries, &ring, &params); rc < 0) {
            params.flags = 0;
            if (int rc2 = io_uring_queue_init_params(entries, &ring, &params); rc2 < 0)
                throw std::system_error(-rc2, std::system_category(), "io_uring_queue_init");
        }
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
    }

    void destroy() noexcept { io_uring_queue_exit(&ring); }

    [[nodiscard]] io_uring_sqe *get_sqe() {
        io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe) [[unlikely]]
            throw std::runtime_error("IO: SQ ring full — call submit() first");
        return sqe;
    }
};


using PlatformIO = IO<PosixState>;

} // namespace io::base::io

namespace io::base::io {

template <>
IO<PosixState>::IO(RingBuffer &buffer, unsigned entries) {
    state_.init(buffer, entries);
}

template <>
IO<PosixState>::~IO() {
    state_.destroy();
}

template <>
void IO<PosixState>::submit_read(int fd, std::size_t count) {
    auto span = state_.buffer->get_writable_span();
    if (span.empty())
        throw BufferOverflowException{};
    auto *sqe = state_.get_sqe();
    io_uring_prep_read(sqe, fd, span.data(), static_cast<unsigned>(std::min(count, span.size())), 0);
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::READ, fd, 0));
}

template <>
void IO<PosixState>::submit_write(int fd, std::size_t count) {
    auto span = state_.buffer->get_readable_span();
    if (span.empty())
        throw std::logic_error("submit_write: empty buffer");
    auto *sqe = state_.get_sqe();
    io_uring_prep_write(sqe, fd, span.data(), static_cast<unsigned>(std::min(count, span.size())), 0);
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::WRITE, fd, 0));
}

template <>
void IO<PosixState>::submit_recv(int fd, std::size_t max_datagram) {
    auto span = state_.buffer->get_writable_span();
    if (span.empty())
        throw BufferOverflowException{};
    state_.iov.iov_base = span.data();
    state_.iov.iov_len = std::min(max_datagram, span.size());
    state_.msg.msg_name = &state_.peer;
    state_.msg.msg_namelen = sizeof(state_.peer);
    auto *sqe = state_.get_sqe();
    io_uring_prep_recvmsg(sqe, fd, &state_.msg, 0);
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::RECV, fd, 0));
}

template <>
void IO<PosixState>::submit_send(int fd, const sockaddr_storage &peer, socklen_t peer_len, std::size_t count) {
    auto span = state_.buffer->get_readable_span();
    if (span.empty())
        throw std::logic_error("submit_send: empty buffer");
    state_.iov.iov_base = const_cast<void *>(static_cast<const void *>(span.data()));
    state_.iov.iov_len = std::min(count, span.size());
    state_.msg.msg_name = const_cast<sockaddr_storage *>(&peer);
    state_.msg.msg_namelen = peer_len;
    auto *sqe = state_.get_sqe();
    io_uring_prep_sendmsg(sqe, fd, &state_.msg, 0);
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::SEND, fd, 0));
}

template <>
void IO<PosixState>::submit_poll(int fd, short events) {
    auto *sqe = state_.get_sqe();
    io_uring_prep_poll_add(sqe, fd, static_cast<unsigned>(events));
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::POLL, fd, 0));
}

template <>
void IO<PosixState>::submit_accept(int fd) {
    auto *sqe = state_.get_sqe();
    io_uring_prep_multishot_accept(sqe, fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::ACCEPT, fd, 0));
}

template <>
void IO<PosixState>::submit_connect(int fd, const sockaddr_storage &addr, socklen_t alen) {
    auto *sqe = state_.get_sqe();
    io_uring_prep_connect(sqe, fd, reinterpret_cast<const sockaddr *>(&addr), alen);
    io_uring_sqe_set_data64(sqe, encode_tag(OpCode::CONNECT, fd, 0));
}

template <>
int IO<PosixState>::submit() {
    const int rc = io_uring_submit(&state_.ring);
    if (rc < 0)
        throw std::system_error(-rc, std::system_category(), "io_uring_submit");
    return rc;
}

template <>
std::vector<CompletionEvent> IO<PosixState>::wait_completions(unsigned min) {
    io_uring_cqe *cqe{};
    if (int rc = io_uring_wait_cqe_nr(&state_.ring, &cqe, min); rc < 0)
        throw std::system_error(-rc, std::system_category(), "io_uring_wait_cqe_nr");

    std::vector<CompletionEvent> out;
    unsigned head{};

    io_uring_for_each_cqe(&state_.ring, head, cqe) {
        const auto tag = io_uring_cqe_get_data64(cqe);
        const auto result = cqe->res;
        const auto flags = cqe->flags;

        if (result > 0) {
            const auto bytes = static_cast<std::size_t>(result);
            switch (tag_kind(tag)) {
            case OpCode::READ:
            case OpCode::RECV:
                state_.buffer->commit_write(bytes);
                break;
            case OpCode::WRITE:
            case OpCode::SEND:
                state_.buffer->advance_read(bytes);
                break;
            default:
                break;
            }
            if (tag_kind(tag) == OpCode::RECV)
                state_.peer_len = state_.msg.msg_namelen;
        }
        out.push_back({tag, result, flags});
    }

    io_uring_cq_advance(&state_.ring, static_cast<unsigned>(out.size()));
    return out;
}

template <>
const sockaddr_storage &IO<PosixState>::last_peer() const noexcept {
    return state_.peer;
}

template <>
socklen_t IO<PosixState>::last_peer_len() const noexcept {
    return state_.peer_len;
}

template <>
std::uintptr_t IO<PosixState>::native_handle() const noexcept {
    return reinterpret_cast<std::uintptr_t>(&state_.ring);
}

} // namespace io::base::io
