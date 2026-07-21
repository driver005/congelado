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
    /** @brief Builds a "not found" result by default — `m_value` starts at NPOS, no motion. */
    constexpr SearchResult() noexcept : m_value(NPOS) {}

    // Main constructor to replace sr_make
    /**
     * @brief Packs an index plus static/full-match flags into one word — two high bits stolen
     * for flags, the rest is the index. This is the whole encoding scheme for the type.
     * @param idx the table index this result points at.
     * @param is_static true if the entry lives in the static table, false (default) for dynamic.
     * @param is_full true if it's a full name+value match, false (default) for name-only.
     */
    constexpr SearchResult(std::size_t idx, bool is_static = false, bool is_full = false) noexcept
        : m_value((is_static ? STATIC_BIT : 0) | (is_full ? FULL_BIT : 0) | (idx & INDEX_MASK)) {}

    // Static helper for "Not Found"
    /** @brief Gets a "not found" result. @return a SearchResult equivalent to the default ctor. */
    static constexpr SearchResult none() noexcept { return {}; }

    // Status checks
    /**
     * @brief Checks whether this result represents an actual hit.
     * @return true unless `m_value` is NPOS.
     */
    [[nodiscard]] constexpr bool found() const noexcept { return m_value != NPOS; }
    /**
     * @brief Checks the static-table flag.
     * @return true if this is a found result pointing into the static table.
     */
    [[nodiscard]] constexpr bool is_static() const noexcept {
        return ((m_value & STATIC_BIT) != 0U) && m_value != NPOS;
    }
    /**
     * @brief Checks the full-match flag.
     * @return true if this is a found result representing a full name+value match, as opposed to
     * name-only.
     */
    [[nodiscard]] constexpr bool is_full_match() const noexcept {
        return ((m_value & FULL_BIT) != 0U) && m_value != NPOS;
    }

    // Data retrieval
    /**
     * @brief Gets the packed table index, flag bits masked off.
     * @warning No `found()` check here — call this on a not-found result and you get whatever
     * `NPOS & INDEX_MASK` happens to be, not a sentinel that screams "invalid." Check found()
     * first, don't trust index() alone.
     * @return the table index.
     */
    [[nodiscard]] constexpr std::size_t index() const noexcept { return m_value & INDEX_MASK; }

    // Optional: Implicit conversion to bool for easy "if (result)" checks
    /** @brief Explicit bool conversion for `if (result)` checks. @return same as found(). */
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
    /**
     * @brief Bundles a decoded integer with the prefix metadata bits and byte count that came
     * along with it — everything Atom::decode_int()/DecodeIntAdaptor need to hand back in one
     * shot.
     * @param value the decoded integer.
     * @param prefix_bits the bits sitting before the integer prefix in the first octet
     * (representation-type flags); only meaningful when `PrefixOffset > 0` was used to decode.
     * @param consumed how many bytes the decode consumed; defaults to 0 for callers (like
     * Atom::decode_int()) that track position externally instead.
     */
    DecodeIntResult(UInt value, std::uint8_t prefix_bits, std::size_t consumed = 0)
        : m_value{value}, m_prefix_bits{prefix_bits}, m_consumed{consumed} {}

    /** @brief Trivial dtor — nothing owned here beyond plain value types. */
    ~DecodeIntResult() = default;
    /** @brief Trivially copyable — plain value types only. */
    DecodeIntResult(const DecodeIntResult &) = default;
    /** @brief Trivially copyable — plain value types only. */
    DecodeIntResult &operator=(const DecodeIntResult &) = default;
    /** @brief Trivially movable — plain value types only. */
    DecodeIntResult(DecodeIntResult &&) noexcept = default;
    /** @brief Trivially movable — plain value types only. */
    DecodeIntResult &operator=(DecodeIntResult &&) noexcept = default;

    /** @brief Gets the decoded integer value. @return the value. */
    [[nodiscard]] constexpr UInt value() const noexcept { return m_value; }
    /**
     * @brief Gets how many bytes the decode consumed.
     * @return byte count, or 0 if the caller tracks position itself.
     */
    [[nodiscard]] constexpr std::size_t consumed() const noexcept { return m_consumed; }
    /**
     * @brief Checks the "never indexed" bit (HPACK §6.2.3 literal representation flag).
     * @return true if that bit's set.
     */
    [[nodiscard]] constexpr bool is_never_indexed() const noexcept { return (m_prefix_bits & 0x02) != 0; }
    /**
     * @brief Checks the "static table" bit in the prefix metadata.
     * @return true if that bit's set.
     */
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
    // Most-specific mask first (bit 7 alone), down to least-specific (bits 7-4) — matches the
    // priority order laid out in the comment block above, otherwise a byte with multiple high
    // bits set would mis-classify against a looser mask.
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
    // Nothing else matched — the 0000xxxx pattern by elimination.
    return PrefixHelper::HPACK_LITERAL_WITHOUT_INDEXING;
}

[[nodiscard]] PrefixHelper detect_representation_qpack_stream(std::uint8_t byte) {
    // Same most-specific-first priority order as the HPACK detector, just against the QPACK
    // request-stream representation set.
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

    // Nothing matched at all — unlike the HPACK version there's no catch-all bucket here, so an
    // unrecognized pattern is treated as hostile/malformed input.
    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

[[nodiscard]] PrefixHelper detect_representation_qpack_encoder(std::uint8_t byte) {
    // Priority-ordered mask checks for the QPACK encoder-stream instruction set.
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

    // No pattern matched — malformed/hostile input on the wire, hard failure.
    throw error::http::DecodeError("Invalid first byte for HPACK representation");
}

[[nodiscard]] PrefixHelper detect_representation_qpack_decoder(std::uint8_t byte) {
    // Priority-ordered mask checks for the QPACK decoder-stream instruction set.
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DEC_ACK)) != 0) {
        return PrefixHelper::QPACK_DEC_ACK;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DEC_STREAM_CANCELLATION)) != 0) {
        return PrefixHelper::QPACK_DEC_STREAM_CANCELLATION;
    }
    if ((byte & std::to_underlying(PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT)) != 0) {
        return PrefixHelper::QPACK_DEC_INSERT_COUNT_INCREMENT;
    }

    // No pattern matched — malformed/hostile input on the wire, hard failure.
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
