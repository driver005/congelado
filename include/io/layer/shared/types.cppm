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
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(io::shared_layer::StreamState state, FormatContext &ctx) const {
        using enum io::shared_layer::StreamState;
        std::string_view name;
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
        return std::format_to(ctx.out(), "{}", name);
    }
};

export template <>
struct std::formatter<io::shared_layer::FrameType> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(io::shared_layer::FrameType type, FormatContext &ctx) const {
        using enum io::shared_layer::FrameType;
        std::string_view name;
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
        return std::format_to(ctx.out(), "{}", name);
    }
};
