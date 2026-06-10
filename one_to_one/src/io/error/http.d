module io.error.http;
@nogc nothrow:

// HTTP/2 Error Codes as defined in RFC 9113
enum Http2ErrorCode : uint {
    // The condition is not a result of an error; used for graceful shutdown [2].
    NO_ERROR_CODE = 0x00,

    // Unspecific protocol error; used when a more specific code is not available [2].
    PROTOCOL_ERROR = 0x01,

    // The endpoint encountered an unexpected internal error [2].
    INTERNAL_ERROR = 0x02,

    // The peer violated the flow-control protocol [2].
    FLOW_CONTROL_ERROR = 0x03,

    // A SETTINGS frame was sent but not acknowledged in a timely manner [3].
    SETTINGS_TIMEOUT = 0x04,

    // A frame was received after a stream was already half-closed [3].
    STREAM_CLOSED = 0x05,

    // A frame was received with an invalid size [3].
    FRAME_SIZE_ERROR = 0x06,

    // The stream was refused prior to any application processing [3].
    REFUSED_STREAM = 0x07,

    // The stream is no longer needed [3].
    CANCEL = 0x08,

    // The compression context for the connection could not be maintained [4].
    COMPRESSION_ERROR = 0x09,

    // The connection for a CONNECT request was reset or abnormally closed [4].
    CONNECT_ERROR = 0x0a,

    // The peer is exhibiting behavior that might be generating excessive load [4].
    ENHANCE_YOUR_CALM = 0x0b,

    // The underlying transport does not meet minimum security requirements [4].
    INADEQUATE_SECURITY = 0x0c,

    // The endpoint requires that HTTP/1.1 be used instead of HTTP/2 [5].
    HTTP_1_1_REQUIRED = 0x0d,
}

Http2ErrorCode get_http2_error_code(uint code) {
    switch (code) {
    case Http2ErrorCode.NO_ERROR_CODE:       return Http2ErrorCode.NO_ERROR_CODE;
    case Http2ErrorCode.PROTOCOL_ERROR:      return Http2ErrorCode.PROTOCOL_ERROR;
    case Http2ErrorCode.INTERNAL_ERROR:      return Http2ErrorCode.INTERNAL_ERROR;
    case Http2ErrorCode.FLOW_CONTROL_ERROR:  return Http2ErrorCode.FLOW_CONTROL_ERROR;
    case Http2ErrorCode.SETTINGS_TIMEOUT:    return Http2ErrorCode.SETTINGS_TIMEOUT;
    case Http2ErrorCode.STREAM_CLOSED:       return Http2ErrorCode.STREAM_CLOSED;
    case Http2ErrorCode.FRAME_SIZE_ERROR:    return Http2ErrorCode.FRAME_SIZE_ERROR;
    case Http2ErrorCode.REFUSED_STREAM:      return Http2ErrorCode.REFUSED_STREAM;
    case Http2ErrorCode.CANCEL:              return Http2ErrorCode.CANCEL;
    case Http2ErrorCode.COMPRESSION_ERROR:   return Http2ErrorCode.COMPRESSION_ERROR;
    case Http2ErrorCode.CONNECT_ERROR:       return Http2ErrorCode.CONNECT_ERROR;
    case Http2ErrorCode.ENHANCE_YOUR_CALM:   return Http2ErrorCode.ENHANCE_YOUR_CALM;
    case Http2ErrorCode.INADEQUATE_SECURITY: return Http2ErrorCode.INADEQUATE_SECURITY;
    case Http2ErrorCode.HTTP_1_1_REQUIRED:   return Http2ErrorCode.HTTP_1_1_REQUIRED;
    default:
        // Default to INTERNAL_ERROR for unknown codes
        return Http2ErrorCode.INTERNAL_ERROR;
    }
}

// PORT-NOTE: All exception classes (Http2Exception, StreamError, ConnectionError, DecodeError,
// InvalidIndexError, EmptyNameError, TableSizeError, TruncatedDataError, HuffmanDecodeError,
// IntegerDecodeError, StringDecodeError, CompressionError) are ported to plain structs carrying
// the error code and a static message tag. Dynamic format strings are omitted (@nogc); callers
// should format messages outside of these structs if needed.
// The to_string(Http2ErrorCode) function replaces std::formatter<Http2ErrorCode>.

// Base class for HTTP/2 protocol errors
// PORT-NOTE: value wrapper struct
struct Http2Exception {
    Http2ErrorCode error_code;

    Http2ErrorCode get_code() const { return error_code; }
}

// Represents a Stream Error (Section 5.4.2) [3]
// PORT-NOTE: value wrapper struct
struct StreamError {
    Http2ErrorCode error_code;
    uint           m_stream_id;

    uint           get_stream_id() const { return m_stream_id; }
    Http2ErrorCode get_code()      const { return error_code; }
}

immutable uint MAX_CONNECTED_STREAMS = (1u << 31) - 1; // 2147483647 (2^31 - 1)

// Represents a Connection Error (Section 5.4.1) [2]
// PORT-NOTE: value wrapper struct
struct ConnectionError {
    Http2ErrorCode error_code;
    uint           m_last_stream_id = MAX_CONNECTED_STREAMS;

    uint           get_last_stream_id() const { return m_last_stream_id; }
    Http2ErrorCode get_code()           const { return error_code; }
}

// Base struct for all HPACK decoding errors
// PORT-NOTE: value wrapper struct; message is not stored (@nogc); tag enum is used instead
enum DecodeErrorKind : ubyte {
    generic,
    invalid_index,
    empty_name,
    table_size,
    truncated_data,
    huffman_decode,
    integer_decode,
    string_decode,
}

struct DecodeError {
    DecodeErrorKind kind = DecodeErrorKind.generic;
}

// Invalid index — index 0 or index beyond table size
// PORT-NOTE: UInt template param collapsed to size_t; value wrapper struct
struct InvalidIndexError {
    DecodeErrorKind kind  = DecodeErrorKind.invalid_index;
    size_t          m_index;

    size_t index() const { return m_index; }
}

// Empty header name in a literal representation
// PORT-NOTE: value wrapper struct
struct EmptyNameError {
    DecodeErrorKind kind = DecodeErrorKind.empty_name;
}

// Dynamic table size update exceeds the acknowledged limit
// PORT-NOTE: value wrapper struct; format string dropped
struct TableSizeError {
    DecodeErrorKind kind        = DecodeErrorKind.table_size;
    size_t          m_requested;
    size_t          m_limit;

    size_t requested() const { return m_requested; }
    size_t limit()     const { return m_limit; }
}

// Unexpected end of header block
// PORT-NOTE: value wrapper struct
struct TruncatedDataError {
    DecodeErrorKind kind = DecodeErrorKind.truncated_data;
}

// Huffman decoding failure
// PORT-NOTE: value wrapper struct; format string dropped
struct HuffmanDecodeError {
    DecodeErrorKind kind = DecodeErrorKind.huffman_decode;
}

// Integer overflow or malformed continuation
// PORT-NOTE: value wrapper struct; format string dropped
struct IntegerDecodeError {
    DecodeErrorKind kind = DecodeErrorKind.integer_decode;
}

// PORT-NOTE: value wrapper struct; format string dropped
struct StringDecodeError {
    DecodeErrorKind kind = DecodeErrorKind.string_decode;
}

// PORT-NOTE: value wrapper struct
struct CompressionError {
    // no extra payload beyond the type tag
}

// Replaces std::formatter<Http2ErrorCode>
const(char)[] to_string(Http2ErrorCode code) {
    final switch (code) {
    case Http2ErrorCode.NO_ERROR_CODE:       return "NO_ERROR_CODE";
    case Http2ErrorCode.PROTOCOL_ERROR:      return "PROTOCOL_ERROR";
    case Http2ErrorCode.INTERNAL_ERROR:      return "INTERNAL_ERROR";
    case Http2ErrorCode.FLOW_CONTROL_ERROR:  return "FLOW_CONTROL_ERROR";
    case Http2ErrorCode.SETTINGS_TIMEOUT:    return "SETTINGS_TIMEOUT";
    case Http2ErrorCode.STREAM_CLOSED:       return "STREAM_CLOSED";
    case Http2ErrorCode.FRAME_SIZE_ERROR:    return "FRAME_SIZE_ERROR";
    case Http2ErrorCode.REFUSED_STREAM:      return "REFUSED_STREAM";
    case Http2ErrorCode.CANCEL:              return "CANCEL";
    case Http2ErrorCode.COMPRESSION_ERROR:   return "COMPRESSION_ERROR";
    case Http2ErrorCode.CONNECT_ERROR:       return "CONNECT_ERROR";
    case Http2ErrorCode.ENHANCE_YOUR_CALM:   return "ENHANCE_YOUR_CALM";
    case Http2ErrorCode.INADEQUATE_SECURITY: return "INADEQUATE_SECURITY";
    case Http2ErrorCode.HTTP_1_1_REQUIRED:   return "HTTP_1_1_REQUIRED";
    }
}
