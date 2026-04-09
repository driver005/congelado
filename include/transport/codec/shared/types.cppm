export module transport_codec_shared:types;

import std;
import :consts;
import transport_error;

export namespace transport::shared_codec {

enum class IndexCalculation { QPack, HPack };

class SearchResult {
  public:
    static constexpr std::size_t NPOS = std::numeric_limits<std::size_t>::max();

    // Default constructor (initializes to NPOS)
    constexpr SearchResult() noexcept : m_value(NPOS) {}

    // Main constructor to replace sr_make
    constexpr SearchResult(std::size_t idx, bool is_static = false, bool is_full = false) noexcept
        : m_value((is_static ? STATIC_BIT : 0) | (is_full ? FULL_BIT : 0) | (idx & INDEX_MASK)) {}

    // Static helper for "Not Found"
    static constexpr SearchResult none() noexcept { return SearchResult(); }

    // Status checks
    constexpr bool found() const noexcept { return m_value != NPOS; }
    constexpr bool is_static() const noexcept { return (m_value & STATIC_BIT) && m_value != NPOS; }
    constexpr bool is_full_match() const noexcept { return (m_value & FULL_BIT) && m_value != NPOS; }

    // Data retrieval
    constexpr std::size_t index() const noexcept { return m_value & INDEX_MASK; }

    // Optional: Implicit conversion to bool for easy "if (result)" checks
    constexpr explicit operator bool() const noexcept { return found(); }

  private:
    static constexpr std::size_t STATIC_BIT = std::size_t{1} << (sizeof(std::size_t) * 8 - 1);
    static constexpr std::size_t FULL_BIT = std::size_t{1} << (sizeof(std::size_t) * 8 - 2);
    static constexpr std::size_t INDEX_MASK = ~(STATIC_BIT | FULL_BIT);

    std::size_t m_value;
};


template <std::unsigned_integral UInt = std::uint32_t>
class DecodeIntResult {
  public:
    DecodeIntResult(UInt value, std::uint8_t prefix_bits) : m_value(value), m_prefix_bits(prefix_bits) {}
    ~DecodeIntResult() = default;

    constexpr UInt value() const { return m_value; }

    constexpr bool is_never_indexed() const { return m_prefix_bits & 0x02; }
    constexpr bool is_static() const { return m_prefix_bits & 0x01; }

  private:
    UInt m_value;
    std::uint8_t m_prefix_bits;
};


template <int W>
concept DecodeWidth = W > 0 && (W & (W - 1)) == 0 && 8 % W == 0;

template <typename T>
concept CastableToUint8 = std::convertible_to<T, std::uint8_t>;

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
    HpackIndexedField = 0x80,           // 1....... (N=7)
    HpackLiteralWithIndexing = 0x40,    // 01...... (N=6)
    HpackDynamicTableSizeUpdate = 0x20, // 001..... (N=5)
    HpackLiteralNeverIndexed = 0x10,    // 0001.... (N=4)
    HpackLiteralWithoutIndexing = 0x00, // 0000.... (N=4)

    /* --- QPACK (HTTP/3) Request Stream --- */
    QpackIndexedField = 0x80,         // 1....... (N=7)
    QpackIndexedName = 0x40,          // 01...... (N=6)
    QpackNewField = 0x20,             // 001..... (N=5)
    QpackPostBaseIndexedField = 0x01, // 0001.... (N=4)
    QpackPostBaseIndexedName = 0x00,  // 0000.... (N=4)

    /* --- QPACK (HTTP/3) Encoder Stream --- */
    QpackInsertIndexedName = 0x80,      // 1....... (N=7)
    QpackInsertLiteralName = 0x40,      // 01...... (N=6)
    QpackDuplicate = 0x00,              // 000..... (N=5)
    QpackDynamicTableSizeUpdate = 0x20, // 001..... (N=5)

    /* --- QPACK (HTTP/3) Decoder Stream --- */
    QpackDecAck = 0x80,                  // 1....... (N=7)
    QpackDecStreamCancellation = 0x40,   // 01...... (N=6)
    QpackDecInsertCountIncrement = 0x00, // 00...... (N=6)

    /* --- Shared String Constants --- */
    HuffmanEnabled = 0x80, // 1.......
    HuffmanDisabled = 0x00 // 0.......
};

/* --- HPACK (HTTP/2) Specific --- */
[[nodiscard]] PrefixHelper detect_representation_hpack(std::uint8_t b) {
    if (b & std::to_underlying(PrefixHelper::HpackIndexedField)) {
        return PrefixHelper::HpackIndexedField;
    } else if (b & std::to_underlying(PrefixHelper::HpackLiteralWithIndexing)) {
        return PrefixHelper::HpackLiteralWithIndexing;
    } else if (b & std::to_underlying(PrefixHelper::HpackDynamicTableSizeUpdate)) {
        return PrefixHelper::HpackDynamicTableSizeUpdate;
    } else if (b & std::to_underlying(PrefixHelper::HpackLiteralNeverIndexed)) {
        return PrefixHelper::HpackLiteralNeverIndexed;
    } else if (b & std::to_underlying(PrefixHelper::HpackLiteralWithoutIndexing)) {
        return PrefixHelper::HpackLiteralWithoutIndexing;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

/* --- QPACK (HTTP/3) Request Stream --- */
[[nodiscard]] PrefixHelper detect_representation_qpack_stream(std::uint8_t b) {
    if (b & std::to_underlying(PrefixHelper::QpackIndexedField)) {
        return PrefixHelper::QpackIndexedField;
    } else if (b & std::to_underlying(PrefixHelper::QpackIndexedName)) {
        return PrefixHelper::QpackIndexedName;
    } else if (b & std::to_underlying(PrefixHelper::QpackNewField)) {
        return PrefixHelper::QpackNewField;
    } else if (b & std::to_underlying(PrefixHelper::QpackPostBaseIndexedField)) {
        return PrefixHelper::QpackPostBaseIndexedField;
    } else if (b & std::to_underlying(PrefixHelper::QpackPostBaseIndexedField)) {
        return PrefixHelper::QpackPostBaseIndexedName;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

/* --- QPACK (HTTP/3) Decoder Stream --- */
[[nodiscard]] PrefixHelper detect_representation_qpack_encoder(std::uint8_t b) {
    if (b & std::to_underlying(PrefixHelper::QpackInsertLiteralName)) {
        return PrefixHelper::QpackInsertLiteralName;
    } else if (b & std::to_underlying(PrefixHelper::QpackInsertLiteralName)) {
        return PrefixHelper::QpackInsertLiteralName;
    } else if (b & std::to_underlying(PrefixHelper::QpackDuplicate)) {
        return PrefixHelper::QpackDuplicate;
    } else if (b & std::to_underlying(PrefixHelper::QpackDynamicTableSizeUpdate)) {
        return PrefixHelper::QpackDynamicTableSizeUpdate;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

/* --- QPACK (HTTP/3) Decoder Stream --- */
[[nodiscard]] PrefixHelper detect_representation_qpack_decoder(std::uint8_t b) {
    if (b & std::to_underlying(PrefixHelper::QpackDecAck)) {
        return PrefixHelper::QpackDecAck;
    } else if (b & std::to_underlying(PrefixHelper::QpackDecStreamCancellation)) {
        return PrefixHelper::QpackDecStreamCancellation;
    } else if (b & std::to_underlying(PrefixHelper::QpackDecInsertCountIncrement)) {
        return PrefixHelper::QpackDecInsertCountIncrement;
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

} // namespace transport::shared_codec
