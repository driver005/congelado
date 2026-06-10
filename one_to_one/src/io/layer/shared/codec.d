module io.layer.shared.codec;
@nogc nothrow:

// PORT-NOTE: namespace io::shared_layer, class Atom<UInt> → struct Atom(UInt).
// C++ used range_adaptor_closure and std::views heavily; D port uses plain
// helper functions that operate on slices.

/// io::shared_layer::Atom!UInt
/// Big-endian and QUIC variable-length integer encode/decode helpers.
struct Atom(UInt = uint) if (__traits(isUnsigned, UInt)) {
  private:
    static immutable ulong LIMIT_6BIT  = 64;          // 2^6
    static immutable ulong LIMIT_14BIT = 16_384;      // 2^14
    static immutable ulong LIMIT_30BIT = 1_073_741_824; // 2^30

  public:
    // ── Big-endian fixed-length ──────────────────────────────────────────

    /// Write UInt as big-endian bytes into out[0..sizeof(UInt)].
    static void write_big_endian(ubyte[] out_, UInt val) {
        enum size_t bytes = UInt.sizeof;
        foreach (i; 0..bytes) {
            const size_t shift = 8 * (bytes - 1 - i);
            out_[i] = cast(ubyte)(val >> shift);
        }
    }

    /// Read big-endian UInt from data slice (data.length must == sizeof(UInt)).
    static UInt read_big_endian(const(ubyte)[] data) {
        UInt val = 0;
        foreach (b; data) {
            val = cast(UInt)((val << 8) | b);
        }
        return val;
    }

    // ── QUIC Variable-Length Integer encode/decode ───────────────────────

    /// Write QUIC varint into out_.  Returns number of bytes written.
    static size_t write_varint(ubyte[] out_, UInt val) {
        auto enc = varint_encoding(val);
        ubyte prefix = enc[0];
        size_t length = enc[1];
        write_varint_helper(out_[0..length], val, prefix, length);
        return length;
    }

    /// Read QUIC varint from data.  Returns (value, bytes_consumed).
    static ulong[2] read_varint(const(ubyte)[] data) {
        if (data.length == 0)
            return [0, 0];
        ubyte first_byte = data[0];
        ubyte prefix = first_byte >> 6;
        ubyte length = cast(ubyte)(1 << prefix);
        UInt value = first_byte & 0x3F;

        foreach (i; 1..length) {
            if (i >= data.length) break;
            value = cast(UInt)((value << 8) | data[i]);
        }
        return [cast(ulong) value, cast(ulong) length];
    }

  private:
    static ubyte[2] varint_encoding(UInt val) {
        if (val < LIMIT_6BIT)  return [0x00, 1];
        if (val < LIMIT_14BIT) return [0x01, 2];
        if (val < LIMIT_30BIT) return [0x02, 4];
        return [0x03, 8];
    }

    static void write_varint_helper(ubyte[] out_, UInt val, ubyte prefix, size_t length) {
        // First byte: 2-bit prefix + high bits of value
        out_[0] = cast(ubyte)((val >> (8 * (length - 1))) | (prefix << 6));
        foreach (i; 1..length) {
            out_[i] = cast(ubyte)(val >> (8 * (length - 1 - i)));
        }
    }
}
