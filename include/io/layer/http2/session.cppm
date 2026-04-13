export module io_layer_http2:session;

import std;
import io_tls;
import io_codec_hpack;
import :settings;
import :stream;

export namespace transport::layer::http2 {

template <bool IsServer = true>
class Session {
  public:
    explicit Session(base::tls::basic::Connection &&conn)
        : m_conn(std::move(conn)), m_running{true}, m_local_settings{Settings{}}, m_remote_settings{Settings{}},
          m_closed_streams{}, m_header_buffer{}, m_decoding_table{codec::hpack::HPackTable{}},
          m_encoding_table{codec::hpack::HPackTable{}},
          m_connection_stream{Stream<false>{0, m_decoding_table, m_encoding_table, m_remote_settings, false}} {}


    void loop() {
        handshake();

        while (m_running) {
            try {
                std::array<std::uint8_t, 9> header_bytes;
                if (!receive_exact(std::as_writable_bytes(std::span{header_bytes}))) {
                    m_running = false;
                    break;
                }
                auto header = FrameHeader::from_bytes(header_bytes, m_local_settings.max_frame_size());

                std::println("Sending on Stream ID {} ", header.get_stream_id());

                std::vector<std::uint8_t> frame_data;
                frame_data.reserve(header.get_length());

                if (header.get_length() > 0) {
                    frame_data.resize(header.get_length());
                    auto payload_span = std::as_writable_bytes(std::span{frame_data});

                    if (!receive_exact(payload_span)) {
                        throw error::http::ConnectionError{error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                           "Connection closed during payload transfer",
                                                           header.get_stream_id()};
                    }
                }

                if (header.get_stream_id() > m_remote_settings.trigger_goaway_after_stream_id()) {
                    throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                       "Received frame for stream ID above GOAWAY threshold",
                                                       m_remote_settings.trigger_goaway_after_stream_id()};
                }

                if (m_remote_settings.is_setting_acknowledged()) {
                    if (m_remote_settings.delta_window_on_settings() > 0) {
                        for (auto &[id, stream] : m_streams) {
                            stream->update_send_window(m_remote_settings.delta_window_on_settings());
                        }

                        m_remote_settings.set_delta_window_on_settings(0);
                    }
                }

                auto frame = Frame<shared_layer::FrameRole::Receiver>{std::move(header), std::move(frame_data)};

                if (header.get_stream_id() == 0) {
                    if (auto frm = m_connection_stream.handle_frame(frame); frm.has_value()) {
                        send_frame(frm.value());
                    }
                } else {
                    auto &stream = get_or_create_stream(header.get_stream_id());
                    stream.handle_frame(frame, false, m_connection_stream);

                    if (stream.is_remote_done()) {
                        response(stream.get_stream_id());
                    }
                }
            } catch (const error::http::ConnectionError &e) {
                m_running = false;

                std::vector<std::uint8_t> goaway_payload{};
                goaway_payload.reserve(8);
                auto it = goaway_payload.begin();
                shared_layer::Atom<>::write_big_endian(it, e.get_last_stream_id(), 4);
                shared_layer::Atom<>::write_big_endian(it, std::to_underlying(e.get_code()), 4);

                auto frame = Frame<shared_layer::FrameRole::Sender>{
                    FrameHeader{8, shared_layer::FrameType::GOAWAY, 0, 0}, goaway_payload};

                send_frame(frame);

                m_conn.close();

                break;
            } catch (const error::http::StreamError &e) {
                std::vector<std::uint8_t> payload;
                payload.reserve(4);

                auto it = std::back_inserter(payload);
                shared_layer::Atom<>::write_big_endian(it, std::to_underlying(e.get_code()), 4);

                auto frame = Frame<shared_layer::FrameRole::Sender>{
                    FrameHeader{4, shared_layer::FrameType::RST_STREAM, 0, e.get_stream_id()}, payload};

                send_frame(frame);
                mark_stream_closed(e.get_stream_id());

                continue;
            }
        }
    }


  private:
    void response(std::uint32_t stream_id) {
        std::vector<std::uint8_t> hpack;
        hpack.reserve(32);

        // :status: 200  (indexed, §6.1)
        hpack.push_back(0x88);

        // content-type: text/plain  (literal with incremental indexing, §6.2.1)
        // static index 31 = content-type
        hpack.push_back(0x40 | 31);                     // name  = idx 31
        hpack.push_back(static_cast<std::uint8_t>(10)); // "text/plain" len
        for (char c : std::string_view{"text/plain"})
            hpack.push_back(static_cast<std::uint8_t>(c));

        Frame<shared_layer::FrameRole::Sender> headers_frame{
            FrameHeader{static_cast<std::uint32_t>(hpack.size()), shared_layer::FrameType::HEADERS,
                        shared_layer::Flags::END_HEADERS | shared_layer::Flags::END_STREAM, stream_id},
            hpack};

        std::println("Prepared response HEADERS frame with payload size {}", hpack.size());

        std::vector<std::uint8_t> payload;

        auto it = std::back_inserter(payload);

        headers_frame.encode(it);
        std::println("Encoded HEADERS frame with payload size {}", payload.size());

        // m_conn.send(std::as_bytes(std::span{payload}));
    }


    void handshake() {
        if constexpr (IsServer) {
            send_settings();
            // std::array<std::uint8_t, 24> buf{};
            // TODO: We should implement a proper read buffer and handle partial reads instead of assuming that the
            // entire preface is received in one go.
            // if (m_conn.recv(std::as_writable_bytes(std::span{buf})) == 0 ||
            //     !std::ranges::equal(buf, HTTP2_CONNECTION_PREFACE)) {
            //     throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
            //                                        "Client did not send valid HTTP/2 connection preface"};
            // }
        } else {
            // TODO: We should implement a proper write buffer and handle partial writes instead of assuming that the
            // entire preface is sent in one go.
            // m_conn.send(std::as_bytes(std::span{HTTP2_CONNECTION_PREFACE}));
            send_settings();
        }
    }


    Stream<> &get_or_create_stream(const std::uint32_t &id) {
        if (id == 0)
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"};

        auto it = m_streams.find(id);
        if (it != m_streams.end()) {
            if (it->second == nullptr) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   std::format("Stream ID {} is closed", id),
                                                   m_remote_settings.trigger_goaway_after_stream_id()};
            }
            return *(it->second);
        }

        auto stream = std::make_unique<Stream<>>(id, m_decoding_table, m_encoding_table, m_remote_settings);

        auto [new_it, inserted] = m_streams.emplace(id, std::move(stream));
        return *(new_it->second);
    }


    void send_settings() {
        std::vector<std::uint8_t> payload;

        m_local_settings.encode(std::back_inserter(payload));

        Frame<shared_layer::FrameRole::Sender> settings_frame{
            {static_cast<std::uint32_t>(payload.size()), shared_layer::FrameType::SETTINGS, 0, 0}, std::move(payload)};

        send_frame(settings_frame);
    }

    bool receive_exact(std::span<std::byte> target) {
        std::size_t offset = 0;
        while (offset < target.size()) {
            // We only ask for the remaining bytes (target.size() - offset)
            // TODO: This is a temporary solution. We should implement a proper read buffer and handle partial reads
            // instead of assuming that the entire target can be received in one go.
            // std::size_t received =
            // m_conn.recv(target.subspan(offset));

            // if (received == 0)
            //     return false;
            // offset += received;
        }
        return true;
    }

    void send_frame(const Frame<shared_layer::FrameRole::Sender> &frame) {
        std::vector<std::uint8_t> frame_bytes;
        auto it = std::back_inserter(frame_bytes);
        frame.encode(it);
        // TODO: This is a temporary solution. We should implement a proper write buffer and handle partial writes.
        // m_conn.send(std::as_bytes(std::span{frame_bytes}));
    }

    void mark_stream_closed(std::uint32_t id) {
        if (id == 0)
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"};

        if (auto it = m_streams.find(id); it != m_streams.end()) {
            it->second.reset();
        }

        throw error::http::ConnectionError{
            error::http::Http2ErrorCode::PROTOCOL_ERROR,
            std::format("Stream with ID {} was not found and therefor could not be closed / finished", id),
            m_remote_settings.trigger_goaway_after_stream_id()};
    }

    base::tls::basic::Connection m_conn;
    bool m_running;
    Settings m_local_settings;
    Settings m_remote_settings;
    std::map<std::uint32_t, std::unique_ptr<Stream<>>> m_streams;
    std::set<std::uint32_t> m_closed_streams;
    std::vector<std::uint8_t> m_header_buffer;
    codec::hpack::HPackTable m_decoding_table;
    codec::hpack::HPackTable m_encoding_table;
    Stream<false> m_connection_stream;
};

} // namespace transport::layer::http2
