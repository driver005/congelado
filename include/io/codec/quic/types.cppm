module;
#include <cassert>
#include <cstdint>
#include <cstring>
export module io_quic:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

export constexpr std::size_t varint_len(VarInt value) noexcept {
    // Cascading range checks, smallest width first — QUIC's 2-bit length tag maps straight
    // onto these four buckets (1/2/4/8 bytes), so whichever range `value` lands in decides the width.
    if (value < (1ULL << 6)) {
        return 1;
    }
    if (value < (1ULL << 14)) {
        return 2;
    }
    if (value < (1ULL << 30)) {
        return 4;
    }
    return 8;
}

export constexpr std::size_t varint_encode(std::byte *buf, VarInt value) noexcept {
    // 1-byte form — top two bits of the tag are 00, value fits straight in the low 6 bits.
    if (value < (1ULL << 6)) {
        buf[0] = static_cast<std::byte>(value);
        return 1;
    }
    // 2-byte form — tag 01 in the top bits of the first byte, value spread MSB-first.
    if (value < (1ULL << 14)) {
        buf[0] = static_cast<std::byte>(0x40 | (value >> 8));
        buf[1] = static_cast<std::byte>(value);
        return 2;
    }
    // 4-byte form — tag 10.
    if (value < (1ULL << 30)) {
        buf[0] = static_cast<std::byte>(0x80 | (value >> 24));
        buf[1] = static_cast<std::byte>(value >> 16);
        buf[2] = static_cast<std::byte>(value >> 8);
        buf[3] = static_cast<std::byte>(value);
        return 4;
    }
    // 8-byte form — tag 11, everything that didn't fit the smaller buckets lands here.
    buf[0] = static_cast<std::byte>(0xC0 | (value >> 56));
    buf[1] = static_cast<std::byte>(value >> 48);
    buf[2] = static_cast<std::byte>(value >> 40);
    buf[3] = static_cast<std::byte>(value >> 32);
    buf[4] = static_cast<std::byte>(value >> 24);
    buf[5] = static_cast<std::byte>(value >> 16);
    buf[6] = static_cast<std::byte>(value >> 8);
    buf[7] = static_cast<std::byte>(value);
    return 8;
}

export constexpr std::pair<VarInt, std::size_t> varint_decode(const std::byte *buf, std::size_t len) noexcept {
    // Nothing to read at all — bail with the "invalid" sentinel {0, 0}.
    if (len == 0) {
        return {0, 0};
    }

    // Top two bits of the first byte are the length tag — decode straight into a byte count.
    auto first = static_cast<std::uint8_t>(buf[0]);
    std::size_t nbytes = 1U << (first >> 6);
    // Buffer got cut short before the full varint landed — same sentinel, no partial decode.
    if (len < nbytes) {
        return {0, 0};
    }

    // Low 6 bits of the first byte seed the value, then every following byte folds in MSB-first.
    VarInt val = first & 0x3F;
    for (std::size_t i = 1; i < nbytes; ++i) {
        val = (val << 8) | static_cast<std::uint8_t>(buf[i]);
    }
    return {val, nbytes};
}

// ── ConnectionId ──────────────────────────────────────────────────────────────

export constexpr std::size_t CID_MAX_LEN = 20;
export constexpr std::size_t CID_DEFAULT_LEN = 8;

export struct ConnectionId {
    std::uint8_t data[CID_MAX_LEN]{};  // NOLINT(readability-identifier-naming) — renaming to m_data would break the public-field access in crypto.cppm (cid.data)
    std::uint8_t len{0};  // NOLINT(readability-identifier-naming) — renaming to m_len would break the public-field access in crypto.cppm (cid.len)

    /** @brief Builds an empty CID — zero length, no motion, fill it in later. */
    ConnectionId() = default;
    /**
     * @brief Copies raw CID bytes in off the wire.
     * @warning `bytes.size()` must fit in `CID_MAX_LEN` (20) — asserted, not thrown, so a bad
     * caller trips UB in release builds where asserts get compiled out. Validate upstream.
     * @param bytes the raw connection-id bytes, straight from a QUIC packet header.
     */
    explicit ConnectionId(std::span<const std::uint8_t> bytes) noexcept : len(static_cast<std::uint8_t>(bytes.size())) {
        assert(bytes.size() <= CID_MAX_LEN);
        // Copy the raw bytes into the fixed-size backing array; length was set above.
        std::memcpy(data, bytes.data(), len);  // FIXME(clang-tidy): array-to-pointer decay
    }

    /**
     * @brief Byte-for-byte equality check, length included.
     * @param other the CID to compare against.
     * @return true if both CIDs are the same length and the same bytes.
     */
    [[nodiscard]] bool operator==(const ConnectionId &other) const noexcept {
        return len == other.len && std::memcmp(data, other.data, len) == 0;  // FIXME(clang-tidy): array-to-pointer decay
    }
    /** @brief Gets a read-only view over the active `len` bytes. @return span over the CID data. */
    [[nodiscard]] std::span<const std::uint8_t> view() const noexcept { return {data, len}; }  // FIXME(clang-tidy): array-to-pointer decay

    /**
     * @brief Hex-encodes this CID's bytes straight into a caller-owned buffer, no allocation.
     * @warning `out` needs at least `len * 2` bytes of room — this doesn't check, it just writes.
     * Buffer's on you.
     * @param out destination buffer for the hex digits (no null terminator appended).
     */
    void hex_into(char *out) const noexcept {
        static constexpr char HEX_DIGITS[] = "0123456789abcdef";
        // Two hex digits per byte — high nibble first, low nibble second.
        for (std::uint8_t i = 0; i < len; ++i) {
            const std::size_t OFFSET = static_cast<std::size_t>(i) * 2;
            out[OFFSET] = HEX_DIGITS[data[i] >> 4];  // FIXME(clang-tidy): non-constant array index
            out[OFFSET + 1] = HEX_DIGITS[data[i] & 0xF];  // FIXME(clang-tidy): non-constant array index
        }
    }
    /**
     * @brief Hex-encodes this CID's bytes into a fresh owned string.
     * @return lowercase hex string, `len * 2` chars.
     */
    [[nodiscard]] std::string hex() const {
        std::string result(static_cast<std::size_t>(len) * 2, '\0');
        hex_into(result.data());
        return result;
    }
};

// ── ConnState ─────────────────────────────────────────────────────────────────

// NOLINTNEXTLINE(readability-identifier-naming) — renaming these enumerators to UPPER_CASE would break every ConnState::Handshaking/Connected/Closing/Closed call site in connection.cppm
export enum class ConnState : std::uint8_t { Handshaking, Connected, Closing, Closed };

// ── Stream direction helpers (used by server.http3) ───────────────────────────

export constexpr bool stream_is_uni(std::uint64_t streamId) noexcept { return (streamId & 0x2) != 0; }
export constexpr bool stream_is_bidi(std::uint64_t streamId) noexcept { return (streamId & 0x2) == 0; }

// ── ByteReader / ByteWriter (used by QPACK and H3 frame parsing) ──────────────

export class ByteReader {
  public:
    /**
     * @brief Wraps a read-only cursor around `buf`, starting at offset 0 — no copy, just a view.
     * @param buf the backing bytes to read from; must outlive this reader.
     */
    explicit ByteReader(std::span<const std::byte> buf) noexcept : m_buf(buf) {}

    /**
     * @brief Checks if the cursor has run off the end of the buffer.
     * @return true if nothing's left to read.
     */
    [[nodiscard]] bool empty() const noexcept { return m_pos >= m_buf.size(); }
    /**
     * @brief Gets how many bytes are still unread.
     * @return byte count from the cursor to the buffer's end.
     */
    [[nodiscard]] std::size_t remaining() const noexcept { return m_buf.size() - m_pos; }
    /** @brief Gets the cursor's current offset into the buffer. @return current read position. */
    [[nodiscard]] std::size_t pos() const noexcept { return m_pos; }

    /**
     * @brief Looks at the next byte without consuming it — cursor doesn't move, bet.
     * @param out written with the peeked byte on success, left untouched on failure.
     * @return true if a byte was available to peek at, false if the buffer's empty.
     */
    [[nodiscard]] bool peek_u8(std::uint8_t &out) const noexcept {
        if (empty()) {
            return false;
        }
        out = static_cast<std::uint8_t>(m_buf[m_pos]);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return true;
    }
    /**
     * @brief Reads one byte and advances the cursor past it.
     * @param out written with the consumed byte on success, left untouched on failure.
     * @return true if a byte was available, false if the buffer's empty.
     */
    bool read_u8(std::uint8_t &out) noexcept {
        if (empty()) {
            return false;
        }
        out = static_cast<std::uint8_t>(m_buf[m_pos++]);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return true;
    }
    /**
     * @brief Decodes a QUIC varint starting at the cursor and advances past however many bytes it
     * took.
     * @param out written with the decoded value on success, left untouched on failure.
     * @return true if a full varint was available to decode, false if the buffer cut off mid-value.
     */
    bool read_varint(VarInt &out) noexcept {
        // Delegate the actual decode, then only commit the cursor/output on a real hit —
        // decoded_len == 0 means varint_decode hit its own "invalid/truncated" sentinel.
        auto [val, decoded_len] = varint_decode(m_buf.data() + m_pos, remaining());
        if (decoded_len == 0) {
            return false;
        }
        out = val;
        m_pos += decoded_len;
        return true;
    }
    /**
     * @brief Slices off the next `count` bytes and advances the cursor past them.
     * @param count how many bytes to take.
     * @return a span over the sliced bytes, or an empty span if fewer than `count` remain — check
     * before trusting the result, an empty span isn't always "I asked for zero".
     */
    [[nodiscard]] std::span<const std::byte> read_bytes(std::size_t count) noexcept {
        // Not enough left to satisfy the request — empty span, cursor untouched.
        if (remaining() < count) {
            return {};
        }
        // Slice the span and advance the cursor past it.
        auto sliced = m_buf.subspan(m_pos, count);
        m_pos += count;
        return sliced;
    }
    /**
     * @brief Advances the cursor by `count` bytes without reading them — for skipping fields
     * nobody cares about.
     * @param count how many bytes to skip.
     * @return true if there were `count` bytes left to skip, false (no-op) otherwise.
     */
    bool skip(std::size_t count) noexcept {
        // Guard first — no partial skips, either the whole `count` bytes are there or nothing moves.
        if (remaining() < count) {
            return false;
        }
        m_pos += count;
        return true;
    }
    /**
     * @brief Gets everything from the cursor to the buffer's end, no consumption.
     * @return the tail span.
     */
    [[nodiscard]] std::span<const std::byte> rest() const noexcept { return m_buf.subspan(m_pos); }

  private:
    std::span<const std::byte> m_buf;
    std::size_t m_pos{0};
};

export class ByteWriter {
  public:
    /**
     * @brief Wraps a write cursor around `buf`, starting at offset 0.
     * @param buf the backing bytes to write into; must outlive this writer.
     */
    explicit ByteWriter(std::span<std::byte> buf) noexcept : m_buf(buf) {}

    /** @brief Gets how many bytes have been written so far. @return the write cursor's offset. */
    [[nodiscard]] std::size_t written() const noexcept { return m_pos; }
    /**
     * @brief Gets how many bytes of buffer space are left.
     * @return byte count from the cursor to the end.
     */
    [[nodiscard]] std::size_t remaining() const noexcept { return m_buf.size() - m_pos; }

    /**
     * @brief Writes one byte and advances the cursor.
     * @param value the byte to write.
     * @return true if there was room, false (no-op) if the buffer's full.
     */
    bool write_u8(std::uint8_t value) noexcept {
        // No room for even one byte — no-op, false back.
        if (remaining() < 1) {
            return false;
        }
        m_buf[m_pos++] = static_cast<std::byte>(value);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return true;
    }
    /**
     * @brief Encodes `value` as a QUIC varint and writes it, advancing the cursor past it.
     * @param value the value to varint-encode.
     * @return true if there was room for the encoded length, false (no-op) otherwise.
     */
    bool write_varint(VarInt value) noexcept {
        // Work out the encoded width first so the room check can happen before any bytes move.
        std::size_t encoded_len = varint_len(value);
        if (remaining() < encoded_len) {
            return false;
        }
        varint_encode(m_buf.data() + m_pos, value);
        m_pos += encoded_len;
        return true;
    }
    /**
     * @brief Copies raw bytes in and advances the cursor past them.
     * @param data the bytes to write.
     * @return true if there was room for all of `data`, false (no-op) if it wouldn't fit.
     */
    bool write_bytes(std::span<const std::byte> data) noexcept {
        // All-or-nothing — either every byte of `data` fits, or nothing gets written.
        if (remaining() < data.size()) {
            return false;
        }
        std::memcpy(m_buf.data() + m_pos, data.data(), data.size());
        m_pos += data.size();
        return true;
    }
    /**
     * @brief Carves out `count` bytes of writable space and advances the cursor past them, letting
     * the caller fill it in directly instead of staging a copy first.
     * @param count how many bytes to reserve.
     * @return a span over the reserved bytes, or an empty span if fewer than `count` remain.
     */
    [[nodiscard]] std::span<std::byte> reserve(std::size_t count) noexcept {
        // Same shape as read_bytes() but on the write side — carve out the span, move the cursor
        // past it, and let the caller fill it in directly.
        if (remaining() < count) {
            return {};
        }
        auto sliced = m_buf.subspan(m_pos, count);
        m_pos += count;
        return sliced;
    }
    /**
     * @brief Gets everything written so far, from the start of the buffer to the cursor.
     * @return the written span.
     */
    [[nodiscard]] std::span<std::byte> written_span() noexcept { return m_buf.first(m_pos); }

  private:
    std::span<std::byte> m_buf;
    std::size_t m_pos{0};
};

} // namespace quic

template <>
struct std::hash<quic::ConnectionId> {
    /**
     * @brief FNV-1a hash over the CID's active bytes — lets ConnectionId ride in
     * `std::unordered_map`/`unordered_set` without a bespoke hasher at every call site.
     * @param cid the connection id to hash.
     * @return the FNV-1a digest of `cid`'s bytes.
     */
    std::size_t operator()(const quic::ConnectionId &cid) const noexcept {
        // Standard FNV-1a: start from the offset basis, XOR-then-multiply one byte at a time.
        std::size_t digest = 14695981039346656037ULL;
        for (std::uint8_t i = 0; i < cid.len; ++i) {
            digest = (digest ^ cid.data[i]) * 1099511628211ULL;  // FIXME(clang-tidy): non-constant array index
        }
        return digest;
    }
};

#ifdef CONGELADO_TEST
namespace quic::tests {
using namespace boost::ut;

suite<"varint_len/varint_encode/varint_decode"> varint_suite = [] {
    "round-trips every length-class boundary"_test = [] {
        for (VarInt value : {VarInt{0}, VarInt{63}, VarInt{64}, VarInt{16383}, VarInt{16384},
                             VarInt{1073741823}, VarInt{1073741824}, VARINT_MAX}) {
            std::array<std::byte, 8> buf{};
            auto len = varint_encode(buf.data(), value);
            expect(len == varint_len(value));

            auto [decoded, consumed] = varint_decode(buf.data(), buf.size());
            expect(consumed == len);
            expect(decoded == value);
        }
    };
    "varint_len picks the smallest width for each RFC 9000 bucket"_test = [] {
        expect(varint_len(0) == 1);
        expect(varint_len(63) == 1);
        expect(varint_len(64) == 2);
        expect(varint_len(16383) == 2);
        expect(varint_len(16384) == 4);
        expect(varint_len(1073741823) == 4);
        expect(varint_len(1073741824) == 8);
    };
    "varint_decode on an empty buffer returns the {0,0} invalid sentinel"_test = [] {
        auto [value, consumed] = varint_decode(nullptr, 0);
        expect(value == 0);
        expect(consumed == 0);
    };
    "varint_decode on a truncated buffer returns the {0,0} invalid sentinel"_test = [] {
        std::array<std::byte, 8> buf{};
        varint_encode(buf.data(), VarInt{16384}); // needs 4 bytes
        auto [value, consumed] = varint_decode(buf.data(), 2);
        expect(value == 0);
        expect(consumed == 0);
    };
};

suite<"stream_is_uni/stream_is_bidi"> stream_direction_suite = [] {
    "bit 0x2 distinguishes unidirectional from bidirectional streams"_test = [] {
        expect(stream_is_bidi(0));
        expect(not stream_is_uni(0));
        expect(stream_is_uni(2));
        expect(not stream_is_bidi(2));
    };
};

suite<"ConnectionId"> connection_id_suite = [] {
    "default-constructed CID has zero length"_test = [] {
        ConnectionId cid;
        expect(cid.len == 0);
        expect(cid.view().empty());
    };
    "constructing from bytes copies them in and sets the length"_test = [] {
        std::array<std::uint8_t, 4> raw{0x01, 0x02, 0x03, 0x04};
        ConnectionId cid{raw};

        expect(cid.len == 4);
        expect(std::ranges::equal(cid.view(), raw));
    };
    "equality compares length and bytes"_test = [] {
        std::array<std::uint8_t, 3> raw{0xAA, 0xBB, 0xCC};
        ConnectionId first{raw};
        ConnectionId second{raw};
        ConnectionId empty;

        expect(first == second);
        expect(not(first == empty));
    };
    "hex() renders lowercase two-digit-per-byte encoding"_test = [] {
        std::array<std::uint8_t, 2> raw{0xDE, 0xAD};
        ConnectionId cid{raw};
        expect(cid.hex() == "dead");
    };
    "std::hash specialization is usable in an unordered_set"_test = [] {
        std::array<std::uint8_t, 2> raw{0x01, 0x02};
        ConnectionId cid{raw};

        std::unordered_set<ConnectionId, std::hash<ConnectionId>> set;
        set.insert(cid);
        expect(set.contains(cid));
    };
    // The constructor's `bytes.size() <= CID_MAX_LEN` bound check is assert-only — compiled out
    // under NDEBUG/release, leaving an unchecked std::memcpy into the fixed 20-byte `data` array.
    // A span exactly CID_MAX_LEN long is the largest input that's safe to exercise here; anything
    // past it would either trip the assert (aborting the whole shared test binary) or, with
    // asserts disabled, perform a real OOB memcpy, so that case is deliberately not exercised.
    "constructing from exactly CID_MAX_LEN bytes succeeds at the boundary"_test = [] {
        std::array<std::uint8_t, CID_MAX_LEN> raw{
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
            0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
        ConnectionId cid{raw};

        expect(cid.len == CID_MAX_LEN);
        expect(std::ranges::equal(cid.view(), raw));
    };
};

suite<"ByteReader"> byte_reader_suite = [] {
    "reads bytes and varints sequentially, advancing the cursor"_test = [] {
        std::array<std::byte, 8> buf{};
        ByteWriter writer{buf};
        expect(writer.write_u8(0x7A));
        expect(writer.write_varint(300));

        ByteReader reader{writer.written_span()};
        expect(reader.pos() == 0);

        std::uint8_t peeked = 0;
        expect(reader.peek_u8(peeked));
        expect(peeked == 0x7A);
        expect(reader.pos() == 0); // peek doesn't consume

        std::uint8_t byte_value = 0;
        expect(reader.read_u8(byte_value));
        expect(byte_value == 0x7A);

        VarInt decoded = 0;
        expect(reader.read_varint(decoded));
        expect(decoded == 300);
        expect(reader.empty());
    };
    "read_u8/read_varint fail cleanly on an empty buffer"_test = [] {
        std::array<std::byte, 0> empty_buf{};
        ByteReader reader{empty_buf};

        std::uint8_t byte_value = 0;
        expect(not reader.read_u8(byte_value));

        VarInt decoded = 0;
        expect(not reader.read_varint(decoded));
    };
    "read_bytes/skip/rest behave around the cursor"_test = [] {
        std::array<std::byte, 4> buf{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
        ByteReader reader{buf};

        auto first_two = reader.read_bytes(2);
        expect(first_two.size() == 2);
        expect(reader.remaining() == 2);

        expect(reader.skip(1));
        expect(reader.rest().size() == 1);

        // Asking for more than remains fails without moving the cursor.
        expect(reader.read_bytes(5).empty());
        expect(not reader.skip(5));
    };
};

suite<"ByteWriter"> byte_writer_suite = [] {
    "write_u8/write_bytes/reserve fail cleanly when out of room"_test = [] {
        std::array<std::byte, 2> buf{};
        ByteWriter writer{buf};

        expect(writer.write_u8(1));
        expect(writer.write_u8(2));
        expect(not writer.write_u8(3)); // full
        expect(writer.written() == 2);
        expect(writer.remaining() == 0);
    };
    "write_bytes is all-or-nothing"_test = [] {
        std::array<std::byte, 2> buf{};
        ByteWriter writer{buf};

        std::array<std::byte, 3> too_big{};
        expect(not writer.write_bytes(too_big));
        expect(writer.written() == 0); // nothing partially written
    };
    "reserve carves out writable space and advances the cursor"_test = [] {
        std::array<std::byte, 4> buf{};
        ByteWriter writer{buf};

        auto slot = writer.reserve(2);
        expect(slot.size() == 2);
        expect(writer.written() == 2);
        expect(writer.reserve(10).empty());
    };
};

} // namespace quic::tests
#endif
