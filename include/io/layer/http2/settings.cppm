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
    /**
     * @brief No-op format-spec parser — this formatter doesn't support any `{:...}` spec
     * options, just accepts whatever's there and moves on.
     * @param ctx the parse context.
     * @return an iterator right at the start of `ctx`, unmoved — no spec characters consumed.
     */
    static constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    /**
     * @brief Formats a `SettingsState` as its enumerator name (`"UNACKNOWLEDGED"` etc), falling
     * back to `"UNKNOWN"` for anything the switch doesn't recognize — lowkey future-proofing
     * against the enum growing without this formatter keeping up.
     * @tparam FormatContext the format context type, deduced per `std::format` call site.
     * @param state the state to format.
     * @param ctx the format context to write into.
     * @return an output iterator past the written text.
     */
    template <typename FormatContext>
    auto format(io::layer::http2::SettingsState state, FormatContext &ctx) const {
        std::string_view name = "UNKNOWN";
        // Map each known enumerator to its name; anything unrecognized keeps the "UNKNOWN" default.
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
    /**
     * @brief Builds a settings set at RFC 9113 spec defaults — 4096-byte header table, push
     * enabled, 100 max concurrent streams (RFC says "initially infinite" but 100's the
     * practical cap advertised here, bet), 65535 initial window, minimum 16384 frame size,
     * unlimited header list size.
     */
    explicit Settings()
        : m_max_header_list_size{std::numeric_limits<std::uint32_t>::max()} {
        core::logger::debug("http2/settings", "init state={}", m_state);
    }

    /**
     * @brief Applies one decoded SETTINGS id/value pair to the matching member, with
     * per-setting validation where the spec demands it (ENABLE_PUSH must be 0/1,
     * INITIAL_WINDOW_SIZE capped at 2^31-1, MAX_FRAME_SIZE bounded to [16384, 2^24-1]).
     * @note Unknown setting ids (anything not 0x1–0x6) are RFC 9113 §6.5.2 spec-legal — they
     * MUST be ignored for negotiation purposes, not rejected — but are recorded into
     * `m_vendor_settings` so an `IHttpExtension` can still react to a vendor id (e.g.
     * RFC 8441's `SETTINGS_ENABLE_CONNECT_PROTOCOL = 0x8`) instead of it vanishing outright.
     * @param setting_id the setting id (0x1 through 0x6 are recognized).
     * @param value the setting value to apply.
     * @throws error::http::ConnectionError if ENABLE_PUSH isn't 0 or 1, INITIAL_WINDOW_SIZE
     * exceeds `MAX_INITIAL_WINDOW_SIZE`, or MAX_FRAME_SIZE is outside `[MIN_FRAME_SIZE,
     * MAX_FRAME_SIZE]`.
     */
    void apply(std::uint16_t setting_id, std::uint32_t value) {
        // Only ids 0x1-0x6 are recognized; anything else falls through to the default arm
        // per RFC 9113 §6.5.2 — unknown settings get ignored for negotiation, not rejected,
        // but are still recorded for extension hooks to observe.
        switch (setting_id) {
        case 0x1: {
            core::logger::debug("http2/settings", "HEADER_TABLE_SIZE={}", value);
            m_header_table_size = value;
            return;
        }
        case 0x2: {
            // Spec-mandated: ENABLE_PUSH is strictly boolean on the wire.
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
            // Capped at 2^31-1 — anything past that overflows the signed window math elsewhere.
            if (value > MAX_INITIAL_WINDOW_SIZE) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                                   "SETTINGS_INITIAL_WINDOW_SIZE exceeds 2^31-1"};
            }
            core::logger::debug("http2/settings", "INITIAL_WINDOW_SIZE={}", value);
            m_initial_window_size = value;
            return;
        }
        case 0x5: {
            // Must stay inside the spec-defined [16384, 2^24-1] frame-size range.
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
            core::logger::debug("http2/settings", "unrecognized setting id={} value={}", setting_id,
                                value);
            m_vendor_settings.emplace_back(setting_id, value);
            return;
        }
    }

    /**
     * @brief Copies the six negotiable SETTINGS fields (header table size, enable push, max
     * concurrent streams, initial window size, max frame size, max header list size) from
     * `other` onto `*this`. Deliberately leaves `m_state`, `m_last_stream_id`, `m_ping_tracker`,
     * and `m_delta_window_on_settings` untouched — those track connection lifecycle, not
     * wire-negotiated values, and must survive across repeated SETTINGS exchanges.
     * @param other the decoded settings to copy the six negotiable fields from.
     */
    void apply_all(const Settings &other) {
        core::logger::debug("http2/settings", "apply_all from decoded settings");

        m_header_table_size = other.m_header_table_size;
        m_enable_push = other.m_enable_push;
        m_max_concurrent_streams = other.m_max_concurrent_streams;
        m_initial_window_size = other.m_initial_window_size;
        m_max_frame_size = other.m_max_frame_size;
        m_max_header_list_size = other.m_max_header_list_size;
        // Carry the decoded vendor ids across too, so an `on_remote_settings` observer can read
        // them back off this instance via get_vendor_settings().
        m_vendor_settings = other.m_vendor_settings;
    }

    /**
     * @brief Registers one vendor/extension setting id/value pair on this settings instance.
     * On the outgoing (local) instance an extension calls this from `on_local_settings()` to
     * advertise a vendor id (e.g. RFC 8441's `SETTINGS_ENABLE_CONNECT_PROTOCOL = 0x8`);
     * `WriteSettingsAdaptor` emits it after the six spec fields.
     * @param setting_id the vendor/extension setting id.
     * @param value the value for it.
     */
    void add_local_setting_override(std::uint16_t setting_id, std::uint32_t value) {
        core::logger::debug("http2/settings", "vendor setting id={} value={}", setting_id, value);

        m_vendor_settings.emplace_back(setting_id, value);
    }

    /**
     * @brief Grabs every vendor/extension setting id/value pair attached to this instance —
     * one list serving both directions since no single `Settings` instance is ever both: on a
     * decoded remote instance it holds the ids `apply()` didn't recognize (ids outside 0x1-0x6),
     * on the local instance it holds `add_local_setting_override()` entries. `on_remote_settings`
     * reads it on the remote side; `WriteSettingsAdaptor` reads it on the local side.
     * @return the vendor id/value pairs on this instance.
     */
    [[nodiscard]] std::span<const std::pair<std::uint16_t, std::uint32_t>>
    get_vendor_settings() const noexcept {
        return m_vendor_settings;
    }

    /**
     * @brief Static factory — builds a bare SETTINGS frame with the ACK flag set and an empty
     * payload, the standard reply motion for an incoming non-ACK SETTINGS frame.
     * @return a ready-to-send SETTINGS ACK frame.
     */
    static FrameBuilder<shared_layer::FrameRole::SENDER> generate_ack() {
        core::logger::debug("http2/settings", "ACK frame");
        return FrameBuilder<shared_layer::FrameRole::SENDER>{}
            .add_type(shared_layer::FrameType::SETTINGS)
            .add_flags(shared_layer::Flags::ACK)
            .add_stream_id(0)
            .build();
    }

    /**
     * @brief Sets the last stream id — used to record the GOAWAY threshold the remote peer has
     * advertised.
     * @param stream_id the stream id to store.
     */
    void set_last_stream_id(const std::uint32_t &stream_id) noexcept {
        core::logger::debug("http2/settings", "last_stream_id={}", stream_id);

        m_last_stream_id = stream_id;
    }


    /**
     * @brief Records the pending send-window delta produced by an INITIAL_WINDOW_SIZE change,
     * to be applied to every open stream once the SETTINGS frame carrying it gets acknowledged
     * (see `Session::receive()`).
     * @param delta the signed window delta to store — can be negative if the window shrank.
     */
    void set_delta_window_on_settings(const std::int32_t &delta) noexcept {
        core::logger::debug("http2/settings", "delta_window={}", delta);

        m_delta_window_on_settings = delta;
    }

    /**
     * @brief Sets the settings-exchange state.
     * @param state the state to store.
     */
    void set_state(const SettingsState &state) noexcept {
        core::logger::debug("http2/settings", "state={}", state);

        m_state = state;
    }

    /**
     * @brief Advances and returns the next stream id tracked by this settings instance,
     * bumping by 2 to stay on the same parity.
     * @note Distinct from `Session::next_client_stream()`, don't get it twisted — this is a
     * lower-level id-bumping primitive on `Settings` itself, not the session's actual
     * stream-creation path for outgoing client requests.
     * @return the newly bumped stream id.
     */
    std::uint32_t next_stream_id() noexcept {
        core::logger::debug("http2/settings", "next stream id={}", m_last_stream_id + 2);

        return m_last_stream_id += 2;
    }

    /**
     * @brief Checks whether the settings exchange has fully completed (peer's SETTINGS was
     * ACKed and window deltas applied).
     * @return true if state is IMPLEMENTED.
     */
    [[nodiscard]] bool is_finished() const noexcept { return m_state == SettingsState::IMPLEMENTED; }
    /**
     * @brief Checks whether the peer's SETTINGS has been ACKed but not yet fully implemented
     * (window deltas not yet propagated to streams).
     * @return true if state is ACKNOWLEDGED.
     */
    [[nodiscard]] bool is_acknowledged() const noexcept { return m_state == SettingsState::ACKNOWLEDGED; }

    /**
     * @brief Grabs SETTINGS_HEADER_TABLE_SIZE.
     * @return the HPACK dynamic table size this side is willing to use.
     */
    [[nodiscard]] const std::uint32_t &get_header_table_size()    const noexcept { return m_header_table_size; }
    /**
     * @brief Grabs SETTINGS_ENABLE_PUSH.
     * @return true if server push is enabled.
     */
    [[nodiscard]] const bool          &get_enable_push()           const noexcept { return m_enable_push; }
    /**
     * @brief Grabs SETTINGS_MAX_CONCURRENT_STREAMS.
     * @return the max number of streams the remote may open simultaneously.
     */
    [[nodiscard]] const std::uint32_t &get_max_concurrent_streams() const noexcept { return m_max_concurrent_streams; }
    /**
     * @brief Grabs SETTINGS_INITIAL_WINDOW_SIZE.
     * @return the initial flow-control window for new streams.
     */
    [[nodiscard]] const std::uint32_t &get_initial_window_size()  const noexcept { return m_initial_window_size; }
    /**
     * @brief Grabs SETTINGS_MAX_FRAME_SIZE.
     * @return the max frame payload size this side accepts.
     */
    [[nodiscard]] const std::uint32_t &get_max_frame_size()       const noexcept { return m_max_frame_size; }
    /**
     * @brief Grabs SETTINGS_MAX_HEADER_LIST_SIZE.
     * @return the advisory max total header field size this side accepts.
     */
    [[nodiscard]] const std::uint32_t &get_max_header_list_size() const noexcept { return m_max_header_list_size; }
    /**
     * @brief Grabs the recorded last stream id (GOAWAY threshold).
     * @return the last stream id.
     */
    [[nodiscard]] const std::uint32_t &get_last_stream_id()       const noexcept { return m_last_stream_id; }
    /**
     * @brief Grabs mutable access to the PING round-trip tracker.
     * @return the ping tracker.
     */
    shared_layer::ping::PingTracker   &get_ping_tracker()          noexcept { return m_ping_tracker; }
    /**
     * @brief Grabs the pending window delta from an unapplied INITIAL_WINDOW_SIZE change.
     * @return the delta — zero once it's been consumed and applied to open streams.
     */
    [[nodiscard]] const std::int32_t  &get_delta_window_on_settings() const noexcept { return m_delta_window_on_settings; }

  private:
    // SETTINGS_HEADER_TABLE_SIZE (0x1)
    // Maximum size of the HPACK dynamic table the sender is willing to use.
    // Default: 4096.  No upper bound specified by the RFC.
    std::uint32_t m_header_table_size{DEFAULT_HEADER_TABLE_SIZE};

    // SETTINGS_ENABLE_PUSH (0x2)
    // Whether the remote peer may send PUSH_PROMISE frames.
    // Default: true (1).  Valid values: 0 or 1 only.
    bool m_enable_push{true};

    // SETTINGS_MAX_CONCURRENT_STREAMS (0x3)
    // Maximum number of streams the sender allows the remote to open simultaneously.
    // Default: no limit (represented as max uint32).  RFC says treat as "initially
    // infinite" but we advertise 100 as a practical server-side limit.
    std::uint32_t m_max_concurrent_streams{100};

    // SETTINGS_INITIAL_WINDOW_SIZE (0x4)
    // Initial flow-control window size for new streams.
    // Default: 65535.  Max: 2^31-1.  Sending > max is a FLOW_CONTROL_ERROR.
    std::uint32_t m_initial_window_size{DEFAULT_INITIAL_WINDOW_SIZE};

    // SETTINGS_MAX_FRAME_SIZE (0x5)
    // Maximum frame payload size the sender is willing to receive.
    // Default: 16384 (2^14).  Valid range: [16384, 2^24-1].
    std::uint32_t m_max_frame_size{MIN_FRAME_SIZE};

    // SETTINGS_MAX_HEADER_LIST_SIZE (0x6)
    // Advisory limit on the total size of header fields the sender will accept.
    // Default: unlimited (represented as max uint32).
    std::uint32_t m_max_header_list_size;


    SettingsState m_state{SettingsState::UNACKNOWLEDGED};

    std::uint32_t m_last_stream_id{MAX_CONNECTED_STREAMS};

    shared_layer::ping::PingTracker m_ping_tracker;

    std::int32_t m_delta_window_on_settings{0};

    // Vendor/extension setting id/value pairs attached to this instance. On a decoded remote
    // instance: the ids apply() didn't recognize (read by on_remote_settings dispatch). On the
    // local instance: add_local_setting_override() entries (emitted by WriteSettingsAdaptor).
    // No instance is ever both, so one vector serves both directions.
    std::vector<std::pair<std::uint16_t, std::uint32_t>> m_vendor_settings;
};


struct ReadSettingsAdaptor : std::ranges::range_adaptor_closure<ReadSettingsAdaptor> {
    /**
     * @brief Defaulted, stateless — nothing to configure for reading settings off the wire.
     */
    explicit constexpr ReadSettingsAdaptor() = default;

    /**
     * @brief Decodes a SETTINGS payload — chunks `data` into 6-byte id/value pairs (dropping
     * any trailing partial chunk that isn't a full 6 bytes) and folds each one through
     * `Settings::apply()` onto a fresh `Settings{}`.
     * @tparam R a viewable range of the raw SETTINGS payload bytes.
     * @param data the payload to decode.
     * @return a fresh `Settings` with every well-formed 6-byte pair applied.
     * @throws error::http::ConnectionError if any decoded pair fails `Settings::apply()`'s
     * validation (see that method for the specific per-setting rules).
     */
    template <std::ranges::viewable_range R>
    Settings operator()(R &&data) const {
        // Slice into 6-byte pairs, drop any trailing partial chunk, decode each into
        // (id, value), then fold every pair onto a fresh Settings via apply().
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
    /**
     * @brief Range adaptor closure ctor — stashes the settings instance to encode. Bet, that's
     * it, that's the whole ctor.
     * @param settings the settings to encode. Stored by reference, must outlive this adaptor.
     */
    explicit constexpr WriteSettingsAdaptor(Settings &settings) : m_settings{settings} {}

    /**
     * @brief Encodes only the settings that differ from spec defaults — each non-default value
     * gets emitted as a 6-byte id/value pair, defaults are skipped entirely to keep the SETTINGS
     * frame lean.
     * @note ENABLE_PUSH is the odd one out: it only gets emitted when *disabled*, since `true`
     * is the default. Every other field compares against its numeric default the normal way.
     * @tparam R a viewable range this appends the encoded settings bytes onto.
     * @param range the range to append the encoded settings onto.
     * @return `range` followed by the encoded non-default settings, concatenated.
     */
    template <std::ranges::viewable_range R>
    auto operator()(R &&range) const {
        std::vector<std::byte> settings_bytes;

        // Small local helper, lowkey does all the heavy lifting — encodes one id/value pair and
        // appends it to the running buffer.
        auto emit = [&](const std::uint16_t SETTING_ID, const std::uint32_t VALUE) {
            auto entry = std::views::empty<std::byte> | utils::codec::WriteBigEndianAdaptor<std::uint16_t>{SETTING_ID} |
                         utils::codec::WriteBigEndianAdaptor<std::uint32_t>{VALUE} |
                         std::ranges::to<std::vector<std::byte>>();

            settings_bytes.insert(settings_bytes.end(), entry.begin(), entry.end());
        };

        // Every field below only gets emitted when it diverges from its spec default — keeps
        // the outgoing SETTINGS frame lean instead of always sending all six.
        if (m_settings.get().get_header_table_size() != DEFAULT_HEADER_TABLE_SIZE) {
            emit(0x1, m_settings.get().get_header_table_size());
        }

        // ENABLE_PUSH is the odd one out — true is the default, so only emit when it's off.
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

        // Vendor/extension overrides registered via add_local_setting_override() go out after
        // the six spec fields — this is how an extension's on_local_settings() mutation (e.g.
        // RFC 8441's SETTINGS_ENABLE_CONNECT_PROTOCOL) actually reaches the wire.
        for (const auto &[setting_id, value] : m_settings.get().get_vendor_settings()) {
            emit(setting_id, value);
        }

        return std::views::concat(std::forward<R>(range), std::move(settings_bytes));
    }

  private:
    std::reference_wrapper<Settings> m_settings;
};

} // namespace io::layer::http2
