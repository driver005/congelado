export module io_layer_shared:codec;

import std;
import shared;

export namespace io::shared_layer {

template <std::unsigned_integral UInt = std::uint32_t>
class Atom {
  private:
    static constexpr std::uint64_t LIMIT_6BIT = 64;          // 2^6
    static constexpr std::uint64_t LIMIT_14BIT = 16384;      // 2^14
    static constexpr std::uint64_t LIMIT_30BIT = 1073741824; // 2^30

  public:
    // --- Standard Fixed-Length Big-Endian ---

    template <shared::ByteRangeWriter R>
    static void write_big_endian(R &&range, UInt val) {
        constexpr std::size_t bytes = sizeof(UInt);
        std::size_t i = 0uz;

        std::ranges::generate(std::views::take(range, bytes), [&i, val]() {
            const auto shift = 8uz * (bytes - 1uz - i++);
            return static_cast<std::byte>(val >> shift);
        });
    }

    template <shared::ByteIteratorWriter Out>
    static void write_big_endian(Out &out, UInt val) {
        constexpr std::size_t bytes = sizeof(UInt);
        write_big_endian(std::views::counted(out, bytes), val);
        std::advance(out, bytes);
    }

    template <shared::ByteRangeReader R>
    static UInt read_big_endian(R &&data) {
        constexpr std::size_t bytes = sizeof(UInt);
        if (std::ranges::size(data) > bytes) {
            throw std::runtime_error("Too many bytes for uint64_t");
        }

        UInt val = 0;
        for (const auto byte : data) {
            val = (val << 8) | std::to_integer<std::uint8_t>(byte);
        }

        return val;
    }

    template <shared::ByteIteratorReader It>
    UInt read_big_endian(It &it) {
        constexpr std::size_t bytes = sizeof(UInt);
        return static_cast<UInt>(read_big_endian(it, bytes));
    }

    // --- QUIC Variable-Length Integer encodeing/decodeing ---

    template <shared::ByteRangeWriter R>
    static void write_varint(R &&range, UInt val) {
        auto [prefix, length] = varint_encoding(val);
        write_varint_helper(std::forward<R>(range), val, prefix, length);
    }

    template <shared::ByteIteratorWriter Out>
    static void write_varint(Out &out, UInt val) {
        auto [prefix, length] = varint_encoding(val);
        write_varint(std::views::counted(out, length), val, prefix, length);
        std::advance(out, length);
    }

    template <shared::ByteRangeReader R>
    static std::pair<UInt, std::uint8_t> read_varint(R &&data) {
        if (std::ranges::empty(data))
            return {0, 0};

        auto view = data | std::views::transform([](std::byte byte) { return std::to_integer<std::uint8_t>(byte); });

        std::uint8_t first_byte = *std::ranges::begin(view);
        std::uint8_t prefix = first_byte >> 6;
        std::uint8_t length = 1 << prefix;
        UInt value = first_byte & 0x3F;

        for (auto byte : view | std::views::drop(1) | std::views::take(length - 1)) {
            value = (value << 8) | byte;
        }

        return {value, length};
    }

    template <shared::ByteIteratorReader It>
    static std::pair<UInt, std::uint8_t> read_varint(It it, std::size_t count) {
        auto result = read_varint(std::views::counted(it, count));
        std::advance(it, result.second);
        return result;
    }

  private:
    static constexpr std::pair<std::uint8_t, std::size_t> varint_encoding(UInt val) {
        if (val < LIMIT_6BIT)
            return {0x00, 1};
        if (val < LIMIT_14BIT)
            return {0x01, 2};
        if (val < LIMIT_30BIT)
            return {0x02, 4};
        return {0x03, 8};
    }

    template <shared::ByteRangeWriter R>
    static void write_varint_helper(R &&range, UInt val, std::uint8_t prefix, std::size_t length) {
        // Write first byte with 2-bit prefix and the first chunk of data
        // (6 bits, 14 bits, 30 bits, or 62 bits usable)
        *std::ranges::begin(range) = static_cast<std::byte>((val >> (8 * (length - 1))) | (prefix << 6));

        for (auto [i, b] : range | std::views::drop(1) | std::views::take(length - 1) | std::views::enumerate) {
            b = static_cast<std::byte>(val >> (8 * (length - 2 - i)));
        }
    };
};

} // namespace io::shared_layer
