export module io_codec_shared:types;

import std;
import :consts;
import io_error;

export namespace io::shared_codec {

enum class IndexCalculation : std::uint8_t { Q_PACK, H_PACK };

class SearchResult {
  public:
    static constexpr std::size_t NPOS = std::numeric_limits<std::size_t>::max();

    // Default constructor (initializes to NPOS)
    constexpr SearchResult() noexcept : m_value(NPOS) {}

    // Main constructor to replace sr_make
    constexpr SearchResult(std::size_t idx, bool is_static = false, bool is_full = false) noexcept
        : m_value((is_static ? STATIC_BIT : 0) | (is_full ? FULL_BIT : 0) | (idx & INDEX_MASK)) {}

    // Static helper for "Not Found"
    static constexpr SearchResult none() noexcept { return {}; }

    // Status checks
    [[nodiscard]] constexpr bool found() const noexcept { return m_value != NPOS; }
    [[nodiscard]] constexpr bool is_static() const noexcept {
        return ((m_value & STATIC_BIT) != 0U) && m_value != NPOS;
    }
    [[nodiscard]] constexpr bool is_full_match() const noexcept {
        return ((m_value & FULL_BIT) != 0U) && m_value != NPOS;
    }

    // Data retrieval
    [[nodiscard]] constexpr std::size_t index() const noexcept { return m_value & INDEX_MASK; }

    // Optional: Implicit conversion to bool for easy "if (result)" checks
    constexpr explicit operator bool() const noexcept { return found(); }

  private:
    static constexpr std::size_t STATIC_BIT = std::size_t{1} << ((sizeof(std::size_t) * 8) - 1);
    static constexpr std::size_t FULL_BIT = std::size_t{1} << ((sizeof(std::size_t) * 8) - 2);
    static constexpr std::size_t INDEX_MASK = ~(STATIC_BIT | FULL_BIT);

    std::size_t m_value;
};


template <std::unsigned_integral UInt = std::uint32_t>
class DecodeIntResult {
  public:
    DecodeIntResult(UInt value, std::uint8_t prefix_bits, std::size_t consumed = 0)
        : m_value{value}, m_prefix_bits{prefix_bits}, m_consumed{consumed} {}

    ~DecodeIntResult() = default;

    constexpr UInt value() const noexcept { return m_value; }
    [[nodiscard]] constexpr std::size_t consumed() const noexcept { return m_consumed; }
    [[nodiscard]] constexpr bool is_never_indexed() const noexcept { return (m_prefix_bits & 0x02) != 0; }
    [[nodiscard]] constexpr bool is_static() const noexcept { return (m_prefix_bits & 0x01) != 0; }

  private:
    UInt m_value;
    std::uint8_t m_prefix_bits;
    std::size_t m_consumed;
};


template <int W>
concept DecodeWidth = W > 0 && (W & (W - 1)) == 0 && 8 % W == 0;

template <typename T>
concept CastableToUint8 = std::convertible_to<T, std::uint8_t> ||
                          (std::is_enum_v<T> && std::same_as<std::underlying_type_t<T>, std::uint8_t>);

//   The first byte of every wire representation carries a fixed bit
//   pattern in its high bits that identifies which representation type
//   follows.  This function checks those patterns in priority order
//   exactly as the RFC specifies:
//
//   bit 7 = 1              → Indexed Field          (§6.1)   1xxxxxxx
//   bits 7-6 = 01          → Literal + Indexing     (§6.2.1) 01xxxxxx
//   bits 7-5 = 001         → Table Size Update      (§6.3)   001xxxxx
//   bits 7-4 = 0001        → Literal Never Indexed  (§6.2.3) 0001xxxx
//   anything else (0000xx) → Literal Without Index  (§6.2.2) 0000xxxx
//
//   Checks are done from most-specific to least-specific so that
//   overlapping masks never mis-classify a byte.
enum class PrefixHelper : std::uint8_t {
    /* --- HPACK (HTTP/2) Specific --- */
    HPACK_INDEXED_FIELD = 0x80,             // 1....... (N=7)
    HPACK_LITERAL_WITH_INDEXING = 0x40,     // 01...... (N=6)
    HPACK_DYNAMIC_TABLE_SIZE_UPDATE = 0x20, // 001..... (N=5)
    HPACK_LITERAL_NEVER_INDEXED = 0x10,     // 0001.... (N=4)
    HPACK_LITERAL_WITHOUT_INDEXING = 0x00,  // 0000.... (N=4)

    /* --- QPACK (HTTP/3) Request Stream --- */
    QPACK_INDEXED_FIELD = 0x80,           // 1....... (N=7)
    QPACK_INDEXED_NAME = 0x40,            // 01...... (N=6)
    QPACK_NEW_FIELD = 0x20,               // 001..... (N=5)
    QPACK_POST_BASE_INDEXED_FIELD = 0x01, // 0001.... (N=4)
    QPACK_POST_BASE_INDEXED_NAME = 0x00,  // 0000.... (N=4)

    /* --- QPACK (HTTP/3) Encoder Stream --- */
    QPACK_INSERT_INDEXED_NAME = 0x80,       // 1....... (N=7)
    QPACK_INSERT_LITERAL_NAME = 0x40,       // 01...... (N=6)
    QPACK_DUPLICATE = 0x00,                 // 000..... (N=5)
    QPACK_DYNAMIC_TABLE_SIZE_UPDATE = 0x20, // 001..... (N=5)

    /* --- QPACK (HTTP/3) Decoder Stream --- */
    QPACK_DEC_ACK = 0x80,                    // 1....... (N=7)
    QPACK_DEC_STREAM_CANCELLATION = 0x40,    // 01...... (N=6)
    QPACK_DEC_INSERT_COUNT_INCREMENT = 0x00, // 00...... (N=6)

    /* --- Shared String Constants --- */
    HUFFMAN_ENABLED = 0x80, // 1.......
    HUFFMAN_DISABLED = 0x00 // 0.......
};

[[nodiscard]] PrefixHelper detect_representation_hpack(std::uint8_t byte) {
    if ((byte & std::to_underlying(PrefixHelper::HPACK_INDEXED_FIELD)) != 0) {
        return PrefixHelper::HPACK_INDEXED_FIELD;
    }
    if ((byte & std::to_underlying(PrefixHelper::HPACK_LITERAL_WITH_INDEXING)) != 0) {
        return PrefixHelper::HPACK_LITERAL_WITH_INDEXING;
    }
    if ((byte & std::to_underlying(PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE)) != 0) {
        return PrefixHelper::HPACK_DYNAMIC_TABLE_SIZE_UPDATE;
    }
    if ((byte & std::to_underlying(PrefixHelper::HPACK_LITERAL_NEVER_INDEXED)) != 0) {
        return PrefixHelper::HPACK_LITERAL_NEVER_INDEXED;
    }
    return PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING;
}

[[nodiscard]] PrefixHelper detect_representation_qpack_stream(std::uint8_t byte) {
    if ((byte & std::to_underlying(PrefixHelper::QPACK_INDEXED_FIELD)) != 0) {
        return PrefixHelper::QPACK_INDEXED_FIELD;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_INDEXED_NAME)) != 0) {
        return PrefixHelper::QPACK_INDEXED_NAME;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_NEW_FIELD)) != 0) {
        return PrefixHelper::QPACK_NEW_FIELD;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_POST_BASE_INDEXED_FIELD)) != 0) {
        return PrefixHelper::QPACK_POST_BASE_INDEXED_FIELD;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_POST_BASE_INDEXED_FIELD)) != 0) {
        return PrefixHelper::QPACK_POST_BASE_INDEXED_NAME;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

[[nodiscard]] PrefixHelper detect_representation_qpack_encoder(std::uint8_t byte) {
    if ((byte & std::to_underlying(PrefixHelper::QPACK_INSERT_LITERAL_NAME)) != 0) {
        return PrefixHelper::QPACK_INSERT_LITERAL_NAME;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_INSERT_LITERAL_NAME)) != 0) {
        return PrefixHelper::QPACK_INSERT_LITERAL_NAME;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DUPLICATE)) != 0) {
        return PrefixHelper::QPACK_DUPLICATE;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DYNAMIC_TABLE_SIZE_UPDATE)) != 0) {
        return PrefixHelper::QPACK_DYNAMIC_TABLE_SIZE_UPDATE;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

[[nodiscard]] PrefixHelper detect_representation_qpack_decoder(std::uint8_t byte) {
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DEC_ACK)) != 0) {
        return PrefixHelper::QPACK_DEC_ACK;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DEC_STREAM_CANCELLATION)) != 0) {
        return PrefixHelper::QPACK_DEC_STREAM_CANCELLATION;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT)) != 0) {
        return PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

// Operator overloads for PrefixHelper
constexpr std::uint8_t operator|(PrefixHelper lhs, std::uint8_t rhs) { return static_cast<std::uint8_t>(lhs) | rhs; }

constexpr std::uint8_t operator|(std::uint8_t lhs, PrefixHelper rhs) { return lhs | static_cast<std::uint8_t>(rhs); }

constexpr std::uint8_t operator|(PrefixHelper lhs, PrefixHelper rhs) {
    return static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs);
}

constexpr PrefixHelper &operator|=(PrefixHelper &lhs, std::uint8_t rhs) {
    lhs = static_cast<PrefixHelper>(static_cast<std::uint8_t>(lhs) | rhs);
    return lhs;
}

constexpr std::uint8_t operator&(PrefixHelper lhs, std::uint8_t rhs) { return static_cast<std::uint8_t>(lhs) & rhs; }

constexpr std::uint8_t operator&(std::uint8_t lhs, PrefixHelper rhs) { return lhs & static_cast<std::uint8_t>(rhs); }

} // namespace io::shared_codec
