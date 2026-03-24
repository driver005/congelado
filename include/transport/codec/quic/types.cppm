module;
#include <cassert>
#include <cstdint>
#include <cstring>
export module quic:types;

import std;

// quic:types retains only what quic:connection, quic:tls, quic:qpack,
// and server.http3 actually use. All packet, frame, recovery, and stream
// types have been removed — OpenSSL 3.6 owns those entirely.

namespace quic {

// ── Timestamp ─────────────────────────────────────────────────────────────────

export using Timestamp = std::uint64_t; // nanoseconds, monotonic
export constexpr Timestamp TS_INFINITE = std::numeric_limits<Timestamp>::max();

// ── VarInt (kept for QPACK and H3 frame parsing) ──────────────────────────────

export using VarInt = std::uint64_t;
export constexpr VarInt VARINT_MAX = (1ULL << 62) - 1;

export constexpr std::size_t varint_len(VarInt v) noexcept {
    if (v < (1ULL << 6))
        return 1;
    if (v < (1ULL << 14))
        return 2;
    if (v < (1ULL << 30))
        return 4;
    return 8;
}

export constexpr std::size_t varint_encode(std::byte *buf, VarInt v) noexcept {
    if (v < (1ULL << 6)) {
        buf[0] = static_cast<std::byte>(v);
        return 1;
    }
    if (v < (1ULL << 14)) {
        buf[0] = static_cast<std::byte>(0x40 | (v >> 8));
        buf[1] = static_cast<std::byte>(v);
        return 2;
    }
    if (v < (1ULL << 30)) {
        buf[0] = static_cast<std::byte>(0x80 | (v >> 24));
        buf[1] = static_cast<std::byte>(v >> 16);
        buf[2] = static_cast<std::byte>(v >> 8);
        buf[3] = static_cast<std::byte>(v);
        return 4;
    }
    buf[0] = static_cast<std::byte>(0xC0 | (v >> 56));
    buf[1] = static_cast<std::byte>(v >> 48);
    buf[2] = static_cast<std::byte>(v >> 40);
    buf[3] = static_cast<std::byte>(v >> 32);
    buf[4] = static_cast<std::byte>(v >> 24);
    buf[5] = static_cast<std::byte>(v >> 16);
    buf[6] = static_cast<std::byte>(v >> 8);
    buf[7] = static_cast<std::byte>(v);
    return 8;
}

export constexpr std::pair<VarInt, std::size_t> varint_decode(const std::byte *buf, std::size_t len) noexcept {
    if (len == 0)
        return {0, 0};
    std::uint8_t first = static_cast<std::uint8_t>(buf[0]);
    std::size_t nbytes = 1u << (first >> 6);
    if (len < nbytes)
        return {0, 0};
    VarInt val = first & 0x3F;
    for (std::size_t i = 1; i < nbytes; ++i)
        val = (val << 8) | static_cast<std::uint8_t>(buf[i]);
    return {val, nbytes};
}

// ── ConnectionId ──────────────────────────────────────────────────────────────

export constexpr std::size_t CID_MAX_LEN = 20;
export constexpr std::size_t CID_DEFAULT_LEN = 8;

export struct ConnectionId {
    std::uint8_t data[CID_MAX_LEN]{};
    std::uint8_t len{0};

    ConnectionId() = default;
    explicit ConnectionId(std::span<const std::uint8_t> bytes) noexcept {
        assert(bytes.size() <= CID_MAX_LEN);
        len = static_cast<std::uint8_t>(bytes.size());
        std::memcpy(data, bytes.data(), len);
    }

    [[nodiscard]] bool operator==(const ConnectionId &o) const noexcept {
        return len == o.len && std::memcmp(data, o.data, len) == 0;
    }
    [[nodiscard]] std::span<const std::uint8_t> view() const noexcept { return {data, len}; }

    void hex_into(char *out) const noexcept {
        static constexpr char d[] = "0123456789abcdef";
        for (std::uint8_t i = 0; i < len; ++i) {
            out[i * 2] = d[data[i] >> 4];
            out[i * 2 + 1] = d[data[i] & 0xF];
        }
    }
    [[nodiscard]] std::string hex() const {
        std::string s(len * 2, '\0');
        hex_into(s.data());
        return s;
    }
};

// ── ConnState ─────────────────────────────────────────────────────────────────

export enum class ConnState { Handshaking, Connected, Closing, Closed };

// ── Stream direction helpers (used by server.http3) ───────────────────────────

export constexpr bool stream_is_uni(std::uint64_t id) noexcept { return (id & 0x2) != 0; }
export constexpr bool stream_is_bidi(std::uint64_t id) noexcept { return (id & 0x2) == 0; }

// ── ByteReader / ByteWriter (used by QPACK and H3 frame parsing) ──────────────

export class ByteReader {
  public:
    explicit ByteReader(std::span<const std::byte> buf) noexcept : m_buf(buf), m_pos(0) {}

    [[nodiscard]] bool empty() const noexcept { return m_pos >= m_buf.size(); }
    [[nodiscard]] std::size_t remaining() const noexcept { return m_buf.size() - m_pos; }
    [[nodiscard]] std::size_t pos() const noexcept { return m_pos; }

    [[nodiscard]] bool peek_u8(std::uint8_t &out) const noexcept {
        if (empty())
            return false;
        out = static_cast<std::uint8_t>(m_buf[m_pos]);
        return true;
    }
    bool read_u8(std::uint8_t &out) noexcept {
        if (empty())
            return false;
        out = static_cast<std::uint8_t>(m_buf[m_pos++]);
        return true;
    }
    bool read_varint(VarInt &out) noexcept {
        auto [val, n] = varint_decode(m_buf.data() + m_pos, remaining());
        if (n == 0)
            return false;
        out = val;
        m_pos += n;
        return true;
    }
    [[nodiscard]] std::span<const std::byte> read_bytes(std::size_t n) noexcept {
        if (remaining() < n)
            return {};
        auto s = m_buf.subspan(m_pos, n);
        m_pos += n;
        return s;
    }
    bool skip(std::size_t n) noexcept {
        if (remaining() < n)
            return false;
        m_pos += n;
        return true;
    }
    [[nodiscard]] std::span<const std::byte> rest() const noexcept { return m_buf.subspan(m_pos); }

  private:
    std::span<const std::byte> m_buf;
    std::size_t m_pos;
};

export class ByteWriter {
  public:
    explicit ByteWriter(std::span<std::byte> buf) noexcept : m_buf(buf), m_pos(0) {}

    [[nodiscard]] std::size_t written() const noexcept { return m_pos; }
    [[nodiscard]] std::size_t remaining() const noexcept { return m_buf.size() - m_pos; }

    bool write_u8(std::uint8_t v) noexcept {
        if (remaining() < 1)
            return false;
        m_buf[m_pos++] = static_cast<std::byte>(v);
        return true;
    }
    bool write_varint(VarInt v) noexcept {
        std::size_t n = varint_len(v);
        if (remaining() < n)
            return false;
        varint_encode(m_buf.data() + m_pos, v);
        m_pos += n;
        return true;
    }
    bool write_bytes(std::span<const std::byte> data) noexcept {
        if (remaining() < data.size())
            return false;
        std::memcpy(m_buf.data() + m_pos, data.data(), data.size());
        m_pos += data.size();
        return true;
    }
    [[nodiscard]] std::span<std::byte> reserve(std::size_t n) noexcept {
        if (remaining() < n)
            return {};
        auto s = m_buf.subspan(m_pos, n);
        m_pos += n;
        return s;
    }
    [[nodiscard]] std::span<std::byte> written_span() noexcept { return m_buf.first(m_pos); }

  private:
    std::span<std::byte> m_buf;
    std::size_t m_pos;
};

} // namespace quic

template <>
struct std::hash<quic::ConnectionId> {
    std::size_t operator()(const quic::ConnectionId &cid) const noexcept {
        std::size_t h = 14695981039346656037ULL;
        for (std::uint8_t i = 0; i < cid.len; ++i)
            h = (h ^ cid.data[i]) * 1099511628211ULL;
        return h;
    }
};
