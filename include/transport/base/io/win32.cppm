export module io:win32;

import std;
import :types;

export namespace transport::base::io {

auto BufferRing::init() -> std::expected<void, std::errc> {
    m_slab = static_cast<std::byte *>(::VirtualAlloc(nullptr, k_ring_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!m_slab)
        return std::unexpected(std::errc{static_cast<int>(::GetLastError())});
    return {};
}

auto BufferRing::write_head() const noexcept -> std::pair<std::byte *, std::size_t> {
    const std::size_t offset = m_write & k_ring_mask;
    const std::size_t contiguous = k_ring_size - offset;
    return {m_slab + offset, contiguous};
}

void BufferRing::commit_write(std::size_t n) noexcept {
    m_write += n;
    if ((m_write - m_read) > k_ring_size)
        m_read = m_write - k_ring_size;
}

auto BufferRing::readable() const noexcept -> std::span<const std::byte> {
    const std::size_t bytes = m_write - m_read;
    if (bytes == 0)
        return {};
    const std::size_t offset = m_read & k_ring_mask;
    return {m_slab + offset, std::min(bytes, k_ring_size - offset)};
}

void BufferRing::consume(std::size_t n) noexcept { m_read += std::min(n, m_write - m_read); }

// ── TCP ───────────────────────────────────────────────────────────────────────

auto BufferRing::post_recv(SOCKET s) noexcept -> std::expected<void, std::errc> {
    auto [ptr, len] = write_head();
    m_overlapped.m_write_offset = m_write;
    m_overlapped.m_wsabuf.buf = reinterpret_cast<char *>(ptr);
    m_overlapped.m_wsabuf.len = static_cast<ULONG>(len);
    ::ZeroMemory(&m_overlapped.m_ov, sizeof(OVERLAPPED));

    DWORD flags = 0, received = 0;
    if (::WSARecv(s, &m_overlapped.m_wsabuf, 1, &received, &flags, &m_overlapped.m_ov, nullptr) == SOCKET_ERROR) {
        if (auto err = ::WSAGetLastError(); err != WSA_IO_PENDING)
            return std::unexpected(std::errc{static_cast<int>(err)});
    }
    return {};
}

void BufferRing::prep_recv(void *socket_ptr, void *user_data) noexcept {
    SOCKET s = *static_cast<SOCKET *>(socket_ptr);
    ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(s), m_iocp, reinterpret_cast<ULONG_PTR>(user_data), 0);
    std::ignore = post_recv(s);
}

auto BufferRing::handle_completion(void *event) const noexcept -> RecvResult {
    const auto &entry = *static_cast<const OVERLAPPED_ENTRY *>(event);
    if (!entry.lpOverlapped)
        return std::unexpected(BufferRingError::NoBuffer);
    if (entry.dwNumberOfBytesTransferred == 0)
        return std::unexpected(BufferRingError::Eof);

    auto *ov = reinterpret_cast<IocpOverlapped *>(entry.lpOverlapped);
    return std::span<const std::byte>{m_slab + (ov->m_write_offset & k_ring_mask), entry.dwNumberOfBytesTransferred};
}

// ── UDP ───────────────────────────────────────────────────────────────────────

auto BufferRing::post_recvfrom(SOCKET s) noexcept -> std::expected<void, std::errc> {
    auto [ptr, len] = write_head();
    m_overlapped.m_write_offset = m_write;
    m_overlapped.m_wsabuf.buf = reinterpret_cast<char *>(ptr);
    m_overlapped.m_wsabuf.len = static_cast<ULONG>(len);
    m_overlapped.m_peer_len = sizeof(::SOCKADDR_STORAGE);
    ::ZeroMemory(&m_overlapped.m_ov, sizeof(OVERLAPPED));

    DWORD flags = 0, received = 0;
    if (::WSARecvFrom(s, &m_overlapped.m_wsabuf, 1, &received, &flags,
                      reinterpret_cast<sockaddr *>(&m_overlapped.m_peer), &m_overlapped.m_peer_len, &m_overlapped.m_ov,
                      nullptr) == SOCKET_ERROR) {
        if (auto err = ::WSAGetLastError(); err != WSA_IO_PENDING)
            return std::unexpected(std::errc{static_cast<int>(err)});
    }
    return {};
}

void BufferRing::prep_recvmsg(void *socket_ptr, void *user_data) noexcept {
    SOCKET s = *static_cast<SOCKET *>(socket_ptr);
    ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(s), m_iocp, reinterpret_cast<ULONG_PTR>(user_data), 0);
    std::ignore = post_recvfrom(s);
}

auto BufferRing::handle_recvmsg_completion(void *event) const noexcept -> RecvMsgExpected {
    const auto &entry = *static_cast<const OVERLAPPED_ENTRY *>(event);
    if (!entry.lpOverlapped)
        return std::unexpected(BufferRingError::NoBuffer);
    if (entry.dwNumberOfBytesTransferred == 0)
        return std::unexpected(BufferRingError::Eof);

    auto *ov = reinterpret_cast<IocpOverlapped *>(entry.lpOverlapped);
    RecvMsgResult res{
        .data = {m_slab + (ov->m_write_offset & k_ring_mask), entry.dwNumberOfBytesTransferred},
        .peer_addr_len = static_cast<std::uint16_t>(ov->m_peer_len),
    };
    std::memcpy(res.peer_addr.data(), &ov->m_peer, ov->m_peer_len);
    return res;
}

// UDP prep_recvmsg and handle_recvmsg_completion would use IORING_OP_RECVMSG
// and parse the msghdr/control data for peer addresses similarly.

} // namespace transport::base::io
