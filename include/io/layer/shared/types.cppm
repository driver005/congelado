export module io_layer_shared:types;

import std;

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
