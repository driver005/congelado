module io.layer.http2.consts;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.consts.
// C++ used std::array<std::byte,24> with a constexpr lambda init.
// D: ubyte[24] static immutable, initialised with a CTFE helper function.

enum ubyte HEADER_SIZE = 9;

// "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
private ubyte[24] build_preface() pure {
    immutable char[] sv = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    ubyte[24] arr;
    foreach (i; 0 .. 24)
        arr[i] = cast(ubyte) sv[i];
    return arr;
}
static immutable ubyte[24] HTTP2_CONNECTION_PREFACE = build_preface();

enum uint DEFAULT_INITIAL_WINDOW_SIZE = (1u << 16) - 1; // 65535 (2^16 - 1)
enum uint MAX_INITIAL_WINDOW_SIZE     = (1u << 31) - 1; // 2147483647 (2^31 - 1)

enum uint MAX_CONNECTED_STREAMS = (1u << 31) - 1; // 2147483647 (2^31 - 1)

enum uint DEFAULT_HEADER_TABLE_SIZE = 4096;
enum uint MIN_FRAME_SIZE            = 1u << 14;       // 16384 (2^14)
enum uint MAX_FRAME_SIZE            = (1u << 24) - 1; // 16777215 (2^24 - 1)
