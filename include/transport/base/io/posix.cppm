module;

#include <liburing.h>
#include <stdlib.h>

export module io:posix;

import std;
import :consts;
import :types;

export namespace transport::base::io {

class UringCore {
  public:
    explicit UringCore(unsigned entries) {
        io_uring_params params{};
        params.flags = IORING_SETUP_COOP_TASKRUN | IORING_SETUP_SINGLE_ISSUER;

        if (const int rc = io_uring_queue_init_params(entries, &m_ring, &params); rc < 0)
            throw std::system_error(-rc, std::system_category(), "io_uring_queue_init_params");
    }

    ~UringCore() { io_uring_queue_exit(&m_ring); }

    UringCore(const UringCore &) = delete;
    UringCore &operator=(const UringCore &) = delete;
    UringCore(UringCore &&) = delete;
    UringCore &operator=(UringCore &&) = delete;

    [[nodiscard]] io_uring_sqe *get_sqe() {
        io_uring_sqe *sqe = io_uring_get_sqe(&m_ring);
        if (!sqe)
            throw std::runtime_error("io_uring SQ full");
        return sqe;
    }

    void submit() {
        if (const int rc = io_uring_submit(&m_ring); rc < 0)
            throw std::system_error(-rc, std::system_category(), "io_uring_submit");
    }

    [[nodiscard]] io_uring &ring() noexcept { return m_ring; }

  private:
    io_uring m_ring{};
};

class UringTcpInternal {
  public:
    explicit UringTcpInternal(unsigned entries) : m_core(entries) {}

    void prep_read(int fd, std::span<std::uint8_t> span) {
        io_uring_sqe *sqe = m_core.get_sqe();
        io_uring_prep_read(sqe, fd, span.data(), static_cast<unsigned>(span.size()), 0);
        io_uring_sqe_set_data64(sqe, encode_tag(OpCode::READ, fd, static_cast<std::uint32_t>(span.size())));
        m_core.submit();
    }

    void prep_write(int fd, std::span<const std::uint8_t> span) {
        io_uring_sqe *sqe = m_core.get_sqe();
        io_uring_prep_write(sqe, fd, span.data(), static_cast<unsigned>(span.size()), 0);
        io_uring_sqe_set_data64(sqe, encode_tag(OpCode::WRITE, fd, static_cast<std::uint32_t>(span.size())));
        m_core.submit();
    }

    void prep_poll(int fd, short events) {
        io_uring_sqe *sqe = m_core.get_sqe();
        io_uring_prep_poll_add(sqe, fd, static_cast<unsigned>(events));
        io_uring_sqe_set_data64(sqe, encode_tag(OpCode::POLL, fd, static_cast<std::uint32_t>(events)));
        m_core.submit();
    }

    [[nodiscard]] io_uring &ring() noexcept { return m_core.ring(); }

  private:
    UringCore m_core;
};

class UringUdpInternal {
  public:
    explicit UringUdpInternal(unsigned entries) : m_core(entries) {}

    void prep_recv(int fd, std::span<std::uint8_t> span) {
        m_recv_iov.iov_base = span.data();
        m_recv_iov.iov_len = span.size();

        m_recv_hdr = {};
        m_recv_hdr.msg_iov = &m_recv_iov;
        m_recv_hdr.msg_iovlen = 1;
        m_recv_hdr.msg_name = &m_peer_addr;
        m_recv_hdr.msg_namelen = static_cast<socklen_t>(sizeof m_peer_addr);

        io_uring_sqe *sqe = m_core.get_sqe();
        io_uring_prep_recvmsg(sqe, fd, &m_recv_hdr, 0);
        io_uring_sqe_set_data64(sqe, encode_tag(OpCode::RECV, fd, static_cast<std::uint32_t>(span.size())));
        m_core.submit();
    }

    void prep_send(int fd, const sockaddr_storage &peer, socklen_t peer_len, std::span<const std::uint8_t> span) {
        m_send_iov.iov_base = const_cast<std::uint8_t *>(span.data());
        m_send_iov.iov_len = span.size();

        m_send_hdr = {};
        m_send_hdr.msg_iov = &m_send_iov;
        m_send_hdr.msg_iovlen = 1;
        m_send_hdr.msg_name = const_cast<sockaddr_storage *>(&peer);
        m_send_hdr.msg_namelen = peer_len;

        io_uring_sqe *sqe = m_core.get_sqe();
        io_uring_prep_sendmsg(sqe, fd, &m_send_hdr, 0);
        io_uring_sqe_set_data64(sqe, encode_tag(OpCode::SEND, fd, static_cast<std::uint32_t>(span.size())));
        m_core.submit();
    }

    void prep_poll(int fd, short events) {
        io_uring_sqe *sqe = m_core.get_sqe();
        io_uring_prep_poll_add(sqe, fd, static_cast<unsigned>(events));
        io_uring_sqe_set_data64(sqe, encode_tag(OpCode::POLL, fd, static_cast<std::uint32_t>(events)));
        m_core.submit();
    }

    [[nodiscard]] const sockaddr_storage &last_peer() const noexcept { return m_peer_addr; }
    [[nodiscard]] socklen_t last_peer_len() const noexcept { return m_recv_hdr.msg_namelen; }
    [[nodiscard]] io_uring &ring() noexcept { return m_core.ring(); }

  private:
    UringCore m_core;

    msghdr m_recv_hdr{};
    iovec m_recv_iov{};
    msghdr m_send_hdr{};
    iovec m_send_iov{};
    sockaddr_storage m_peer_addr{};
};


} // namespace transport::base::io
