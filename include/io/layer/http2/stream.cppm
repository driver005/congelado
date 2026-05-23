module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing me
#include <optional>
#include <ranges>
#include <type_traits>

export module io_layer_http2:stream;

import std;
import io_layer_shared;
import io_shared;
import io_error;
import io_codec_hpack;
import utils_codec;
import core_logger;
import utils_buffering;
import :settings;
import :frame;
import :helper;
import :request;
import :response;

template <typename T, typename... Args>
static constexpr T make_conditional(Args &&...args) {
    if constexpr (std::is_same_v<T, std::monostate>) {
        return {};
    } else {
        return T{std::forward<Args>(args)...};
    }
}

export namespace io::layer::http2 {

template <bool IsStreamBased = true>
class Stream;

class ConnectionLevelHelper {
  public:
    ConnectionLevelHelper(
        error::http::Http2ErrorCode connection_error_code = error::http::Http2ErrorCode::NO_ERROR_CODE)
        : m_connection_error_code{connection_error_code} {}

    void set_connection_error_code(error::http::Http2ErrorCode error_code) noexcept {
        core::logger::debug("ConnectionLevelHelper - HTTP/2",
                            "Setting connection error code to `{}` for connection-level stream", error_code);

        m_connection_error_code = error_code;
    }

    [[nodiscard]] const error::http::Http2ErrorCode &get_connection_error_code() const noexcept {
        return m_connection_error_code;
    }

  private:
    error::http::Http2ErrorCode m_connection_error_code;
};

class StreamLevelHelper {
  public:
    StreamLevelHelper(const std::uint32_t STREAM_ID, Stream<false> &connection_stream,
                      std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
                      std::reference_wrapper<codec::hpack::HPackTable> encoding_table)
        : m_connection_stream{connection_stream}, m_request{STREAM_ID}, m_response{STREAM_ID},
          m_hpack{decoding_table, encoding_table, m_request, m_response},
          m_header_block{std::make_optional<utils::buffering::BufferView>()}, m_expecting_continuation{false},
          m_remote_done{false} {
        core::logger::debug("StreamLevelHelper - HTTP/2", "Created for stream ID `{}` with provided HPACK tables",
                            STREAM_ID);
    }

    void set_expecting_continuation(bool value) noexcept {
        core::logger::debug("StreamLevelHelper - HTTP/2", "Setting expecting_continuation to `{}` for stream ID `{}`",
                            value, m_request.get_stream_id());

        m_expecting_continuation = value;
    }

    void set_remote_done(bool value) noexcept {
        core::logger::debug("StreamLevelHelper - HTTP/2", "Setting remote_done to `{}` for stream ID `{}`", value,
                            m_request.get_stream_id());

        m_remote_done = value;
    }


    void clear_header_block() noexcept {
        core::logger::debug("StreamLevelHelper - HTTP/2", "Clearing header block for stream ID `{}`",
                            m_request.get_stream_id());

        m_header_block.reset();
    }

    utils::buffering::BufferView &get_header_block() {
        if (!m_header_block.has_value()) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                "You are trying to receive second collection of headers on the same stream ID `{}`",
                m_request.get_stream_id());
        }
        return m_header_block.value();
    }
    std::reference_wrapper<Stream<false>> get_connection_stream() noexcept { return m_connection_stream; }
    HttpRequest &get_request() noexcept { return m_request; }
    HttpResponse &get_response() noexcept { return m_response; }
    codec::hpack::Hpack<HttpRequest, shared::http::HeaderEntry, shared::http::Token, HttpResponse> &
    get_hpack() noexcept {
        return m_hpack;
    }
    [[nodiscard]] bool get_expecting_continuation() const noexcept { return m_expecting_continuation; }
    [[nodiscard]] bool get_is_remote_done() const noexcept { return m_remote_done; }

  private:
    std::reference_wrapper<Stream<false>> m_connection_stream;
    HttpRequest m_request;
    HttpResponse m_response;
    codec::hpack::Hpack<HttpRequest, shared::http::HeaderEntry, shared::http::Token, HttpResponse> m_hpack;
    std::optional<utils::buffering::BufferView> m_header_block;
    bool m_expecting_continuation;
    bool m_remote_done;
};


template <bool IsStreamBased = true>
class Stream {
  public:
    Stream(std::reference_wrapper<Settings> local_settings, std::reference_wrapper<Settings> remote_settings)
        requires(!IsStreamBased)
        : m_state_machine{StreamStateMachine{0}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_local_settings{local_settings}, m_remote_settings{remote_settings},
          m_stream_helper{ConnectionLevelHelper{error::http::Http2ErrorCode::NO_ERROR_CODE}} {
        core::logger::debug("Stream - HTTP/2", "Created Connection-level Stream (ID 0)");
    }

    Stream(const std::uint32_t STREAM_ID, Stream<false> &connection_stream,
           std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
           std::reference_wrapper<codec::hpack::HPackTable> encoding_table,
           std::reference_wrapper<Settings> local_settings, std::reference_wrapper<Settings> remote_settings,
           bool is_client_initiated = true)
        requires(IsStreamBased)
        : m_state_machine{StreamStateMachine{STREAM_ID}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_local_settings{local_settings}, m_remote_settings{remote_settings},
          m_stream_helper{StreamLevelHelper{STREAM_ID, connection_stream, decoding_table, encoding_table}} {
        if (is_client_initiated) {
            // As a server, frames we receive from a client MUST be odd if non-zero
            if (STREAM_ID > 0 && (STREAM_ID % 2 == 0)) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Client-initiated stream ID must be odd");
            }
        } else {
            if (STREAM_ID % 2 != 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::INTERNAL_ERROR,
                                                   "Server-initiated stream ID must be even");
            }
        }

        core::logger::debug("Stream - HTTP/2", "Created Stream ID `{}`", STREAM_ID);
    }

    ~Stream() { cleanup_resources(); }

    template <shared_layer::FrameRole Role>
        requires(!IsStreamBased)
    std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>> receive(const FrameHeader<Role> &header,
                                                                         utils::buffering::BufferReader &reader) {

        const auto &type = header.get_type();
        std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>> response = std::nullopt;

        core::logger::info("Stream (connection-level) - HTTP/2",
                           "Handling frame of type `{}` on connection-level stream with length `{}` and flags `{}`",
                           type, header.get_length(), header.get_flags());

        switch (type) {
        case shared_layer::FrameType::WINDOW_UPDATE: {
            auto increment = reader | ReadWindowIncrementAdaptor{};

            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received WINDOW_UPDATE frame with increment `{}` on connection-level stream, updating send window",
                increment);

            update_send_window(increment);
            break;
        }
        case shared_layer::FrameType::GOAWAY: {
            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received GOAWAY frame with last stream ID `{}` and error code `{}`, marking connection as closing",
                header.get_stream_id(), m_stream_helper.get_connection_error_code());

            m_remote_settings.get().set_last_stream_id(header.get_stream_id());

            auto payload = std::views::empty<std::byte> |
                           utils::codec::WriteBigEndianAdaptor<std::uint32_t>{header.get_stream_id()} |
                           utils::codec::WriteBigEndianAdaptor<std::uint32_t>{
                               std::to_underlying(m_stream_helper.get_connection_error_code())};

            response = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                           .add_type(shared_layer::FrameType::GOAWAY)
                           .add_flags(0)
                           .add_stream_id(0)
                           .add_payload(payload)
                           .build();
            break;
        }

        case shared_layer::FrameType::PING: {
            core::logger::info("Stream (connection-level) - HTTP/2",
                               "Received PING frame with flags `{}` on connection-level stream, processing PING",
                               header.get_flags());

            const auto &flags = header.get_flags();

            if (flags & shared_layer::Flags::ACK) {
                auto payload_array = reader | std::views::take(8) | std::ranges::to<std::vector<std::byte>>();
                if (!m_remote_settings.get().ping_tracker().on_ack(payload_array)) {
                    core::logger::warning("Stream (connection-level) - HTTP/2",
                                          "Received unsolicited PING ACK with payload `{}`, ignoring",
                                          payload_array.size());
                }
            } else {
                m_remote_settings.get().ping_tracker().note_activity();

                core::logger::info("Stream (connection-level) - HTTP/2",
                                   "Received PING frame without ACK flag, sending PING ACK with same payload");

                response = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                               .add_type(shared_layer::FrameType::PING)
                               .add_flags(shared_layer::Flags::ACK)
                               .add_stream_id(0)
                               .add_payload(reader | std::views::take(8))
                               .build();
            }

            break;
        }

        case shared_layer::FrameType::SETTINGS: {
            core::logger::info(
                "Stream (connection-level) - HTTP/2",
                "Received SETTINGS frame with length `{}` on connection-level stream, processing SETTINGS",
                header.get_length());

            response = handle_settings(header, reader);
            break;
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
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               std::format("Type `{}` is not valid on connection-level stream", type));
        }
        default:
            break;
        }

        reader.consume(header.get_length());

        return response;
    }


    template <shared_layer::FrameRole Role>
        requires(IsStreamBased)
    void receive(const FrameHeader<Role> &header, utils::buffering::BufferReader &reader) {

        const auto &type = header.get_type();

        core::logger::info("Stream - HTTP/2",
                           "Handling frame of type `{}` on stream ID `{}` with length `{}` and flags `{}`", type,
                           get_stream_id(), header.get_length(), header.get_flags());

        if (m_stream_helper.get_expecting_continuation() && type != shared_layer::FrameType::CONTINUATION) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               std::format("Stream expecting CONTINUATION but received `{}`", type),
                                               get_stream_id());
        }

        m_state_machine.advance(header.get_type(), header.get_flags(), false);

        switch (type) {
        case shared_layer::FrameType::DATA: {
            auto &view = m_stream_helper.get_request().get_body();
            if (header.get_length() > 0) {
                consume_window(header.get_length(), false);

                reader.expand_view(view, header.get_length());
                if (view.size() != header.get_length()) {
                    throw error::http::ConnectionError(
                        error::http::Http2ErrorCode::INTERNAL_ERROR,
                        std::format("Failed to read DATA payload expected `{}`, got `{}` bytes", header.get_length(),
                                    view.size()),
                        get_stream_id());
                }
            }


            const auto &flags = header.get_flags();
            if (flags & shared_layer::Flags::END_STREAM) {
                core::logger::info("Stream - HTTP/2", "Stream ID `{}` - Received DATA frame with payload size `{}`",
                                   get_stream_id(), view.size());
            } else {
                core::logger::info(
                    "Stream - HTTP/2",
                    "Stream ID `{}` - Received DATA frame with partial payload size `{}`, expecting more DATA frames",
                    get_stream_id(), view.size());
            }

            break;
        }

        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION: {
            auto &view = m_stream_helper.get_header_block();
            if (header.get_length() > 0) {
                reader.expand_view(view, header.get_length());
                if (view.size() != header.get_length()) {
                    throw error::http::ConnectionError(
                        error::http::Http2ErrorCode::INTERNAL_ERROR,
                        std::format("Failed to read {} payload expected `{}`, got `{}` bytes", type,
                                    header.get_length(), view.size()),
                        get_stream_id());
                }
            }

            const auto &flags = header.get_flags();
            if (flags & shared_layer::Flags::END_HEADERS) {
                m_stream_helper.set_expecting_continuation(false);
                handle_header(view);

                core::logger::info("Stream - HTTP/2", "Stream ID `{}` - Received {} frame with payload size `{}`", type,
                                   get_stream_id(), view.size());
            } else {
                m_stream_helper.set_expecting_continuation(true);

                core::logger::info(
                    "Stream - HTTP/2",
                    "Stream ID `{}` - Received {} frame with partial payload size `{}`, expecting CONTINUATION", type,
                    get_stream_id(), view.size());
            }

            break;
        }

        case shared_layer::FrameType::WINDOW_UPDATE: {
            auto increment = reader | ReadWindowIncrementAdaptor{};

            core::logger::info(
                "Stream - HTTP/2",
                "Received WINDOW_UPDATE frame with increment `{}` on stream ID `{}`, updating send window", increment,
                get_stream_id());

            update_send_window(increment);

            break;
        }

        case shared_layer::FrameType::RST_STREAM: {
            auto error_code = reader | std::views::take(4) | utils::codec::ReadBigEndianAdaptor<>{};

            cleanup_resources();

            throw error::http::StreamError(get_stream_id(), error::http::get_http2_error_code(error_code),
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
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               std::format("Type `{}` is not valid on stream-level stream", type));
        }

        default:
            break;
        }

        reader.consume(header.get_length());

        if (m_state_machine.get_state() == shared_layer::StreamState::HALF_CLOSED_REMOTE) {
            m_stream_helper.set_remote_done(true);
        }

        if (m_state_machine.is_closed()) {
            cleanup_resources();
        }
    }

    void handle_header(utils::buffering::BufferView &view) {
        if (view.size() > 0) {
            try {
                if (auto consumed = m_stream_helper.get_hpack().decode(view); consumed < view.size()) {
                    throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                                                   std::format("HPACK decoding did not consume entire payload: "
                                                               "consumed `{}` bytes but payload size is `{}` bytes",
                                                               consumed, view.size()));
                }
            } catch (error::http::Http2Exception &e) {
                m_stream_helper.clear_header_block();
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                                               std::format("HPACK decoding error `{}`", e.what()));
            }
            m_stream_helper.clear_header_block();
        }
    }

    void consume_window(std::int32_t size, bool is_sender)
        requires(IsStreamBased)
    {
        core::logger::debug("Stream - HTTP/2",
                            "Consuming flow control window for stream ID `{}`: size `{}`, is_sender `{}`",
                            get_stream_id(), size, is_sender);

        m_stream_helper.get_connection_stream().get().consume_window(size, is_sender);

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


    void consume_window(std::int32_t size, bool is_sender)
        requires(!IsStreamBased)
    {
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

    void update_send_window(const std::uint32_t &increment) {
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
        if constexpr (IsStreamBased) {
            m_stream_helper.get_connection_stream().get().update_send_window(increment);
        }
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

    [[nodiscard]] bool is_remote_done() const noexcept
        requires(IsStreamBased)
    {
        return m_stream_helper.get_is_remote_done();
    }

    void advance_send(const shared_layer::FrameType &type, const std::uint8_t &flags)
        requires(IsStreamBased)
    {
        m_state_machine.advance(type, flags, false);
        if (m_state_machine.is_closed()) {
            cleanup_resources();
        }
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
    using StreamHelper = std::conditional_t<IsStreamBased, StreamLevelHelper, ConnectionLevelHelper>;

    std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>>
    handle_settings(const FrameHeader<shared_layer::FrameRole::RECEIVER> &header,
                    utils::buffering::BufferReader &reader)
        requires(!IsStreamBased)
    {
        if ((header.get_flags() & shared_layer::Flags::ACK) != 0) {
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

        auto settings = reader | ReadSettingsAdaptor{};

        std::uint32_t new_window = m_remote_settings.get().initial_window_size();

        if (old_window != new_window) {
            std::int32_t delta = static_cast<std::int32_t>(new_window) - static_cast<std::int32_t>(old_window);

            update_send_window(delta);

            m_remote_settings.get().set_delta_window_on_settings(delta);
        }

        m_remote_settings.get().set_state(SettingsState::ACKNOWLEDGED);
        auto setting_ack = Settings::generate_ack();

        core::logger::info(
            "Stream (connection-level) - HTTP/2",
            "Processed SETTINGS frame, old initial window size was `{}`, new initial window size is `{}`, "
            "delta applied to send window is `{}`",
            old_window, new_window, new_window - old_window);

        return std::make_optional(setting_ack);
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
