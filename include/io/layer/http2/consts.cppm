export module io_layer_http2:consts;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::layer::http2 {

inline constexpr std::uint8_t HEADER_SIZE = 9;

// "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
// Ensure this is an array of std::byte
inline constexpr std::array<std::byte, 24> HTTP2_CONNECTION_PREFACE = [] {
    std::string_view sv = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    std::array<std::byte, 24> arr{};
    // Copy each char of the literal preface into the byte array one at a time — no
    // reinterpret_cast shortcuts here, keeps it constexpr-friendly, bet.
    for (std::size_t i = 0; i < 24; ++i) {
        arr[i] = static_cast<std::byte>(
            sv[i]
        ); // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }
    return arr;
}();

inline constexpr std::uint32_t DEFAULT_INITIAL_WINDOW_SIZE = (1U << 16) - 1; // 65535 (2^16 - 1)
inline constexpr std::uint32_t MAX_INITIAL_WINDOW_SIZE = (1U << 31) - 1; // 2147483647 (2^31 - 1)

inline constexpr std::uint32_t MAX_CONNECTED_STREAMS = (1U << 31) - 1; // 2147483647 (2^31 - 1)

inline constexpr std::uint32_t DEFAULT_HEADER_TABLE_SIZE = 4'096;
inline constexpr std::uint32_t MIN_FRAME_SIZE = 1U << 14;       // 16384 (2^14)
inline constexpr std::uint32_t MAX_FRAME_SIZE = (1U << 24) - 1; // 16777215 (2^24 - 1)

} // namespace io::layer::http2

#ifdef CONGELADO_TEST
namespace io::layer::http2::tests {
using namespace boost::ut;

suite<"http2 consts"> http2_consts_suite = [] {
    "HEADER_SIZE is the fixed 9-byte HTTP/2 frame header"_test = [] {
        expect(HEADER_SIZE == 9);
    };

    "HTTP2_CONNECTION_PREFACE spells out the RFC 9113 magic string"_test = [] {
        constexpr std::string_view EXPECTED = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";

        expect(HTTP2_CONNECTION_PREFACE.size() == 24);
        expect(HTTP2_CONNECTION_PREFACE.size() == EXPECTED.size());

        bool matches = true;
        for (std::size_t i = 0; i < HTTP2_CONNECTION_PREFACE.size(); ++i) {
            if (HTTP2_CONNECTION_PREFACE[i] != static_cast<std::byte>(EXPECTED[i])) {
                matches = false;
                break;
            }
        }
        expect(matches);
    };

    "window/stream/table/frame size limits match RFC 9113 defaults"_test = [] {
        expect(DEFAULT_INITIAL_WINDOW_SIZE == 65'535U);
        expect(MAX_INITIAL_WINDOW_SIZE == 2'147'483'647U);
        expect(MAX_CONNECTED_STREAMS == 2'147'483'647U);
        expect(DEFAULT_HEADER_TABLE_SIZE == 4'096U);
        expect(MIN_FRAME_SIZE == 16'384U);
        expect(MAX_FRAME_SIZE == 16'777'215U);
    };
};

} // namespace io::layer::http2::tests
#endif
