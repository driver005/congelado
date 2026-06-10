module io.codec.quic.types;
@nogc nothrow:

import core.stdc.string : memcpy, memcmp;

// quic:types retains only what quic:connection, quic:tls, quic:qpack,
// and server.http3 actually use. All packet, frame, recovery, and stream
// types have been removed — OpenSSL 3.6 owns those entirely.

// ── Timestamp ─────────────────────────────────────────────────────────────────

alias Timestamp = ulong; // nanoseconds, monotonic
enum Timestamp TS_INFINITE = Timestamp.max;

// ── VarInt (kept for QPACK and H3 frame parsing) ──────────────────────────────

alias VarInt = ulong;
enum VarInt VARINT_MAX = (1UL << 62) - 1;

size_t varint_len(VarInt v) pure {
    if (v < (1UL << 6))  return 1;
    if (v < (1UL << 14)) return 2;
    if (v < (1UL << 30)) return 4;
    return 8;
}

size_t varint_encode(ubyte* buf, VarInt v) pure {
    if (v < (1UL << 6)) {
        buf[0] = cast(ubyte)v;
        return 1;
    }
    if (v < (1UL << 14)) {
        buf[0] = cast(ubyte)(0x40 | (v >> 8));
        buf[1] = cast(ubyte)v;
        return 2;
    }
    if (v < (1UL << 30)) {
        buf[0] = cast(ubyte)(0x80 | (v >> 24));
        buf[1] = cast(ubyte)(v >> 16);
        buf[2] = cast(ubyte)(v >> 8);
        buf[3] = cast(ubyte)v;
        return 4;
    }
    buf[0] = cast(ubyte)(0xC0 | (v >> 56));
    buf[1] = cast(ubyte)(v >> 48);
    buf[2] = cast(ubyte)(v >> 40);
    buf[3] = cast(ubyte)(v >> 32);
    buf[4] = cast(ubyte)(v >> 24);
    buf[5] = cast(ubyte)(v >> 16);
    buf[6] = cast(ubyte)(v >> 8);
    buf[7] = cast(ubyte)v;
    return 8;
}

// PORT-NOTE: C++ std::pair<VarInt, size_t> → D struct VarIntDecodeResult (value wrapper)
struct VarIntDecodeResult {
    // PORT-NOTE: value wrapper (struct), exempt from class-only rule
    VarInt val;
    size_t nbytes;
}

VarIntDecodeResult varint_decode(const(ubyte)* buf, size_t len) pure {
    if (len == 0) return VarIntDecodeResult(0, 0);
    ubyte first = buf[0];
    size_t nbytes = 1u << (first >> 6);
    if (len < nbytes) return VarIntDecodeResult(0, 0);
    VarInt val = first & 0x3F;
    for (size_t i = 1; i < nbytes; ++i)
        val = (val << 8) | buf[i];
    return VarIntDecodeResult(val, nbytes);
}

// ── ConnectionId ──────────────────────────────────────────────────────────────

enum size_t CID_MAX_LEN     = 20;
enum size_t CID_DEFAULT_LEN = 8;

// PORT-NOTE: C++ struct ConnectionId → D extern(C) struct (ABI POD).
// PORT-NOTE: ABI POD, extern(C) struct + PORT-NOTE per rules.
extern(C) struct ConnectionId {
    ubyte[CID_MAX_LEN] data;
    ubyte len = 0;

    static ConnectionId from_bytes(const(ubyte)[] bytes) {
        assert(bytes.length <= CID_MAX_LEN);
        ConnectionId cid;
        cid.len = cast(ubyte)bytes.length;
        memcpy(cid.data.ptr, bytes.ptr, cid.len);
        return cid;
    }

    bool opEquals(ref const ConnectionId o) const pure {
        return len == o.len && memcmp(data.ptr, o.data.ptr, len) == 0;
    }

    const(ubyte)[] view() const pure { return data[0 .. len]; }

    void hex_into(char* out_buf) const {
        static immutable char[16] d = "0123456789abcdef";
        for (ubyte i = 0; i < len; ++i) {
            out_buf[i * 2]     = d[data[i] >> 4];
            out_buf[i * 2 + 1] = d[data[i] & 0xF];
        }
    }
}

// PORT-NOTE: std::hash<ConnectionId> specialization → D toHash free function.
size_t connection_id_hash(ref const ConnectionId cid) pure {
    size_t h = 14695981039346656037UL;
    for (ubyte i = 0; i < cid.len; ++i)
        h = (h ^ cid.data[i]) * 1099511628211UL;
    return h;
}

// ── ConnState ─────────────────────────────────────────────────────────────────

enum ConnState { Handshaking, Connected, Closing, Closed }

// ── Stream direction helpers (used by server.http3) ───────────────────────────

bool stream_is_uni(ulong id) pure  { return (id & 0x2) != 0; }
bool stream_is_bidi(ulong id) pure { return (id & 0x2) == 0; }

// ── ByteReader / ByteWriter (used by QPACK and H3 frame parsing) ──────────────

// PORT-NOTE: C++ class ByteReader → D class ByteReader (has behavior).
class ByteReader {
  public:
    this(const(ubyte)[] buf) {
        m_buf = buf;
        m_pos = 0;
    }

    bool empty()     const pure { return m_pos >= m_buf.length; }
    size_t remaining() const pure { return m_buf.length - m_pos; }
    size_t pos()       const pure { return m_pos; }

    bool peek_u8(out ubyte out_val) const {
        if (empty()) return false;
        out_val = m_buf[m_pos];
        return true;
    }

    bool read_u8(out ubyte out_val) {
        if (empty()) return false;
        out_val = m_buf[m_pos++];
        return true;
    }

    bool read_varint(out VarInt out_val) {
        auto result = varint_decode(m_buf.ptr + m_pos, remaining());
        if (result.nbytes == 0) return false;
        out_val = result.val;
        m_pos += result.nbytes;
        return true;
    }

    const(ubyte)[] read_bytes(size_t n) {
        if (remaining() < n) return null;
        auto s = m_buf[m_pos .. m_pos + n];
        m_pos += n;
        return s;
    }

    bool skip(size_t n) {
        if (remaining() < n) return false;
        m_pos += n;
        return true;
    }

    const(ubyte)[] rest() const pure { return m_buf[m_pos .. $]; }

  private:
    const(ubyte)[] m_buf;
    size_t         m_pos;
}

// PORT-NOTE: C++ class ByteWriter → D class ByteWriter (has behavior).
class ByteWriter {
  public:
    this(ubyte[] buf) {
        m_buf = buf;
        m_pos = 0;
    }

    size_t written()   const pure { return m_pos; }
    size_t remaining() const pure { return m_buf.length - m_pos; }

    bool write_u8(ubyte v) {
        if (remaining() < 1) return false;
        m_buf[m_pos++] = v;
        return true;
    }

    bool write_varint(VarInt v) {
        const size_t n = varint_len(v);
        if (remaining() < n) return false;
        varint_encode(m_buf.ptr + m_pos, v);
        m_pos += n;
        return true;
    }

    bool write_bytes(const(ubyte)[] data) {
        if (remaining() < data.length) return false;
        memcpy(m_buf.ptr + m_pos, data.ptr, data.length);
        m_pos += data.length;
        return true;
    }

    ubyte[] reserve(size_t n) {
        if (remaining() < n) return null;
        auto s = m_buf[m_pos .. m_pos + n];
        m_pos += n;
        return s;
    }

    ubyte[] written_span() { return m_buf[0 .. m_pos]; }

  private:
    ubyte[] m_buf;
    size_t  m_pos;
}
