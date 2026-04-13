export module io_layer_http2:consts;

import std;

export namespace transport::layer::http2 {

inline constexpr std::uint8_t HEADER_SIZE = 9;

// "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
inline constexpr std::string_view HTTP2_CONNECTION_PREFACE = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";

inline constexpr std::uint32_t DEFAULT_INITIAL_WINDOW_SIZE = (1u << 16) - 1; // 65535 (2^16 - 1)
inline constexpr std::uint32_t MAX_INITIAL_WINDOW_SIZE = (1u << 31) - 1;     // 2147483647 (2^31 - 1)

inline constexpr std::uint32_t MAX_CONNECTED_STREAMS = (1u << 31) - 1; // 2147483647 (2^31 - 1)

} // namespace transport::layer::http2
