module;

#include <cassert>

export module shared:atom;
import error;
import std;
import :types;
import :huffman;

export namespace codec::shared::raw {

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

    template <std::output_iterator<std::uint8_t> Out, typename PrefixType>
        requires CastableToUint8<PrefixType>
    static void encode_int(UInt data, std::uint8_t prefix_size, PrefixType prefix_data, Out out) {
        if (prefix_size < 1 || prefix_size > 8)
            throw std::invalid_argument("prefix must be in range [1,8] (inclusive)");

        // 2^N - 1
        const std::uint32_t max_prefix = static_cast<std::uint32_t>((1u << prefix_size) - 1u);
        // Mask to clear prefix bits.
        const std::uint8_t mask = static_cast<std::uint8_t>(max_prefix);

        // Extract Enum type
        constexpr std::uint8_t prefix =
            std::is_same_v<PrefixType, std::uint8_t> ? prefix_data : static_cast<std::uint8_t>(prefix_data);

        // if I < 2^N - 1, encode I on N bits
        //    else
        //        encode (2^N - 1) on N bits
        //        I = I - (2^N - 1)
        //        while I >= 128
        //             encode (I % 128 + 128) on 8 bits
        //             I = I / 128
        //        encode I on 8 bits
        if (data < max_prefix) {
            // Value fits in one octet, the function to calculate the max value is:  2 ^ prefix - 1 (inclusive).
            *out++ = (prefix & ~mask) | static_cast<std::uint8_t>(data);
        } else {
            // Value is exeds the limit of one octet. In the following we need to encode the value in multiple octets.

            // Encode the prefix as well as the max value for the prefix (all bits of prefix must be set to 1).
            *out++ = static_cast<std::uint8_t>(prefix | mask);

            // Suvtract the max value of the prefix from the value, as we have already encoded that part in the first
            // octet.
            data -= max_prefix;

            // while data > 127 (2 ^ 7 - 1)
            while (data > 0x7F) {
                *out++ = static_cast<std::uint8_t>((data % 0x80) + 0x80);
                data >>= 7;
            }

            // Since the data is less than (2 ^ 7) we can be sure that the MSB is 0.
            *out++ = static_cast<std::uint8_t>(data);
        }
    }

    static UInt decode_int(std::span<const std::uint8_t> data, std::size_t &pos, std::uint8_t prefix_size) {
        if (prefix_size < 1 || prefix_size > 8)
            throw std::invalid_argument{"prefix must be in range [1,8] (inclusive)"};

        if (pos >= data.size())
            throw error::http::TruncatedDataError{};

        // 2^N - 1
        const UInt mask = static_cast<UInt>((1u << prefix_size) - 1u);

        // Read the data from the first octet, masking out the prefix bits.
        UInt value = static_cast<UInt>(data[pos++] & mask);

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
        if (value < mask)
            // Value is less than the max value of the prefix, so we can return it directly.
            return value;

        // Bit shift needed to align the bytes inside of uint32_t.
        std::uint8_t bit_shift = 0;
        constexpr std::uint8_t digits = std::numeric_limits<UInt>::digits;

        while (true) {
            if (pos >= data.size())
                throw error::http::IntegerDecodeError{"truncated continuation"};

            if (bit_shift >= digits)
                throw error::http::IntegerDecodeError{"overflow"};

            const std::uint8_t continuation_byte = data[pos++];

            // Get data execpt for the first continuation bit. Then shift into the correct postion inside of the Uint
            // and add to the value.
            const UInt shifted = static_cast<UInt>(continuation_byte & 0x7Fu) << bit_shift;

            // Check for overflow before adding the shifted value to the total value.
            // Clang and GCC only sorry!
            if (__builtin_add_overflow(value, shifted, &value))
                throw error::http::IntegerDecodeError{"overflow"};

            bit_shift += 7;

            // MSB = 0 → last continuation byte
            if (!(continuation_byte & 0x80u))
                break;
        }

        return value;
    };

    //   0   1   2   3   4   5   6   7
    // +---+---+---+---+---+---+---+---+
    // | H |    String Length (7+)     |
    // +---+---------------------------+
    // |  String Data (Length octets)  |
    // +-------------------------------+
    // H: Huffman flag (1 bit) –> 1 = Huffman-encoded | 0 = raw string
    static constexpr std::size_t kMaxLength = 65536;

    template <std::output_iterator<std::uint8_t> Out>
    static void encode_string(const huffman::Huffman<Width> *huffman, std::string_view str, Out out) {
        if (huffman) {
            std::vector<std::uint8_t> encoded;
            encoded.reserve((str.size() * 5 + 7) / 8);

            // TODO: implement Huffman by passing a ref via props which acts as flag isntead of using use_huffman
            huffman->encode(str, std::back_inserter(encoded));

            // TODO: wait for cpp26 and use std::narrowing_cast
            assert(encoded.size() <= std::numeric_limits<UInt>::max());
            // Write length then Huffman bytes directly into out.
            encode_int<Out>(encoded.size(), 7, PrefixHelper::HuffmanEnabled, out);

            for (std::uint8_t byte : encoded) {
                *out++ = byte;
            }
        } else {
            // TODO: wait for cpp26 and use std::narrowing_cast
            assert(str.size() <= std::numeric_limits<UInt>::max());
            encode_int<Out>(str.size(), 7, PrefixHelper::HuffmanDisabled, out);

            for (std::uint8_t ch : str) {
                *out++ = ch;
            }
        }
    };

    static std::string decode_string(const huffman::Huffman<Width> &h, std::span<const std::uint8_t> data,
                                     std::size_t &pos) {
        if (pos >= data.size())
            throw error::http::TruncatedDataError{};

        // Check the Huffman flag (H-bit) in the first byte.
        const bool h_flag = data[pos] & 0x80;

        // String length is encoded as a 7-bit prefix integer.
        const auto length = decode_int(data, pos, 7);

        if (length > kMaxLength)
            throw error::http::StringDecodeError{"exceeds kMaxLength"};
        if (pos + length > data.size())
            throw error::http::StringDecodeError("truncated string data");

        // Get the string data as a span, then advance the position.
        std::span<const std::uint8_t> body = data.subspan(pos, length);
        pos += length;

        if (h_flag)
            return h.decode(body);

        return std::string(reinterpret_cast<const char *>(body.data()), body.size());
    };
};

} // namespace codec::shared::raw
