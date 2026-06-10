module io.codec.shared.lowlevel;
@nogc nothrow:

import io.codec.shared.types;
import io.codec.shared.huffman;

// PORT-NOTE: C++ range-adaptor-closure classes translated to D InputRange structs and
// plain functions. Lazy ranges (EncodeIntView) become forward-range structs.
// Adaptor sinks (DecodeIntAdaptor, EncodeStringAdaptor, DecodeStringAdaptor)
// become free functions operating on slices.

// ──────────────────────────────────────────────────────────────────────────────
// EncodeIntRange — lazy forward range that produces the integer encoding bytes.
// PORT-NOTE: C++ EncodeIntView<UInt> → D EncodeIntRange!UInt (struct, value wrapper)
// PORT-NOTE: ABI POD value wrapper, exempt from class-only rule.
struct EncodeIntRange(UInt = uint) {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    UInt   m_data;
    UInt   m_max_prefix;
    ubyte  m_prefix;
    size_t m_pos;
    size_t m_size;

    this(UInt data, ubyte prefix_size, ubyte prefix) {
        assert(prefix_size >= 1 && prefix_size <= 8);
        m_data       = data;
        m_max_prefix = cast(UInt)((1U << prefix_size) - 1U);
        m_prefix     = prefix;
        m_pos        = 0;
        m_size       = compute_size(data, prefix_size);
    }

    bool   empty() const pure { return m_pos >= m_size; }
    size_t length() const pure { return m_size - m_pos; }

    ubyte front() const pure {
        const ubyte MASK = cast(ubyte)m_max_prefix;
        if (m_pos == 0) {
            if (m_data < m_max_prefix)
                return (m_prefix & ~MASK) | cast(ubyte)m_data;
            return m_prefix | MASK;
        }
        UInt remainder = cast(UInt)(m_data - m_max_prefix) >> (7U * cast(uint)(m_pos - 1U));
        const bool MORE = (remainder >> 7U) > 0U;
        return MORE ? cast(ubyte)((remainder & 0x7FU) | 0x80U)
                    : cast(ubyte)(remainder & 0x7FU);
    }

    void popFront() pure { ++m_pos; }

    void save(ref EncodeIntRange!UInt copy) const pure { copy = this; }

private:
    static size_t compute_size(UInt data, ubyte prefix_size) pure {
        const UInt MAX_PREFIX = cast(UInt)((1U << prefix_size) - 1U);
        if (data < MAX_PREFIX)
            return 1;
        data -= MAX_PREFIX;
        size_t comp_size = 2;
        while (data > 0x7FU) {
            data >>= 7;
            ++comp_size;
        }
        return comp_size;
    }
}

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

// encode_int: returns a lazy range over the encoded integer bytes.
EncodeIntRange!UInt encode_int(UInt = uint)(UInt data, ubyte prefix_size, ubyte prefix_data) {
    return EncodeIntRange!UInt(data, prefix_size, prefix_data);
}

EncodeIntRange!UInt encode_int(UInt = uint)(UInt data, ubyte prefix_size, PrefixHelper prefix_enum) {
    return EncodeIntRange!UInt(data, prefix_size, cast(ubyte)prefix_enum);
}

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
// PORT-NOTE: C++ DecodeIntAdaptor → free function decode_int.
// Returns a DecodeIntResult; m_consumed == 0 signals error (truncated/overflow).
DecodeIntResult!UInt decode_int(UInt = uint, size_t PrefixOffset = 0)(
        const(ubyte)[] data, ubyte prefix_size) {
    assert(prefix_size >= 1 && prefix_size <= 8);

    if (data.length == 0) {
        DecodeIntResult!UInt r;
        return r;
    }

    const ubyte FIRST_BYTE = data[0];
    size_t consumed = 1;

    ubyte prefix_metadata = 0;
    static if (PrefixOffset > 0) {
        prefix_metadata = cast(ubyte)(FIRST_BYTE >> prefix_size);
    }

    const UInt MASK  = cast(UInt)((1U << prefix_size) - 1U);
    UInt value = cast(UInt)(FIRST_BYTE & MASK);

    if (value < MASK) {
        DecodeIntResult!UInt r;
        r.m_value       = value;
        r.m_prefix_bits = prefix_metadata;
        r.m_consumed    = consumed;
        return r;
    }

    const(ubyte)[] sub = data[consumed .. $];

    foreach (ref byte_val; sub) {
        const size_t SHIFT = (consumed - 1) * 7U;

        // Guard against bit-shift overflow (e.g., shifting by 32+ on a uint32_t)
        if (SHIFT >= UInt.sizeof * 8) {
            // overflow: integer exceeds type capacity
            DecodeIntResult!UInt r;
            return r;
        }

        const UInt SHIFTED = (cast(UInt)(byte_val & 0x7Fu)) << SHIFT;

        // Check for overflow before adding the shifted value to the total value.
        UInt new_value = value + SHIFTED;
        if (new_value < value) {
            // overflow: accumulation wrapped around
            DecodeIntResult!UInt r;
            return r;
        }
        value = new_value;

        ++consumed;

        // Terminate the iteration instantly when the MSB is 0 (terminal byte)
        if ((byte_val & 0x80U) == 0)
            break;
    }

    DecodeIntResult!UInt r;
    r.m_value       = value;
    r.m_prefix_bits = prefix_metadata;
    r.m_consumed    = consumed;
    return r;
}

// encode_string — writes length-prefixed string bytes into out_buf starting at out_pos.
// Returns the number of bytes written.
// PORT-NOTE: C++ EncodeStringAdaptor → free function encode_string.
size_t encode_string(int Width)(
        bool huffman_encode,
        const(ubyte)[] data,
        ref ubyte[] out_buf, ref size_t out_pos,
        ubyte prefix_size = 7U) {
    const size_t start = out_pos;
    // if (huffman_encode) {
    //     auto encoded = HuffmanEncodeRange(data);
    //     // ... (TODO: implement Huffman encode path)
    // }

    const uint LEN = cast(uint)data.length;
    auto int_range = encode_int!uint(LEN, prefix_size, cast(ubyte)PrefixHelper.HUFFMAN_DISABLED);
    while (!int_range.empty()) {
        out_buf[out_pos++] = int_range.front();
        int_range.popFront();
    }
    foreach (b; data)
        out_buf[out_pos++] = b;
    return out_pos - start;
}

// decode_string — decodes a length-prefixed string from data[0..].
// Returns (number_of_bytes_consumed, string_as_char_slice).
// PORT-NOTE: C++ DecodeStringAdaptor → free function decode_string.
// PORT-NOTE: decoded chars placed into caller-owned out_chars (no GC alloc).
struct DecodeStringResult {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    size_t consumed;
    const(char)[] value; // slice into data (raw) or into out_chars (Huffman)
}

enum size_t MAX_STRING_LENGTH = 65536;

DecodeStringResult decode_string(int Width)(const(ubyte)[] data, char[] out_chars) {
    if (data.length == 0) {
        DecodeStringResult r;
        return r;
    }

    const bool H_FLAG = (data[0] & 0x80U) != 0;

    const auto LENGTH = decode_int!uint(data, 7U);
    if (LENGTH.m_consumed == 0) {
        DecodeStringResult r;
        return r;
    }

    const size_t HEADER = LENGTH.m_consumed;

    if (LENGTH.m_value > MAX_STRING_LENGTH) {
        DecodeStringResult r;
        return r;
    }
    if (HEADER + LENGTH.m_value > data.length) {
        DecodeStringResult r;
        return r;
    }

    const(ubyte)[] body = data[HEADER .. HEADER + LENGTH.m_value];
    const size_t CONSUMED = HEADER + LENGTH.m_value;

    if (H_FLAG) {
        Huffman!Width h;
        size_t n = h.decode(body, out_chars);
        DecodeStringResult r;
        r.consumed = CONSUMED;
        r.value    = out_chars[0 .. n];
        return r;
    }

    DecodeStringResult r;
    r.consumed = CONSUMED;
    r.value    = (cast(const(char)*)body.ptr)[0 .. body.length];
    return r;
}
