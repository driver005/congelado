module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing me
#include <ranges>

export module io_layer_http2:session;

import std;
import io_codec_hpack;
import io_base_buffering;
import shared;
import :settings;
import :stream;

export namespace io::layer::http2 {

class Session {
  public:
    explicit Session(::shared::SendCallback send_callback, ::shared::CloseCallback close_callback)
        : m_running{true}, m_local_settings{Settings{}}, m_remote_settings{Settings{}}, m_closed_streams{},
          m_header_buffer{}, m_decoding_table{codec::hpack::HPackTable{}}, m_encoding_table{codec::hpack::HPackTable{}},
          m_connection_stream{Stream<false>{0, m_decoding_table, m_encoding_table, m_remote_settings, false}},
          m_submiter{std::move(send_callback)}, m_closer{std::move(close_callback)}, m_safe_header{std::nullopt} {}


    void receive(base::buffering::BufferView view) {
        try {
            if (!m_safe_header.has_value()) {
                auto header_opt = receive_header(view);
                if (!header_opt.has_value()) {
                    return;
                }

                auto header = header_opt.value();

                std::println("Sending on Stream ID {} ", header.get_stream_id());

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

                m_safe_header = std::move(header_opt);
            }

            auto frame_opt = receive_frame(view);
            if (!frame_opt.has_value()) {
                return;
            }

            auto frame = frame_opt.value();
            auto stream_id = frame.get_header().get_stream_id();

            if (stream_id == 0) {
                if (auto frm = m_connection_stream.handle_frame(frame); frm.has_value()) {
                    send_frame(std::move(frm.value()));
                }
            } else {
                auto &stream = get_or_create_stream(stream_id);
                stream.handle_frame(frame, false, m_connection_stream);

                if (stream.is_remote_done()) {
                    response(stream.get_stream_id());
                }
            }
        } catch (const error::http::ConnectionError &e) {
            close(e.get_code(), e.get_last_stream_id());
        } catch (const error::http::StreamError &e) {
            std::array<std::byte, 4> payload;

            shared_layer::Atom<>::write_big_endian(payload, std::to_underlying(e.get_code()));

            auto frame = Frame<shared_layer::FrameRole::Sender>{}
                             .add_header(FrameHeader{}
                                             .add_length(4)
                                             .add_type(shared_layer::FrameType::RST_STREAM)
                                             .add_flags(0)
                                             .add_stream_id(e.get_stream_id()))
                             .add_payload(payload)
                             .build();

            send_frame(std::move(frame));
            mark_stream_closed(e.get_stream_id());
        }
    }

    void send(base::buffering::BufferNode &&node) { m_submiter(std::move(node)); }

    void close(error::http::Http2ErrorCode code, std::uint32_t stream_id = 0) {
        m_running = false;

        std::array<std::byte, 8> payload;
        shared_layer::Atom<>::write_big_endian(payload | std::views::take(4), stream_id);
        shared_layer::Atom<>::write_big_endian(payload | std::views::drop(4), std::to_underlying(code));

        auto frame =
            Frame<shared_layer::FrameRole::Sender>{}
                .add_header(
                    FrameHeader{}.add_length(8).add_type(shared_layer::FrameType::GOAWAY).add_flags(0).add_stream_id(0))
                .add_payload(payload)
                .build();

        send_frame(std::move(frame));
        m_closer();
    }

    const Settings &get_local_settings() const noexcept { return m_local_settings; }
    Settings &get_local_settings() noexcept { return m_local_settings; }

  private:
    void response(std::uint32_t stream_id) {
        // std::vector<std::byte> hpack{};
        // hpack.reserve(32);
        //
        // // :status: 200  (indexed, §6.1)
        // hpack.push_back(0x88);
        //
        // // content-type: text/plain  (literal with incremental indexing, §6.2.1)
        // // static index 31 = content-type
        // hpack.push_back(0x40 | 31);                     // name  = idx 31
        // hpack.push_back(static_cast<std::uint8_t>(10)); // "text/plain" len
        // for (char c : std::string_view{"text/plain"})
        //     hpack.push_back(static_cast<std::uint8_t>(c));
        //
        // std::span<std::byte> hpack_span{hpack};
        // Frame<shared_layer::FrameRole::Sender> headers_frame{
        //     FrameHeader{static_cast<std::uint32_t>(hpack.size()), shared_layer::FrameType::HEADERS,
        //                 shared_layer::Flags::END_HEADERS | shared_layer::Flags::END_STREAM, stream_id},
        //     hpack_span};
        //
        // std::println("Prepared response HEADERS frame with payload size {}", hpack.size());
        //
        // std::vector<std::uint8_t> payload;
        //
        // auto it = std::back_inserter(payload);
        //
        // headers_frame.encode(it);
        // std::println("Encoded HEADERS frame with payload size {}", payload.size());
        //
        //
        std::println("Sending response on Stream ID {} ", stream_id);

        // m_conn.send(std::as_bytes(std::span{payload}));
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

    std::optional<FrameHeader> receive_header(base::buffering::BufferView &target) {
        if (target.size() < HEADER_SIZE)
            return std::nullopt;

        auto it = target.begin();
        return FrameHeader::from_bytes(it, m_local_settings.max_frame_size());
    }


    // only call if m_safe_headers is set
    std::optional<Frame<shared_layer::FrameRole::Receiver>> receive_frame(base::buffering::BufferView &target) {
        if (target.size() < m_safe_header->get_length())
            return std::nullopt;

        auto it = target.begin();
        auto header = m_safe_header.value();
        m_safe_header.reset();
        return Frame<shared_layer::FrameRole::Receiver>::decode_post_header(it, header);
    }

    // Last stop for a frame delete after!!!
    void send_frame(const Frame<shared_layer::FrameRole::Sender> frame) {
        base::buffering::BufferNode node{frame.get_size()};
        frame.encode(node);
        m_submiter(std::move(node));
    }

    void mark_stream_closed(std::uint32_t id) {
        if (id == 0)
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"};

        if (auto it = m_streams.find(id); it != m_streams.end()) {
            return it->second.reset();
        }

        throw error::http::ConnectionError{
            error::http::Http2ErrorCode::PROTOCOL_ERROR,
            std::format("Stream with ID {} was not found and therefor could not be closed / finished", id),
            m_remote_settings.trigger_goaway_after_stream_id()};
    }

    bool m_running;
    Settings m_local_settings;
    Settings m_remote_settings;
    std::map<std::uint32_t, std::unique_ptr<Stream<>>> m_streams;
    std::set<std::uint32_t> m_closed_streams;
    std::vector<std::uint8_t> m_header_buffer;
    codec::hpack::HPackTable m_decoding_table;
    codec::hpack::HPackTable m_encoding_table;
    Stream<false> m_connection_stream;
    ::shared::SendCallback m_submiter;
    ::shared::CloseCallback m_closer;
    std::optional<FrameHeader> m_safe_header;
};

} // namespace io::layer::http2
