export module io_layer_http2:stream;

import std;
import io_layer_shared;
import io_shared;
import io_error;
import io_codec_hpack;
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

export namespace transport::layer::http2 {


class StreamBasedHelper {
  public:
    StreamBasedHelper(const std::uint32_t stream_id, std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
                      std::reference_wrapper<codec::hpack::HPackTable> encoding_table)
        : m_request{stream_id}, m_response{stream_id},
          m_hpack{codec::hpack::Hpack<>{decoding_table, encoding_table, m_request, m_response}},
          m_expecting_continuation{false}, m_remote_done{false} {}

    codec::hpack::Hpack<> &get_hpack() noexcept { return m_hpack; }
    bool get_expecting_continuation() const noexcept { return m_expecting_continuation; }
    bool get_is_remote_done() const noexcept { return m_remote_done; }

    void set_expecting_continuation(bool value) noexcept { m_expecting_continuation = value; }
    void set_remote_done(bool value) noexcept { m_remote_done = value; }

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
    Stream(const std::uint32_t &id, std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
           std::reference_wrapper<codec::hpack::HPackTable> encoding_table,
           std::reference_wrapper<Settings> remote_settings, bool is_client_initiated = true)
        : m_state_machine{StreamStateMachine{id}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().initial_window_size())},
          m_remote_settings{remote_settings},
          m_stream_helper{make_conditional<StreamHelper>(id, decoding_table, encoding_table)} {
        if constexpr (IsStreamBased) {
            if (is_client_initiated) {
                // As a server, frames we receive from a client MUST be odd if non-zero
                if (id > 0 && (id % 2 == 0)) {
                    throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                       "Client-initiated stream ID must be odd");
                }
            } else {
                if (id % 2 != 0) {
                    throw error::http::ConnectionError(error::http::Http2ErrorCode::INTERNAL_ERROR,
                                                       "Server-initiated stream ID must be even");
                }
            }
        } else {
            if (id != 0)
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Non-zero Stream ID on connection-level stream");
        }
    }

    ~Stream() { cleanup_resources(); }

    template <shared_layer::FrameRole Role>
        requires(IsStreamBased == false)
    std::optional<Frame<shared_layer::FrameRole::Sender>>
    handle_frame(const Frame<Role> &frame,
                 error::http::Http2ErrorCode connection_error_code = error::http::Http2ErrorCode::NO_ERROR_CODE) {
        const auto &header = frame.get_header();
        const auto &type = header.get_type();

        switch (type) {
        case shared_layer::FrameType::WINDOW_UPDATE:
            update_send_window(frame.get_window_increament());
            break;

        case shared_layer::FrameType::GOAWAY: {
            m_remote_settings.get().set_trigger_goaway_after_stream_id(frame.get_stream_id());

            std::vector<std::uint8_t> goaway_payload{};
            goaway_payload.reserve(8);
            auto it = goaway_payload.begin();
            shared_layer::Atom<>::write_big_endian(it, frame.get_stream_id(), 4);
            shared_layer::Atom<>::write_big_endian(it, std::to_underlying(connection_error_code), 4);

            return std::make_optional(Frame<shared_layer::FrameRole::Sender>{
                FrameHeader{8, shared_layer::FrameType::GOAWAY, 0, 0}, goaway_payload});
        }

        case shared_layer::FrameType::PING: {
            const auto &flags = header.get_flags();

            if (flags & shared_layer::Flags::ACK) {
                auto payload_view = std::span<const std::uint8_t, 8>(frame.get_payload().data(), 8);
                std::array<std::uint8_t, 8> payload_array;
                std::ranges::copy(payload_view, payload_array.begin());

                if (!m_remote_settings.get().ping_tracker().on_ack(payload_array)) {
                    // TODO: implment logging
                    std::println("[warn] Received unsolicited PING ACK — ignoring");
                }
            } else {
                m_remote_settings.get().ping_tracker().note_activity();
                return std::make_optional(Frame<shared_layer::FrameRole::Sender>{
                    FrameHeader{8, shared_layer::FrameType::PING, shared_layer::Flags::ACK, 0}, frame.get_payload()});
            }

            break;
        }

        case shared_layer::FrameType::SETTINGS:
            if (m_remote_settings.get().is_setting_acknowledged()) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Received SETTINGS after initial exchange");
            }
            return handle_settings(frame);

        case shared_layer::FrameType::PRIORITY:
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PRIORITY frames are not supported in this implementation - deprecated "
                                               "in HTTP/2 (RFC9113)");


        case shared_layer::FrameType::DATA:
        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION:
        case shared_layer::FrameType::RST_STREAM:
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type {} is not valid on connection-level stream", static_cast<int>(type)));

        default:
            break;
        }

        return std::nullopt;
    }

    template <shared_layer::FrameRole Role>
        requires(IsStreamBased == true)
    void handle_frame(const Frame<Role> &frame, bool is_sender,
                      std::reference_wrapper<Stream<false>> connection_stream) {
        const auto &header = frame.get_header();
        const auto &type = header.get_type();

        if (m_stream_helper.get_expecting_continuation() && type != shared_layer::FrameType::CONTINUATION) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Stream expecting CONTINUATION but received {}", static_cast<int>(type)), get_stream_id());
        }

        m_state_machine.advance(frame, is_sender);


        switch (type) {
        case shared_layer::FrameType::DATA:
            consume_window(frame.get_payload_size(), is_sender, connection_stream);
            if (!is_sender) {
                append_received_data(frame.get_payload());
            }
            break;

        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION: {
            if (frame.get_payload_size() > 0) {
                try {
                    m_stream_helper.get_hpack().decode(frame.get_payload());
                } catch (error::http::Http2Exception &e) {
                    throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                                                   std::format("HPACK decoding error: {}", e.what()));
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
            const auto increment = frame.get_window_increament();

            update_send_window(increment);
            connection_stream.get().update_send_window(increment);

            break;
        }


        case shared_layer::FrameType::RST_STREAM:
            cleanup_resources();
            throw error::http::StreamError(
                get_stream_id(),
                error::http::get_http2_error_code(shared_layer::Atom<>::read_big_endian(frame.get_payload())),
                "Stream reset via RST_STREAM frame");

        case shared_layer::FrameType::PRIORITY:
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PRIORITY frames are not supported in this implementation - deprecated "
                                               "in HTTP/2 (RFC9113)");

        case shared_layer::FrameType::SETTINGS:
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "SETTINGS frame are not valid on stream-level stream and can only "
                                               "be processed after the connection preface");

        case shared_layer::FrameType::PING:
        case shared_layer::FrameType::GOAWAY:
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Frame type {} is not valid on stream-level stream", static_cast<int>(type)));

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
        requires(IsStreamBased == true)
    void consume_window(std::int32_t size, bool is_sender, std::reference_wrapper<Stream<false>> connection_stream) {
        connection_stream.get().consume_window(size, is_sender);

        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                                               "Attempted to send DATA exceeding flow control window");
            }
            m_send_window -= size;
        } else {
            m_recv_window -= size;
            if (m_recv_window < 0) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                               "Receive window underflow");
            }
        }
    }

    template <typename = void>
        requires(IsStreamBased == false)
    void consume_window(std::int32_t size, bool is_sender) {
        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                                               "Attempted to send DATA exceeding flow control window");
            }
            m_send_window -= size;
        } else {
            m_recv_window -= size;
            if (m_recv_window < 0) {
                throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                               "Receive window underflow");
            }
        }
    }

    template <typename = void>
        requires(IsStreamBased == false)
    std::optional<Frame<shared_layer::FrameRole::Sender>>
    handle_settings(const Frame<shared_layer::FrameRole::Receiver> &frame) {
        if (frame.get_header().get_flags() & shared_layer::Flags::ACK) {
            m_remote_settings.get().mark_acknowledged();
            return std::nullopt;
        }

        std::uint32_t old_window = m_remote_settings.get().initial_window_size();

        auto setting_ack = m_remote_settings.get().decode(frame);

        std::uint32_t new_window = m_remote_settings.get().initial_window_size();

        if (old_window != new_window) {
            std::int32_t delta = static_cast<std::int32_t>(new_window) - static_cast<std::int32_t>(old_window);

            update_send_window(delta);

            m_remote_settings.get().set_delta_window_on_settings(delta);
        }

        return std::make_optional(std::move(setting_ack));
    }

    bool can_send_data_of_size(std::int32_t size) const noexcept {
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

        m_send_window += increment;
    }

    void append_received_data(std::vector<std::uint8_t> data) {
        if (!m_state_machine.can_receive_data()) {
            throw error::http::StreamError(get_stream_id(), error::http::Http2ErrorCode::STREAM_CLOSED,
                                           "Received data on stream not in receiving state");
        }
        m_recv_buffer.insert(m_recv_buffer.end(), data.begin(), data.end());
    }

    template <typename = void>
        requires(IsStreamBased == true)
    bool is_remote_done() const noexcept {
        return m_stream_helper.get_is_remote_done();
    }

    [[nodiscard]] bool needs_recv_window_update() const {
        return m_recv_window < m_remote_settings.get().initial_window_size() / 2;
    }

    [[nodiscard]] std::uint32_t recv_window_increment() const {
        return static_cast<std::uint32_t>(m_remote_settings.get().initial_window_size() - m_recv_window);
    }

    std::vector<std::uint8_t> take_received_data() { return std::move(m_recv_buffer); }

    const std::int32_t &send_window() const noexcept { return m_send_window; }
    const std::int32_t &recv_window() const noexcept { return m_recv_window; }
    const std::uint32_t &get_stream_id() const noexcept { return m_state_machine.id(); }
    const shared_layer::StreamState &get_state() const noexcept { return m_state_machine.get_state(); }

  private:
    using StreamHelper = std::conditional_t<IsStreamBased, StreamBasedHelper, std::monostate>;

    void cleanup_resources() {
        m_recv_buffer.clear();
        if constexpr (IsStreamBased) {
            m_stream_helper.set_expecting_continuation(false);
        }
    }

    std::vector<std::uint8_t> m_recv_buffer;
    StreamStateMachine m_state_machine;
    std::int32_t m_send_window;
    std::int32_t m_recv_window;
    std::reference_wrapper<Settings> m_remote_settings;

    [[no_unique_address]] StreamHelper m_stream_helper;
    // [[no_unique_address]] ConnectionSettingsPtr m_remote_settings;
};

} // namespace transport::layer::http2
