module;

#include <cassert>

export module io_codec_shared:atom;
import io_error;
import std;
import :types;
import :huffman;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared_codec::raw {

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires DecodeWidth<Width>
class Atom {
  public:
    // The integer is less than 2^N - 1 (N = 5):
    //   0   1   2   3   4   5   6   7
    // +---+---+---+---+---+---+---+---+
    // | ? | ? | ? |       Value       |
    // +---+---+---+-------------------+
    //
    // Or the integer is greater(grammer is a bitch) than 2^N - 1 (N = 5):
    //
    //   0   1   2   3   4   5   6   7
    // +---+---+---+---+---+---+---+---+
    // | ? | ? | ? | 1   1   1   1   1 |
    // +---+---+---+-------------------+
    // | 1 |    Value-(2^N-1) LSB      |
    // +---+---------------------------+
    //                ...
    // +---+---------------------------+
    // | 0 |    Value-(2^N-1) MSB      |
    // +---+---------------------------+
    /**
     * @brief HPACK/QPACK integer encoding (RFC 7541 §5.1) — writes `data` prefix-encoded into
     * `prefix_size` bits of the first octet, spilling into continuation bytes if it doesn't fit.
     * @tparam Out output iterator type data gets written through.
     * @tparam PrefixType type of `prefix_data` — must be castable to `std::uint8_t` (raw byte or
     * a matching-width enum).
     * @param data the integer value to encode.
     * @param prefix_size how many bits of the first octet belong to the prefix — must be 1-8.
     * @param prefix_data the fixed high bits to OR into the first octet alongside the prefix
     * (e.g. a representation-type tag).
     * @param out output iterator bytes get written to, one `*out++ =` at a time.
     * @throws std::invalid_argument if `prefix_size` is outside [1,8].
     */
    template <std::output_iterator<std::uint8_t> Out, typename PrefixType>
        requires CastableToUint8<PrefixType>
    static void encode_int(UInt data, std::uint8_t prefix_size, PrefixType prefix_data, Out out) {
        if (prefix_size < 1 || prefix_size > 8) {
            throw std::invalid_argument("prefix must be in range [1,8] (inclusive)");
        }

        // 2^N - 1
        const auto MAX_PREFIX = static_cast<std::uint32_t>((1U << prefix_size) - 1U);
        // Mask to clear prefix bits.
        const auto MASK = static_cast<std::uint8_t>(MAX_PREFIX);

        // Extract Enum type
        const std::uint8_t PREFIX = static_cast<std::uint8_t>(prefix_data);

        // if I < 2^N - 1, encode I on N bits
        //    else
        //        encode (2^N - 1) on N bits
        //        I = I - (2^N - 1)
        //        while I >= 128
        //             encode (I % 128 + 128) on 8 bits
        //             I = I / 128
        //        encode I on 8 bits
        if (data < MAX_PREFIX) {
            // Value fits in one octet, the function to calculate the max value is:  2 ^ prefix - 1 (inclusive).
            *out++ = (PREFIX & ~MASK) | static_cast<std::uint8_t>(data);
        } else {
            // Value is exeds the limit of one octet. In the following we need to encode the value in multiple octets.

            // Encode the prefix as well as the max value for the prefix (all bits of prefix must be set to 1).
            *out++ = static_cast<std::uint8_t>(PREFIX | MASK);

            // Suvtract the max value of the prefix from the value, as we have already encoded that part in the first
            // octet.
            data -= MAX_PREFIX;

            // while data > 127 (2 ^ 7 - 1)
            while (data > 0x7F) {
                *out++ = static_cast<std::uint8_t>((data % 0x80) + 0x80);
                data >>= 7;
            }

            // Since the data is less than (2 ^ 7) we can be sure that the MSB is 0.
            *out++ = static_cast<std::uint8_t>(data);
        }
    }

    /**
     * @brief Inverse of encode_int(), bet — decodes an HPACK/QPACK prefix-encoded integer
     * starting at `pos`, advancing `pos` past however many octets it consumed.
     * @tparam PrefixOffset when > 0, also captures the bits sitting before the integer prefix in
     * the first octet (representation-type flags) into the result's metadata; 0 skips that.
     * @param data the buffer to decode from.
     * @param pos in/out cursor — read from on entry, advanced past the consumed octets on exit.
     * @param prefix_size how many bits of the first octet belong to the prefix — must be 1-8.
     * @return the decoded value plus prefix metadata bits.
     * @throws std::invalid_argument if `prefix_size` is outside [1,8].
     * @throws error::http::TruncatedDataError if `pos` is already past the end of `data`.
     * @throws error::http::IntegerDecodeError if a continuation byte is missing (truncated
     * stream) or the accumulated value would overflow `UInt`'s bit width — no silent wraparound,
     * ever, this is a strict decode.
     */
    template <std::size_t PrefixOffset = 0>
    static DecodeIntResult<UInt> decode_int(std::span<const std::uint8_t> data, std::size_t &pos,
                                                  std::uint8_t prefix_size) {
        if (prefix_size < 1 || prefix_size > 8) {
            throw std::invalid_argument{"prefix must be in range [1,8] (inclusive)"};
        }

        if (pos >= data.size()) {
            throw error::http::TruncatedDataError{};
        }

        const std::uint8_t FIRST_BYTE = data[pos++];  // FIXME(clang-tidy): unchecked operator[], consider .at()

        std::uint8_t prefix_metadata = 0;
        if constexpr (PrefixOffset > 0) {
            // Shift right to isolate the bits before the integer prefix
            prefix_metadata = static_cast<std::uint8_t>(FIRST_BYTE >> prefix_size);
        }
        // 2^N - 1
        const UInt MASK = static_cast<UInt>((1U << prefix_size) - 1U);

        // Read the data from the first octet, masking out the prefix bits.
        UInt value = static_cast<UInt>(FIRST_BYTE & MASK);

        // decode I from the next N bits
        //  if I < 2^N - 1, return I
        //  else
        //      M = 0
        //      repeat
        //          B = next octet
        //          I = I + (B & 127) * 2^M
        //          M = M + 7
        //      while B & 128 == 128
        //      return I
        if (value < MASK) {
            // Value is less than the max value of the prefix, so we can return it directly.
            return {value, prefix_metadata};
        }

        // Bit shift needed to align the bytes inside of uint32_t.
        std::uint8_t bit_shift = 0;
        constexpr std::uint8_t DIGITS = std::numeric_limits<UInt>::digits;

        while (true) {
            if (pos >= data.size()) {
                throw error::http::IntegerDecodeError{"truncated continuation"};
            }

            if (bit_shift >= DIGITS) {
                throw error::http::IntegerDecodeError{"overflow"};
            }

            const std::uint8_t CONTINUATION_BYTE = data[pos++];  // FIXME(clang-tidy): unchecked operator[], consider .at()

            // Get data execpt for the first continuation bit. Then shift into the correct postion inside of the Uint
            // and add to the value.
            const UInt SHIFTED = static_cast<UInt>(CONTINUATION_BYTE & 0x7FU) << bit_shift;

            // Check for overflow before adding the shifted value to the total value.
            // Clang and GCC only sorry!
            if (__builtin_add_overflow(value, SHIFTED, &value)) {
                throw error::http::IntegerDecodeError{"overflow"};
            }

            bit_shift += 7;

            // MSB = 0 → last continuation byte
            if (!(CONTINUATION_BYTE & 0x80U)) {
                break;
            }
        }

        return {value, prefix_metadata};
    };

    //   0   1   2   3   4   5   6   7
    // +---+---+---+---+---+---+---+---+
    // | H |    String Length (7+)     |
    // +---+---------------------------+
    // |  String Data (Length octets)  |
    // +-------------------------------+
    // H: Huffman flag (1 bit) –> 1 = Huffman-encoded | 0 = raw string
    static constexpr std::size_t MAX_LENGTH = 65536;

    /**
     * @brief HPACK/QPACK string encoding — writes the H-bit + length prefix then the string body,
     * optionally Huffman-compressing it first. Real low-key power move when `huffman` is
     * non-null: smaller wire size for free.
     * @tparam Out output iterator type bytes get written through.
     * @param huffman pointer to a Huffman coder to compress `str` through, or null for raw
     * (uncompressed) encoding.
     * @param str the string to encode.
     * @param out output iterator bytes get written to.
     * @param prefix_size how many bits of the length's first octet belong to the length prefix —
     * defaults to 7, matching the HPACK/QPACK string-length wire format.
     * @note Only debug-asserted, not thrown: `encoded.size()` (or `str.size()` in the raw path)
     * must fit in `UInt`'s range — a release build with a string bigger than that overflows
     * silently instead of erroring. Don't be encoding anything near `UInt`'s max in prod without
     * checking first.
     */
    template <std::output_iterator<std::uint8_t> Out>
    static void encode_string(const huffman::Huffman<Width> *huffman, std::string_view str, Out out,
                              std::uint8_t prefix_size = 7U) {
        if (huffman) {
            std::vector<std::uint8_t> encoded;
            encoded.reserve(((str.size() * 5) + 7) / 8);

            // TODO: implement Huffman by passing a ref via props which acts as flag isntead of using use_huffman
            for (std::byte value : str | std::views::transform([](char character) {
                                             return static_cast<std::byte>(character);
                                         }) |
                                        huffman::Huffman<Width>::encode()) {
                encoded.push_back(static_cast<std::uint8_t>(value));
            }

            // TODO: wait for cpp26 and use std::narrowing_cast
            assert(encoded.size() <= std::numeric_limits<UInt>::max());
            // Write length then Huffman bytes directly into out.
            encode_int<Out>(encoded.size(), prefix_size, PrefixHelper::HUFFMAN_ENABLED, out);

            for (std::uint8_t byte : encoded) {
                *out++ = byte;
            }
        } else {
            // No Huffman coder passed in — write the length prefix with the H-bit cleared, then
            // the string's raw bytes, uncompressed.
            // TODO: wait for cpp26 and use std::narrowing_cast
            assert(str.size() <= std::numeric_limits<UInt>::max());
            encode_int<Out>(str.size(), prefix_size, PrefixHelper::HUFFMAN_DISABLED, out);

            for (std::uint8_t ch : str) {
                *out++ = ch;
            }
        }
    };

    /**
     * @brief Inverse of encode_string() — reads the H-bit + length-prefixed string starting at
     * `pos`, Huffman-decoding the body if the H-bit's set, and advances `pos` past the whole
     * thing (length prefix + body).
     * @param huffman_coder the Huffman coder to decode through, used only when the H-bit is set.
     * @param data the buffer to decode from.
     * @param pos in/out cursor — read from on entry, advanced past the consumed bytes on exit.
     * @return the decoded string, plain UTF-8/ASCII bytes either way (Huffman-decoded or raw).
     * @throws error::http::TruncatedDataError if `pos` is already past the end of `data`.
     * @throws error::http::StringDecodeError if the decoded length exceeds `MAX_LENGTH` (65536)
     * or the string body would run past the end of `data` — both are treated as hostile/malformed
     * input, not recoverable states.
     */
    static std::string decode_string(const huffman::Huffman<Width> &huffman_coder, std::span<const std::uint8_t> data,
                                     std::size_t &pos) {
        if (pos >= data.size()) {
            throw error::http::TruncatedDataError{};
        }

        // Check the Huffman flag (H-bit) in the first byte.
        const bool HUFFMAN_FLAG = (data[pos] & 0x80) != 0;  // FIXME(clang-tidy): unchecked operator[], consider .at()

        // String length is encoded as a 7-bit prefix integer.
        const auto LENGTH = decode_int(data, pos, 7U);

        if (LENGTH.value() > MAX_LENGTH) {
            throw error::http::StringDecodeError{"exceeds MAX_LENGTH"};
        }
        if (pos + LENGTH.value() > data.size()) {
            throw error::http::StringDecodeError("truncated string data");
        }

        // Get the string data as a span, then advance the position.
        std::span<const std::uint8_t> body = data.subspan(pos, LENGTH.value());
        pos += LENGTH.value();

        if (HUFFMAN_FLAG) {
            static_cast<void>(huffman_coder); // instance not needed — decode() is static, see below
            std::string decoded;
            for (char character : body | std::views::transform([](std::uint8_t byte_value) {
                                            return static_cast<std::byte>(byte_value);
                                        }) |
                                       huffman::Huffman<Width>::decode()) {
                decoded += character;
            }
            return decoded;
        }

        return {reinterpret_cast<const char *>(body.data()), body.size()};  // FIXME(clang-tidy): reinterpret_cast usage
    };
};

} // namespace io::shared_codec::raw

#ifdef CONGELADO_TEST
namespace io::shared_codec::raw::tests {
using namespace boost::ut;

suite<"Atom::encode_int/decode_int"> atom_int_suite = [] {
    "single-octet value round-trips"_test = [] {
        std::vector<std::uint8_t> bytes;
        Atom<>::encode_int(10U, 5U, std::uint8_t{0}, std::back_inserter(bytes));

        expect(bytes.size() == 1);

        std::size_t pos = 0;
        auto result = Atom<>::decode_int(bytes, pos, 5U);
        expect(result.value() == 10U);
        expect(pos == 1U);
    };

    "multi-octet value round-trips (RFC 7541 C.1.2 vector)"_test = [] {
        std::vector<std::uint8_t> bytes;
        Atom<>::encode_int(1337U, 5U, std::uint8_t{0}, std::back_inserter(bytes));

        expect(bytes.size() == 3);
        expect(bytes[0] == 0x1F);
        expect(bytes[1] == 0x9A);
        expect(bytes[2] == 0x0A);

        std::size_t pos = 0;
        auto result = Atom<>::decode_int(bytes, pos, 5U);
        expect(result.value() == 1337U);
        expect(pos == 3U);
    };

    "decode_int captures prefix metadata bits when PrefixOffset > 0"_test = [] {
        // prefix_size=5, metadata bits = 0b011, value = 10 (< 2^5-1, single octet).
        std::vector<std::uint8_t> bytes{static_cast<std::uint8_t>((0x03U << 5) | 10U)};

        std::size_t pos = 0;
        auto result = Atom<>::decode_int<1>(bytes, pos, 5U);
        expect(result.value() == 10U);
        expect(result.is_static());
        expect(result.is_never_indexed());
    };

    "encode_int rejects an out-of-range prefix size"_test = [] {
        std::vector<std::uint8_t> bytes;
        expect(throws<std::invalid_argument>(
            [&] { Atom<>::encode_int(1U, 0U, std::uint8_t{0}, std::back_inserter(bytes)); }));
        expect(throws<std::invalid_argument>(
            [&] { Atom<>::encode_int(1U, 9U, std::uint8_t{0}, std::back_inserter(bytes)); }));
    };

    "decode_int rejects an out-of-range prefix size"_test = [] {
        std::vector<std::uint8_t> bytes{0x00};
        std::size_t pos = 0;
        expect(throws<std::invalid_argument>([&] { Atom<>::decode_int(bytes, pos, 0U); }));
    };

    "decode_int on an empty buffer throws TruncatedDataError"_test = [] {
        std::vector<std::uint8_t> bytes;
        std::size_t pos = 0;
        expect(throws<error::http::TruncatedDataError>([&] { Atom<>::decode_int(bytes, pos, 5U); }));
    };

    "decode_int on a stream with no terminal continuation byte throws IntegerDecodeError"_test = [] {
        // prefix_size=3 (MASK=7): first octet maxes the prefix, then five continuation bytes
        // all keep the high bit set, so the value never terminates and either overflows
        // UInt's bit width or runs off the end of the buffer — both raise IntegerDecodeError.
        std::vector<std::uint8_t> bytes{0x07, 0x80, 0x80, 0x80, 0x80, 0x80};
        std::size_t pos = 0;
        expect(throws<error::http::IntegerDecodeError>([&] { Atom<>::decode_int(bytes, pos, 3U); }));
    };
};

suite<"Atom::encode_string/decode_string"> atom_string_suite = [] {
    "raw string round-trips without a Huffman coder"_test = [] {
        huffman::Huffman<4> coder;
        std::string original = "hello";

        std::vector<std::uint8_t> bytes;
        Atom<>::encode_string(nullptr, original, std::back_inserter(bytes));

        std::size_t pos = 0;
        std::string decoded = Atom<>::decode_string(coder, bytes, pos);

        expect(decoded == original);
        expect(pos == bytes.size());
    };

    "Huffman-encoded string round-trips"_test = [] {
        huffman::Huffman<4> coder;
        std::string original = "www.example.com";

        std::vector<std::uint8_t> bytes;
        Atom<>::encode_string(&coder, original, std::back_inserter(bytes));

        std::size_t pos = 0;
        std::string decoded = Atom<>::decode_string(coder, bytes, pos);

        expect(decoded == original);
        expect(pos == bytes.size());
    };

    "decode_string on an empty buffer throws TruncatedDataError"_test = [] {
        huffman::Huffman<4> coder;
        std::vector<std::uint8_t> bytes;
        std::size_t pos = 0;
        expect(throws<error::http::TruncatedDataError>([&] { Atom<>::decode_string(coder, bytes, pos); }));
    };

    "decode_string rejects a length exceeding MAX_LENGTH"_test = [] {
        huffman::Huffman<4> coder;
        std::vector<std::uint8_t> bytes;
        Atom<>::encode_int(70000U, 7U, PrefixHelper::HUFFMAN_DISABLED, std::back_inserter(bytes));

        std::size_t pos = 0;
        expect(throws<error::http::StringDecodeError>([&] { Atom<>::decode_string(coder, bytes, pos); }));
    };
};

} // namespace io::shared_codec::raw::tests
#endif
