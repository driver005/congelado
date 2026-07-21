module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing
// me
#include <ranges>
#include <variant>

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

class Session {
  public:
    /**
     * @brief Spins up a fresh HTTP/2 session — local/remote `Settings` both start at spec
     * defaults, `m_last_client_stream_id` starts at 1 so the first client-initiated stream
     * lands on the correct odd id 3 (next_client_stream() bumps by 2 before use), and the
     * connection-level `Stream<false>` gets wired to the same settings pair everything else
     * shares.
     * @param send_callback callback the session uses to push bytes out to the transport.
     * @param close_callback callback the session calls to tear the connection down.
     * @param dispatch request/response dispatch hook, fired once a stream's remote side is
     * done sending.
     */
    explicit Session(::shared::SendCallback send_callback, ::shared::CloseCallback close_callback,
                     interfaces::io::ReceiveDispatchFn dispatch = {})
        : m_connection_stream{m_local_settings, m_remote_settings},
          m_submiter{std::move(send_callback)}, m_closer{std::move(close_callback)},
          m_safe_header{std::nullopt}, m_dispatch{std::move(dispatch)} {
        std::println("Session created {}", m_last_server_stream_id);
    }

    /**
     * @brief Assigns `request` a fresh client stream id, frames it into HEADERS(+CONTINUATION)
     * bytes, and ships it out.
     * @note The `advance_send()` call right before `send_node()` fires with `Flags::END_STREAM`
     * unconditionally, marked `TODO: check if that is correct?` in the source — meaning every
     * request sent through here gets treated as ending the stream on the send side regardless
     * of whether the request actually has a body still to come. Worth watching if this ever
     * grows streaming-request support.
     * @param request the request to send. Gets its stream id overwritten by this call.
     */
    void send(HttpRequest &request) {
        // Grab a fresh odd client stream id and tag the request with it.
        auto &stream = next_client_stream();
        const auto SID = stream.get_stream_id();

        request.set_stream_id(SID);

        // Pre-size the buffer node up front from the estimated wire size, then encode straight
        // into it.
        auto node =
            utils::buffering::BufferNode{request.get_size(m_local_settings.get_max_frame_size())};

        node | WriteHttpRequestAdaptor{request, m_encoding_table,
                                       m_local_settings.get_max_frame_size()};

        core::logger::debug("http2/session", "HEADERS frame size={}", node.get_written());

        // Drive the stream state machine forward for the send side before actually shipping bytes.
        // TODO: check if that is correct?
        stream.advance_send(shared_layer::FrameType::HEADERS, shared_layer::Flags::END_STREAM);

        send_node(std::move(node));
    }

    /**
     * @brief The core read-side state machine — parses one frame header at a time (buffered
     * across calls via `m_safe_header` when a partial header or payload arrives), applies
     * pending remote-settings window deltas once ACKed, then routes the frame to either the
     * connection-level stream (id 0) or a per-stream `Stream<>`, kicking off `response()` if
     * the peer's done sending on that stream.
     * @warning This is a genuine multi-call state machine, not a one-shot parse. `m_safe_header`
     * holds a parsed-but-not-yet-fully-payload-available header across calls — if you're
     * tracing through this expecting one `receive()` call to always fully consume one frame,
     * that's wrong for split reads, which is the entire point of the buffering in the first
     * place. Two early-return points (incomplete header, incomplete payload) are `return`s
     * with zero side effects other than logging, that's expected steady-state, not an error
     * path.
     * @warning `ConnectionError` and `StreamError` get handled very differently here:
     * `ConnectionError` calls `close()`, which tears down the *entire* session (GOAWAY, kills
     * every stream past the given id, invokes `m_closer`). `StreamError` instead sends a
     * targeted RST_STREAM and calls `mark_stream_closed()` for just that one stream — the rest
     * of the connection keeps running. Mixing up which exception type a given violation should
     * throw upstream (in `FrameHeader::validate_*`, `StreamStateMachine::advance`, etc.) has
     * connection-wide blast-radius consequences, not just local ones.
     * @param reader the bytes received off the wire so far — gets consumed incrementally as
     * frames are fully parsed off the front.
     */
    void receive(utils::buffering::BufferReader &reader) {
        // Session already tore itself down — every further receive() call is a hard no-op.
        if (!m_running) {
            return;
        }
        try {
            // No header buffered from a previous partial call — try to parse a fresh one.
            if (!m_safe_header.has_value()) {
                core::logger::debug("http2/session", "reading header");

                auto header_opt = receive_header(reader);
                // Not enough bytes yet for a full 9-byte header — bail quietly, next call
                // with more bytes will pick back up.
                if (!header_opt.has_value()) {
                    core::logger::debug("http2/session", "incomplete header, waiting");
                    return;
                }

                auto header = header_opt.value();

                core::logger::debug("http2/session", "header type={} len={} stream={}",
                                    header.get_type(), header.get_length(), header.get_stream_id());

                // Reject anything past the GOAWAY threshold we've already advertised.
                if (header.get_stream_id() > m_remote_settings.get_last_stream_id()) {
                    throw error::http::ConnectionError{
                        error::http::Http2ErrorCode::PROTOCOL_ERROR,
                        "Received frame for stream ID above GOAWAY threshold",
                        m_remote_settings.get_last_stream_id()};
                }

                // Remote settings just got ACKed — apply any pending initial-window delta to
                // every live stream's send window, then flip settings to IMPLEMENTED.
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

                // Stash the parsed header across calls until its payload fully arrives too.
                m_safe_header = header_opt;
            }

            core::logger::debug("http2/session", "reading frame payload size={}",
                                m_safe_header->get_length());

            // Payload not fully buffered yet either — wait for more bytes, header stays parked
            // in m_safe_header for the next call.
            if (reader.size() < m_safe_header->get_length()) {
                core::logger::debug("http2/session", "incomplete payload expected={} got={}",
                                    m_safe_header->get_length(), reader.size());

                return;
            }

            // Full frame (header + payload) is available now — pull the header back out and
            // clear the stash so the next receive() starts fresh.
            auto header = m_safe_header.value();
            m_safe_header.reset();

            auto stream_id = header.get_stream_id();

            core::logger::debug("http2/session", "frame type={} stream={} len={}",
                                header.get_type(), stream_id, header.get_length());

            // Stream 0 is connection-level (SETTINGS, PING, GOAWAY, ...); anything else routes
            // to its own per-stream Stream<>, kicking off the response once the peer's done
            // sending on that stream.
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
            // Connection-wide violation — tear the whole session down via close()/GOAWAY.
            core::logger::warning("http2/session", "connection error: {}", e.what());

            close(e.get_code(), e.get_last_stream_id());
        } catch (const error::http::StreamError &e) {
            // Scoped to one stream — send a targeted RST_STREAM and close just that stream,
            // the rest of the connection keeps running.
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

    /**
     * @brief Hands an already-encoded buffer node off to the transport. Thin wrapper, bet —
     * every other send-path in this class eventually funnels down through here (or duplicates
     * the same `m_submiter` call directly).
     * @param node the pre-encoded bytes to submit.
     */
    void send_node(utils::buffering::BufferNode &&node) { m_submiter(std::move(node)); }

    /**
     * @brief Encodes a `FrameBuilder` (header + payload, single frame — no chunking, unlike the
     * `WriteFrameClosureAdapter` path used for HEADERS/DATA) and ships it out.
     * @param frame the built frame to encode and send.
     */
    void send_frame(const FrameBuilder<shared_layer::FrameRole::SENDER> &frame) {
        // Encode header + payload into a single node, no chunking, then hand it to the transport.
        auto size = frame.get_size();
        auto node = std::views::empty<std::byte> |
                    WriteFrameBuilderAdaptor{frame, m_local_settings.get_max_frame_size()} |
                    std::ranges::to<utils::buffering::BufferNode>(size);

        core::logger::debug("http2/session", "sending frame size={}", node.get_written());

        m_submiter(std::move(node));
    }

    /**
     * @brief Tears the whole connection down — flips `m_running` false (so any further
     * `receive()` calls become instant no-ops), sends GOAWAY with `code` and `stream_id`,
     * drops any in-flight partial header, prunes every stream with an id greater than
     * `stream_id`, then invokes the close callback.
     * @warning Streams with id <= `stream_id` are deliberately kept around in `m_streams` after
     * this — GOAWAY semantics say those may still get a response, per RFC 9113 §6.8. Don't
     * mistake "session closed" for "every stream map entry gone", that's not what this does.
     * @param code the GOAWAY error code to report to the peer.
     * @param stream_id the last stream id the peer should consider as possibly still
     * processed — defaults to 0, meaning "nothing more, full stop."
     */
    void close(error::http::Http2ErrorCode code, std::uint32_t stream_id = 0) {
        // Flip running off first — any receive() call that lands mid-teardown becomes a no-op.
        m_running = false;

        // Build and send the GOAWAY frame itself.
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

        // Drop any in-flight partial header — nothing's going to finish parsing it now.
        m_safe_header.reset();

        // Prune every stream past the announced last-stream-id; streams at or below it are
        // deliberately kept, per RFC 9113 §6.8 they may still get a response.
        std::erase_if(m_streams,
                      [stream_id](const auto &entry) { return entry.first > stream_id; });

        m_closer();
    }

    /**
     * @brief Grabs the most recently assigned client-initiated stream id. Bet, plain getter.
     * @return the last client stream id handed out by `next_client_stream()`.
     */
    [[nodiscard]] std::uint32_t get_last_client_stream_id() const noexcept {
        return m_last_client_stream_id;
    }

    /**
     * @brief Grabs the local settings, read-only. Lowkey just a getter.
     * @return the local `Settings`.
     */
    [[nodiscard]] const Settings &get_local_settings() const noexcept { return m_local_settings; }
    /**
     * @brief Grabs the local settings, mutable — used by `Handshake` to read the max frame size
     * while wiring up the initial SETTINGS exchange.
     * @return the local `Settings`.
     */
    Settings &get_local_settings() noexcept { return m_local_settings; }

  private:
    /**
     * @brief Runs the registered dispatch handler for a stream whose remote side is done
     * sending, defaults the response to NOT_FOUND up front (so an unhandled route/exception
     * still gets a real HTTP status instead of nothing), then encodes and sends whatever the
     * handler produced.
     * @warning Any exception the dispatch handler throws gets caught and downgraded to a plain
     * INTERNAL_SERVER_ERROR status — the actual exception details only make it as far as the
     * logger, never back to the peer. If you're debugging a handler that's misbehaving, check
     * the logs, not the wire response, that's where the real error went.
     * @param stream_id the stream to respond on — must already exist in `m_streams`, this uses
     * `.at()` which throws `std::out_of_range` on a bad id (uncaught here).
     */
    void response(std::uint32_t stream_id) {
        auto &stream = *m_streams.at(stream_id);
        auto &req = stream.get_request();
        auto &res = stream.get_response();

        // Default to NOT_FOUND up front so an unhandled route still ships a real status.
        res.set_status(interfaces::io::types::Status::NOT_FOUND);

        // Run the registered handler — any exception it throws gets downgraded to a plain 500,
        // details only make it to the logger, never back to the peer.
        try {
            m_dispatch(req, res);
        } catch (const std::exception &e) {
            core::logger::error("http2/session", "handler threw: {}", e.what());
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        }

        // Encode whatever the handler (or the NOT_FOUND/500 fallback) produced and ship it.
        auto node =
            utils::buffering::BufferNode{res.get_size(m_local_settings.get_max_frame_size())};
        node |
            WriteHttpResponseAdaptor{res, m_encoding_table, m_local_settings.get_max_frame_size()};
        send_node(std::move(node));
    }

    /**
     * @brief Advances to the next client-initiated stream id (odd, +2 from the last one used —
     * starting at 1, so the very first stream lands on 3, not 1... actually the very first call
     * lands on `1 + 2 = 3`) and creates the stream for it.
     * @note If a true stream id 1 is ever needed (e.g. for HTTP/1.1 Upgrade-based h2c, which
     * reserves stream 1), this class's starting value of `m_last_client_stream_id = 1` means
     * it's never handed out through this path — first assignable id is 3.
     * @return the freshly created stream for the next client id.
     */
    Stream<> &next_client_stream() {
        // Odd client stream ids only, always +2 from the last one handed out.
        m_last_client_stream_id += 2;
        return get_or_create_stream(m_last_client_stream_id);
    }

    /**
     * @brief Looks up an existing stream by id, or creates a fresh one wired to the shared
     * connection-level stream, HPACK tables, and settings if it doesn't exist yet.
     * @warning A stream id that's present in `m_streams` but mapped to a `nullptr` entry means
     * "this stream existed and was explicitly closed" (see `mark_stream_closed()`) — that's
     * treated as a hard `ConnectionError`, not silently re-created. Don't confuse "never seen
     * this id" with "saw it, it's done" — they're different map states and this function reacts
     * to them very differently.
     * @param stream_id the stream id to look up or create. Must be nonzero.
     * @return a reference to the existing or newly-created stream.
     * @throws error::http::ConnectionError if `stream_id` is 0, or if it maps to a
     * previously-closed (nulled) entry.
     */
    Stream<> &get_or_create_stream(const std::uint32_t &stream_id) {
        // Guard — stream 0 is reserved for connection-level frames, never a real per-request stream.
        if (stream_id == 0) {
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Stream ID 0 is reserved"};
        }

        // Already tracked — either return the live stream or reject a tombstoned (closed) one.
        auto it = m_streams.find(stream_id);
        if (it != m_streams.end()) {
            if (it->second == nullptr) {
                throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   std::format("Stream ID {} is closed", stream_id),
                                                   m_remote_settings.get_last_stream_id()};
            }
            return *(it->second);
        }

        // First time seeing this id — spin up a fresh stream wired to the shared connection
        // stream, HPACK tables, and settings.
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

    /**
     * @brief Tries to parse one frame header (9 bytes) off the front of `target`, no cap,
     * consuming those bytes only if there's enough available.
     * @param target the bytes received so far.
     * @return the parsed header if at least `HEADER_SIZE` bytes were available (and consumed),
     * `std::nullopt` if there's not enough yet — caller's expected to wait for more bytes and
     * retry.
     */
    std::optional<FrameHeader<shared_layer::FrameRole::RECEIVER>>
    receive_header(utils::buffering::BufferReader &target) {
        // Not enough bytes buffered yet for a full fixed-size header — caller retries later.
        if (target.size() < HEADER_SIZE) {
            core::logger::debug("http2/session", "incomplete header expected={} got={}",
                                HEADER_SIZE, target.size());

            return std::nullopt;
        }

        // Parse the header then advance the reader past those 9 bytes in the same pipeline.
        auto header = target | std::views::take(HEADER_SIZE) |
                      ReadFrameHeaderAdaptor{m_local_settings.get_max_frame_size()} |
                      utils::buffering::AdvanceReaderAdaptor{target, HEADER_SIZE};


        core::logger::debug("http2/session", "header parsed type={} len={} stream={}",
                            header.get_type(), header.get_length(), header.get_stream_id());

        return header;
    }


    /**
     * @brief Marks a stream as closed without removing its map entry — resets the
     * `unique_ptr` to null rather than erasing the key, so a later lookup for this id in
     * `get_or_create_stream()` finds the tombstone and rejects it instead of silently
     * re-opening a stream that's already been through its full lifecycle.
     * @param stream_id the stream to close. Must be nonzero and must already exist in
     * `m_streams`.
     * @throws error::http::ConnectionError if `stream_id` is 0, or if it isn't found in
     * `m_streams` at all.
     */
    void mark_stream_closed(std::uint32_t stream_id) {
        // Guard — same reserved-id rule as get_or_create_stream().
        if (stream_id == 0) {
            throw error::http::ConnectionError{error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Stream ID 0 is reserved"};
        }

        // Reset the unique_ptr to null rather than erasing the map entry — leaves a tombstone
        // so a later lookup rejects the id instead of silently re-opening a finished stream.
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

    bool m_running = true;
    std::uint32_t m_last_server_stream_id = 0;
    std::uint32_t m_last_client_stream_id = 1;
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
    interfaces::io::ReceiveDispatchFn m_dispatch;
};

} // namespace io::layer::http2
