module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing me
#include <ranges>

export module io_layer_http2:stream;

import std;
import io_layer_shared;
import io_shared;
import io_error;
import io_codec_hpack;
import utils_codec;
import core_logger;
import :settings;
import :frame;
import :helper;

template <typename T, typename... Args>
static constexpr T make_conditional(Args &&...args) {
    if constexpr (std::is_same_v<T, std::monostate>) {
        return {};
    } else {
        return T{std::forward<Args>(args)...};
    }
}

export namespace io::layer::http2 {


class StreamBasedHelper {
  public:
    StreamBasedHelper(const std::uint32_t STREAM_ID, std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
                      std::reference_wrapper<codec::hpack::HPackTable> encoding_table)
        : m_request{STREAM_ID}, m_response{STREAM_ID},
          m_hpack{codec::hpack::Hpack<>{decoding_table, encoding_table, m_request, m_response}},
          m_expecting_continuation{false}, m_remote_done{false} {
        core::logger::debug("StreamBasedHelper - HTTP/2", "Created for stream ID `{}` with provided HPACK tables",
                            STREAM_ID);
    }

    codec::hpack::Hpack<> &get_hpack() noexcept { return m_hpack; }
    [[nodiscard]] bool get_expecting_continuation() const noexcept { return m_expecting_continuation; }
    [[nodiscard]] bool get_is_remote_done() const noexcept { return m_remote_done; }

    void set_expecting_continuation(bool value) noexcept {
        core::logger::debug("StreamBasedHelper - HTTP/2", "Setting expecting_continuation to `{}` for stream ID `{}`",
                            value, m_request.get_stream_id());
        m_expecting_continuation = value;
    }
    void set_remote_done(bool value) noexcept {
        core::logger::debug("StreamBasedHelper - HTTP/2", "Setting remote_done to `{}` for stream ID `{}`", value,
                            m_request.get_stream_id());
        m_remote_done = value;
    }

  private:
    shared::http::HttpRequest m_request;
    shared::http::HttpResponse m_response;
    codec::hpack::Hpack<> m_hpack;
    bool m_expecting_continuation;
    bool m_remote_done;
};


template <bool IsStreamBased = true>
class Stream {
  public:
    Stream(const std::uint32_t &stream_id, std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
           std::reference_wrapper<codec::hpack::HPackTable> encoding_table,
           std::reference_wrapper<Settings> local_settings, std::reference_wrapper<Settings> remote_settings,
           bool is_client_initiated = true)
        : m_state_machine{StreamStateMachine{stream_id}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_local_settings{local_settings}, m_remote_settings{remote_settings},
          m_stream_helper{make_conditional<StreamHelper>(stream_id, decoding_table, encoding_table)} {
        if constexpr (IsStreamBased) {
            if (is_client_initiated) {
                // As a server, frames we receive from a client MUST be odd if non-zero
                if (stream_id > 0 && (stream_id % 2 == 0)) {
                    throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                       "Client-initiated stream ID must be odd");
                }
            } else {
                if (stream_id % 2 != 0) {
                    throw error::http::ConnectionError(error::http::Http2ErrorCode::INTERNAL_ERROR,
                                                       "Server-initiated stream ID must be even");
                }
            }
        } else {
            if (stream_id != 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Non-zero Stream ID on connection-level stream");
            }
        }

        core::logger::debug("Stream - HTTP/2", "Created stream with ID `{}` and initial window size `{}`", stream_id,
                            remote_settings.get().initial_window_size());
    }

    ~Stream() { cleanup_resources(); }

    template <shared_layer::FrameRole Role>
        requires(!IsStreamBased)
    std::optional<Frame<shared_layer::FrameRole::Sender>>
    handle_frame(const Frame<Role> &frame,
                 error::http::Http2ErrorCode connection_error_code = error::http::Http2ErrorCode::NO_ERROR_CODE) {
        const auto &header = frame.get_header();
        const auto &type = header.get_type();

        core::logger::info("Stream (connection-level) - HTTP/2",
                           "Handling frame of type `{}` on connection-level stream with length `{}` and flags `{}`",
                           type, header.get_length(), header.get_flags());

        switch (type) {
        case shared_layer::FrameType::WINDOW_UPDATE: {
            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received WINDOW_UPDATE frame with increment `{}` on connection-level stream, updating send window",
                frame.get_window_increament());
            update_send_window(frame.get_window_increament());
            break;
        }
        case shared_layer::FrameType::GOAWAY: {
            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received GOAWAY frame with last stream ID `{}` and error code `{}`, marking connection as closing",
                frame.get_stream_id(), connection_error_code);
            m_remote_settings.get().set_trigger_goaway_after_stream_id(frame.get_stream_id());

            auto payload =
                std::views::empty<std::byte> |
                utils::codec::WriteBigEndianAdaptor<std::uint32_t>{frame.get_stream_id()} |
                utils::codec::WriteBigEndianAdaptor<std::uint32_t>{std::to_underlying(connection_error_code)};

            return std::make_optional(Frame<shared_layer::FrameRole::Sender>{}
                                          .add_header(FrameHeader{}
                                                          .add_length(8)
                                                          .add_type(shared_layer::FrameType::GOAWAY)
                                                          .add_flags(0)
                                                          .add_stream_id(0))
                                          .add_payload(payload)
                                          .build());
        }

        case shared_layer::FrameType::PING: {
            core::logger::info("Stream (connection-level) - HTTP/2",
                               "Received PING frame with flags `{}` on connection-level stream, processing PING",
                               header.get_flags());
            const auto &flags = header.get_flags();

            if (flags & shared_layer::Flags::ACK) {
                std::array<std::byte, 8> payload_array;
                auto payload_view = frame.get_payload();
                std::ranges::copy(payload_view, payload_array.begin());

                if (!m_remote_settings.get().ping_tracker().on_ack(payload_array)) {
                    core::logger::warning("Stream (connection-level) - HTTP/2",
                                          "Received unsolicited PING ACK with payload `{}`, ignoring",
                                          payload_array.size());
                }
            } else {
                m_remote_settings.get().ping_tracker().note_activity();

                core::logger::info("Stream (connection-level) - HTTP/2",
                                   "Received PING frame without ACK flag, sending PING ACK with same payload");

                return std::make_optional(Frame<shared_layer::FrameRole::Sender>{}
                                              .add_header(FrameHeader{}
                                                              .add_length(8)
                                                              .add_type(shared_layer::FrameType::PING)
                                                              .add_flags(shared_layer::Flags::ACK)
                                                              .add_stream_id(0))
                                              .add_payload(frame.get_payload())
                                              .build());
            }

            break;
        }

        case shared_layer::FrameType::SETTINGS: {

            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received SETTINGS frame with length `{}` on connection-level stream, processing SETTINGS",
                header.get_length());

            return handle_settings(frame);
        }
        case shared_layer::FrameType::PRIORITY: {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PRIORITY frames are not supported in this implementation - deprecated "
                                               "in HTTP/2 (RFC9113)");
        }

        case shared_layer::FrameType::DATA:
        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION:
        case shared_layer::FrameType::RST_STREAM: {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type `{}` is not valid on connection-level stream", type));
        }
        default:
            break;
        }

        return std::nullopt;
    }

    template <shared_layer::FrameRole Role>
        requires(IsStreamBased)
    void handle_frame(const Frame<Role> &frame, bool is_sender,
                      std::reference_wrapper<Stream<false>> connection_stream) {
        const auto &header = frame.get_header();
        const auto &type = header.get_type();

        core::logger::info("Stream - HTTP/2",
                           "Handling frame of type `{}` on stream ID `{}` with length `{}` and flags `{}`", type,
                           get_stream_id(), header.get_length(), header.get_flags());

        if (m_stream_helper.get_expecting_continuation() && type != shared_layer::FrameType::CONTINUATION) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               std::format("Stream expecting CONTINUATION but received `{}`", type),
                                               get_stream_id());
        }

        m_state_machine.advance(frame, is_sender);

        switch (type) {
        case shared_layer::FrameType::DATA: {
            core::logger::info("Stream - HTTP/2",
                               "Received DATA frame with payload size `{}` on stream ID `{}`, consuming flow control "
                               "window and appending data if receiver",
                               frame.get_payload_size(), get_stream_id());
            consume_window(frame.get_payload_size(), is_sender, connection_stream);
            if (!is_sender) {
                append_received_data(frame.get_payload());
            }
            break;
        }
        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION: {
            core::logger::info(
                "Stream - HTTP/2",
                "Received `{}` frame with payload size `{}` on stream ID `{}`, processing HPACK decoding "
                "and continuation state",
                type, frame.get_payload_size(), get_stream_id());
            if (frame.get_payload_size() > 0) {
                try {
                    if (auto consumed = m_stream_helper.get_hpack().decode(frame.get_payload());
                        consumed < frame.get_payload_size()) {
                        throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                                                       std::format("HPACK decoding did not consume entire payload: "
                                                                   "consumed `{}` bytes but payload size is `{}` bytes",
                                                                   consumed, frame.get_payload_size()));
                    }
                } catch (error::http::Http2Exception &e) {
                    throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                                                   std::format("HPACK decoding error `{}`", e.what()));
                }
            }

            const auto &flags = header.get_flags();
            if (flags & shared_layer::Flags::END_HEADERS) {
                m_stream_helper.set_expecting_continuation(false);
            } else {
                m_stream_helper.set_expecting_continuation(true);
            }

            break;
        }

        case shared_layer::FrameType::WINDOW_UPDATE: {
            core::logger::info(
                "Stream - HTTP/2",
                "Received WINDOW_UPDATE frame with increment `{}` on stream ID `{}`, updating send window",
                frame.get_window_increament(), get_stream_id());
            const auto INCREMENT = frame.get_window_increament();

            update_send_window(INCREMENT);
            connection_stream.get().update_send_window(INCREMENT);

            break;
        }


        case shared_layer::FrameType::RST_STREAM: {
            cleanup_resources();
            throw error::http::StreamError(
                get_stream_id(),
                error::http::get_http2_error_code(shared_layer::Atom<>::read_big_endian(frame.get_payload())),
                "Stream reset via RST_STREAM frame");
        }

        case shared_layer::FrameType::PRIORITY: {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PRIORITY frames are not supported in this implementation - deprecated "
                                               "in HTTP/2 (RFC9113)");
        }

        case shared_layer::FrameType::SETTINGS: {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "SETTINGS frame are not valid on stream-level stream and can only "
                                               "be processed after the connection preface");
        }

        case shared_layer::FrameType::PING:
        case shared_layer::FrameType::GOAWAY: {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type `{}` is not valid on stream-level stream", type));
        }

        default:
            break;
        }

        if (m_state_machine.get_state() == shared_layer::StreamState::HalfClosedRemote) {
            m_stream_helper.set_remote_done(true);
        }

        if (m_state_machine.is_closed()) {
            cleanup_resources();
        }
    }


    template <typename = void>
        requires(IsStreamBased)
    void consume_window(std::int32_t size, bool is_sender, std::reference_wrapper<Stream<false>> connection_stream) {
        core::logger::debug("Stream - HTTP/2",
                            "Consuming flow control window for stream ID `{}`: size `{}`, is_sender `{}`",
                            get_stream_id(), size, is_sender);

        connection_stream.get().consume_window(size, is_sender);

        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                                               "Attempted to send DATA exceeding flow control window");
            }
            core::logger::debug("Stream - HTTP/2",
                                "Reducing send window for stream ID `{}` by `{}`, new send window is `{}`",
                                get_stream_id(), size, m_send_window - size);
            m_send_window -= size;
        } else {
            m_recv_window -= size;
            core::logger::debug("Stream - HTTP/2",
                                "Reducing receive window for stream ID `{}` by `{}`, new receive window is `{}`",
                                get_stream_id(), size, m_recv_window);
            if (m_recv_window < 0) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                               "Receive window underflow");
            }
        }
    }

    template <typename = void>
        requires(!IsStreamBased)
    void consume_window(std::int32_t size, bool is_sender) {
        core::logger::debug("Stream (connection-level) - HTTP/2",
                            "Consuming flow control window of size `{}` for {} on connection-level stream", size,
                            is_sender ? "sender" : "receiver");
        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                                               "Attempted to send DATA exceeding flow control window");
            }
            core::logger::debug("Stream (connection-level) - HTTP/2",
                                "Reducing send window for connection-level stream by `{}`, new send window is `{}`",
                                size, m_send_window - size);
            m_send_window -= size;
        } else {
            m_recv_window -= size;
            core::logger::debug(
                "Stream (connection-level) - HTTP/2",
                "Reducing receive window for connection-level stream by `{}`, new receive window is `{}`", size,
                m_recv_window);
            if (m_recv_window < 0) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                               "Receive window underflow");
            }
        }
    }


    [[nodiscard]] bool can_send_data_of_size(std::int32_t size) const noexcept {
        return m_state_machine.can_send_data() && (m_send_window >= size);
    }

    void update_send_window(std::uint32_t increment) {
        if (increment <= 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Window update increment must be positive", get_stream_id());
        }

        if (m_send_window + increment > MAX_INITIAL_WINDOW_SIZE) {
            throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                           "Flow control window overflow");
        }

        core::logger::debug("Stream - HTTP/2",
                            "Updating send window for stream ID `{}` by increment `{}`, new send window is `{}`",
                            get_stream_id(), increment, m_send_window + increment);
        m_send_window += increment;
    }

    void append_received_data(std::span<const std::byte> data) {
        if (!m_state_machine.can_receive_data()) {
            throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::STREAM_CLOSED,
                                           "Received data on stream not in receiving state");
        }
        core::logger::debug("Stream - HTTP/2",
                            "Appending received data of size `{}` to receive buffer for stream ID `{}`", data.size(),
                            get_stream_id());
        m_recv_buffer.insert(m_recv_buffer.end(), data.begin(), data.end());
    }

    template <typename = void>
        requires(IsStreamBased)
    bool is_remote_done() const noexcept {
        return m_stream_helper.get_is_remote_done();
    }

    [[nodiscard]] bool needs_recv_window_update() const {
        return m_recv_window < m_remote_settings.get().initial_window_size() / 2;
    }

    [[nodiscard]] std::uint32_t recv_window_increment() const {
        return static_cast<std::uint32_t>(m_remote_settings.get().initial_window_size() - m_recv_window);
    }

    std::vector<std::byte> take_received_data() { return std::move(m_recv_buffer); }

    [[nodiscard]] const std::int32_t &send_window() const noexcept { return m_send_window; }
    [[nodiscard]] const std::int32_t &recv_window() const noexcept { return m_recv_window; }
    [[nodiscard]] const std::uint32_t &get_stream_id() const noexcept { return m_state_machine.id(); }
    [[nodiscard]] const shared_layer::StreamState &get_state() const noexcept { return m_state_machine.get_state(); }

  private:
    using StreamHelper = std::conditional_t<IsStreamBased, StreamBasedHelper, std::monostate>;

    template <typename = void>
        requires(!IsStreamBased)
    std::optional<Frame<shared_layer::FrameRole::Sender>>
    handle_settings(const Frame<shared_layer::FrameRole::Receiver> &frame) {
        if (frame.get_header().get_flags() & shared_layer::Flags::ACK) {
            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received SETTINGS ACK frame on connection-level stream, marking settings as acknowledged");

            m_local_settings.get().set_state(SettingsState::ACKNOWLEDGED);
            return std::nullopt;
        }

        if (m_remote_settings.get().is_finished()) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Received SETTINGS after initial exchange");
        }

        std::uint32_t old_window = m_remote_settings.get().initial_window_size();

        auto settings = frame.get_payload() | ReadSettingsAdaptor{};
        auto setting_ack = Settings::generate_ack();

        std::uint32_t new_window = m_remote_settings.get().initial_window_size();

        if (old_window != new_window) {
            std::int32_t delta = static_cast<std::int32_t>(new_window) - static_cast<std::int32_t>(old_window);

            update_send_window(delta);

            m_remote_settings.get().set_delta_window_on_settings(delta);
        }

        m_remote_settings.get().set_state(SettingsState::ACKNOWLEDGED);

        core::logger::info(
            "Stream (connection-level) - HTTP/2",
            "Processed SETTINGS frame, old initial window size was `{}`, new initial window size is `{}`, "
            "delta applied to send window is `{}`",
            old_window, new_window, new_window - old_window);

        return std::make_optional(std::move(setting_ack));
    }

    void cleanup_resources() {
        core::logger::info("Stream - HTTP/2", "Cleaning up resources for stream ID `{}`", get_stream_id());
        m_recv_buffer.clear();
        if constexpr (IsStreamBased) {
            m_stream_helper.set_expecting_continuation(false);
        }
    }

    std::vector<std::byte> m_recv_buffer;
    StreamStateMachine m_state_machine;
    std::int32_t m_send_window;
    std::int32_t m_recv_window;
    std::reference_wrapper<Settings> m_local_settings;
    std::reference_wrapper<Settings> m_remote_settings;

    [[no_unique_address]] StreamHelper m_stream_helper;
};

} // namespace io::layer::http2
