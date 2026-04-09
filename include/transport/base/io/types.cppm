module;

#include <liburing.h>

export module io:types;

import std;
import :buffer;
import :consts;


export namespace transport::base::io {

enum class OpCode : std::uint8_t {
    READ = 0,
    WRITE = 1,
    POLL = 2,
    RECV = 3, // UDP RECVMSG
    SEND = 4  // UDP SENDMSG
};

[[nodiscard]] constexpr std::uint64_t encode_tag(OpCode kind, int fd, std::uint32_t count) noexcept {
    return (static_cast<std::uint64_t>(kind) << 56) |
           (static_cast<std::uint64_t>(static_cast<std::uint32_t>(fd) & 0x00FF'FFFFu) << 32) |
           static_cast<std::uint64_t>(count);
}

[[nodiscard]] constexpr OpCode tag_kind(std::uint64_t t) noexcept { return static_cast<OpCode>(t >> 56); }
[[nodiscard]] constexpr int tag_fd(std::uint64_t t) noexcept { return static_cast<int>((t >> 32) & 0x00FF'FFFFu); }
[[nodiscard]] constexpr uint32_t tag_count(std::uint64_t t) noexcept {
    return static_cast<std::uint32_t>(t & 0xFFFF'FFFFu);
}

template <typename InternalPolicy>
class UringIO {
  public:
    explicit UringIO(RingBuffer &buffer, unsigned entries = 32) : m_policy(entries), m_buffer(buffer) {}

    void submit_read(int fd, std::size_t count)
        requires requires(InternalPolicy &p, int f, std::span<std::uint8_t> s) { p.prep_read(f, s); }
    {
        auto span = m_buffer.get().get_writable_span();
        if (span.empty())
            throw BufferOverflowException{};
        m_policy.prep_read(fd, span.first(std::min(count, span.size())));
    }

    void submit_write(int fd, std::size_t count)
        requires requires(InternalPolicy &p, int f, std::span<const std::uint8_t> s) { p.prep_write(f, s); }
    {
        auto span = m_buffer.get().get_readable_span();
        if (span.empty())
            throw std::logic_error("submit_write: empty span");
        m_policy.prep_write(fd, span.first(std::min(count, span.size())));
    }

    void submit_recv(int fd, std::size_t max_datagram)
        requires requires(InternalPolicy &p, int f, std::span<std::uint8_t> s) { p.prep_recv(f, s); }
    {
        auto span = m_buffer.get().get_writable_span();
        if (span.empty())
            throw BufferOverflowException{};
        m_policy.prep_recv(fd, span.first(std::min(max_datagram, span.size())));
    }

    void submit_send(int fd, const sockaddr_storage &peer, socklen_t peer_len, std::size_t count)
        requires requires(InternalPolicy &p, int f, const sockaddr_storage &addr, socklen_t l,
                          std::span<const std::uint8_t> s) { p.prep_send(f, addr, l, s); }
    {
        auto span = m_buffer.get().get_readable_span();
        if (span.empty())
            throw std::logic_error("submit_send: empty span");
        m_policy.prep_send(fd, peer, peer_len, span.first(std::min(count, span.size())));
    }

    void submit_poll(int fd, short events) { m_policy.prep_poll(fd, events); }

    [[nodiscard]] const sockaddr_storage &last_peer() const noexcept
        requires requires(const InternalPolicy &p) { p.last_peer(); }
    {
        return m_policy.last_peer();
    }

    [[nodiscard]] socklen_t last_peer_len() const noexcept
        requires requires(const InternalPolicy &p) { p.last_peer_len(); }
    {
        return m_policy.last_peer_len();
    }

    std::size_t wait_and_process() {
        io_uring_cqe *cqe{};
        if (const int rc = io_uring_wait_cqe(&m_policy.ring(), &cqe); rc < 0)
            throw std::system_error(-rc, std::system_category(), "io_uring_wait_cqe");

        std::size_t handled = 0;
        unsigned head{};

        io_uring_for_each_cqe(&m_policy.ring(), head, cqe) {
            ++handled;
            const std::uint64_t tag = io_uring_cqe_get_data64(cqe);
            const OpCode kind = tag_kind(tag);
            const int res = cqe->res;

            if (res > 0) {
                const auto bytes = static_cast<std::size_t>(res);
                if (kind == OpCode::READ || kind == OpCode::RECV)
                    m_buffer.get().commit_write(bytes);
                else if (kind == OpCode::WRITE || kind == OpCode::SEND)
                    m_buffer.get().advance_read(bytes);
            }
        }

        io_uring_cq_advance(&m_policy.ring(), static_cast<unsigned>(handled));
        return handled;
    }

  private:
    InternalPolicy m_policy;
    std::reference_wrapper<RingBuffer> m_buffer;
};

} // namespace transport::base::io
