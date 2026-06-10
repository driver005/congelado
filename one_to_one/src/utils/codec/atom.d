module utils.codec.atom;
@nogc nothrow:

// ---------------------------------------------------------------------------
// BigEndianView — forward range that emits the bytes of a UInt MSB-first.
//
// PORT-NOTE: class range, save() deep-copies
// ---------------------------------------------------------------------------
class BigEndianView(UInt = uint) if (__traits(isUnsigned, UInt)) {
    @disable this(this);

    this(UInt val) { m_val = val; m_pos = 0; }

    // Forward range primitives
    bool empty() const { return m_pos >= UInt.sizeof; }
    ubyte front() const {
        // Convetring the current 8 bits of the value into the format directed by the view before it
        // and return it
        return cast(ubyte)(m_val >> (8 * (UInt.sizeof - 1 - m_pos)));
    }
    void popFront() { ++m_pos; }

    // save() deep-copies state (class range)
    BigEndianView!UInt save() {
        import util.alloc : make;
        auto copy = make!(BigEndianView!UInt)(m_val);
        copy.m_pos = m_pos;
        return copy;
    }

  private:
    UInt m_val;
    size_t m_pos;
}

// ---------------------------------------------------------------------------
// WriteBigEndianAdaptor — produces a BigEndianView for a given integer value.
// PORT-NOTE: value wrapper, exempt from classes-only rule
// ---------------------------------------------------------------------------
struct WriteBigEndianAdaptor(UInt = uint) if (__traits(isUnsigned, UInt)) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    UInt m_val;

    this(UInt val) { m_val = val; }

    // Returns a fresh BigEndianView
    BigEndianView!UInt opCall() const {
        import util.alloc : make;
        return make!(BigEndianView!UInt)(m_val);
    }
}

// ---------------------------------------------------------------------------
// ReadBigEndianAdaptor — consumes up to sizeof(UInt) bytes and assembles UInt.
// PORT-NOTE: value wrapper, exempt from classes-only rule
// ---------------------------------------------------------------------------
struct ReadBigEndianAdaptor(UInt = uint) if (__traits(isUnsigned, UInt)) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule

    // Reads from a ubyte[] slice. Returns 0 if empty, asserts if too many bytes.
    UInt opCall(const(ubyte)[] data) const {
        assert(data.length <= UInt.sizeof,
               "Too many bytes to fit into integer");
        if (data.length == 0)
            return UInt(0);
        UInt acc = UInt(0);
        foreach (b; data)
            acc = cast(UInt)((acc << 8) | b);
        return acc;
    }
}

// ---------------------------------------------------------------------------
// VariantEndianView — QUIC-style variable-length big-endian encoding.
// 1/2/4/8 byte encoding selected by the two MSBs of the first byte.
//
// PORT-NOTE: class range, save() deep-copies
// ---------------------------------------------------------------------------
class VariantEndianView(UInt = uint) if (__traits(isUnsigned, UInt)) {
    @disable this(this);

    enum ulong LIMIT_6BIT  = 64;          // 2^6
    enum ulong LIMIT_14BIT = 16384;       // 2^14
    enum ulong LIMIT_30BIT = 1073741824;  // 2^30

    this(UInt val) {
        m_val = val;
        auto enc = _encoding(val);
        m_prefix = enc[0];
        m_length = enc[1];
        m_pos    = 0;
    }

    bool empty() const { return m_pos >= m_length; }
    ubyte front() const {
        // Convetring the current 8 bits of the value into the format directed by the view before it
        // and return it
        if (m_pos == 0)
            return cast(ubyte)((m_val >> (8 * (m_length - 1))) | (m_prefix << 6));
        return cast(ubyte)(m_val >> (8 * (m_length - 1 - m_pos)));
    }
    void popFront() { ++m_pos; }

    // save() deep-copies state (class range)
    VariantEndianView!UInt save() {
        import util.alloc : make;
        auto copy = make!(VariantEndianView!UInt)(m_val);
        copy.m_pos = m_pos;
        return copy;
    }

  private:
    static ubyte[2] _encoding(UInt val) {
        if (val < LIMIT_6BIT)  return [0x00, 1];
        if (val < LIMIT_14BIT) return [0x01, 2];
        if (val < LIMIT_30BIT) return [0x02, 4];
        return [0x03, 8];
    }

    UInt   m_val;
    size_t m_length;
    size_t m_pos;
    ubyte  m_prefix;
}

// ---------------------------------------------------------------------------
// WriteVariantEndianAdaptor
// PORT-NOTE: value wrapper, exempt from classes-only rule
// ---------------------------------------------------------------------------
struct WriteVariantEndianAdaptor(UInt = uint) if (__traits(isUnsigned, UInt)) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    UInt m_val;

    this(UInt val) { m_val = val; }

    VariantEndianView!UInt opCall() const {
        import util.alloc : make;
        return make!(VariantEndianView!UInt)(m_val);
    }
}

// ---------------------------------------------------------------------------
// ReadVariantEndianAdaptor — decode QUIC variable-length integer from slice.
// PORT-NOTE: value wrapper, exempt from classes-only rule
// ---------------------------------------------------------------------------
struct ReadVariantEndianAdaptor(UInt = uint) if (__traits(isUnsigned, UInt)) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule

    UInt opCall(const(ubyte)[] data) const {
        if (data.length == 0) return UInt(0);
        ubyte first  = data[0];
        ubyte prefix = (first >> 6) & 0x03;
        size_t length = cast(size_t)(1) << prefix;

        UInt acc = cast(UInt)(first & 0x3F);
        foreach (b; data[1 .. length])
            acc = cast(UInt)((acc << 8) | b);
        return acc;
    }
}
