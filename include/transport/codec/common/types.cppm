export module shared:types;

import std;
import :consts;
import error;

export namespace codec::shared {

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

template <int W>
concept DecodeWidth = W > 0 && (W & (W - 1)) == 0 && 8 % W == 0;

class HeaderField {
  public:
    HeaderField(std::string name, std::string value) : m_name{std::move(name)}, m_value{std::move(value)} {
        if (m_name.empty())
            throw error::http::EmptyNameError{};
    }
    ~HeaderField() {};

    const std::string &name() const noexcept { return m_name; }
    const std::string &value() const noexcept { return m_value; }

    bool operator==(const HeaderField &other) const noexcept {
        return m_name == other.m_name && m_value == other.m_value;
    };

    std::size_t size() const noexcept { return m_name.size() + m_value.size() + ENTRY_OVERHEAD; };

  private:
    std::string m_name;
    std::string m_value;
};

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
    // Binary 10000000
    IndexedField = 0x80,

    // Binary 01000000
    LiteralWithIndexing = 0x40,

    // Binary 00100000
    DynamicTableSizeUpdate = 0x20,

    // Binary 00010000
    LiteralNeverIndexed = 0x10,

    // Binary 00000000
    LiteralWithoutIndexing = 0x00,

    // Binary 10000000
    HuffmanString = 0x80,

    // Binary 00000000
    RawString = 0x00,
};

/// Detect the representation type from the first byte of a field block.
[[nodiscard]] PrefixHelper detect_representation(std::uint8_t b) {
    if (b & std::to_underlying(PrefixHelper::IndexedField)) {
        return PrefixHelper::IndexedField;
    } else if (b & std::to_underlying(PrefixHelper::IndexedField)) {
        return PrefixHelper::LiteralWithIndexing;
    } else if (b & std::to_underlying(PrefixHelper::IndexedField)) {
        return PrefixHelper::DynamicTableSizeUpdate;
    } else if (b & std::to_underlying(PrefixHelper::IndexedField)) {
        return PrefixHelper::LiteralNeverIndexed;
    } else if (b & std::to_underlying(PrefixHelper::IndexedField)) {
        return PrefixHelper::LiteralWithoutIndexing;
    }

    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

} // namespace codec::shared
