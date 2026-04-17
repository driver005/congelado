export module io_layer_shared:types;

import std;

export namespace io::shared_layer {

enum class StreamState : std::uint8_t {
    Idle,
    Open,
    HalfClosedLocal,
    HalfClosedRemote,
    Closed,
    ReservedLocal,
    ReservedRemote,
};

enum class FrameRole { Sender, Receiver };

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
