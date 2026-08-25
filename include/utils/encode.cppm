export module utils_encode;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::encode {

/**
 * @brief Percent-encodes a byte range for safe embedding in a URL — unreserved characters
 * (`A-Za-z0-9-._~`) pass through literal, everything else gets `%XX`-escaped. Standard motion for
 * query params/path segments carrying bytes a URL can't just swallow raw.
 * @note This is a free function sitting directly in `namespace utils::encode`, not a class static
 * method — flagging it since the rest of this codebase's convention is class-only, no free
 * functions. Leaving it as-is per this pass's comment-only scope, not touching the structure.
 * @tparam Range an input range whose elements convert to `unsigned char`.
 * @param range the bytes to encode.
 * @return the percent-encoded string.
 */
template <std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, unsigned char>
[[nodiscard]] std::string url_encode(Range &&range) {
    static constexpr std::string_view HEX = "0123456789ABCDEF";
    std::string out;
    // Reserve up front when the range knows its own size, saves a few reallocs.
    if constexpr (std::ranges::sized_range<Range>) {
        out.reserve(std::ranges::size(range));
    }
    // Walk every byte — unreserved characters pass through as-is, everything else gets
    // `%`-escaped as two hex digits.
    for (auto elem : std::forward<Range>(range)) {
        const auto BYTE = static_cast<unsigned char>(elem);
        if ((BYTE >= 'A' && BYTE <= 'Z') || (BYTE >= 'a' && BYTE <= 'z') ||
            (BYTE >= '0' && BYTE <= '9') || BYTE == '-' || BYTE == '.' ||
            BYTE == '_' || BYTE == '~') {
            out += static_cast<char>(BYTE);
        } else {
            out += '%';
            out += HEX[(BYTE >> 4) & 0xF];  // FIXME(clang-tidy): unchecked operator[], consider .at()
            out += HEX[BYTE & 0xF];  // FIXME(clang-tidy): unchecked operator[], consider .at()
        }
    }
    return out;
}

/**
 * @brief Base64-encodes a byte range with the standard alphabet, padding with `=` up to a
 * multiple of 4 characters. Textbook base64, bet — 6 bits out for every 8 bits in, buffered
 * through a running bit accumulator (`val`/`bits`) as it walks the range.
 * @note Same deal as url_encode() right above — free function, not a class static. Flagging the
 * convention mismatch, not touching the structure in this comment-only pass.
 * @tparam Range an input range whose elements convert to `unsigned char`.
 * @param range the bytes to encode.
 * @return the base64-encoded string.
 */
template <std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, unsigned char>
[[nodiscard]] std::string base64_encode(Range &&range) {
    static constexpr std::string_view TABLE =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    if constexpr (std::ranges::sized_range<Range>) {
        out.reserve(4 * ((std::ranges::size(range) + 2) / 3));
    }
    std::uint32_t val = 0;
    int bits = 0;
    // Buffer incoming bytes 8 bits at a time into `val`, draining 6-bit groups out into base64
    // characters as soon as there's enough accumulated to do so.
    for (auto elem : std::forward<Range>(range)) {
        const auto BYTE = static_cast<unsigned char>(elem);
        val = (val << 8) | BYTE;
        bits += 8;
        while (bits >= 6) {
            bits -= 6;
            out += TABLE[(val >> bits) & 0x3F];  // FIXME(clang-tidy): unchecked operator[], consider .at()
        }
    }
    // Leftover bits (1 or 2 bytes' worth) still need to go out as one more character, then pad
    // with `=` up to a multiple of 4.
    if (bits > 0) {
        out += TABLE[(val << (6 - bits)) & 0x3F];  // FIXME(clang-tidy): unchecked operator[], consider .at()
        while (out.size() % 4 != 0) {
            out += '=';
        }
    }
    return out;
}

} // namespace utils::encode

#ifdef CONGELADO_TEST
namespace utils::encode::tests {
using namespace boost::ut;

suite<"url_encode"> url_encode_suite = [] {
    "unreserved characters pass through untouched"_test = [] {
        expect(url_encode(std::string_view{"abcXYZ019-._~"}) == "abcXYZ019-._~");
    };
    "reserved characters get percent-escaped uppercase hex"_test = [] {
        expect(url_encode(std::string_view{"a b/c"}) == "a%20b%2Fc");
    };
    "empty range yields empty string"_test = [] {
        expect(url_encode(std::string_view{""}).empty());
    };
};

suite<"base64_encode"> base64_encode_suite = [] {
    "matches RFC 4648 test vectors"_test = [] {
        expect(base64_encode(std::string_view{""}).empty());
        expect(base64_encode(std::string_view{"f"}) == "Zg==");
        expect(base64_encode(std::string_view{"fo"}) == "Zm8=");
        expect(base64_encode(std::string_view{"foo"}) == "Zm9v");
        expect(base64_encode(std::string_view{"foob"}) == "Zm9vYg==");
        expect(base64_encode(std::string_view{"fooba"}) == "Zm9vYmE=");
        expect(base64_encode(std::string_view{"foobar"}) == "Zm9vYmFy");
    };
    "output length is always a multiple of 4"_test = [] {
        expect(base64_encode(std::string_view{"x"}).size() % 4 == 0);
        expect(base64_encode(std::string_view{"xy"}).size() % 4 == 0);
        expect(base64_encode(std::string_view{"xyz"}).size() % 4 == 0);
    };
};

} // namespace utils::encode::tests
#endif
