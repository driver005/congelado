export module io_layer_http2:consts;

import std;

export namespace io::layer::http2 {

inline constexpr std::uint8_t HEADER_SIZE = 9;

// "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
// Ensure this is an array of std::byte
inline constexpr std::array<std::byte, 24> HTTP2_CONNECTION_PREFACE = [] {
    std::string_view sv = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    std::array<std::byte, 24> arr{};
    for (std::size_t i = 0; i < 24; ++i)
        arr[i] = static_cast<std::byte>(sv[i]);
    return arr;
}();

inline constexpr std::uint32_t DEFAULT_INITIAL_WINDOW_SIZE = (1u << 16) - 1; // 65535 (2^16 - 1)
inline constexpr std::uint32_t MAX_INITIAL_WINDOW_SIZE = (1u << 31) - 1;     // 2147483647 (2^31 - 1)

inline constexpr std::uint32_t MAX_CONNECTED_STREAMS = (1u << 31) - 1; // 2147483647 (2^31 - 1)

inline constexpr std::uint32_t DEFAULT_HEADER_TABLE_SIZE = 4096;
inline constexpr std::uint32_t MIN_FRAME_SIZE = 1u << 14;       // 16384 (2^14)
inline constexpr std::uint32_t MAX_FRAME_SIZE = (1u << 24) - 1; // 16777215 (2^24 - 1)

} // namespace io::layer::http2
