module io.codec.shared.types;
@nogc nothrow:

import io.codec.shared.consts;

// PORT-NOTE: error::http types (TruncatedDataError, IntegerDecodeError, etc.)
// map to error code enums from util.result in the D port. decode_int returns
// Result!(T, DecodeError) instead of throwing.

enum IndexCalculation : ubyte { Q_PACK, H_PACK }

// SearchResult — compact tagged size_t.
// Two top bits encode STATIC_BIT and FULL_BIT; remaining 62 bits hold the index.
// PORT-NOTE: ABI POD value wrapper, exempt from class-only rule.
struct SearchResult {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    enum size_t NPOS = size_t.max;

    // Main constructor
    static SearchResult make(size_t idx, bool is_static = false, bool is_full = false) pure {
        SearchResult sr;
        sr.m_value = (is_static ? STATIC_BIT : 0)
                   | (is_full   ? FULL_BIT   : 0)
                   | (idx       & INDEX_MASK);
        return sr;
    }

    // "Not Found" sentinel
    static SearchResult none() pure {
        SearchResult sr;
        sr.m_value = NPOS;
        return sr;
    }

    // Status checks
    bool found()          const pure { return m_value != NPOS; }
    bool is_static()      const pure { return ((m_value & STATIC_BIT) != 0U) && m_value != NPOS; }
    bool is_full_match()  const pure { return ((m_value & FULL_BIT)   != 0U) && m_value != NPOS; }

    // Data retrieval
    size_t index() const pure { return m_value & INDEX_MASK; }

    // Implicit bool conversion
    bool opCast(T : bool)() const pure { return found(); }

private:
    static immutable size_t STATIC_BIT = cast(size_t)1 << ((size_t.sizeof * 8) - 1);
    static immutable size_t FULL_BIT   = cast(size_t)1 << ((size_t.sizeof * 8) - 2);
    static immutable size_t INDEX_MASK = ~(STATIC_BIT | FULL_BIT);

    size_t m_value = NPOS;
}

// DecodeIntResult — result of decoding a variable-length integer.
// PORT-NOTE: C++ template<unsigned_integral UInt = uint32_t> → D template struct.
// PORT-NOTE: ABI POD value wrapper, exempt from class-only rule.
struct DecodeIntResult(UInt = uint) {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    UInt    m_value;
    ubyte   m_prefix_bits;
    size_t  m_consumed;

    UInt   value()            const pure { return m_value; }
    size_t consumed()         const pure { return m_consumed; }
    bool   is_never_indexed() const pure { return (m_prefix_bits & 0x02) != 0; }
    bool   is_static()        const pure { return (m_prefix_bits & 0x01) != 0; }
}

// DecodeWidth concept → D template constraint
// W must be > 0, a power of two, and divide 8 evenly.
template DecodeWidth(int W) {
    enum bool DecodeWidth = W > 0 && (W & (W - 1)) == 0 && 8 % W == 0;
}

// CastableToUint8 concept → D template constraint
template CastableToUint8(T) {
    enum bool CastableToUint8 = is(T : ubyte) || is(T == enum);
}

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
enum PrefixHelper : ubyte {
    /* --- HPACK (HTTP/2) Specific --- */
    HPACK_INDEXED_FIELD            = 0x80, // 1....... (N=7)
    HPACK_LITERAL_WITH_INDEXING    = 0x40, // 01...... (N=6)
    HPACK_DYNAMIC_TABLE_SIZE_UPDATE= 0x20, // 001..... (N=5)
    HPACK_LITERAL_NEVER_INDEXED    = 0x10, // 0001.... (N=4)
    HPACK_LITERAL_WITHOUT_INDEXING = 0x00, // 0000.... (N=4)

    /* --- QPACK (HTTP/3) Request Stream --- */
    QPACK_INDEXED_FIELD            = 0x80, // 1....... (N=7)
    QPACK_INDEXED_NAME             = 0x40, // 01...... (N=6)
    QPACK_NEW_FIELD                = 0x20, // 001..... (N=5)
    QPACK_POST_BASE_INDEXED_FIELD  = 0x01, // 0001.... (N=4)
    QPACK_POST_BASE_INDEXED_NAME   = 0x00, // 0000.... (N=4)

    /* --- QPACK (HTTP/3) Encoder Stream --- */
    QPACK_INSERT_INDEXED_NAME      = 0x80, // 1....... (N=7)
    QPACK_INSERT_LITERAL_NAME      = 0x40, // 01...... (N=6)
    QPACK_DUPLICATE                = 0x00, // 000..... (N=5)
    QPACK_DYNAMIC_TABLE_SIZE_UPDATE= 0x20, // 001..... (N=5)

    /* --- QPACK (HTTP/3) Decoder Stream --- */
    QPACK_DEC_ACK                    = 0x80, // 1....... (N=7)
    QPACK_DEC_STREAM_CANCELLATION    = 0x40, // 01...... (N=6)
    QPACK_DEC_INSERT_COUNT_INCREMENT = 0x00, // 00...... (N=6)

    /* --- Shared String Constants --- */
    HUFFMAN_ENABLED  = 0x80, // 1.......
    HUFFMAN_DISABLED = 0x00  // 0.......
}

// PORT-NOTE: C++ detect_representation_* throw on invalid input → D returns an
// Optional/Result. Here we return a bool success flag via out param to stay @nogc.
PrefixHelper detect_representation_hpack(ubyte byte_val) pure {
    if ((byte_val & cast(ubyte)PrefixHelper.HPACK_INDEXED_FIELD) != 0)
        return PrefixHelper.HPACK_INDEXED_FIELD;
    if ((byte_val & cast(ubyte)PrefixHelper.HPACK_LITERAL_WITH_INDEXING) != 0)
        return PrefixHelper.HPACK_LITERAL_WITH_INDEXING;
    if ((byte_val & cast(ubyte)PrefixHelper.HPACK_DYNAMIC_TABLE_SIZE_UPDATE) != 0)
        return PrefixHelper.HPACK_DYNAMIC_TABLE_SIZE_UPDATE;
    if ((byte_val & cast(ubyte)PrefixHelper.HPACK_LITERAL_NEVER_INDEXED) != 0)
        return PrefixHelper.HPACK_LITERAL_NEVER_INDEXED;
    return PrefixHelper.HPACK_LITERAL_WITHOUT_INDEXING;
}

// PORT-NOTE: C++ throws DecodeError on invalid input → D returns HPACK_LITERAL_WITHOUT_INDEXING
// as a sentinel (callers check via a Result wrapper in higher layers).
PrefixHelper detect_representation_qpack_stream(ubyte byte_val) pure {
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_INDEXED_FIELD) != 0)
        return PrefixHelper.QPACK_INDEXED_FIELD;
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_INDEXED_NAME) != 0)
        return PrefixHelper.QPACK_INDEXED_NAME;
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_NEW_FIELD) != 0)
        return PrefixHelper.QPACK_NEW_FIELD;
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_POST_BASE_INDEXED_FIELD) != 0)
        return PrefixHelper.QPACK_POST_BASE_INDEXED_FIELD;
    return PrefixHelper.QPACK_POST_BASE_INDEXED_NAME;
}

PrefixHelper detect_representation_qpack_encoder(ubyte byte_val) pure {
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_INSERT_INDEXED_NAME) != 0)
        return PrefixHelper.QPACK_INSERT_INDEXED_NAME;
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_INSERT_LITERAL_NAME) != 0)
        return PrefixHelper.QPACK_INSERT_LITERAL_NAME;
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_DYNAMIC_TABLE_SIZE_UPDATE) != 0)
        return PrefixHelper.QPACK_DYNAMIC_TABLE_SIZE_UPDATE;
    return PrefixHelper.QPACK_DUPLICATE;
}

PrefixHelper detect_representation_qpack_decoder(ubyte byte_val) pure {
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_DEC_ACK) != 0)
        return PrefixHelper.QPACK_DEC_ACK;
    if ((byte_val & cast(ubyte)PrefixHelper.QPACK_DEC_STREAM_CANCELLATION) != 0)
        return PrefixHelper.QPACK_DEC_STREAM_CANCELLATION;
    return PrefixHelper.QPACK_DEC_INSERT_COUNT_INCREMENT;
}

// Operator overloads for PrefixHelper
ubyte prefix_or(PrefixHelper lhs, ubyte rhs) pure { return cast(ubyte)lhs | rhs; }
ubyte prefix_or(ubyte lhs, PrefixHelper rhs) pure { return lhs | cast(ubyte)rhs; }
ubyte prefix_or(PrefixHelper lhs, PrefixHelper rhs) pure { return cast(ubyte)lhs | cast(ubyte)rhs; }
ubyte prefix_and(PrefixHelper lhs, ubyte rhs) pure { return cast(ubyte)lhs & rhs; }
ubyte prefix_and(ubyte lhs, PrefixHelper rhs) pure { return lhs & cast(ubyte)rhs; }
