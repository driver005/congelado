module;
#include <cstddef>
#include <cstdint>
#include <utility>
export module io_codec_shared:lowlevel;

import std;
import io_error;
import utils_codec;
import :types;
import :huffman;

export namespace io::shared_codec::lowlevel {

template <std::unsigned_integral UInt = std::uint32_t>
class EncodeIntView : public std::ranges::view_interface<EncodeIntView<UInt>> {
  public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using value_type = std::byte;
        using difference_type = std::ptrdiff_t;
        using reference = std::byte;

        Iterator(UInt data, std::uint8_t prefix_size, std::uint8_t prefix, std::size_t pos = 0)
            : m_data{data}, m_max_prefix{static_cast<UInt>((1U << prefix_size) - 1U)}, m_prefix{prefix}, m_pos{pos} {}

        reference operator*() const noexcept {
            const auto MASK = static_cast<std::uint8_t>(m_max_prefix);
            if (m_pos == 0) {
                if (m_data < m_max_prefix) {
                    return static_cast<std::byte>((m_prefix & ~MASK) | static_cast<std::uint8_t>(m_data));
                }
                return static_cast<std::byte>(m_prefix | MASK);
            }

            UInt remainder = static_cast<UInt>(m_data - m_max_prefix) >> (7U * (m_pos - 1U));

            const bool MORE = (remainder >> 7U) > 0U;
            return MORE ? static_cast<std::byte>((remainder & 0x7FU) | 0x80U)
                        : static_cast<std::byte>(remainder & 0x7FU);
        }

        Iterator &operator++() noexcept {
            ++m_pos;
            return *this;
        }
        Iterator operator++(int) noexcept {
            auto old = *this;
            ++*this;
            return old;
        }
        bool operator==(const Iterator &other) const noexcept { return m_pos == other.m_pos; }

      private:
        UInt m_data;
        UInt m_max_prefix;
        std::uint8_t m_prefix;
        std::size_t m_pos;
    };

    template <typename PrefixType>
        requires CastableToUint8<PrefixType>
    explicit EncodeIntView(UInt data, std::uint8_t prefix_size, PrefixType prefix_data)
        : m_data{data}, m_prefix_size{prefix_size},
          m_prefix{std::is_same_v<PrefixType, std::uint8_t> ? prefix_data : static_cast<std::uint8_t>(prefix_data)},
          m_size{compute_size(data, prefix_size)} {
        if (prefix_size < 1 || 8 < prefix_size) {
            throw std::invalid_argument{"prefix must be in range [1,8] (inclusive)"};
        }
    }

    [[nodiscard]] Iterator begin() const noexcept { return {m_data, m_prefix_size, m_prefix, 0}; }
    [[nodiscard]] Iterator end() const noexcept { return {m_data, m_prefix_size, m_prefix, m_size}; }
    [[nodiscard]] std::size_t size() const noexcept { return m_size; }

  private:
    UInt m_data;
    std::uint8_t m_prefix_size;
    std::uint8_t m_prefix;
    std::size_t m_size;

    static constexpr std::size_t compute_size(UInt data, std::uint8_t prefix_size) noexcept {
        const UInt MAX_PREFIX = static_cast<UInt>((1U << prefix_size) - 1U);
        if (data < MAX_PREFIX) {
            return 1;
        }
        data -= MAX_PREFIX;
        std::size_t comp_size = 2;
        while (data > 0x7FU) {
            data >>= 7;
            ++comp_size;
        }
        return comp_size;
    }
};

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
template <std::unsigned_integral UInt = std::uint32_t>
struct EncodeIntAdaptor : std::ranges::range_adaptor_closure<EncodeIntAdaptor<UInt>> {
    template <typename PrefixType>
        requires CastableToUint8<PrefixType>
    explicit constexpr EncodeIntAdaptor(std::uint8_t prefix_size, PrefixType prefix_data) noexcept
        : m_prefix_size{prefix_size}, m_prefix{static_cast<std::uint8_t>(prefix_data)} {}

    [[nodiscard]] EncodeIntView<UInt> operator()(UInt data) const {
        return EncodeIntView<UInt>{data, m_prefix_size, m_prefix};
    }

  private:
    std::uint8_t m_prefix_size;
    std::uint8_t m_prefix;
};

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
template <std::unsigned_integral UInt = std::uint32_t, std::size_t PrefixOffset = 0>
struct DecodeIntAdaptor : std::ranges::range_adaptor_closure<DecodeIntAdaptor<UInt, PrefixOffset>> {
    explicit constexpr DecodeIntAdaptor(std::uint8_t prefix_size) noexcept : m_prefix_size{prefix_size} {}

    template <std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] DecodeIntResult<UInt> operator()(R &&data) const {
        if (m_prefix_size < 1 || 8 < m_prefix_size) {
            throw std::invalid_argument{"prefix must be in range [1,8] (inclusive)"};
        }

        auto all = std::forward<R>(data);
        // const auto TOTAL = static_cast<std::size_t>(std::ranges::distance(all));

        if (std::ranges::empty(all)) {
            throw error::http::TruncatedDataError{};
        }

        const auto FIRST_BYTE = all | std::views::take(1) | utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};

        std::size_t consumed = 1;

        std::uint8_t prefix_metadata = 0;

        if constexpr (PrefixOffset > 0) {
            prefix_metadata = static_cast<std::uint8_t>(FIRST_BYTE >> m_prefix_size);
        }

        const UInt MASK = static_cast<UInt>((1U << m_prefix_size) - 1U);
        UInt value = static_cast<UInt>(FIRST_BYTE & MASK);

        if (value < MASK) {
            return {value, prefix_metadata, consumed};
        }

        auto sub_range = all | std::views::drop(consumed);

        auto terminal_it = std::ranges::find_if(sub_range, [&](auto byte) {
            const std::size_t SHIFT = consumed * 7U;

            // Guard against bit-shift overflow (e.g., shifting by 32+ on a uint32_t)
            if (SHIFT >= std::numeric_limits<UInt>::digits) {
                throw error::http::IntegerDecodeError{"overflow: integer exceeds type capacity"};
            }

            const UInt SHIFTED = (std::to_integer<UInt>(byte) & 0x7FU) << SHIFT;

            // Check for overflow before adding the shifted value to the total value.
            // Clang and GCC only sorry!
            if (__builtin_add_overflow(value, SHIFTED, &value)) {
                throw error::http::IntegerDecodeError{"overflow: accumulation wrapped around"};
            }

            ++consumed;

            // Terminate the iteration instantly when the MSB is 0 (terminal byte)
            return (std::to_integer<std::uint8_t>(byte) & 0x80U) == 0;
        });

        // If we exhaust the range but no terminal byte is found, the stream is cut off.
        if (terminal_it == std::ranges::end(sub_range)) {
            throw error::http::IntegerDecodeError{"truncated continuation: missing final byte"};
        }

        return {value, prefix_metadata, consumed};
    }

  private:
    std::uint8_t m_prefix_size;
};

template <int Width>
struct EncodeStringAdaptor : std::ranges::range_adaptor_closure<EncodeStringAdaptor<Width>> {
    explicit constexpr EncodeStringAdaptor(bool huffman_encode, std::uint8_t prefix_size = 7U) noexcept
        : m_huffman{huffman_encode}, m_prefix_size{prefix_size} {}

    template <std::ranges::forward_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R &&data) const {
        if (m_huffman) {
            const auto LEN_FIRST = std::ranges::distance(data);

            auto encoded = std::forward<R>(data) | huffman::HuffmanEncodeAdaptor{};

            const auto LEN = static_cast<std::uint32_t>(std::ranges::distance(encoded) - LEN_FIRST);

            return std::views::concat(
                LEN | EncodeIntAdaptor<std::uint32_t>{m_prefix_size, PrefixHelper::HUFFMAN_ENABLED}, encoded);
        }

        const auto LEN = static_cast<std::uint32_t>(std::ranges::distance(data));

        return std::views::concat(LEN | EncodeIntAdaptor<std::uint32_t>{m_prefix_size, PrefixHelper::HUFFMAN_DISABLED},
                                  std::forward<R>(data));
    }

  private:
    bool m_huffman;
    std::uint8_t m_prefix_size;
};


template <int Width>
struct DecodeStringAdaptor : std::ranges::range_adaptor_closure<DecodeStringAdaptor<Width>> {
    explicit constexpr DecodeStringAdaptor() noexcept = default;

    template <std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::pair<std::string, std::size_t> operator()(R &&data) const {
        auto all = std::views::all(std::forward<R>(data));
        const auto TOTAL = static_cast<std::size_t>(std::ranges::distance(all));

        if (TOTAL == 0) {
            throw error::http::TruncatedDataError{};
        }

        const bool H_FLAG = (std::to_integer<std::uint8_t>(all[0]) & 0x80U) != 0U;

        const auto LENGTH = all | DecodeIntAdaptor<std::uint32_t>{7U};
        const auto HEADER = LENGTH.consumed();

        if (LENGTH.value() > MAX_LENGTH) {
            throw error::http::StringDecodeError{"exceeds kMaxLength"};
        }
        if (HEADER + LENGTH.value() > TOTAL) {
            throw error::http::StringDecodeError{"truncated string data"};
        }

        auto body = all | std::views::drop(HEADER) | std::views::take(LENGTH.value());

        const std::size_t CONSUMED = HEADER + LENGTH.value();

        if (H_FLAG) {
            return {body | huffman::HuffmanDecodeAdaptor<Width>{} | std::ranges::to<std::string>(), CONSUMED};
        }

        return {body | std::views::transform([](std::byte byte) noexcept {
                    return static_cast<char>(std::to_integer<std::uint8_t>(byte));
                }) | std::ranges::to<std::string>(),
                CONSUMED};
    }

  private:
    static constexpr std::size_t MAX_LENGTH = 65536;
};

} // namespace io::shared_codec::lowlevel
