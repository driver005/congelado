module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing me
#include <ranges>

export module io_layer_http2:session;

import std;
import io_codec_hpack;
import io_base_buffering;
import core_logger;
import shared;
import :settings;
import :stream;

export namespace io::layer::http2 {

class Session {
  public:
    explicit Session(::shared::SendCallback send_callback, ::shared::CloseCallback close_callback)
        : m_running{true}, m_local_settings{}, m_remote_settings{}, m_closed_streams{}, m_header_buffer{},
          m_decoding_table{}, m_encoding_table{}, m_connection_stream{m_local_settings, m_remote_settings},
          m_submiter{std::move(send_callback)}, m_closer{std::move(close_callback)}, m_safe_header{std::nullopt} {
        core::logger::debug("Session - HTTP/2",
                            "Created with send and close callbacks, initialized settings and tables");
    }


    void receive(base::buffering::BufferReader &reader) {
        try {
            if (!m_safe_header.has_value()) {
                core::logger::debug("Session - HTTP/2",
                                    "No safe header available, attempting to receive header from incoming data");

                auto header_opt = receive_header(reader);
                if (!header_opt.has_value()) {
                    core::logger::debug("Session - HTTP/2",
                                        "Not enough data to receive a complete header, awaiting more data");
                    return;
                }

                auto header = header_opt.value();

                core::logger::info("Session - HTTP/2",
                                   "Received complete header: type `{}`, length `{}`, stream_id `{}`",
                                   header.get_type(), header.get_length(), header.get_stream_id());

                if (header.get_stream_id() > m_remote_settings.trigger_goaway_after_stream_id()) {
                    throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                       "Received frame for stream ID above GOAWAY threshold",
                                                       m_remote_settings.trigger_goaway_after_stream_id()};
                }

                if (m_remote_settings.is_acknowledged()) {
                    if (m_remote_settings.delta_window_on_settings() > 0) {
                        for (auto &[id, stream] : m_streams) {
                            core::logger::debug(
                                "Session - HTTP/2",
                                "Updating send window for stream ID `{}` by delta `{}` due to acknowledged settings",
                                id, m_remote_settings.delta_window_on_settings());

                            stream->update_send_window(m_remote_settings.delta_window_on_settings());
                        }

                        m_remote_settings.set_delta_window_on_settings(0);
                        core::logger::info(
                            "Session - HTTP/2",
                            "Remote settings acknowledged, send windows updated for all streams with delta");
                    }

                    m_remote_settings.set_state(SettingsState::IMPLEMENTED);
                }

                m_safe_header = header_opt;
            }

            core::logger::info("Session - HTTP/2",
                               "Safe header available, attempting to receive frame with payload size `{}`",
                               m_safe_header->get_length());

            if (reader.size() < m_safe_header->get_length()) {
                core::logger::debug("Session - HTTP/2",
                                    "Not enough data to receive complete frame payload, expected size `{}`, got `{}`",
                                    m_safe_header->get_length(), reader.size());

                return;
            }

            auto header = m_safe_header.value();
            m_safe_header.reset();

            auto stream_id = header.get_stream_id();

            core::logger::info("Session - HTTP/2",
                               "Received complete frame of type `{}` on stream ID `{}` with payload size `{}`",
                               header.get_type(), stream_id, header.get_length());

            if (stream_id == 0) {
                core::logger::debug("Session - HTTP/2",
                                    "Frame received on stream ID 0, handling as connection-level frame");

                if (auto frm = m_connection_stream.receive(header, reader); frm.has_value()) {
                    core::logger::info("Session - HTTP/2",
                                       "Connection-level frame handling produced a response frame, sending it");

                    send_frame(frm.value());
                }
            } else {
                core::logger::debug("Session - HTTP/2",
                                    "Frame received on stream ID `{}`, handling with corresponding stream", stream_id);

                auto &stream = get_or_create_stream(stream_id);
                stream.receive(header, reader);

                core::logger::info(
                    "Session - HTTP/2",
                    "Handled frame on stream ID `{}`, checking if stream remote is done to send response", stream_id);

                if (stream.is_remote_done()) {
                    core::logger::info("Session - HTTP/2",
                                       "Stream ID `{}` remote is done, sending response and marking stream as closed",
                                       stream_id);

                    response(stream.get_stream_id());
                }
            }
        } catch (const error::http::ConnectionError &e) {
            core::logger::warning("Session - HTTP/2", "Connection error occurred: `{}`, closing connection with GOAWAY",
                                  e.what());

            close(e.get_code(), e.get_last_stream_id());
        } catch (const error::http::StreamError &e) {
            std::array<std::byte, 4> payload{};

            shared_layer::Atom<>::write_big_endian(payload, std::to_underlying(e.get_code()));

            auto frame = Frame<shared_layer::FrameRole::Sender>{}
                             .add_header(FrameHeader<shared_layer::FrameRole::Sender>{}
                                             .add_length(4)
                                             .add_type(shared_layer::FrameType::RST_STREAM)
                                             .add_flags(0)
                                             .add_stream_id(e.get_stream_id()))
                             .add_payload(payload)
                             .build();

            core::logger::warning(
                "Session - HTTP/2",
                "Stream error occurred on stream ID `{}`: `{}`, sending RST_STREAM and marking stream as closed",
                e.get_stream_id(), e.what());

            send_frame(frame);
            mark_stream_closed(e.get_stream_id());
        }
    }

    void send(base::buffering::BufferNode &&node) { m_submiter(std::move(node)); }

    void close(error::http::Http2ErrorCode code, std::uint32_t stream_id = 0) {
        m_running = false;

        auto payload = std::views::empty<std::byte> | utils::codec::WriteBigEndianAdaptor{stream_id} |
                       utils::codec::WriteBigEndianAdaptor{std::to_underlying(code)} |
                       std::ranges::to<std::vector<std::byte>>();

        auto frame = Frame<shared_layer::FrameRole::Sender>{}
                         .add_header(FrameHeader<shared_layer::FrameRole::Sender>{}
                                         .add_length(8)
                                         .add_type(shared_layer::FrameType::GOAWAY)
                                         .add_flags(0)
                                         .add_stream_id(0))
                         .add_payload(payload)
                         .build();

        send_frame(frame);

        core::logger::info("Session - HTTP/2", "Sent GOAWAY frame with code `{}` and last stream ID `{}`", code,
                           stream_id);

        m_closer();
    }

    [[nodiscard]] const Settings &get_local_settings() const noexcept { return m_local_settings; }
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


    Stream<> &get_or_create_stream(const std::uint32_t &stream_id) {
        if (stream_id == 0) {
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"};
        }

        auto it = m_streams.find(stream_id);
        if (it != m_streams.end()) {
            if (it->second == nullptr) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   std::format("Stream ID {} is closed", stream_id),
                                                   m_remote_settings.trigger_goaway_after_stream_id()};
            }
            return *(it->second);
        }

        auto stream = std::make_unique<Stream<>>(stream_id, m_connection_stream, m_decoding_table, m_encoding_table,
                                                 m_local_settings, m_remote_settings);

        auto [new_it, inserted] = m_streams.emplace(stream_id, std::move(stream));
        if (!inserted) {
            core::logger::info("Session - HTTP/2", "Stream with ID `{}` found", stream_id);
        } else {
            core::logger::info("Session - HTTP/2", "Stream with ID `{}` not found, created new stream", stream_id);
        }
        return *(new_it->second);
    }

    std::optional<FrameHeader<shared_layer::FrameRole::Receiver>>
    receive_header(base::buffering::BufferReader &target) {
        if (target.size() < HEADER_SIZE) {
            core::logger::debug("Session - HTTP/2",
                                "Not enough data to receive complete frame header, expected size `{}`, got `{}`",
                                HEADER_SIZE, target.size());

            return std::nullopt;
        }

        auto header = target | std::views::take(HEADER_SIZE) |
                      ReadFrameHeaderAdaptor{m_local_settings.max_frame_size()} |
                      base::buffering::AdvanceReaderAdaptor{target, HEADER_SIZE};


        core::logger::debug("Session - HTTP/2",
                            "Received complete frame header with type `{}`, length `{}`, and stream ID `{}`",
                            header.get_type(), header.get_length(), header.get_stream_id());

        return header;
    }

    // Last stop for a frame delete after!!!
    void send_frame(const Frame<shared_layer::FrameRole::Sender> &frame) {
        auto node = std::views::empty<std::byte> | WriteFrameBuilderAdaptor{frame} |

                    std::ranges::to<base::buffering::BufferNode>();
        core::logger::debug("Session - HTTP/2", "Prepared frame for sending with total size `{}`", node.get_written());

        m_submiter(std::move(node));
    }

    void mark_stream_closed(std::uint32_t stream_id) {
        if (stream_id == 0) {
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"};
        }

        if (auto it = m_streams.find(stream_id); it != m_streams.end()) {
            it->second.reset();
            return;
        }

        throw error::http::ConnectionError{
            error::http::Http2ErrorCode::PROTOCOL_ERROR,
            std::format("Stream with ID {} was not found and therefor could not be closed / finished", stream_id),
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
    std::optional<FrameHeader<shared_layer::FrameRole::Receiver>> m_safe_header;
};

} // namespace io::layer::http2
