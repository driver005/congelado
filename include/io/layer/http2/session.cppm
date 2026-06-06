module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing
// me
#include <ranges>

export module io_layer_http2:session;

import std;
import io_codec_hpack;
import utils_buffering;
import io_shared;
import core_logger;
import shared;
import utils_codec;
import interfaces;
import :request;
import :response;
import :settings;
import :stream;

export namespace io::layer::http2 {

using DispatchFn = interfaces::DispatchFn;

class Session {
  public:
    explicit Session(::shared::SendCallback send_callback, ::shared::CloseCallback close_callback,
                     DispatchFn dispatch = {})
        : m_running{true}, m_last_server_stream_id{0}, m_last_client_stream_id{1},
          m_local_settings{}, m_remote_settings{}, m_closed_streams{}, m_header_buffer{},
          m_decoding_table{}, m_encoding_table{},
          m_connection_stream{m_local_settings, m_remote_settings},
          m_submiter{std::move(send_callback)}, m_closer{std::move(close_callback)},
          m_safe_header{std::nullopt}, m_dispatch{std::move(dispatch)} {
        std::println("Session created {}", m_last_server_stream_id);
    }

    void send(HttpRequest &request) {
        auto &stream = next_client_stream();
        const auto SID = stream.get_stream_id();

        request.set_stream_id(SID);

        auto node =
            utils::buffering::BufferNode{request.get_size(m_local_settings.get_max_frame_size())};

        node | WriteHttpRequestAdaptor{request, m_encoding_table,
                                       m_local_settings.get_max_frame_size()};

        core::logger::debug("http2/session", "HEADERS frame size={}", node.get_written());

        // TODO: check if that is correct?
        stream.advance_send(shared_layer::FrameType::HEADERS, shared_layer::Flags::END_STREAM);

        send_node(std::move(node));
    }

    void receive(utils::buffering::BufferReader &reader) {
        if (!m_running) {
            return;
        }
        try {
            if (!m_safe_header.has_value()) {
                core::logger::debug("http2/session", "reading header");

                auto header_opt = receive_header(reader);
                if (!header_opt.has_value()) {
                    core::logger::debug("http2/session", "incomplete header, waiting");
                    return;
                }

                auto header = header_opt.value();

                core::logger::debug("http2/session", "header type={} len={} stream={}",
                                    header.get_type(), header.get_length(), header.get_stream_id());

                if (header.get_stream_id() > m_remote_settings.get_last_stream_id()) {
                    throw error::http::ConnectionError{
                        error::http::Http2ErrorCode::PROTOCOL_ERROR,
                        "Received frame for stream ID above GOAWAY threshold",
                        m_remote_settings.get_last_stream_id()};
                }

                if (m_remote_settings.is_acknowledged()) {
                    if (m_remote_settings.get_delta_window_on_settings() > 0) {
                        for (auto &[id, stream] : m_streams) {
                            core::logger::debug("http2/session", "stream {} send_window +{}", id,
                                                m_remote_settings.get_delta_window_on_settings());

                            stream->update_send_window(
                                m_remote_settings.get_delta_window_on_settings());
                        }

                        m_remote_settings.set_delta_window_on_settings(0);
                        core::logger::debug("http2/session",
                                            "remote settings ACK, windows updated");
                    }

                    m_remote_settings.set_state(SettingsState::IMPLEMENTED);
                }

                m_safe_header = header_opt;
            }

            core::logger::debug("http2/session", "reading frame payload size={}",
                                m_safe_header->get_length());

            if (reader.size() < m_safe_header->get_length()) {
                core::logger::debug("http2/session", "incomplete payload expected={} got={}",
                                    m_safe_header->get_length(), reader.size());

                return;
            }

            auto header = m_safe_header.value();
            m_safe_header.reset();

            auto stream_id = header.get_stream_id();

            core::logger::debug("http2/session", "frame type={} stream={} len={}",
                                header.get_type(), stream_id, header.get_length());

            if (stream_id == 0) {
                core::logger::debug("http2/session", "conn-level frame");

                if (auto frm = m_connection_stream.receive(header, reader); frm.has_value()) {
                    core::logger::debug("http2/session", "conn-level response, sending");

                    send_frame(frm.value());
                }
            } else {
                core::logger::debug("http2/session", "stream {} frame", stream_id);

                auto &stream = get_or_create_stream(stream_id);
                stream.receive(header, reader);

                core::logger::debug("http2/session", "stream {} handled, checking remote done",
                                    stream_id);

                if (stream.is_remote_done()) {
                    core::logger::debug("http2/session", "stream {} remote done, responding",
                                        stream_id);

                    response(stream.get_stream_id());
                }
            }
        } catch (const error::http::ConnectionError &e) {
            core::logger::warning("http2/session", "connection error: {}", e.what());

            close(e.get_code(), e.get_last_stream_id());
        } catch (const error::http::StreamError &e) {
            std::array<std::byte, 4> payload{};

            shared_layer::Atom<>::write_big_endian(payload, std::to_underlying(e.get_code()));

            auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                             .add_type(shared_layer::FrameType::RST_STREAM)
                             .add_flags(0)
                             .add_stream_id(e.get_stream_id())
                             .add_payload(payload)
                             .build();

            core::logger::warning("http2/session", "stream {} error: {}", e.get_stream_id(),
                                  e.what());

            send_frame(frame);
            mark_stream_closed(e.get_stream_id());
        }
    }

    void send_node(utils::buffering::BufferNode &&node) { m_submiter(std::move(node)); }

    void send_frame(const FrameBuilder<shared_layer::FrameRole::SENDER> &frame) {
        auto size = frame.get_size();
        auto node = std::views::empty<std::byte> |
                    WriteFrameBuilderAdaptor{frame, m_local_settings.get_max_frame_size()} |
                    std::ranges::to<utils::buffering::BufferNode>(size);

        core::logger::debug("http2/session", "sending frame size={}", node.get_written());

        m_submiter(std::move(node));
    }

    void close(error::http::Http2ErrorCode code, std::uint32_t stream_id = 0) {
        m_running = false;

        auto payload = std::views::empty<std::byte> |
                       utils::codec::WriteBigEndianAdaptor{stream_id} |
                       utils::codec::WriteBigEndianAdaptor{std::to_underlying(code)} |
                       std::ranges::to<std::vector<std::byte>>();

        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::GOAWAY)
                         .add_flags(0)
                         .add_stream_id(0)
                         .add_payload(payload)
                         .build();

        send_frame(frame);

        core::logger::info("http2/session", "GOAWAY sent: {} last_stream={}", code, stream_id);

        m_safe_header.reset();

        std::erase_if(m_streams,
                      [stream_id](const auto &entry) { return entry.first > stream_id; });

        m_closer();
    }

    [[nodiscard]] std::uint32_t get_last_client_stream_id() const noexcept {
        return m_last_client_stream_id;
    }

    [[nodiscard]] const Settings &get_local_settings() const noexcept { return m_local_settings; }
    Settings &get_local_settings() noexcept { return m_local_settings; }

  private:
    void response(std::uint32_t stream_id) {
        auto &stream = *m_streams.at(stream_id);
        auto &req = stream.get_request();
        auto &res = stream.get_response();

        res.set_status(interfaces::Status::NOT_FOUND);

        try {
            if (m_dispatch)
                m_dispatch(req, res);
        } catch (const std::exception &e) {
            core::logger::error("http2/session", "handler threw: {}", e.what());
            res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
        }

        auto node =
            utils::buffering::BufferNode{res.get_size(m_local_settings.get_max_frame_size())};
        node |
            WriteHttpResponseAdaptor{res, m_encoding_table, m_local_settings.get_max_frame_size()};
        send_node(std::move(node));
    }

    Stream<> &next_client_stream() {
        m_last_client_stream_id += 2;
        return get_or_create_stream(m_last_client_stream_id);
    }

    Stream<> &get_or_create_stream(const std::uint32_t &stream_id) {
        if (stream_id == 0) {
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Stream ID 0 is reserved"};
        }

        auto it = m_streams.find(stream_id);
        if (it != m_streams.end()) {
            if (it->second == nullptr) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   std::format("Stream ID {} is closed", stream_id),
                                                   m_remote_settings.get_last_stream_id()};
            }
            return *(it->second);
        }

        auto stream =
            std::make_unique<Stream<>>(stream_id, m_connection_stream, m_decoding_table,
                                       m_encoding_table, m_local_settings, m_remote_settings);

        auto [new_it, inserted] = m_streams.emplace(stream_id, std::move(stream));
        if (!inserted) {
            core::logger::debug("http2/session", "stream {} found", stream_id);
        } else {
            core::logger::debug("http2/session", "stream {} created", stream_id);
        }
        return *(new_it->second);
    }

    std::optional<FrameHeader<shared_layer::FrameRole::RECEIVER>>
    receive_header(utils::buffering::BufferReader &target) {
        if (target.size() < HEADER_SIZE) {
            core::logger::debug("http2/session", "incomplete header expected={} got={}",
                                HEADER_SIZE, target.size());

            return std::nullopt;
        }

        auto header = target | std::views::take(HEADER_SIZE) |
                      ReadFrameHeaderAdaptor{m_local_settings.get_max_frame_size()} |
                      utils::buffering::AdvanceReaderAdaptor{target, HEADER_SIZE};


        core::logger::debug("http2/session", "header parsed type={} len={} stream={}",
                            header.get_type(), header.get_length(), header.get_stream_id());

        return header;
    }


    void mark_stream_closed(std::uint32_t stream_id) {
        if (stream_id == 0) {
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Stream ID 0 is reserved"};
        }

        if (auto it = m_streams.find(stream_id); it != m_streams.end()) {
            it->second.reset();
            return;
        }

        throw error::http::ConnectionError{
            error::http::Http2ErrorCode::PROTOCOL_ERROR,
            std::format(
                "Stream with ID {} was not found and therefor could not be closed / finished",
                stream_id),
            m_remote_settings.get_last_stream_id()};
    }

    bool m_running;
    std::uint32_t m_last_server_stream_id;
    std::uint32_t m_last_client_stream_id;
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
    std::optional<FrameHeader<shared_layer::FrameRole::RECEIVER>> m_safe_header;
    DispatchFn m_dispatch;
};

} // namespace io::layer::http2
