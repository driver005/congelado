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
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared_codec::lowlevel {

template<std::unsigned_integral UInt = std::uint32_t>
class EncodeIntView : public std::ranges::view_interface<EncodeIntView<UInt>>
{
public:
    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using value_type = std::byte;
        using reference = std::byte;
        using difference_type = std::ptrdiff_t;

        /** @brief Builds a sentinel/empty iterator — for `= default` scenarios only. */
        Iterator() = default;

        /**
         * @brief Positions the iterator at octet `pos` of the lazily-computed prefix-int
         * encoding — each `*` computes its octet on demand, nothing's precomputed or stored.
         * @param data the integer value being encoded.
         * @param prefix_size how many bits of the first octet are prefix bits.
         * @param prefix the fixed high bits to OR into the first octet.
         * @param pos which octet this iterator currently points at (0 = begin(), size() =
         * end()).
         */
        Iterator(UInt data, std::uint8_t prefix_size, std::uint8_t prefix, std::size_t pos = 0) :
            m_data{data},
            m_max_prefix{static_cast<UInt>((1U << prefix_size) - 1U)},
            m_prefix{prefix},
            m_pos{pos}
        {
        }

        /**
         * @brief Computes and returns the octet at the current position — first octet holds the
         * prefix (+ inline value if it fits), every octet after is a 7-bit continuation chunk
         * with the high bit set unless it's the last one.
         * @return the computed byte at `m_pos`.
         */
        reference operator*() const noexcept
        {
            const auto MASK = static_cast<std::uint8_t>(m_max_prefix);
            // First octet is special — either the value fits inline alongside the prefix bits,
            // or (if not) the octet is just the prefix maxed out, with the value's overflow
            // spilling into continuation octets.
            if (m_pos == 0) {
                if (m_data < m_max_prefix) {
                    return static_cast<std::byte>(
                        (m_prefix & ~MASK) | static_cast<std::uint8_t>(m_data)
                    );
                }
                return static_cast<std::byte>(m_prefix | MASK);
            }

            // Continuation octet — shift out the 7-bit chunk for this position and set the
            // high bit if there's more to come after it.
            UInt remainder = static_cast<UInt>(m_data - m_max_prefix) >> (7U * (m_pos - 1U));

            const bool MORE = (remainder >> 7U) > 0U;
            return MORE ? static_cast<std::byte>((remainder & 0x7FU) | 0x80U)
                        : static_cast<std::byte>(remainder & 0x7FU);
        }

        /** @brief Advances to the next octet position. @return `*this`, one octet further
         * along. */
        Iterator& operator++() noexcept
        {
            ++m_pos;
            return *this;
        }

        /**
         * @brief Postfix increment — returns a copy of the pre-increment state.
         * @return the iterator's prior position.
         */
        Iterator operator++(int) noexcept
        {
            auto old = *this;
            ++*this;
            return old;
        }

        /**
         * @brief Position equality — only `m_pos` matters since both iterators share the same
         * fixed encoding parameters.
         * @param other the iterator to compare against.
         * @return true if both point at the same octet index.
         */
        bool operator==(const Iterator& other) const noexcept
        {
            return m_pos == other.m_pos;
        }

    private:
        UInt m_data;
        UInt m_max_prefix;
        std::uint8_t m_prefix{};
        std::size_t m_pos{};
    };

    /**
     * @brief Builds a lazy view over the prefix-int encoding of `data` — no bytes get
     * materialized up front, `begin()`/`end()` just hand out an Iterator that computes octets
     * on demand.
     * @tparam PrefixType type of `prefix_data` — must be castable to `std::uint8_t`.
     * @param data the integer value to encode.
     * @param prefix_size how many bits of the first octet belong to the prefix — must be 1-8.
     * @param prefix_data the fixed high bits to OR into the first octet.
     * @throws std::invalid_argument if `prefix_size` is outside [1,8].
     */
    template<typename PrefixType>
        requires CastableToUint8<PrefixType>
    explicit EncodeIntView(UInt data, std::uint8_t prefix_size, PrefixType prefix_data) :
        m_data{data},
        m_prefix_size{prefix_size},
        m_prefix{
            std::is_same_v<PrefixType, std::uint8_t> ? prefix_data
                                                     : static_cast<std::uint8_t>(prefix_data)
        },
        m_size{compute_size(data, prefix_size)}
    {
        if (prefix_size < 1 || 8 < prefix_size) {
            throw std::invalid_argument{"prefix must be in range [1,8] (inclusive)"};
        }
    }

    /**
     * @brief Gets an iterator at the first (prefix) octet.
     * @return an Iterator positioned at octet 0.
     */
    [[nodiscard]] Iterator begin() const noexcept
    {
        return {m_data, m_prefix_size, m_prefix, 0};
    }

    /**
     * @brief Gets the end iterator, positioned one past the last computed octet.
     * @return an Iterator at `m_size`.
     */
    [[nodiscard]] Iterator end() const noexcept
    {
        return {m_data, m_prefix_size, m_prefix, m_size};
    }

    /**
     * @brief Gets the total encoded length, precomputed at construction.
     * @return octet count of the full encoding.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_size;
    }

private:
    UInt m_data;
    std::uint8_t m_prefix_size;
    std::uint8_t m_prefix;
    std::size_t m_size;

    /**
     * @brief Works out how many octets `data` will take once prefix-int encoded, without
     * actually encoding it — same math as encode_int(), just counting instead of writing.
     * @param data the value being sized up.
     * @param prefix_size how many bits of the first octet belong to the prefix.
     * @return total octet count of the encoding: 1 if `data` fits the prefix outright,
     * otherwise 1 + however many 7-bit continuation octets the remainder needs.
     */
    static constexpr std::size_t compute_size(UInt data, std::uint8_t prefix_size) noexcept
    {
        const UInt MAX_PREFIX = static_cast<UInt>((1U << prefix_size) - 1U);
        // Fits entirely in the prefix octet — one byte, done.
        if (data < MAX_PREFIX) {
            return 1;
        }
        // Otherwise the prefix octet is fixed (counts as 1), and every remaining 7-bit group of
        // the overflow needs its own continuation octet.
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
template<std::unsigned_integral UInt = std::uint32_t>
class EncodeIntAdaptor : public std::ranges::range_adaptor_closure<EncodeIntAdaptor<UInt>>
{
public:
    /**
     * @brief Stashes the prefix params so `operator()` can be reused across multiple values —
     * bet, one adaptor, many pipes.
     * @tparam PrefixType type of `prefix_data` — must be castable to `std::uint8_t`.
     * @param prefix_size how many bits of the first octet belong to the prefix.
     * @param prefix_data the fixed high bits to OR into the first octet.
     */
    template<typename PrefixType>
        requires CastableToUint8<PrefixType>
    explicit constexpr EncodeIntAdaptor(std::uint8_t prefix_size, PrefixType prefix_data) noexcept :
        m_prefix_size{prefix_size},
        m_prefix{static_cast<std::uint8_t>(prefix_data)}
    {
    }

    /**
     * @brief Pipe-adaptor call: turns `data | EncodeIntAdaptor{...}` into a lazy EncodeIntView.
     * @param data the value to encode.
     * @return an EncodeIntView over `data` with this adaptor's stashed prefix params.
     */
    [[nodiscard]] EncodeIntView<UInt> operator()(UInt data) const
    {
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
template<std::unsigned_integral UInt = std::uint32_t, std::size_t PrefixOffset = 0>
class DecodeIntAdaptor :
    public std::ranges::range_adaptor_closure<DecodeIntAdaptor<UInt, PrefixOffset>>
{
public:
    /**
     * @brief Stashes the prefix size so `operator()` can be reused across multiple decodes.
     * @param prefix_size how many bits of the first octet belong to the prefix.
     */
    explicit constexpr DecodeIntAdaptor(std::uint8_t prefix_size) noexcept :
        m_prefix_size{prefix_size}
    {
    }

    /**
     * @brief Pipe-adaptor call: turns `range | DecodeIntAdaptor<UInt, PrefixOffset>{...}` into
     * a decoded prefix-int, no cursor needed since it works off `std::ranges` primitives
     * instead of raw pointers.
     * @tparam R the viewable range type piped in.
     * @param data the `std::byte` range to decode from — only as much as needed gets consumed.
     * @return the decoded value, prefix metadata (if `PrefixOffset > 0`), and how many bytes
     * were consumed.
     * @throws std::invalid_argument if `m_prefix_size` is outside [1,8].
     * @throws error::http::TruncatedDataError if `data` is empty.
     * @throws error::http::IntegerDecodeError if the continuation bytes overflow `UInt`'s bit
     * width, or if the range runs out before a terminal byte (MSB clear) shows up — strict
     * decode, no silent wraparound or truncation tolerance. Untrusted wire data in, hard
     * failure on anything malformed.
     */
    template<std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] DecodeIntResult<UInt> operator()(R&& data) const
    {
        // Guard the prefix width up front — everything below assumes it's a sane bit count.
        if (m_prefix_size < 1 || 8 < m_prefix_size) {
            throw std::invalid_argument{"prefix must be in range [1,8] (inclusive)"};
        }

        auto all = std::forward<R>(data);
        // const auto TOTAL = static_cast<std::size_t>(std::ranges::distance(all));

        if (std::ranges::empty(all)) {
            throw error::http::TruncatedDataError{};
        }

        // Pull the first byte and mask out the prefix metadata bits (if the caller wants them).
        const auto FIRST_BYTE =
            all | std::views::take(1) | utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};

        std::size_t consumed = 1;

        std::uint8_t prefix_metadata = 0;

        if constexpr (PrefixOffset > 0) {
            prefix_metadata = static_cast<std::uint8_t>(FIRST_BYTE >> m_prefix_size);
        }

        const UInt MASK = static_cast<UInt>((1U << m_prefix_size) - 1U);
        UInt value = static_cast<UInt>(FIRST_BYTE & MASK);

        // Value fits entirely in the prefix — short-circuit, no continuation bytes to read.
        if (value < MASK) {
            return {value, prefix_metadata, consumed};
        }

        // Otherwise walk continuation bytes one at a time via find_if, folding each one's low 7
        // bits into `value` until a terminal byte (MSB clear) shows up.
        auto sub_range = all | std::views::drop(consumed);

        auto terminal_it = std::ranges::find_if(sub_range, [&](auto byte) {
            const std::size_t SHIFT = (consumed - 1) * 7U;

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

template<int Width>
class EncodeStringAdaptor : public std::ranges::range_adaptor_closure<EncodeStringAdaptor<Width>>
{
public:
    /**
     * @brief Stashes the encoding params for reuse across multiple strings.
     * @param huffman_encode intended to select Huffman vs. raw encoding — see the @warning
     * below, this flag doesn't actually do anything right now.
     * @param prefix_size how many bits of the length's first octet belong to the length prefix;
     * defaults to 7.
     */
    explicit constexpr EncodeStringAdaptor(
        bool huffman_encode, std::uint8_t prefix_size = 7U
    ) noexcept :
        m_huffman{huffman_encode},
        m_prefix_size{prefix_size}
    {
    }

    /**
     * @brief Pipe-adaptor call: turns `range | EncodeStringAdaptor{...}` into a length-prefixed
     * encoded view — concatenates the length prefix with the (currently always raw) body.
     * @warning The Huffman-encoding branch is commented out in the body — `m_huffman` is stored
     * but never actually checked here, so this unconditionally falls through to the raw
     * (`HUFFMAN_DISABLED`) path no matter what was passed to the constructor. Don't rely on
     * this for compression yet; it's a raw-only encoder in its current state.
     * @tparam R the forward range type piped in.
     * @param data the char/byte range to encode.
     * @return a concatenated view: length-prefix octets followed by `data`'s raw bytes.
     */
    template<std::ranges::forward_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] auto operator()(R&& data) const
    {
        // Huffman path's disabled for now — see the @warning above, m_huffman is stored but
        // never actually branched on here.
        // if (m_huffman) {
        //     auto encoded = std::forward<R>(data) | huffman::HuffmanEncodeAdaptor{};
        //
        //     const auto LEN = static_cast<std::uint32_t>(std::ranges::distance(encoded));
        //
        //     return std::views::concat(
        //         LEN | EncodeIntAdaptor<std::uint32_t>{m_prefix_size,
        //         PrefixHelper::HUFFMAN_ENABLED}, encoded);
        // }

        // Raw-only path: measure the range, then concat the length-prefix encoding with the
        // range's own bytes.
        const auto LEN = static_cast<std::uint32_t>(std::ranges::distance(data));

        return std::views::concat(
            LEN | EncodeIntAdaptor<std::uint32_t>{m_prefix_size, PrefixHelper::HUFFMAN_DISABLED},
            std::forward<R>(data)
        );
    }

private:
    bool m_huffman;
    std::uint8_t m_prefix_size;
};

template<int Width>
class DecodeStringAdaptor : public std::ranges::range_adaptor_closure<DecodeStringAdaptor<Width>>
{
public:
    /**
     * @brief Stateless adaptor, defaulted ctor — nothing to configure, `Width` covers Huffman
     * decode width.
     */
    explicit constexpr DecodeStringAdaptor() noexcept = default;

    /**
     * @brief Pipe-adaptor call: turns `range | DecodeStringAdaptor<Width>{}` into a decoded
     * length-prefixed string, Huffman-decoding the body if the H-bit's set.
     * @warning `data` gets forwarded into `all` via `std::views::all(std::forward<R>(data))`,
     * but then `data` gets read again a few lines later (`data | std::views::take(1) | ...`) to
     * check the H-bit. If `R` binds to an owning type where `std::forward` triggers a move
     * (e.g. an rvalue `std::vector<std::byte>`), that second read is off an already-moved-from
     * `data` — that's a real footgun, not a hypothetical one. Pass an lvalue/view-backed range
     * in, not a temporary owning container, or this can read garbage.
     * @tparam R the viewable range type piped in.
     * @param data the `std::byte` range to decode from.
     * @return the decoded string plus how many bytes were consumed (length prefix + body).
     * @throws error::http::TruncatedDataError if `data` is empty.
     * @throws error::http::StringDecodeError if the decoded length exceeds `MAX_LENGTH` (65536)
     * or the body would run past the end of `data`.
     */
    template<std::ranges::viewable_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    [[nodiscard]] std::pair<std::string, std::size_t> operator()(R&& data) const
    {
        auto all = std::views::all(std::forward<R>(data));
        const auto TOTAL = static_cast<std::size_t>(std::ranges::distance(all));

        if (TOTAL == 0) {
            throw error::http::TruncatedDataError{};
        }

        // Peek the H-bit off the first byte to decide raw vs. Huffman decode further down.
        const bool H_FLAG =
            (data | std::views::take(1) | utils::codec::ReadBigEndianAdaptor<std::uint8_t>{}) &
            0x80U;

        // Decode the 7-bit-prefixed length, then validate it against both the sanity cap and
        // what's actually left in the range before trusting it for slicing.
        const auto LENGTH = all | DecodeIntAdaptor<std::uint32_t>{7U};
        const auto HEADER = LENGTH.consumed();

        if (LENGTH.value() > MAX_LENGTH) {
            throw error::http::StringDecodeError{"exceeds kMaxLength"};
        }
        if (HEADER + LENGTH.value() > TOTAL) {
            throw error::http::StringDecodeError{"truncated string data"};
        }

        // Slice out just the body bytes, past the length header.
        auto body = all | std::views::drop(HEADER) | std::views::take(LENGTH.value());

        const std::size_t CONSUMED = HEADER + LENGTH.value();

        // H-bit set means the body's Huffman-compressed — decode it through the adaptor.
        if (H_FLAG) {
            return {
                body | huffman::HuffmanDecodeAdaptor<Width>{} | std::ranges::to<std::string>(),
                CONSUMED
            };
        }

        // Otherwise it's plain bytes — just reinterpret each std::byte as a char.
        return {
            body | std::views::transform([](std::byte byte) noexcept {
                return static_cast<char>(std::to_integer<std::uint8_t>(byte));
            }) | std::ranges::to<std::string>(),
            CONSUMED
        };
    }

private:
    static constexpr std::size_t MAX_LENGTH = 65'536;
};

} // namespace io::shared_codec::lowlevel

#ifdef CONGELADO_TEST
namespace io::shared_codec::lowlevel::tests {
using namespace boost::ut;

suite<"EncodeIntView/EncodeIntAdaptor"> encode_int_view_suite = [] {
    "single-octet value"_test = [] {
        EncodeIntView<std::uint32_t> view{10U, 5U, std::uint8_t{0}};

        expect(view.size() == 1);
        std::vector<std::byte> bytes(view.begin(), view.end());
        expect(bytes.size() == 1);
        // Plain `==` on std::byte forces boost::ut's failure-diagnostic printer to instantiate
        // operator<<(ostream&, std::byte), which doesn't exist — comparing via std::to_integer
        // keeps this a plain integer comparison instead.
        expect(std::to_integer<int>(bytes[0]) == 10);
    };

    "multi-octet value matches the RFC 7541 C.1.2 known vector"_test = [] {
        auto view = 1'337U | EncodeIntAdaptor<std::uint32_t>{5U, std::uint8_t{0}};

        expect(view.size() == 3);
        std::vector<std::byte> bytes(view.begin(), view.end());
        expect(bytes.size() == 3);
        expect(std::to_integer<int>(bytes[0]) == 0x1F);
        expect(std::to_integer<int>(bytes[1]) == 0x9A);
        expect(std::to_integer<int>(bytes[2]) == 0x0A);
    };
};

suite<"DecodeIntAdaptor"> decode_int_adaptor_suite = [] {
    "round-trips through EncodeIntAdaptor"_test = [] {
        auto encoded_view = 1'337U | EncodeIntAdaptor<std::uint32_t>{5U, std::uint8_t{0}};
        std::vector<std::byte> bytes(encoded_view.begin(), encoded_view.end());

        auto result = bytes | DecodeIntAdaptor<std::uint32_t>{5U};
        expect(result.value() == 1'337U);
        expect(result.consumed() == 3U);
    };

    "captures prefix metadata bits when PrefixOffset > 0"_test = [] {
        // prefix_size=5, metadata bits = 0b01 (is_static), value = 10 (single octet).
        std::vector<std::byte> bytes{std::byte{static_cast<std::uint8_t>((0x01U << 5) | 10U)}};

        auto result = bytes | DecodeIntAdaptor<std::uint32_t, 1>{5U};
        expect(result.value() == 10U);
        expect(result.is_static());
        expect(not result.is_never_indexed());
    };

    "rejects an out-of-range prefix size"_test = [] {
        std::vector<std::byte> bytes{std::byte{0x00}};
        expect(throws<std::invalid_argument>([&] {
            auto result = bytes | DecodeIntAdaptor<std::uint32_t>{0U};
        }));
    };

    "an empty range throws TruncatedDataError"_test = [] {
        std::vector<std::byte> bytes;
        expect(throws<error::http::TruncatedDataError>([&] {
            auto result = bytes | DecodeIntAdaptor<std::uint32_t>{5U};
        }));
    };

    "a stream with no terminal continuation byte throws IntegerDecodeError"_test = [] {
        std::vector<std::byte> bytes{std::byte{0x07}, std::byte{0x80}, std::byte{0x80},
                                     std::byte{0x80}, std::byte{0x80}, std::byte{0x80}};
        expect(throws<error::http::IntegerDecodeError>([&] {
            auto result = bytes | DecodeIntAdaptor<std::uint32_t>{3U};
        }));
    };
};

suite<"EncodeStringAdaptor/DecodeStringAdaptor"> string_adaptor_suite = [] {
    "raw round-trip through the pipe adaptors"_test = [] {
        std::string original = "hello";
        auto byte_view = original | std::views::transform([](char character) {
                             return static_cast<std::byte>(character);
                         });

        std::vector<std::byte> encoded;
        for (std::byte value: byte_view | EncodeStringAdaptor<4>{false}) {
            encoded.push_back(value);
        }

        auto [decoded, consumed] = encoded | DecodeStringAdaptor<4>{};
        expect(decoded == original);
        expect(consumed == encoded.size());
    };

    "the huffman_encode flag is currently a no-op — output is identical either way"_test = [] {
        std::string original = "hello";
        auto byte_view = original | std::views::transform([](char character) {
                             return static_cast<std::byte>(character);
                         });

        std::vector<std::byte> raw_encoded;
        for (std::byte value: byte_view | EncodeStringAdaptor<4>{false}) {
            raw_encoded.push_back(value);
        }
        std::vector<std::byte> flagged_encoded;
        for (std::byte value: byte_view | EncodeStringAdaptor<4>{true}) {
            flagged_encoded.push_back(value);
        }

        // Plain `==` on two std::vector<std::byte> forces boost::ut's failure-diagnostic
        // printer to instantiate operator<<(ostream&, std::byte), which doesn't exist —
        // wrapping in std::ranges::equal() keeps the comparison a plain bool instead.
        expect(std::ranges::equal(raw_encoded, flagged_encoded));
    };

    "DecodeStringAdaptor huffman-decodes the body when the H-bit is set"_test = [] {
        std::string original = "www.example.com";
        auto byte_view = original | std::views::transform([](char character) {
                             return static_cast<std::byte>(character);
                         });

        std::vector<std::byte> huffman_body;
        for (std::byte value: byte_view | huffman::HuffmanEncodeAdaptor{}) {
            huffman_body.push_back(value);
        }

        std::vector<std::byte> data;
        auto length_view = static_cast<std::uint32_t>(huffman_body.size()) |
                           EncodeIntAdaptor<std::uint32_t>{7U, PrefixHelper::HUFFMAN_ENABLED};
        for (std::byte value: length_view) {
            data.push_back(value);
        }
        data.insert(data.end(), huffman_body.begin(), huffman_body.end());

        auto [decoded, consumed] = data | DecodeStringAdaptor<4>{};
        expect(decoded == original);
        expect(consumed == data.size());
    };

    "an empty range throws TruncatedDataError"_test = [] {
        std::vector<std::byte> data;
        expect(throws<error::http::TruncatedDataError>([&] {
            auto result = data | DecodeStringAdaptor<4>{};
        }));
    };
};

} // namespace io::shared_codec::lowlevel::tests
#endif
