export module utils_encode;

import std;

export namespace utils::encode {

template <std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, unsigned char>
[[nodiscard]] std::string url_encode(Range &&range) {
    static constexpr std::string_view HEX = "0123456789ABCDEF";
    std::string out;
    if constexpr (std::ranges::sized_range<Range>) {
        out.reserve(std::ranges::size(range));
    }
    for (auto elem : range) {
        const auto byte = static_cast<unsigned char>(elem);
        if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') ||
            (byte >= '0' && byte <= '9') || byte == '-' || byte == '.' ||
            byte == '_' || byte == '~') {
            out += static_cast<char>(byte);
        } else {
            out += '%';
            out += HEX[(byte >> 4) & 0xF];
            out += HEX[byte & 0xF];
        }
    }
    return out;
}

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
    for (auto elem : range) {
        const auto byte = static_cast<unsigned char>(elem);
        val = (val << 8) | byte;
        bits += 8;
        while (bits >= 6) {
            bits -= 6;
            out += TABLE[(val >> bits) & 0x3F];
        }
    }
    if (bits > 0) {
        out += TABLE[(val << (6 - bits)) & 0x3F];
        while (out.size() % 4 != 0) {
            out += '=';
        }
    }
    return out;
}

} // namespace utils::encode
