module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing me
#include <ranges>

export module io_layer_http2:settings;

import std;
import io_error;
import io_layer_shared;
import shared;
import core_logger;
import utils_codec;
import :consts;
import :frame;

export namespace io::layer::http2 {

enum class SettingsState : std::uint8_t { UNACKNOWLEDGED = 0, ACKNOWLEDGED = 1, IMPLEMENTED = 2 };

}

template <>
struct std::formatter<io::layer::http2::SettingsState> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(io::layer::http2::SettingsState state, FormatContext &ctx) const {
        std::string_view name = "UNKNOWN";
        switch (state) {
            using enum io::layer::http2::SettingsState;
        case UNACKNOWLEDGED:
            name = "UNACKNOWLEDGED";
            break;
        case ACKNOWLEDGED:
            name = "ACKNOWLEDGED";
            break;
        case IMPLEMENTED:
            name = "IMPLEMENTED";
            break;
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};

export namespace io::layer::http2 {

class Settings {
  public:
    explicit Settings()
        : m_header_table_size{DEFAULT_HEADER_TABLE_SIZE}, m_enable_push{true}, m_max_concurrent_streams{100},
          m_initial_window_size{DEFAULT_INITIAL_WINDOW_SIZE}, m_max_frame_size{MIN_FRAME_SIZE},
          m_max_header_list_size{std::numeric_limits<std::uint32_t>::max()}, m_state{SettingsState::UNACKNOWLEDGED},
          m_last_stream_id{MAX_CONNECTED_STREAMS}, m_delta_window_on_settings{0} {
        core::logger::debug("http2/settings", "init state={}", m_state);
    }

    void apply(std::uint16_t id, std::uint32_t value) {
        switch (id) {
        case 0x1: {
            core::logger::debug("http2/settings", "HEADER_TABLE_SIZE={}", value);
            m_header_table_size = value;
            return;
        }
        case 0x2: {
            if (value > 1) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "SETTINGS_ENABLE_PUSH must be 0 or 1"};
            }
            core::logger::debug("http2/settings", "ENABLE_PUSH={}", value);
            m_enable_push = (value == 1);
            return;
        }
        case 0x3: {
            core::logger::debug("http2/settings", "MAX_CONCURRENT_STREAMS={}", value);
            m_max_concurrent_streams = value;
            return;
        }
        case 0x4: {
            if (value > MAX_INITIAL_WINDOW_SIZE) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                                   "SETTINGS_INITIAL_WINDOW_SIZE exceeds 2^31-1"};
            }
            core::logger::debug("http2/settings", "INITIAL_WINDOW_SIZE={}", value);
            m_initial_window_size = value;
            return;
        }
        case 0x5: {
            if (value < MIN_FRAME_SIZE || value > MAX_FRAME_SIZE) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "SETTINGS_MAX_FRAME_SIZE must be in [16384, 2^24-1]"};
            }
            core::logger::debug("http2/settings", "MAX_FRAME_SIZE={}", value);
            m_max_frame_size = value;
            return;
        }
        case 0x6: {
            core::logger::debug("http2/settings", "MAX_HEADER_LIST_SIZE={}", value);
            m_max_header_list_size = value;
            return;
        }
        default:
            return;
        }
    }

    static FrameBuilder<shared_layer::FrameRole::SENDER> generate_ack() {
        core::logger::debug("http2/settings", "ACK frame");
        return FrameBuilder<shared_layer::FrameRole::SENDER>{}
            .add_type(shared_layer::FrameType::SETTINGS)
            .add_flags(shared_layer::Flags::ACK)
            .add_stream_id(0)
            .build();
    }

    void set_last_stream_id(const std::uint32_t &stream_id) noexcept {
        core::logger::debug("http2/settings", "last_stream_id={}", stream_id);

        m_last_stream_id = stream_id;
    }


    void set_delta_window_on_settings(const std::int32_t &delta) noexcept {
        core::logger::debug("http2/settings", "delta_window={}", delta);

        m_delta_window_on_settings = delta;
    }

    void set_state(const SettingsState &state) noexcept {
        core::logger::debug("http2/settings", "state={}", state);

        m_state = state;
    }

    std::uint32_t next_stream_id() noexcept {
        core::logger::debug("http2/settings", "next stream id={}", m_last_stream_id + 2);

        return m_last_stream_id += 2;
    }

    [[nodiscard]] bool is_finished() const noexcept { return m_state == SettingsState::IMPLEMENTED; }
    [[nodiscard]] bool is_acknowledged() const noexcept { return m_state == SettingsState::ACKNOWLEDGED; }

    [[nodiscard]] const std::uint32_t &get_header_table_size()    const noexcept { return m_header_table_size; }
    [[nodiscard]] const bool          &get_enable_push()           const noexcept { return m_enable_push; }
    [[nodiscard]] const std::uint32_t &get_max_concurrent_streams() const noexcept { return m_max_concurrent_streams; }
    [[nodiscard]] const std::uint32_t &get_initial_window_size()  const noexcept { return m_initial_window_size; }
    [[nodiscard]] const std::uint32_t &get_max_frame_size()       const noexcept { return m_max_frame_size; }
    [[nodiscard]] const std::uint32_t &get_max_header_list_size() const noexcept { return m_max_header_list_size; }
    [[nodiscard]] const std::uint32_t &get_last_stream_id()       const noexcept { return m_last_stream_id; }
    shared_layer::ping::PingTracker   &get_ping_tracker()          noexcept { return m_ping_tracker; }
    [[nodiscard]] const std::int32_t  &get_delta_window_on_settings() const noexcept { return m_delta_window_on_settings; }

  private:
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


    SettingsState m_state;

    std::uint32_t m_last_stream_id;

    shared_layer::ping::PingTracker m_ping_tracker;

    std::int32_t m_delta_window_on_settings;
};


struct ReadSettingsAdaptor : std::ranges::range_adaptor_closure<ReadSettingsAdaptor> {
    explicit constexpr ReadSettingsAdaptor() = default;

    template <std::ranges::viewable_range R>
    Settings operator()(R &&data) const {
        return std::ranges::fold_left(
            std::forward<R>(data) | std::views::chunk(6) | std::views::filter([](auto &&chunk) {
                return std::ranges::distance(chunk) == 6;
            }) | std::views::transform([](auto &&chunk) {
                return std::pair{chunk | std::views::take(2) | utils::codec::ReadBigEndianAdaptor<std::uint16_t>{},
                                 chunk | std::views::drop(2) | std::views::take(4) |
                                     utils::codec::ReadBigEndianAdaptor<>{}};
            }),
            Settings{}, [](Settings acc, auto &&pair) {
                acc.apply(pair.first, pair.second);
                return acc;
            });
    }
};


struct WriteSettingsAdaptor : std::ranges::range_adaptor_closure<WriteSettingsAdaptor> {
    explicit constexpr WriteSettingsAdaptor(Settings &settings) : m_settings{settings} {}

    template <std::ranges::viewable_range R>
    auto operator()(R &&range) const {
        std::vector<std::byte> settings_bytes;

        auto emit = [&](const std::uint16_t SETTING_ID, const std::uint32_t VALUE) {
            auto entry = std::views::empty<std::byte> | utils::codec::WriteBigEndianAdaptor<std::uint16_t>{SETTING_ID} |
                         utils::codec::WriteBigEndianAdaptor<std::uint32_t>{VALUE} |
                         std::ranges::to<std::vector<std::byte>>();

            settings_bytes.insert(settings_bytes.end(), entry.begin(), entry.end());
        };

        if (m_settings.get().get_header_table_size() != DEFAULT_HEADER_TABLE_SIZE) {
            emit(0x1, m_settings.get().get_header_table_size());
        }

        if (!m_settings.get().get_enable_push()) {
            emit(0x2, 0);
        }

        if (m_settings.get().get_max_concurrent_streams() != std::numeric_limits<std::uint32_t>::max()) {
            emit(0x3, m_settings.get().get_max_concurrent_streams());
        }

        if (m_settings.get().get_initial_window_size() != DEFAULT_INITIAL_WINDOW_SIZE) {
            emit(0x4, m_settings.get().get_initial_window_size());
        }

        if (m_settings.get().get_max_frame_size() >= MIN_FRAME_SIZE) {
            emit(0x5, m_settings.get().get_max_frame_size());
        }

        if (m_settings.get().get_max_header_list_size() != std::numeric_limits<std::uint32_t>::max()) {
            emit(0x6, m_settings.get().get_max_header_list_size());
        }

        return std::views::concat(std::forward<R>(range), std::move(settings_bytes));
    }

  private:
    std::reference_wrapper<Settings> m_settings;
};

} // namespace io::layer::http2
