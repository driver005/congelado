module io.layer.http2.settings;
@nogc nothrow:

// PORT-NOTE: namespace io::layer::http2 → module io.layer.http2.settings.
// C++ threw exceptions from apply(); D port returns an error enum instead and
// callers must check.  std::formatter → settings_state_name() helper.
// ReadSettingsAdaptor/WriteSettingsAdaptor use C++ range pipelines; ported as
// plain functions operating on ubyte slices.

import io.layer.shared.types;
import io.layer.shared.ping;
import io.layer.http2.consts;
import io.layer.http2.frame;

// PORT-NOTE: C++ enum class SettingsState.
enum SettingsState : ubyte { UNACKNOWLEDGED = 0, ACKNOWLEDGED = 1, IMPLEMENTED = 2 }

/// Human-readable name for SettingsState (replaces std::formatter).
const(char)[] settings_state_name(SettingsState state) {
    final switch (state) {
    case SettingsState.UNACKNOWLEDGED: return "UNACKNOWLEDGED";
    case SettingsState.ACKNOWLEDGED:   return "ACKNOWLEDGED";
    case SettingsState.IMPLEMENTED:    return "IMPLEMENTED";
    }
}

/// io::layer::http2::Settings
class Settings {
  public:
    this() {
        m_header_table_size    = DEFAULT_HEADER_TABLE_SIZE;
        m_enable_push          = true;
        m_max_concurrent_streams = 100;
        m_initial_window_size  = DEFAULT_INITIAL_WINDOW_SIZE;
        m_max_frame_size       = MIN_FRAME_SIZE;
        m_max_header_list_size = uint.max;
        m_state                = SettingsState.UNACKNOWLEDGED;
        m_last_stream_id       = MAX_CONNECTED_STREAMS;
        m_delta_window_on_settings = 0;
        m_ping_tracker         = new PingTracker();
        // core_debug("http2/settings", "init state=%s", settings_state_name(m_state));
    }

    // PORT-NOTE: C++ threw ConnectionError on invalid values; D port returns false.
    bool apply(ushort id, uint value) {
        switch (id) {
        case 0x1: {
            m_header_table_size = value;
            return true;
        }
        case 0x2: {
            if (value > 1)
                return false; // PROTOCOL_ERROR: ENABLE_PUSH must be 0 or 1
            m_enable_push = (value == 1);
            return true;
        }
        case 0x3: {
            m_max_concurrent_streams = value;
            return true;
        }
        case 0x4: {
            if (value > MAX_INITIAL_WINDOW_SIZE)
                return false; // FLOW_CONTROL_ERROR
            m_initial_window_size = value;
            return true;
        }
        case 0x5: {
            if (value < MIN_FRAME_SIZE || value > MAX_FRAME_SIZE)
                return false; // PROTOCOL_ERROR
            m_max_frame_size = value;
            return true;
        }
        case 0x6: {
            m_max_header_list_size = value;
            return true;
        }
        default:
            return true; // unknown settings are ignored per RFC 9113
        }
    }

    /// Build a SETTINGS ACK FrameBuilder.
    static FrameBuilder generate_ack() {
        auto fb = new FrameBuilder();
        fb.add_type(FrameType.SETTINGS);
        fb.add_flags(Flags.ACK);
        fb.add_stream_id(0);
        return fb;
    }

    void set_last_stream_id(uint stream_id) {
        m_last_stream_id = stream_id;
    }

    void set_delta_window_on_settings(int delta) {
        m_delta_window_on_settings = delta;
    }

    void set_state(SettingsState state) {
        m_state = state;
    }

    uint next_stream_id() {
        m_last_stream_id += 2;
        return m_last_stream_id;
    }

    bool is_finished()     const { return m_state == SettingsState.IMPLEMENTED; }
    bool is_acknowledged() const { return m_state == SettingsState.ACKNOWLEDGED; }

    uint get_header_table_size()     const { return m_header_table_size; }
    bool get_enable_push()           const { return m_enable_push; }
    uint get_max_concurrent_streams() const { return m_max_concurrent_streams; }
    uint get_initial_window_size()   const { return m_initial_window_size; }
    uint get_max_frame_size()        const { return m_max_frame_size; }
    uint get_max_header_list_size()  const { return m_max_header_list_size; }
    uint get_last_stream_id()        const { return m_last_stream_id; }
    int  get_delta_window_on_settings() const { return m_delta_window_on_settings; }
    PingTracker get_ping_tracker()         { return m_ping_tracker; }

  private:
    // SETTINGS_HEADER_TABLE_SIZE (0x1)
    // Maximum size of the HPACK dynamic table the sender is willing to use.
    // Default: 4096.  No upper bound specified by the RFC.
    uint m_header_table_size;

    // SETTINGS_ENABLE_PUSH (0x2)
    // Whether the remote peer may send PUSH_PROMISE frames.
    // Default: true (1).  Valid values: 0 or 1 only.
    bool m_enable_push;

    // SETTINGS_MAX_CONCURRENT_STREAMS (0x3)
    // Maximum number of streams the sender allows the remote to open simultaneously.
    // Default: no limit (represented as max uint32).  RFC says treat as "initially
    // infinite" but we advertise 100 as a practical server-side limit.
    uint m_max_concurrent_streams;

    // SETTINGS_INITIAL_WINDOW_SIZE (0x4)
    // Initial flow-control window size for new streams.
    // Default: 65535.  Max: 2^31-1.  Sending > max is a FLOW_CONTROL_ERROR.
    uint m_initial_window_size;

    // SETTINGS_MAX_FRAME_SIZE (0x5)
    // Maximum frame payload size the sender is willing to receive.
    // Default: 16384 (2^14).  Valid range: [16384, 2^24-1].
    uint m_max_frame_size;

    // SETTINGS_MAX_HEADER_LIST_SIZE (0x6)
    // Advisory limit on the total size of header fields the sender will accept.
    // Default: unlimited (represented as max uint32).
    uint m_max_header_list_size;

    SettingsState m_state;
    uint          m_last_stream_id;
    PingTracker   m_ping_tracker;
    int           m_delta_window_on_settings;
}

// ─── ReadSettingsAdaptor ──────────────────────────────────────────────────────
// PORT-NOTE: C++ was a range_adaptor_closure consuming a byte range chunked by 6.
// D port is a plain function: read_settings(data) → Settings.
// Reads 6-byte TLV records: 2-byte big-endian ID + 4-byte big-endian value.

Settings read_settings(const(ubyte)[] data) {
    import util.alloc : make;
    // PORT-NOTE: heap-allocate because Settings is a class.
    auto settings = new Settings();
    size_t offset = 0;
    while (offset + 6 <= data.length) {
        ushort id = cast(ushort)((data[offset] << 8) | data[offset + 1]);
        uint   val = (cast(uint) data[offset + 2] << 24)
                   | (cast(uint) data[offset + 3] << 16)
                   | (cast(uint) data[offset + 4] <<  8)
                   | (cast(uint) data[offset + 5]);
        settings.apply(id, val);
        offset += 6;
    }
    return settings;
}

// ─── WriteSettingsAdaptor ─────────────────────────────────────────────────────
// PORT-NOTE: C++ was a range_adaptor_closure emitting big-endian 6-byte TLV entries
// for non-default fields.  D port writes directly into a caller-owned slice.
// Returns number of bytes written.

size_t write_settings(Settings settings, ubyte[] out_) {
    size_t pos = 0;

    void emit(ushort setting_id, uint value) {
        if (pos + 6 > out_.length) return; // overflow guard
        out_[pos    ] = cast(ubyte)(setting_id >> 8);
        out_[pos + 1] = cast(ubyte)(setting_id & 0xFF);
        out_[pos + 2] = cast(ubyte)(value >> 24);
        out_[pos + 3] = cast(ubyte)(value >> 16);
        out_[pos + 4] = cast(ubyte)(value >>  8);
        out_[pos + 5] = cast(ubyte)(value & 0xFF);
        pos += 6;
    }

    if (settings.get_header_table_size() != DEFAULT_HEADER_TABLE_SIZE)
        emit(0x1, settings.get_header_table_size());
    if (!settings.get_enable_push())
        emit(0x2, 0);
    if (settings.get_max_concurrent_streams() != uint.max)
        emit(0x3, settings.get_max_concurrent_streams());
    if (settings.get_initial_window_size() != DEFAULT_INITIAL_WINDOW_SIZE)
        emit(0x4, settings.get_initial_window_size());
    if (settings.get_max_frame_size() >= MIN_FRAME_SIZE)
        emit(0x5, settings.get_max_frame_size());
    if (settings.get_max_header_list_size() != uint.max)
        emit(0x6, settings.get_max_header_list_size());

    return pos;
}
