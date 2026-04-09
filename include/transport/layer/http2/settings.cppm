module;
#include <cstdint>
export module layer_http2:settings;

import std;
import transport_error;
import transport_layer_shared;
import :consts;
import :frame;

export namespace transport::layer::http2 {

class Settings {
  public:
    explicit Settings()
        : m_header_table_size{DEFAULT_HEADER_TABLE_SIZE}, m_enable_push{true}, m_max_concurrent_streams{100},
          m_initial_window_size{DEFAULT_INITIAL_WINDOW_SIZE}, m_max_frame_size{MIN_FRAME_SIZE},
          m_max_header_list_size{std::numeric_limits<std::uint32_t>::max()},
          m_trigger_goaway_after_stream_id{MAX_CONNECTED_STREAMS}, m_acknowledged_settings{false},
          m_ping_tracker{shared_layer::ping::PingTracker{}}, m_delta_window_on_settings{0} {}

    void apply(std::uint16_t id, std::uint32_t value) {
        switch (id) {
        case 0x1:
            m_header_table_size = value;
            return;
        case 0x2:
            if (value > 1)
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "SETTINGS_ENABLE_PUSH must be 0 or 1"};
            m_enable_push = (value == 1);
            return;
        case 0x3:
            m_max_concurrent_streams = value;
            return;
        case 0x4:
            if (value > MAX_INITIAL_WINDOW_SIZE)
                throw error::http::ConnectionError{error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                                   "SETTINGS_INITIAL_WINDOW_SIZE exceeds 2^31-1"};
            m_initial_window_size = value;
            return;
        case 0x5:
            if (value < MIN_FRAME_SIZE || value > MAX_FRAME_SIZE)
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "SETTINGS_MAX_FRAME_SIZE must be in [16384, 2^24-1]"};
            m_max_frame_size = value;
            return;
        case 0x6:
            m_max_header_list_size = value;
            return;
        default:
            return;
        }
    }

    template <std::output_iterator<std::uint8_t> Out>
    Out encode(Out out) const {
        auto emit = [&](const std::uint16_t &id, const std::uint32_t &val) {
            shared_layer::Atom<std::uint16_t>::write_big_endian(out, id, 2);
            shared_layer::Atom<std::uint32_t>::write_big_endian(out, val, 4);
        };

        if (m_header_table_size != DEFAULT_HEADER_TABLE_SIZE)
            emit(0x1, m_header_table_size);

        if (!m_enable_push)
            emit(0x2, 0);

        if (m_max_concurrent_streams != std::numeric_limits<std::uint32_t>::max())
            emit(0x3, m_max_concurrent_streams);

        if (m_initial_window_size != DEFAULT_INITIAL_WINDOW_SIZE)
            emit(0x4, m_initial_window_size);

        if (m_max_frame_size != MIN_FRAME_SIZE)
            emit(0x5, m_max_frame_size);

        if (m_max_header_list_size != std::numeric_limits<std::uint32_t>::max())
            emit(0x6, m_max_header_list_size);

        return out;
    }


    Frame<shared_layer::FrameRole::Sender> decode(const Frame<shared_layer::FrameRole::Receiver> &frame) {
        auto payload = std::span{frame.get_payload()};

        for (std::size_t i = 0; i < payload.size(); i += 6) {
            const std::uint16_t id = shared_layer::Atom<std::uint16_t>::read_big_endian(payload.subspan(i, 2));
            const std::uint32_t value = shared_layer::Atom<std::uint32_t>::read_big_endian(payload.subspan(i + 2, 4));

            apply(id, value);
        }

        return generate_ack();
    }

    static Frame<shared_layer::FrameRole::Sender> generate_ack() {
        return Frame<shared_layer::FrameRole::Sender>{FrameHeader{0, shared_layer::FrameType::SETTINGS, 0x01, 0}, {}};
    }

    void set_trigger_goaway_after_stream_id(const std::uint32_t &stream_id) noexcept {
        m_trigger_goaway_after_stream_id = stream_id;
    }

    void mark_acknowledged() noexcept { m_acknowledged_settings = true; }

    bool is_setting_acknowledged() const noexcept { return m_acknowledged_settings; }

    void set_delta_window_on_settings(const std::int32_t &delta) noexcept { m_delta_window_on_settings = delta; }

    const std::uint32_t &header_table_size() const noexcept { return m_header_table_size; }
    const bool &enable_push() const noexcept { return m_enable_push; }
    const std::uint32_t &max_concurrent_streams() const noexcept { return m_max_concurrent_streams; }
    const std::uint32_t &initial_window_size() const noexcept { return m_initial_window_size; }
    const std::uint32_t &max_frame_size() const noexcept { return m_max_frame_size; }
    const std::uint32_t &max_header_list_size() const noexcept { return m_max_header_list_size; }
    const std::uint32_t &trigger_goaway_after_stream_id() const noexcept { return m_trigger_goaway_after_stream_id; }
    shared_layer::ping::PingTracker &ping_tracker() noexcept { return m_ping_tracker; }
    const std::int32_t &delta_window_on_settings() const noexcept { return m_delta_window_on_settings; }

  private:
    static constexpr std::uint32_t DEFAULT_HEADER_TABLE_SIZE = 4096;
    static constexpr std::uint32_t MIN_FRAME_SIZE = 1u << 14;       // 16384 (2^14)
    static constexpr std::uint32_t MAX_FRAME_SIZE = (1u << 24) - 1; // 16777215 (2^24 - 1)

    // SETTINGS_HEADER_TABLE_SIZE (0x1)
    // Maximum size of the HPACK dynamic table the sender is willing to use.
    // Default: 4096.  No upper bound specified by the RFC.
    std::uint32_t m_header_table_size;

    // SETTINGS_ENABLE_PUSH (0x2)
    // Whether the remote peer may send PUSH_PROMISE frames.
    // Default: true (1).  Valid values: 0 or 1 only.
    bool m_enable_push;

    // SETTINGS_MAX_CONCURRENT_STREAMS (0x3)
    // Maximum number of streams the sender allows the remote to open simultaneously.
    // Default: no limit (represented as max uint32).  RFC says treat as "initially
    // infinite" but we advertise 100 as a practical server-side limit.
    std::uint32_t m_max_concurrent_streams;

    // SETTINGS_INITIAL_WINDOW_SIZE (0x4)
    // Initial flow-control window size for new streams.
    // Default: 65535.  Max: 2^31-1.  Sending > max is a FLOW_CONTROL_ERROR.
    std::uint32_t m_initial_window_size;

    // SETTINGS_MAX_FRAME_SIZE (0x5)
    // Maximum frame payload size the sender is willing to receive.
    // Default: 16384 (2^14).  Valid range: [16384, 2^24-1].
    std::uint32_t m_max_frame_size;

    // SETTINGS_MAX_HEADER_LIST_SIZE (0x6)
    // Advisory limit on the total size of header fields the sender will accept.
    // Default: unlimited (represented as max uint32).
    std::uint32_t m_max_header_list_size;


    std::uint32_t m_trigger_goaway_after_stream_id;
    bool m_acknowledged_settings;
    shared_layer::ping::PingTracker m_ping_tracker;
    std::int32_t m_delta_window_on_settings;
};

} // namespace transport::layer::http2
