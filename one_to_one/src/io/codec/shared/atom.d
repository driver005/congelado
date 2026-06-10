module io.codec.shared.atom;
@nogc nothrow:

import io.codec.shared.types;
import io.codec.shared.huffman : Huffman;

// PORT-NOTE: C++ template class Atom<UInt, Width> → D template struct Atom!(UInt, Width).
// The class is stateless (all static methods); made a struct for value semantics.
// PORT-NOTE: C++ encode_int throws on invalid prefix → D asserts (debug) and clamps.
// PORT-NOTE: C++ decode_int throws TruncatedDataError/IntegerDecodeError → returns
// DecodeIntResult with m_consumed==0 as error sentinel (callers must check).

struct Atom(UInt = uint, int Width = 4) if (DecodeWidth!Width) {
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
    static void encode_int(UInt data, ubyte prefix_size, ubyte prefix_data, ref ubyte[] out_buf, ref size_t out_pos) {
        assert(prefix_size >= 1 && prefix_size <= 8);

        // 2^N - 1
        const uint max_prefix = cast(uint)((1u << prefix_size) - 1u);
        const ubyte mask = cast(ubyte)max_prefix;

        if (data < cast(UInt)max_prefix) {
            // Value fits in one octet, the function to calculate the max value is:  2 ^ prefix - 1 (inclusive).
            out_buf[out_pos++] = (prefix_data & ~mask) | cast(ubyte)data;
        } else {
            // Value is exeds the limit of one octet. In the following we need to encode the value in multiple octets.

            // Encode the prefix as well as the max value for the prefix (all bits of prefix must be set to 1).
            out_buf[out_pos++] = cast(ubyte)(prefix_data | mask);

            // Suvtract the max value of the prefix from the value, as we have already encoded that part in the first
            // octet.
            data -= cast(UInt)max_prefix;

            // while data > 127 (2 ^ 7 - 1)
            while (data > 0x7F) {
                out_buf[out_pos++] = cast(ubyte)((data % 0x80) + 0x80);
                data >>= 7;
            }

            // Since the data is less than (2 ^ 7) we can be sure that the MSB is 0.
            out_buf[out_pos++] = cast(ubyte)data;
        }
    }

    // PORT-NOTE: PrefixType enum overload — cast to ubyte at call site.
    static void encode_int(UInt data, ubyte prefix_size, PrefixHelper prefix_enum, ref ubyte[] out_buf, ref size_t out_pos) {
        encode_int(data, prefix_size, cast(ubyte)prefix_enum, out_buf, out_pos);
    }

    static DecodeIntResult!UInt decode_int(size_t PrefixOffset = 0)(
            const(ubyte)[] data, ref size_t pos, ubyte prefix_size) {
        assert(prefix_size >= 1 && prefix_size <= 8);

        // PORT-NOTE: truncated → return zero-consumed result
        if (pos >= data.length) {
            DecodeIntResult!UInt r;
            return r;
        }

        const ubyte first_byte = data[pos++];

        ubyte prefix_metadata = 0;
        static if (PrefixOffset > 0) {
            // Shift right to isolate the bits before the integer prefix
            prefix_metadata = cast(ubyte)(first_byte >> prefix_size);
        }
        // 2^N - 1
        const UInt mask = cast(UInt)((1u << prefix_size) - 1u);

        // Read the data from the first octet, masking out the prefix bits.
        UInt value = cast(UInt)(first_byte & mask);

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
        if (value < mask) {
            // Value is less than the max value of the prefix, so we can return it directly.
            DecodeIntResult!UInt r;
            r.m_value        = value;
            r.m_prefix_bits  = prefix_metadata;
            r.m_consumed     = pos;
            return r;
        }

        // Bit shift needed to align the bytes inside of uint.
        ubyte bit_shift = 0;
        enum ubyte digits = cast(ubyte)(UInt.sizeof * 8);

        while (true) {
            if (pos >= data.length) {
                // truncated continuation — return sentinel
                DecodeIntResult!UInt r;
                return r;
            }

            if (bit_shift >= digits) {
                // overflow — return sentinel
                DecodeIntResult!UInt r;
                return r;
            }

            const ubyte continuation_byte = data[pos++];

            // Get data execpt for the first continuation bit. Then shift into the correct postion inside of the Uint
            // and add to the value.
            const UInt shifted = cast(UInt)(continuation_byte & 0x7Fu) << bit_shift;

            // Check for overflow by adding
            UInt new_value = value + shifted;
            if (new_value < value) {
                // overflow
                DecodeIntResult!UInt r;
                return r;
            }
            value = new_value;

            bit_shift += 7;

            // MSB = 0 → last continuation byte
            if (!(continuation_byte & 0x80u))
                break;
        }

        DecodeIntResult!UInt r;
        r.m_value       = value;
        r.m_prefix_bits = prefix_metadata;
        r.m_consumed    = pos;
        return r;
    }

    //   0   1   2   3   4   5   6   7
    // +---+---+---+---+---+---+---+---+
    // | H |    String Length (7+)     |
    // +---+---------------------------+
    // |  String Data (Length octets)  |
    // +-------------------------------+
    // H: Huffman flag (1 bit) –> 1 = Huffman-encoded | 0 = raw string
    enum size_t kMaxLength = 65536;

    static void encode_string(
            const(Huffman!Width)* huffman,
            const(char)[] str,
            ref ubyte[] out_buf, ref size_t out_pos,
            ubyte prefix_size = 7u) {
        if (huffman !is null) {
            // TODO: implement Huffman by passing a ref via props which acts as flag isntead of using use_huffman
            // For now: encode raw (huffman flag = 0)
            assert(str.length <= UInt.max);
            encode_int(cast(UInt)str.length, prefix_size, cast(ubyte)PrefixHelper.HUFFMAN_DISABLED, out_buf, out_pos);
            foreach (char ch; str)
                out_buf[out_pos++] = cast(ubyte)ch;
        } else {
            // TODO: wait for cpp26 and use std::narrowing_cast
            assert(str.length <= UInt.max);
            encode_int(cast(UInt)str.length, prefix_size, cast(ubyte)PrefixHelper.HUFFMAN_DISABLED, out_buf, out_pos);
            foreach (char ch; str)
                out_buf[out_pos++] = cast(ubyte)ch;
        }
    }

    // PORT-NOTE: C++ decode_string returns std::string (GC-free via @nogc: returns
    // a slice into a caller-supplied output buffer instead).
    // Returns number of bytes consumed from `data[pos..]`, zero on error.
    static size_t decode_string(
            ref const(Huffman!Width) h,
            const(ubyte)[] data,
            ref size_t pos,
            ref char[] out_str) {
        if (pos >= data.length) return 0;

        // Check the Huffman flag (H-bit) in the first byte.
        const bool h_flag = (data[pos] & 0x80) != 0;

        // String length is encoded as a 7-bit prefix integer.
        const auto length = decode_int!0(data, pos, 7u);

        if (length.m_consumed == 0) return 0; // truncated
        if (length.m_value > kMaxLength) return 0;
        if (pos + length.m_value > data.length) return 0;

        const(ubyte)[] body = data[pos .. pos + length.m_value];
        pos += length.m_value;

        if (h_flag) {
            // PORT-NOTE: Huffman decode writes into caller-supplied out_str
            return h.decode(body, out_str);
        }

        // Raw copy: resize out_str to body.length and fill
        out_str = (cast(char*)body.ptr)[0 .. body.length];
        return length.m_value;
    }
}
