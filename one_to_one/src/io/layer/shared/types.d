module io.layer.shared.types;
@nogc nothrow:

// PORT-NOTE: namespace io::shared_layer → module io.layer.shared.types.

/// io::shared_layer::StreamState
enum StreamState : ubyte {
    IDLE,
    OPEN,
    HALF_CLOSED_LOCAL,
    HALF_CLOSED_REMOTE,
    CLOSED,
    RESERVED_LOCAL,
    RESERVED_REMOTE,
}

/// io::shared_layer::FrameRole
enum FrameRole : bool { SENDER = false, RECEIVER = true }

/// io::shared_layer::Flags
// PORT-NOTE: C++ was a struct with static constexpr members.
// D: module-level enum constants (same semantics).
enum ubyte FLAG_END_STREAM  = 0x01;
enum ubyte FLAG_ACK         = 0x01;
enum ubyte FLAG_END_HEADERS = 0x04;
enum ubyte FLAG_PADDED      = 0x08;
enum ubyte FLAG_PRIORITY    = 0x20;

// Aliases matching C++ Flags:: member names for call-site compatibility.
struct Flags {
    static immutable ubyte END_STREAM  = FLAG_END_STREAM;
    static immutable ubyte ACK         = FLAG_ACK;
    static immutable ubyte END_HEADERS = FLAG_END_HEADERS;
    static immutable ubyte PADDED      = FLAG_PADDED;
    static immutable ubyte PRIORITY    = FLAG_PRIORITY;
}

/// io::shared_layer::FrameType
enum FrameType : ubyte {
    DATA         = 0x0,
    HEADERS      = 0x1,
    PRIORITY     = 0x2, // deprecated §5.3.2, still parsed for interop
    RST_STREAM   = 0x3,
    SETTINGS     = 0x4,
    PUSH_PROMISE = 0x5,
    PING         = 0x6,
    GOAWAY       = 0x7,
    WINDOW_UPDATE= 0x8,
    CONTINUATION = 0x9,
}

/// StreamState → human-readable name (replaces std::formatter).
const(char)[] stream_state_name(StreamState state) {
    final switch (state) {
    case StreamState.IDLE:               return "Idle";
    case StreamState.OPEN:               return "Open";
    case StreamState.HALF_CLOSED_LOCAL:  return "HalfClosedLocal";
    case StreamState.HALF_CLOSED_REMOTE: return "HalfClosedRemote";
    case StreamState.CLOSED:             return "Closed";
    case StreamState.RESERVED_LOCAL:     return "ReservedLocal";
    case StreamState.RESERVED_REMOTE:    return "ReservedRemote";
    }
}

/// FrameType → human-readable name (replaces std::formatter).
const(char)[] frame_type_name(FrameType type) {
    final switch (type) {
    case FrameType.DATA:          return "DATA";
    case FrameType.HEADERS:       return "HEADERS";
    case FrameType.PRIORITY:      return "PRIORITY (DEPRECATED)";
    case FrameType.RST_STREAM:    return "RST_STREAM";
    case FrameType.SETTINGS:      return "SETTINGS";
    case FrameType.PUSH_PROMISE:  return "PUSH_PROMISE";
    case FrameType.PING:          return "PING";
    case FrameType.GOAWAY:        return "GOAWAY";
    case FrameType.WINDOW_UPDATE: return "WINDOW_UPDATE";
    case FrameType.CONTINUATION:  return "CONTINUATION";
    }
}
