export module io_error:http;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::error::http {

// HTTP/2 Error Codes as defined in RFC 9113
enum class Http2ErrorCode : std::uint8_t {
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

constexpr Http2ErrorCode get_http2_error_code(std::uint32_t code) {
    // Straight roundtrip for every known RFC 9113 code — the switch just validates `code`
    // actually maps to a real enumerator instead of blindly casting an arbitrary uint32.
    switch (code) {
    case std::to_underlying(Http2ErrorCode::NO_ERROR_CODE):
        return Http2ErrorCode::NO_ERROR_CODE;
    case std::to_underlying(Http2ErrorCode::PROTOCOL_ERROR):
        return Http2ErrorCode::PROTOCOL_ERROR;
    case std::to_underlying(Http2ErrorCode::INTERNAL_ERROR):
        return Http2ErrorCode::INTERNAL_ERROR;
    case std::to_underlying(Http2ErrorCode::FLOW_CONTROL_ERROR):
        return Http2ErrorCode::FLOW_CONTROL_ERROR;
    case std::to_underlying(Http2ErrorCode::SETTINGS_TIMEOUT):
        return Http2ErrorCode::SETTINGS_TIMEOUT;
    case std::to_underlying(Http2ErrorCode::STREAM_CLOSED):
        return Http2ErrorCode::STREAM_CLOSED;
    case std::to_underlying(Http2ErrorCode::FRAME_SIZE_ERROR):
        return Http2ErrorCode::FRAME_SIZE_ERROR;
    case std::to_underlying(Http2ErrorCode::REFUSED_STREAM):
        return Http2ErrorCode::REFUSED_STREAM;
    case std::to_underlying(Http2ErrorCode::CANCEL):
        return Http2ErrorCode::CANCEL;
    case std::to_underlying(Http2ErrorCode::COMPRESSION_ERROR):
        return Http2ErrorCode::COMPRESSION_ERROR;
    case std::to_underlying(Http2ErrorCode::CONNECT_ERROR):
        return Http2ErrorCode::CONNECT_ERROR;
    case std::to_underlying(Http2ErrorCode::ENHANCE_YOUR_CALM):
        return Http2ErrorCode::ENHANCE_YOUR_CALM;
    case std::to_underlying(Http2ErrorCode::INADEQUATE_SECURITY):
        return Http2ErrorCode::INADEQUATE_SECURITY;
    case std::to_underlying(Http2ErrorCode::HTTP_1_1_REQUIRED):
        return Http2ErrorCode::HTTP_1_1_REQUIRED;
    default:
        // Default to INTERNAL_ERROR for unknown codes
        return Http2ErrorCode::INTERNAL_ERROR;
    }
}

// Base class for HTTP/2 protocol errors
class Http2Exception : public std::runtime_error {
  public:
    /**
     * @brief Builds an Http2Exception tagging the RFC 9113 error code onto a plain runtime_error
     * message.
     * @param code the HTTP/2 error code (RFC 9113 §7) this exception represents.
     * @param msg human-readable description, forwarded straight to `std::runtime_error`.
     */
    Http2Exception(Http2ErrorCode code, const std::string &msg) : std::runtime_error(msg), m_error_code(code) {}
    /**
     * @brief Gets the RFC 9113 error code this exception carries.
     * @return the HTTP/2 error code.
     */
    [[nodiscard]] Http2ErrorCode get_code() const { return m_error_code; }

  private:
    Http2ErrorCode m_error_code;
};

// Represents a Stream Error (Section 5.4.2) [3]
class StreamError : public Http2Exception {
  public:
    /**
     * @brief Builds a StreamError — a per-stream HTTP/2 failure (RFC 9113 §5.4.2), scoped to one
     * stream instead of tanking the whole connection.
     * @param stream_id the stream this error is scoped to.
     * @param code the HTTP/2 error code for this failure.
     * @param msg human-readable description.
     */
    StreamError(std::uint32_t stream_id, Http2ErrorCode code, const std::string &msg)
        : Http2Exception(code, msg), m_stream_id{stream_id} {}

    /**
     * @brief Gets the stream id this error is scoped to.
     * @return the affected stream's id.
     */
    [[nodiscard]] std::uint32_t get_stream_id() const { return m_stream_id; }

  private:
    std::uint32_t m_stream_id;
};

inline constexpr std::uint32_t MAX_CONNECTED_STREAMS = (1U << 31) - 1; // 2147483647 (2^31 - 1)
// Represents a Connection Error (Section 5.4.1) [2]
class ConnectionError : public Http2Exception {
  public:
    /**
     * @brief Builds a ConnectionError — a connection-wide HTTP/2 failure (RFC 9113 §5.4.1), the
     * whole session's cooked once one of these fires, not just a single stream.
     * @param code the HTTP/2 error code for this failure.
     * @param msg human-readable description.
     * @param last_stream_id the highest stream id the sender processed before bailing, defaults
     * to `MAX_CONNECTED_STREAMS` when the caller doesn't know or care.
     */
    ConnectionError(Http2ErrorCode code, const std::string &msg, std::uint32_t last_stream_id = MAX_CONNECTED_STREAMS)
        : Http2Exception{code, msg}, m_last_stream_id{last_stream_id} {}

    /**
     * @brief Gets the last stream id the sender processed before the connection went down.
     * @return the last processed stream id.
     */
    [[nodiscard]] std::uint32_t get_last_stream_id() const { return m_last_stream_id; }

  private:
    std::uint32_t m_last_stream_id;
};

// Base class for all HPACK decoding errors
class DecodeError : public std::runtime_error {
  public:
    /**
     * @brief Builds a bare DecodeError wrapping any HPACK decode failure message.
     * @param msg the failure description, moved into the base `runtime_error`.
     */
    explicit DecodeError(const std::string &msg) : std::runtime_error{msg} {}
};

// Invalid index — index 0 or index beyond table size
template <std::unsigned_integral UInt = std::uint32_t>
class InvalidIndexError : public DecodeError {
  public:
    /**
     * @brief Builds an InvalidIndexError for an HPACK index that's either 0 or past the combined
     * static+dynamic table size — lowkey just straight out of bounds, no valid header lives
     * there.
     * @param index the offending index.
     */
    explicit InvalidIndexError(UInt index)
        : DecodeError{std::format("hpack: invalid index {}", index)}, m_index{index} {}

    /**
     * @brief Gets the out-of-range index that triggered this error.
     * @return the offending index.
     */
    [[nodiscard]] std::size_t index() const noexcept { return m_index; }

  private:
    std::size_t m_index;
};

// Empty header name in a literal representation
class EmptyNameError : public DecodeError {
  public:
    /**
     * @brief Builds an EmptyNameError — fires when a literal header field representation shows up
     * with a zero-length name, which HPACK doesn't allow.
     */
    EmptyNameError() : DecodeError{"hpack: literal header field has empty name"} {}
};

// Dynamic table size update exceeds the acknowledged limit
class TableSizeError : public DecodeError {
  public:
    /**
     * @brief Builds a TableSizeError — the peer tried to grow the HPACK dynamic table past the
     * size limit both sides already agreed on. Straight L, that update's getting rejected.
     * @param requested the table size the peer asked for.
     * @param limit the acknowledged max size that got exceeded.
     */
    TableSizeError(std::size_t requested, std::size_t limit)
        : DecodeError{std::format("hpack: table size update {} exceeds acknowledged limit {}", requested, limit)},
          m_requested{requested}, m_limit{limit} {}

    /**
     * @brief Gets the table size that was requested.
     * @return the requested size.
     */
    [[nodiscard]] std::size_t requested() const noexcept { return m_requested; }
    /**
     * @brief Gets the acknowledged limit that got exceeded.
     * @return the size limit.
     */
    [[nodiscard]] std::size_t limit() const noexcept { return m_limit; }

  private:
    std::size_t m_requested;
    std::size_t m_limit;
};

// Unexpected end of header block
class TruncatedDataError : public DecodeError {
  public:
    /**
     * @brief Builds a TruncatedDataError — the header block ran out of bytes mid-decode, cut off
     * before a complete field could be read.
     */
    TruncatedDataError() : DecodeError{"hpack: unexpected end of header block"} {}
};

// Huffman decoding failure
class HuffmanDecodeError : public DecodeError {
  public:
    /**
     * @brief Builds a HuffmanDecodeError wrapping a Huffman-specific decode failure.
     * @param msg the failure description.
     */
    explicit HuffmanDecodeError(const std::string &msg)
        : DecodeError{std::format("hpack: huffman error — {}", msg)} {}
};

// Integer overflow or malformed continuation
class IntegerDecodeError : public DecodeError {
  public:
    /**
     * @brief Builds an IntegerDecodeError — fires on HPACK varint overflow or a malformed
     * continuation byte sequence, bet.
     * @param msg the failure description.
     */
    explicit IntegerDecodeError(const std::string &msg)
        : DecodeError{std::format("hpack: integer decode error — {}", msg)} {}
};

class StringDecodeError : public DecodeError {
  public:
    /**
     * @brief Builds a StringDecodeError wrapping a string-literal decode failure.
     * @param msg the failure description.
     */
    explicit StringDecodeError(const std::string &msg)
        : DecodeError{std::format("hpack: string decode error — {}", msg)} {}
};


class CompressionError : public std::runtime_error {
  public:
    /**
     * @brief Builds a bare CompressionError wrapping any HPACK compression-context failure.
     * @param msg the failure description, moved into the base `runtime_error`.
     */
    explicit CompressionError(const std::string &msg) : std::runtime_error{msg} {}
};

} // namespace io::error::http

export template <>
struct std::formatter<io::error::http::Http2ErrorCode> {
    /**
     * @brief Parses the format spec for `Http2ErrorCode` — there isn't one, this thing only ever
     * prints its name, so parsing is just handing back the untouched begin iterator.
     * @param ctx the format parse context.
     * @return iterator to the (unconsumed) start of the format spec.
     */
    static constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    /**
     * @brief Formats a `Http2ErrorCode` as its RFC 9113 name (e.g. `PROTOCOL_ERROR`), no cap
     * falling back to `"UNKNOWN"` for anything that doesn't match a known enumerator.
     * @tparam FormatContext the format context type, deduced by `std::format`.
     * @param error_code the error code to format.
     * @param ctx the format context to write into.
     * @return output iterator past the written name.
     */
    template <typename FormatContext>
    auto format(io::error::http::Http2ErrorCode error_code, FormatContext &ctx) const {
        using enum io::error::http::Http2ErrorCode;
        std::string_view name;
        // Every enumerator maps to its own literal name; anything unmatched falls to "UNKNOWN"
        // down in the default case below.
        switch (error_code) {
        case NO_ERROR_CODE: {
            name = "NO_ERROR_CODE";
            break;
        }
        case PROTOCOL_ERROR: {
            name = "PROTOCOL_ERROR";
            break;
        }
        case INTERNAL_ERROR: {
            name = "INTERNAL_ERROR";
            break;
        }
        case FLOW_CONTROL_ERROR: {
            name = "FLOW_CONTROL_ERROR";
            break;
        }
        case SETTINGS_TIMEOUT: {
            name = "SETTINGS_TIMEOUT";
            break;
        }
        case STREAM_CLOSED: {
            name = "STREAM_CLOSED";
            break;
        }
        case FRAME_SIZE_ERROR: {
            name = "FRAME_SIZE_ERROR";
            break;
        }
        case REFUSED_STREAM: {
            name = "REFUSED_STREAM";
            break;
        }
        case CANCEL: {
            name = "CANCEL";
            break;
        }
        case COMPRESSION_ERROR: {
            name = "COMPRESSION_ERROR";
            break;
        }
        case CONNECT_ERROR: {
            name = "CONNECT_ERROR";
            break;
        }
        case ENHANCE_YOUR_CALM: {
            name = "ENHANCE_YOUR_CALM";
            break;
        }
        case INADEQUATE_SECURITY: {
            name = "INADEQUATE_SECURITY";
            break;
        }
        case HTTP_1_1_REQUIRED: {
            name = "HTTP_1_1_REQUIRED";
            break;
        }
        default: {
            name = "UNKNOWN";
            break;
        }
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};

#ifdef CONGELADO_TEST
namespace io::error::http::tests {
using namespace boost::ut;

suite<"get_http2_error_code"> get_http2_error_code_suite = [] {
    "round-trips every known RFC 9113 code"_test = [] {
        expect(get_http2_error_code(0x00) == Http2ErrorCode::NO_ERROR_CODE);
        expect(get_http2_error_code(0x01) == Http2ErrorCode::PROTOCOL_ERROR);
        expect(get_http2_error_code(0x02) == Http2ErrorCode::INTERNAL_ERROR);
        expect(get_http2_error_code(0x03) == Http2ErrorCode::FLOW_CONTROL_ERROR);
        expect(get_http2_error_code(0x04) == Http2ErrorCode::SETTINGS_TIMEOUT);
        expect(get_http2_error_code(0x05) == Http2ErrorCode::STREAM_CLOSED);
        expect(get_http2_error_code(0x06) == Http2ErrorCode::FRAME_SIZE_ERROR);
        expect(get_http2_error_code(0x07) == Http2ErrorCode::REFUSED_STREAM);
        expect(get_http2_error_code(0x08) == Http2ErrorCode::CANCEL);
        expect(get_http2_error_code(0x09) == Http2ErrorCode::COMPRESSION_ERROR);
        expect(get_http2_error_code(0x0a) == Http2ErrorCode::CONNECT_ERROR);
        expect(get_http2_error_code(0x0b) == Http2ErrorCode::ENHANCE_YOUR_CALM);
        expect(get_http2_error_code(0x0c) == Http2ErrorCode::INADEQUATE_SECURITY);
        expect(get_http2_error_code(0x0d) == Http2ErrorCode::HTTP_1_1_REQUIRED);
    };

    "falls back to INTERNAL_ERROR for an unrecognized code"_test = [] {
        expect(get_http2_error_code(0xffffffffU) == Http2ErrorCode::INTERNAL_ERROR);
    };
};

suite<"Http2Exception"> http2_exception_suite = [] {
    "carries the error code and message it was built with"_test = [] {
        Http2Exception error(Http2ErrorCode::PROTOCOL_ERROR, "boom");

        expect(error.get_code() == Http2ErrorCode::PROTOCOL_ERROR);
        expect(std::string_view(error.what()) == "boom");
    };
};

suite<"StreamError"> stream_error_suite = [] {
    "carries the stream id alongside the inherited code and message"_test = [] {
        StreamError error(7, Http2ErrorCode::CANCEL, "stream gone");

        expect(error.get_stream_id() == 7);
        expect(error.get_code() == Http2ErrorCode::CANCEL);
        expect(std::string_view(error.what()) == "stream gone");
    };
};

suite<"ConnectionError"> connection_error_suite = [] {
    "defaults the last stream id to MAX_CONNECTED_STREAMS"_test = [] {
        ConnectionError error(Http2ErrorCode::INTERNAL_ERROR, "down");

        expect(error.get_last_stream_id() == MAX_CONNECTED_STREAMS);
        expect(error.get_code() == Http2ErrorCode::INTERNAL_ERROR);
    };

    "accepts an explicit last stream id"_test = [] {
        ConnectionError error(Http2ErrorCode::FLOW_CONTROL_ERROR, "down", 42);

        expect(error.get_last_stream_id() == 42);
    };
};

suite<"DecodeError"> decode_error_suite = [] {
    "wraps the given message"_test = [] {
        DecodeError error("bad decode");

        expect(std::string_view(error.what()) == "bad decode");
    };
};

suite<"InvalidIndexError"> invalid_index_error_suite = [] {
    "carries the offending index and formats a message"_test = [] {
        InvalidIndexError<> error(5);

        expect(error.index() == 5);
        expect(std::string_view(error.what()) == "hpack: invalid index 5");
    };
};

suite<"EmptyNameError"> empty_name_error_suite = [] {
    "has a fixed message"_test = [] {
        EmptyNameError error;

        expect(std::string_view(error.what()) == "hpack: literal header field has empty name");
    };
};

suite<"TableSizeError"> table_size_error_suite = [] {
    "carries the requested size and the exceeded limit"_test = [] {
        TableSizeError error(100, 50);

        expect(error.requested() == 100);
        expect(error.limit() == 50);
        expect(std::string_view(error.what()) ==
               "hpack: table size update 100 exceeds acknowledged limit 50");
    };
};

suite<"TruncatedDataError"> truncated_data_error_suite = [] {
    "has a fixed message"_test = [] {
        TruncatedDataError error;

        expect(std::string_view(error.what()) == "hpack: unexpected end of header block");
    };
};

suite<"HuffmanDecodeError"> huffman_decode_error_suite = [] {
    "wraps the given message"_test = [] {
        HuffmanDecodeError error("bad code");

        expect(std::string_view(error.what()) == "hpack: huffman error — bad code");
    };
};

suite<"IntegerDecodeError"> integer_decode_error_suite = [] {
    "wraps the given message"_test = [] {
        IntegerDecodeError error("overflow");

        expect(std::string_view(error.what()) == "hpack: integer decode error — overflow");
    };
};

suite<"StringDecodeError"> string_decode_error_suite = [] {
    "wraps the given message"_test = [] {
        StringDecodeError error("bad string");

        expect(std::string_view(error.what()) == "hpack: string decode error — bad string");
    };
};

suite<"CompressionError"> compression_error_suite = [] {
    "wraps the given message"_test = [] {
        CompressionError error("context lost");

        expect(std::string_view(error.what()) == "context lost");
    };
};

suite<"Http2ErrorCode formatter"> http2_error_code_formatter_suite = [] {
    "formats a known code as its RFC 9113 name"_test = [] {
        expect(std::format("{}", Http2ErrorCode::PROTOCOL_ERROR) == "PROTOCOL_ERROR");
        expect(std::format("{}", Http2ErrorCode::HTTP_1_1_REQUIRED) == "HTTP_1_1_REQUIRED");
    };

    "formats an unrecognized value as UNKNOWN"_test = [] {
        auto bogus = static_cast<Http2ErrorCode>(0xff);
        expect(std::format("{}", bogus) == "UNKNOWN");
    };
};

} // namespace io::error::http::tests
#endif
