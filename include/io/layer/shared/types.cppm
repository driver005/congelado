export module io_layer_shared:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::shared_layer {

enum class StreamState : std::uint8_t {
    IDLE,
    OPEN,
    HALF_CLOSED_LOCAL,
    HALF_CLOSED_REMOTE,
    CLOSED,
    RESERVED_LOCAL,
    RESERVED_REMOTE,
};

enum class FrameRole : bool { SENDER, RECEIVER };

struct Flags {
    static constexpr std::uint8_t END_STREAM = 0x01;
    static constexpr std::uint8_t ACK = 0x01;
    static constexpr std::uint8_t END_HEADERS = 0x04;
    static constexpr std::uint8_t PADDED = 0x08;
    static constexpr std::uint8_t PRIORITY = 0x20;
};

enum class FrameType : std::uint8_t {
    DATA = 0x0,
    HEADERS = 0x1,
    PRIORITY = 0x2, // deprecated §5.3.2, still parsed for interop
    RST_STREAM = 0x3,
    SETTINGS = 0x4,
    PUSH_PROMISE = 0x5,
    PING = 0x6,
    GOAWAY = 0x7,
    WINDOW_UPDATE = 0x8,
    CONTINUATION = 0x9
};


} // namespace io::shared_layer

export template <>
struct std::formatter<io::shared_layer::StreamState> {
    /**
     * @brief No format-spec support here — StreamState only ever prints as its plain name, so
     * parsing just accepts an empty spec and bounces straight back.
     * @param ctx the format parse context.
     * @return iterator to the start of the (expected-empty) format spec.
     */
    static constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    /**
     * @brief Maps a StreamState onto its human-readable name (e.g. "Idle", "HalfClosedLocal")
     * and writes it straight out — any value outside the known enumerators falls back to
     * "UNKNOWN", no crashes.
     * @tparam FormatContext the format context type, deduced by `std::format`.
     * @param state the StreamState to format.
     * @param ctx the format context to write into.
     * @return output iterator past the written name.
     */
    template <typename FormatContext>
    auto format(io::shared_layer::StreamState state, FormatContext &ctx) const {
        using enum io::shared_layer::StreamState;
        std::string_view name;
        // Map the enum value onto its human-readable name; anything unrecognized falls back to "UNKNOWN".
        switch (state) {
        case IDLE: {
            name = "Idle";
            break;
        }
        case OPEN: {
            name = "Open";
            break;
        }
        case HALF_CLOSED_LOCAL: {
            name = "HalfClosedLocal";
            break;
        }
        case HALF_CLOSED_REMOTE: {
            name = "HalfClosedRemote";
            break;
        }
        case CLOSED: {
            name = "Closed";
            break;
        }
        case RESERVED_LOCAL: {
            name = "ReservedLocal";
            break;
        }
        case RESERVED_REMOTE: {
            name = "ReservedRemote";
            break;
        }
        default: {
            name = "UNKNOWN";
            break;
        }
        }
        // Write the resolved name out through the format context.
        return std::format_to(ctx.out(), "{}", name);
    }
};

export template <>
struct std::formatter<io::shared_layer::FrameType> {
    /**
     * @brief Same deal as the StreamState formatter above — no format-spec support, empty spec
     * only.
     * @param ctx the format parse context.
     * @return iterator to the start of the (expected-empty) format spec.
     */
    static constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    /**
     * @brief Maps a FrameType onto its wire-format name (e.g. "DATA", "HEADERS") and writes it
     * out — PRIORITY gets flagged "(DEPRECATED)" since it's still parsed for interop but nobody
     * should be sending it fresh. Unknown values fall back to "UNKNOWN" — safe default, that's
     * the W.
     * @tparam FormatContext the format context type, deduced by `std::format`.
     * @param type the FrameType to format.
     * @param ctx the format context to write into.
     * @return output iterator past the written name.
     */
    template <typename FormatContext>
    auto format(io::shared_layer::FrameType type, FormatContext &ctx) const {
        using enum io::shared_layer::FrameType;
        std::string_view name;
        // Map the enum value onto its wire-format name; PRIORITY gets flagged deprecated, unknowns fall back to "UNKNOWN".
        switch (type) {
        case DATA: {
            name = "DATA";
            break;
        }
        case HEADERS: {
            name = "HEADERS";
            break;
        }
        case PRIORITY: {
            name = "PRIORITY (DEPRECATED)";
            break;
        }
        case RST_STREAM: {
            name = "RST_STREAM";
            break;
        }
        case SETTINGS: {
            name = "SETTINGS";
            break;
        }
        case PUSH_PROMISE: {
            name = "PUSH_PROMISE";
            break;
        }
        case PING: {
            name = "PING";
            break;
        }
        case GOAWAY: {
            name = "GOAWAY";
            break;
        }
        case WINDOW_UPDATE: {
            name = "WINDOW_UPDATE";
            break;
        }
        case CONTINUATION: {
            name = "CONTINUATION";
            break;
        }
        default: {
            name = "UNKNOWN";
            break;
        }
        }
        // Write the resolved name out through the format context.
        return std::format_to(ctx.out(), "{}", name);
    }
};

#ifdef CONGELADO_TEST
namespace io::shared_layer::tests {
using namespace boost::ut;

suite<"StreamState"> stream_state_suite = [] {
    "formats every enumerator to its human-readable name"_test = [] {
        using enum io::shared_layer::StreamState;

        expect(std::format("{}", IDLE) == "Idle");
        expect(std::format("{}", OPEN) == "Open");
        expect(std::format("{}", HALF_CLOSED_LOCAL) == "HalfClosedLocal");
        expect(std::format("{}", HALF_CLOSED_REMOTE) == "HalfClosedRemote");
        expect(std::format("{}", CLOSED) == "Closed");
        expect(std::format("{}", RESERVED_LOCAL) == "ReservedLocal");
        expect(std::format("{}", RESERVED_REMOTE) == "ReservedRemote");
    };

    "unknown value falls back to UNKNOWN"_test = [] {
        auto bogus = static_cast<io::shared_layer::StreamState>(255);
        expect(std::format("{}", bogus) == "UNKNOWN");
    };
};

suite<"FrameType"> frame_type_suite = [] {
    "formats every enumerator to its wire-format name"_test = [] {
        using enum io::shared_layer::FrameType;

        expect(std::format("{}", DATA) == "DATA");
        expect(std::format("{}", HEADERS) == "HEADERS");
        expect(std::format("{}", PRIORITY) == "PRIORITY (DEPRECATED)");
        expect(std::format("{}", RST_STREAM) == "RST_STREAM");
        expect(std::format("{}", SETTINGS) == "SETTINGS");
        expect(std::format("{}", PUSH_PROMISE) == "PUSH_PROMISE");
        expect(std::format("{}", PING) == "PING");
        expect(std::format("{}", GOAWAY) == "GOAWAY");
        expect(std::format("{}", WINDOW_UPDATE) == "WINDOW_UPDATE");
        expect(std::format("{}", CONTINUATION) == "CONTINUATION");
    };

    "unknown value falls back to UNKNOWN"_test = [] {
        auto bogus = static_cast<io::shared_layer::FrameType>(255);
        expect(std::format("{}", bogus) == "UNKNOWN");
    };

    "wire values match RFC 9113 §11.2 assigned numbers"_test = [] {
        using enum io::shared_layer::FrameType;

        expect(std::to_underlying(DATA) == 0x0);
        expect(std::to_underlying(HEADERS) == 0x1);
        expect(std::to_underlying(PRIORITY) == 0x2);
        expect(std::to_underlying(RST_STREAM) == 0x3);
        expect(std::to_underlying(SETTINGS) == 0x4);
        expect(std::to_underlying(PUSH_PROMISE) == 0x5);
        expect(std::to_underlying(PING) == 0x6);
        expect(std::to_underlying(GOAWAY) == 0x7);
        expect(std::to_underlying(WINDOW_UPDATE) == 0x8);
        expect(std::to_underlying(CONTINUATION) == 0x9);
    };
};

suite<"Flags"> flags_suite = [] {
    "flag bit values match RFC 9113 wire layout"_test = [] {
        expect(io::shared_layer::Flags::END_STREAM == 0x01);
        expect(io::shared_layer::Flags::ACK == 0x01);
        expect(io::shared_layer::Flags::END_HEADERS == 0x04);
        expect(io::shared_layer::Flags::PADDED == 0x08);
        expect(io::shared_layer::Flags::PRIORITY == 0x20);
    };
};

} // namespace io::shared_layer::tests
#endif
