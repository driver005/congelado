export module io_layer_shared:codec;

import std;
import shared;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared_layer {

template<std::unsigned_integral UInt = std::uint32_t>
class Atom
{
private:
    static constexpr std::uint64_t LIMIT_6BIT = 64;             // 2^6
    static constexpr std::uint64_t LIMIT_14BIT = 16'384;        // 2^14
    static constexpr std::uint64_t LIMIT_30BIT = 1'073'741'824; // 2^30

public:
    // --- Standard Fixed-Length Big-Endian ---

    /**
     * @brief Writes `val` into `range` as a fixed-width big-endian integer — most significant
     * byte first, straight-up network byte order, no funny business.
     * @tparam R a writable byte range satisfying `shared::ByteRangeWriter`.
     * @param range the byte range to write into; must hold at least `sizeof(UInt)` bytes.
     * @param val the value to encode.
     */
    template<shared::ByteRangeWriter R>
    static void write_big_endian(R&& range, UInt val)
    {
        constexpr std::size_t BYTES = sizeof(UInt);
        std::size_t index = 0UZ;

        std::ranges::generate(std::views::take(std::forward<R>(range), BYTES), [&index, val]() {
            const auto SHIFT = 8UZ * (BYTES - 1UZ - index++);
            return static_cast<std::byte>(val >> SHIFT);
        });
    }

    /**
     * @brief Same big-endian write as the range overload above, just through an output iterator
     * instead — that's the motion, `out` gets walked forward by `sizeof(UInt)` positions once
     * it's done.
     * @tparam Out a writable byte iterator satisfying `shared::ByteIteratorWriter`.
     * @param out the iterator to write through; advanced in place past the written bytes.
     * @param val the value to encode.
     */
    template<shared::ByteIteratorWriter Out>
    static void write_big_endian(Out& out, UInt val)
    {
        constexpr std::size_t BYTES = sizeof(UInt);
        // Delegate the actual byte-packing to the range overload, aimed at a counted view of
        // `out`.
        write_big_endian(std::views::counted(out, BYTES), val);
        // Walk `out` forward past what just got written, ready for whatever's next.
        std::advance(out, BYTES);
    }

    /**
     * @brief Decodes a big-endian `UInt` out of `data`, most significant byte first.
     * @tparam R a readable byte range satisfying `shared::ByteRangeReader`.
     * @param data the byte range to decode; shorter than `sizeof(UInt)` is fine (the missing
     * bytes just don't contribute), but going over `sizeof(UInt)` is where it draws the line.
     * @return the decoded value.
     * @throws std::runtime_error if `data` holds more than `sizeof(UInt)` bytes.
     */
    template<shared::ByteRangeReader R>
    static UInt read_big_endian(R&& data)
    {
        constexpr std::size_t BYTES = sizeof(UInt);
        // Guard clause — more bytes than UInt can hold means the caller messed up, bail loud.
        if (std::ranges::size(data) > BYTES) {
            throw std::runtime_error("Too many bytes for uint64_t");
        }

        // Shift-and-OR each byte in, most significant first — that's big-endian, straight up.
        UInt val = 0;
        for (const auto BYTE: std::forward<R>(data)) {
            val = (val << 8) | std::to_integer<std::uint8_t>(BYTE);
        }

        return val;
    }

    /**
     * @brief Iterator-based big-endian read, companion to the range overload above.
     * @tparam It a readable byte iterator satisfying `shared::ByteIteratorReader`.
     * @param it the iterator to read from.
     * @return the decoded value — clean decode's the W here.
     */
    template<shared::ByteIteratorReader It>
    UInt read_big_endian(It& itr)
    {
        constexpr std::size_t BYTES = sizeof(UInt);
        return static_cast<UInt>(read_big_endian(itr, BYTES));
    }

    // --- QUIC Variable-Length Integer encodeing/decodeing ---

    /**
     * @brief Encodes `val` into `range` using the QUIC variable-length integer format — picks
     * the tightest 1/2/4/8-byte bucket that fits, no wasted bytes, lowkey efficient.
     * @tparam R a writable byte range satisfying `shared::ByteRangeWriter`.
     * @param range the byte range to write into; must hold enough bytes for the chosen bucket.
     * @param val the value to encode.
     */
    template<shared::ByteRangeWriter R>
    static void write_varint(R&& range, UInt val)
    {
        // Pick the tightest 1/2/4/8-byte bucket for val.
        auto [prefix, length] = varint_encoding(val);
        // Hand off the actual byte-packing to the helper.
        write_varint_helper(std::forward<R>(range), val, prefix, length);
    }

    /**
     * @brief Iterator-based varint write — same bucket-picking as the range overload, `out`
     * gets advanced past however many bytes the encoding actually took.
     * @tparam Out a writable byte iterator satisfying `shared::ByteIteratorWriter`.
     * @param out the iterator to write through; advanced in place.
     * @param val the value to encode.
     */
    template<shared::ByteIteratorWriter Out>
    static void write_varint(Out& out, UInt val)
    {
        // Same bucket math as the range overload.
        auto [prefix, length] = varint_encoding(val);
        // Pack the value into a counted view of `out`.
        write_varint(std::views::counted(out, length), val, prefix, length);
        // Then walk `out` forward past however many bytes that bucket took.
        std::advance(out, length);
    }

    /**
     * @brief Decodes a QUIC variable-length integer from the front of `data`.
     * @tparam R a readable byte range satisfying `shared::ByteRangeReader`.
     * @param data the byte range to decode from; only the leading bytes needed for the encoded
     * length actually get touched.
     * @return the decoded value paired with how many bytes it took up; `{0, 0}` if `data` is
     * empty — bet, that's the bail-out case.
     */
    template<shared::ByteRangeReader R>
    static std::pair<UInt, std::uint8_t> read_varint(R&& data)
    {
        // Guard clause — empty input means bet, nothing to decode.
        if (std::ranges::empty(data)) {
            return {0, 0};
        }

        // View data as plain uint8_t so the bit ops below don't need std::to_integer
        // everywhere.
        auto view = std::forward<R>(data) | std::views::transform([](std::byte byte) {
                        return std::to_integer<std::uint8_t>(byte);
                    });

        // First byte's top 2 bits pick the length bucket; the low 6 bits are value's first
        // chunk.
        std::uint8_t first_byte = *std::ranges::begin(view);
        std::uint8_t prefix = first_byte >> 6;
        std::uint8_t length = 1 << prefix;
        UInt value = first_byte & 0x3F;

        // Fold in the rest of the bucket's bytes, most significant first.
        for (auto byte: view | std::views::drop(1) | std::views::take(length - 1)) {
            value = (value << 8) | byte;
        }

        return {value, length};
    }

    /**
     * @brief Iterator-based varint read over `count` bytes starting at `it`.
     * @tparam It a readable byte iterator satisfying `shared::ByteIteratorReader`.
     * @param it the starting iterator, taken by value and advanced by the decoded length.
     * @param count how many bytes are available to read from `it`.
     * @return the decoded value paired with how many bytes it took up.
     */
    template<shared::ByteIteratorReader It>
    static std::pair<UInt, std::uint8_t> read_varint(It itr, std::size_t count)
    {
        // Decode via the range overload over a counted view.
        auto result = read_varint(std::views::counted(itr, count));
        // Then walk the (by-value) iterator forward past what got consumed.
        std::advance(itr, result.second);
        return result;
    }

private:
    /**
     * @brief Figures out which QUIC varint bucket `val` falls into — the 2-bit prefix code and
     * the byte length that goes with it.
     * @param val the value to size up.
     * @return the `{prefix, length}` pair for the tightest bucket that fits `val`.
     */
    static constexpr std::pair<std::uint8_t, std::size_t> varint_encoding(UInt val)
    {
        // Smallest bucket that fits wins — 1/2/4/8 bytes, QUIC's four tiers.
        if (val < LIMIT_6BIT) {
            return {0x00, 1};
        }
        if (val < LIMIT_14BIT) {
            return {0x01, 2};
        }
        if (val < LIMIT_30BIT) {
            return {0x02, 4};
        }
        return {0x03, 8};
    }

    /**
     * @brief Packs `val` into `range` given an already-picked `prefix`/`length` bucket — this
     * is where the actual byte-shuffling happens for write_varint(), no cap.
     * @tparam R a writable byte range satisfying `shared::ByteRangeWriter`.
     * @param range the byte range to write into; must hold at least `length` bytes.
     * @param val the value to encode.
     * @param prefix the 2-bit QUIC length prefix for this bucket.
     * @param length the total byte length of this bucket.
     */
    template<shared::ByteRangeWriter R>
    static void write_varint_helper(R&& range, UInt val, std::uint8_t prefix, std::size_t length)
    {
        // Write first byte with 2-bit prefix and the first chunk of data
        // (6 bits, 14 bits, 30 bits, or 62 bits usable)
        *std::ranges::begin(range) =
            static_cast<std::byte>((val >> (8 * (length - 1))) | (prefix << 6));

        // Then the rest of the bucket, one byte at a time, most significant first.
        for (auto [i, b]: std::forward<R>(range) | std::views::drop(1) |
                              std::views::take(length - 1) | std::views::enumerate) {
            b = static_cast<std::byte>(val >> (8 * (length - 2 - i)));
        }
    };
};

} // namespace io::shared_layer

#ifdef CONGELADO_TEST
namespace io::shared_layer::tests {
using namespace boost::ut;

suite<"Atom-big-endian"> atom_big_endian_suite = [] {
    "write/read round trip for a 32-bit value"_test = [] {
        std::array<std::byte, 4> buffer{};
        Atom<std::uint32_t>::write_big_endian(buffer, 0x01'02'03'04U);

        expect(buffer[0] == std::byte{0x01});
        expect(buffer[1] == std::byte{0x02});
        expect(buffer[2] == std::byte{0x03});
        expect(buffer[3] == std::byte{0x04});

        expect(Atom<std::uint32_t>::read_big_endian(buffer) == 0x01'02'03'04U);
    };

    "write/read round trip for a 16-bit value"_test = [] {
        std::array<std::byte, 2> buffer{};
        Atom<std::uint16_t>::write_big_endian(buffer, 0xBE'EFU);

        expect(Atom<std::uint16_t>::read_big_endian(buffer) == 0xBE'EFU);
    };

    "read tolerates fewer bytes than sizeof(UInt), missing bytes don't contribute"_test = [] {
        std::array<std::byte, 2> buffer{std::byte{0x00}, std::byte{0x2A}};
        expect(Atom<std::uint32_t>::read_big_endian(buffer) == 0x2AU);
    };

    "read throws when given more bytes than sizeof(UInt)"_test = [] {
        std::array<std::byte, 5> buffer{};
        expect(throws([&] {
            std::ignore = Atom<std::uint32_t>::read_big_endian(buffer);
        }));
    };
};

suite<"Atom-varint"> atom_varint_suite = [] {
    "1-byte bucket round trip for a value under 64"_test = [] {
        std::array<std::byte, 8> buffer{};
        Atom<std::uint32_t>::write_varint(buffer, 42U);

        auto [value, length] = Atom<std::uint32_t>::read_varint(buffer);
        expect(value == 42U);
        expect(length == 1);
    };

    "2-byte bucket round trip for a value under 16384"_test = [] {
        std::array<std::byte, 8> buffer{};
        Atom<std::uint32_t>::write_varint(buffer, 1'000U);

        auto [value, length] = Atom<std::uint32_t>::read_varint(buffer);
        expect(value == 1'000U);
        expect(length == 2);
    };

    "4-byte bucket round trip for a value under 2^30"_test = [] {
        std::array<std::byte, 8> buffer{};
        Atom<std::uint32_t>::write_varint(buffer, 100'000U);

        auto [value, length] = Atom<std::uint32_t>::read_varint(buffer);
        expect(value == 100'000U);
        expect(length == 4);
    };

    "8-byte bucket round trip for a value at or above 2^30"_test = [] {
        std::array<std::byte, 8> buffer{};
        Atom<std::uint64_t>::write_varint(buffer, 5'000'000'000ULL);

        auto [value, length] = Atom<std::uint64_t>::read_varint(buffer);
        expect(value == 5'000'000'000ULL);
        expect(length == 8);
    };

    "reading an empty range bails out with {0, 0}"_test = [] {
        std::array<std::byte, 0> buffer{};
        auto [value, length] = Atom<std::uint32_t>::read_varint(buffer);
        expect(value == 0U);
        expect(length == 0);
    };
};

} // namespace io::shared_layer::tests
#endif
