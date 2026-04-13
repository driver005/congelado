export module io_layer_shared:codec;

import std;

export namespace transport::shared_layer {

template <std::unsigned_integral UInt = std::uint32_t>
class Atom {
  private:
    static constexpr std::uint64_t LIMIT_6BIT = 64;          // 2^6
    static constexpr std::uint64_t LIMIT_14BIT = 16384;      // 2^14
    static constexpr std::uint64_t LIMIT_30BIT = 1073741824; // 2^30

  public:
    // --- Standard Fixed-Length Big-Endian ---

    template <std::output_iterator<std::uint8_t> Out>
    static void write_big_endian(Out &out, const UInt &val, const std::uint8_t &bytes) {
        for (std::uint8_t i = 0; i < bytes; ++i) {
            *out++ = static_cast<std::uint8_t>(val >> (8 * (bytes - 1 - i)));
        }
    }

    static std::uint64_t read_big_endian(std::span<const std::uint8_t> data) {
        std::uint64_t val = 0;
        for (const auto byte : data) {
            val = (val << 8) | byte;
        }
        return val;
    }

    // --- QUIC Variable-Length Integer encodeing/decodeing ---

    template <std::output_iterator<std::uint8_t> Out>
    static void write_varint(Out &out, const UInt &val) {
        std::uint8_t prefix = 0;
        std::size_t length = 0;

        if (val < LIMIT_6BIT) {
            prefix = 0x00;
            length = 1;
        } else if (val < LIMIT_14BIT) {
            prefix = 0x01;
            length = 2;
        } else if (val < LIMIT_30BIT) {
            prefix = 0x02;
            length = 4;
        } else {
            prefix = 0x03;
            length = 8;
        }

        // Write first byte with 2-bit prefix and the first chunk of data
        // (6 bits, 14 bits, 30 bits, or 62 bits usable)
        *out++ = static_cast<std::uint8_t>((val >> (8 * (--length))) | (prefix << 6));

        while (length > 0) {
            *out++ = static_cast<std::uint8_t>(val >> (8 * --length));
        }
    }

    static std::pair<UInt, std::uint8_t> read_varint(std::span<const std::uint8_t> data) {
        if (data.empty())
            return {0, 0};

        std::uint8_t first_byte = data[0];
        std::uint8_t prefix = first_byte >> 6;
        std::uint8_t length = 1 << prefix;

        if (data.size() < length)
            return {0, 0};

        UInt value = first_byte & 0x3F;
        std::uint8_t idx = 1;

        while (idx < length) {
            value = (value << 8) + data[idx++];
        }

        return {value, length};
    }
};

} // namespace transport::shared_layer
