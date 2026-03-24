export module error:http;

import std;

export namespace error::http {

// HTTP/2 Error Codes as defined in RFC 9113
enum class Http2ErrorCode : std::uint32_t {
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
    HTTP_1_1_REQUIRED = 0x0d
};

// Base class for HTTP/2 protocol errors
class Http2Exception : public std::runtime_error {
  public:
    Http2Exception(Http2ErrorCode code, const std::string &msg) : std::runtime_error(msg), error_code(code) {}
    Http2ErrorCode code() const { return error_code; }

  private:
    Http2ErrorCode error_code;
};

// Represents a Stream Error (Section 5.4.2) [3]
class StreamError : public Http2Exception {
  public:
    StreamError(std::uint32_t stream_id, Http2ErrorCode code, const std::string &msg)
        : Http2Exception(code, msg), stream_id(stream_id) {}
    std::uint32_t streamId() const { return stream_id; }

  private:
    std::uint32_t stream_id;
};

// Represents a Connection Error (Section 5.4.1) [2]
class ConnectionError : public Http2Exception {
  public:
    using Http2Exception::Http2Exception;
};

// Base class for all HPACK decoding errors
class DecodeError : public std::runtime_error {
  public:
    explicit DecodeError(std::string msg) : std::runtime_error{std::move(msg)} {}
};

// Invalid index — index 0 or index beyond table size
class InvalidIndexError : public DecodeError {
  public:
    explicit InvalidIndexError(std::size_t index)
        : DecodeError{std::format("hpack: invalid index {}", index)}, m_index{index} {}

    std::size_t index() const noexcept { return m_index; }

  private:
    std::size_t m_index;
};

// Empty header name in a literal representation
class EmptyNameError : public DecodeError {
  public:
    EmptyNameError() : DecodeError{"hpack: literal header field has empty name"} {}
};

// Dynamic table size update exceeds the acknowledged limit
class TableSizeError : public DecodeError {
  public:
    TableSizeError(std::size_t requested, std::size_t limit)
        : DecodeError{std::format("hpack: table size update {} exceeds acknowledged limit {}", requested, limit)},
          m_requested{requested}, m_limit{limit} {}

    std::size_t requested() const noexcept { return m_requested; }
    std::size_t limit() const noexcept { return m_limit; }

  private:
    std::size_t m_requested;
    std::size_t m_limit;
};

// Unexpected end of header block
class TruncatedDataError : public DecodeError {
  public:
    TruncatedDataError() : DecodeError{"hpack: unexpected end of header block"} {}
};

// Huffman decoding failure
class HuffmanDecodeError : public DecodeError {
  public:
    explicit HuffmanDecodeError(std::string msg)
        : DecodeError{std::format("hpack: huffman error — {}", std::move(msg))} {}
};

// Integer overflow or malformed continuation
class IntegerDecodeError : public DecodeError {
  public:
    explicit IntegerDecodeError(std::string msg)
        : DecodeError{std::format("hpack: integer decode error — {}", std::move(msg))} {}
};

class StringDecodeError : public DecodeError {
  public:
    explicit StringDecodeError(std::string msg)
        : DecodeError{std::format("hpack: string decode error — {}", std::move(msg))} {}
};
} // namespace error::http
