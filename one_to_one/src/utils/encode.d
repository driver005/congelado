module utils.encode;
@nogc nothrow:

// url_encode: encode a byte slice as a percent-encoded URL string.
// Caller supplies output buffer; returns written slice length.
// PORT-NOTE: C++ version returned std::string (GC). D version writes into
// caller-owned char[] and returns the number of bytes written.
size_t url_encode(const(ubyte)[] input, char[] out_buf) {
    static immutable char[16] HEX = "0123456789ABCDEF";
    size_t pos = 0;
    foreach (b; input) {
        if ((b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z') ||
            (b >= '0' && b <= '9') || b == '-' || b == '.' ||
            b == '_' || b == '~') {
            if (pos < out_buf.length)
                out_buf[pos++] = cast(char) b;
        } else {
            if (pos + 2 < out_buf.length) {
                out_buf[pos++] = '%';
                out_buf[pos++] = HEX[(b >> 4) & 0xF];
                out_buf[pos++] = HEX[b & 0xF];
            }
        }
    }
    return pos;
}

// base64_encode: encode a byte slice as base64 into caller-owned buffer.
// Returns the number of bytes written (including padding '=').
// PORT-NOTE: C++ version returned std::string (GC). D version writes into
// caller-owned char[] and returns the number of bytes written.
size_t base64_encode(const(ubyte)[] input, char[] out_buf) {
    static immutable char[64] TABLE =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t pos = 0;
    uint val = 0;
    int bits = 0;
    foreach (b; input) {
        val = (val << 8) | b;
        bits += 8;
        while (bits >= 6) {
            bits -= 6;
            if (pos < out_buf.length)
                out_buf[pos++] = TABLE[(val >> bits) & 0x3F];
        }
    }
    if (bits > 0) {
        if (pos < out_buf.length)
            out_buf[pos++] = TABLE[(val << (6 - bits)) & 0x3F];
        while (pos % 4 != 0) {
            if (pos < out_buf.length)
                out_buf[pos++] = '=';
            else
                break;
        }
    }
    return pos;
}
