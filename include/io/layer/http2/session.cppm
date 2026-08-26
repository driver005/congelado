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
import core_events;
import core_logger;
import shared;
import utils_codec;
import interfaces;
import :extension;
import :request;
import :response;
import :settings;
import :stream;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::layer::http2 {

/// @brief Which end of the connection a `Session` represents — decides inbound decode/routing.
enum class Role : bool
{
    SERVER = false,
    CLIENT = true
};

class Session
{
public:
    /**
     * @brief Spins up a fresh HTTP/2 session — local/remote `Settings` both start at spec
     * defaults, `m_last_client_stream_id` starts at 1 so the first client-initiated stream
     * lands on the correct odd id 3 (next_client_stream() bumps by 2 before use), and the
     * connection-level `Stream<false>` gets wired to the same settings pair everything else
     * shares.
     * @param send_callback callback the session uses to push bytes out to the transport.
     * @param close_callback callback the session calls to tear the connection down.
     * @param extension_registry the process's one `HttpExtensionRegistry` — stored by reference
     * (must outlive this session) and handed down to every `Stream`/`Handshake` call that needs
     * to check for a registered `IHttpExtension`. An empty registry (no extension plugin
     * configured) makes every one of those calls a no-op, same as before this mechanism
     * existed.
     * @param role `Role::CLIENT` for a client session (it initiates streams and receives
     * responses), `Role::SERVER` for a server session (it receives requests) — drives inbound
     * decode target and received-stream routing.
     * @param dispatch request/response dispatch hook, fired once a stream's remote side is
     * done sending.
     */
    explicit Session(
        ::shared::SendCallback send_callback,
        ::shared::CloseCallback close_callback,
        HttpExtensionRegistry& extension_registry,
        Role role,
        interfaces::io::ReceiveDispatchFn dispatch = {}
    ) :
        m_connection_stream{m_local_settings, m_remote_settings},
        m_submiter{std::move(send_callback)},
        m_closer{std::move(close_callback)},
        m_safe_header{std::nullopt},
        m_dispatch{std::move(dispatch)},
        m_extension_registry{extension_registry},
        m_role{role}
    {
        // Client hands out odd stream ids from 1; server starts at 0 and tracks the highest
        // received client id (see get_or_create_stream).
        m_last_client_stream_id = role == Role::CLIENT ? 1 : 0;
        core::logger::info("http2/session", "session created");

        // A new connection just opened — notify every extension.
        m_extension_registry.get().for_each([](auto& extension) {
            extension->on_connection_open();
        });
    }

    /**
     * @brief Grabs the extension registry this session was constructed with — `ServerFlow`/
     * `ClientFlow` read this back to hand the same registry to `Handshake::process()`, since
     * `Handshake` doesn't own or construct one itself.
     * @return the extension registry.
     */
    [[nodiscard]] HttpExtensionRegistry& get_extension_registry() noexcept
    {
        return m_extension_registry;
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
     * @return the stream id assigned to this request — the key the response is correlated on.
     */
    std::uint32_t send(HttpRequest& request)
    {
        // Grab a fresh odd client stream id and tag the request with it.
        auto& stream = next_client_stream();
        const auto SID = stream.get_stream_id();

        request.set_stream_id(SID);

        // Let every extension observe (and optionally mutate) the request before it's framed.
        m_extension_registry.get().for_each([&](auto& extension) {
            extension->on_request_outgoing(SID, request);
        });

        // Pre-size the buffer node up front from the estimated wire size, then encode straight
        // into it.
        auto node =
            utils::buffering::BufferNode{request.get_size(m_local_settings.get_max_frame_size())};

        node | WriteHttpRequestAdaptor{
                   request, m_encoding_table, m_local_settings.get_max_frame_size()
               };

        core::logger::debug("http2/session", "HEADERS frame size={}", node.get_written());

        // WIREDUMP (temporary): dump the exact bytes the writer produced for this request.
        {
            std::string hex;
            const std::size_t LIMIT = std::min<std::size_t>(node.get_written(), 48);
            for (std::size_t i = 0; i < LIMIT; ++i) {
                hex += std::format(
                    "{:02x} ",
                    static_cast<unsigned>(std::to_integer<std::uint8_t>(node.get_data()[i]))
                );
            }
            core::logger::warning(
                "WIREDUMP", "worker-outbound written={} bytes[{}]: {}", node.get_written(), LIMIT,
                hex
            );
        }

        // Drive the stream state machine forward for the send side before actually shipping
        // bytes. is_local=true — this is our own outgoing request, so END_STREAM half-closes
        // the LOCAL side (leaving the remote open to send the response). Omitting it defaulted
        // to false, which wrongly marked the stream half-closed-REMOTE and made the inbound
        // response get rejected as "DATA/HEADERS on half-closed (remote) stream". Mirrors the
        // server response path's explicit `true`.
        stream.advance_send(
            shared_layer::FrameType::HEADERS, shared_layer::Flags::END_STREAM, true
        );

        send_node(std::move(node));
        return SID;
    }

    /**
     * @brief The core read-side state machine — parses one frame header at a time (buffered
     * across calls via `m_safe_header` when a partial header or payload arrives), applies
     * pending remote-settings window deltas once ACKed, then routes the frame to either the
     * connection-level stream (id 0) or a per-stream `Stream<>`, kicking off `response()` if
     * the peer's done sending on that stream.
     * @warning This is a genuine multi-call state machine, not a one-shot parse.
     * `m_safe_header` holds a parsed-but-not-yet-fully-payload-available header across calls —
     * if you're tracing through this expecting one `receive()` call to always fully consume one
     * frame, that's wrong for split reads, which is the entire point of the buffering in the
     * first place. Two early-return points (incomplete header, incomplete payload) are
     * `return`s with zero side effects other than logging, that's expected steady-state, not an
     * error path.
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
    void receive(utils::buffering::BufferReader& reader)
    {
        try {
            // No header buffered from a previous partial call — try to parse a fresh one.
            if (!m_safe_header.has_value()) {
                auto header_opt = receive_header(reader);
                // Not enough bytes yet for a full 9-byte header — bail quietly, next call
                // with more bytes will pick back up.
                if (!header_opt.has_value()) {
                    return;
                }

                auto header = header_opt.value();

                // Reject anything past the GOAWAY threshold we've already advertised.
                if (header.get_stream_id() > m_remote_settings.get_last_stream_id()) {
                    throw error::http::ConnectionError{
                        error::http::Http2ErrorCode::PROTOCOL_ERROR,
                        "Received frame for stream ID above GOAWAY threshold",
                        m_remote_settings.get_last_stream_id()
                    };
                }

                // Remote settings just got ACKed — apply any pending initial-window delta to
                // every live stream's send window, then flip settings to IMPLEMENTED.
                if (m_remote_settings.is_acknowledged()) {
                    if (m_remote_settings.get_delta_window_on_settings() > 0) {
                        for (auto& [id, stream]: m_streams) {
                            core::logger::debug(
                                "http2/session", "stream {} send_window +{}", id,
                                m_remote_settings.get_delta_window_on_settings()
                            );

                            stream->update_send_window(
                                m_remote_settings.get_delta_window_on_settings()
                            );
                        }

                        m_remote_settings.set_delta_window_on_settings(0);
                        core::logger::debug(
                            "http2/session", "remote settings ACK, windows updated"
                        );
                    }

                    m_remote_settings.set_state(SettingsState::IMPLEMENTED);
                }

                // Stash the parsed header across calls until its payload fully arrives too.
                m_safe_header = header_opt;
            }

            // Payload not fully buffered yet either — wait for more bytes, header stays parked
            // in m_safe_header for the next call.
            if (reader.size() < m_safe_header->get_length()) {
                return;
            }

            // Full frame (header + payload) is available now — pull the header back out and
            // clear the stash so the next receive() starts fresh.
            auto header = m_safe_header.value();
            m_safe_header.reset();

            auto stream_id = header.get_stream_id();

            core::logger::debug(
                "http2/session", "frame type={} stream={} len={}", header.get_type(), stream_id,
                header.get_length()
            );

            // Stream 0 is connection-level (SETTINGS, PING, GOAWAY, ...); anything else routes
            // to its own per-stream Stream<>, kicking off the response once the peer's done
            // sending on that stream.
            if (stream_id == 0) {
                if (auto frm =
                        m_connection_stream.receive(header, reader, m_extension_registry.get());
                    frm.has_value()) {
                    send_frame(frm.value());
                }
                // A received GOAWAY is recorded by the connection stream (no echo, RFC 9113
                // §6.8); the connection-level teardown is the session's job — run it once.
                if (m_connection_stream.got_goaway() && !m_goaway_received) {
                    handle_goaway();
                }
            } else {
                auto& stream = get_or_create_stream(stream_id);
                stream.receive(header, reader, m_extension_registry.get());

                if (stream.is_remote_done()) {
                    response(stream.get_stream_id());
                }
            }
        } catch (const error::http::ConnectionError& e) {
            // Connection-wide violation — tear the whole session down via close()/GOAWAY.
            core::logger::warning("http2/session", "connection error: {}", e.what());
            core::events::publish("http2.session.connection_error", {{"error", e.what()}});

            close(e.get_code(), e.get_last_stream_id());
        } catch (const error::http::StreamError& e) {
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

            core::logger::warning(
                "http2/session", "stream {} error: {}", e.get_stream_id(), e.what()
            );
            core::events::publish(
                "http2.session.stream_error",
                {{"stream_id", std::to_string(e.get_stream_id())}, {"error", e.what()}}
            );

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
    void send_node(utils::buffering::BufferNode&& node)
    {
        m_submiter(std::move(node));
    }

    /**
     * @brief Encodes a `FrameBuilder` (header + payload, single frame — no chunking, unlike the
     * `WriteFrameClosureAdapter` path used for HEADERS/DATA) and ships it out.
     * @param frame the built frame to encode and send.
     */
    void send_frame(const FrameBuilder<shared_layer::FrameRole::SENDER>& frame)
    {
        // Encode header + payload into a single node, no chunking, then hand it to the
        // transport.
        auto size = frame.get_size();
        auto node = std::views::empty<std::byte> |
                    WriteFrameBuilderAdaptor{frame, m_local_settings.get_max_frame_size()} |
                    std::ranges::to<utils::buffering::BufferNode>(size);

        core::logger::debug("http2/session", "sending frame size={}", node.get_written());

        m_submiter(std::move(node));
    }

    /**
     * @brief Tears the whole connection down — sends GOAWAY with `code` and `stream_id`, drops
     * any in-flight partial header, prunes every stream with an id greater than `stream_id`,
     * then (unless `graceful`) invokes the close callback.
     * @warning Streams with id <= `stream_id` are deliberately kept around in `m_streams` after
     * this — GOAWAY semantics say those may still get a response, per RFC 9113 §6.8.
     * @param code the GOAWAY error code to report to the peer.
     * @param stream_id the last stream id the peer should consider as possibly still
     * processed — defaults to 0, meaning "nothing more, full stop."
     * @param graceful when true, skip the close callback so the transport stays open — lets the
     * queued GOAWAY (and any pending responses) flush before the socket is torn down elsewhere.
     */
    void close(error::http::Http2ErrorCode code, std::uint32_t stream_id = 0, bool graceful = false)
    {
        // GOAWAY is going out — refuse any new streams from here on (see get_or_create_stream).
        m_closed = true;

        // Graceful shutdown announces the last processed client stream (not 0) so already-open
        // streams at or below it survive the prune below and get to finish.
        if (graceful && stream_id == 0) {
            stream_id = get_last_client_stream_id();
        }

        // Notify every extension the connection is going down (seam for releasing any
        // per-connection state). No-op if no extensions are registered.
        m_extension_registry.get().for_each([&](auto& extension) {
            extension->on_connection_close(std::to_underlying(code));
        });

        send_goaway(code, stream_id);

        // Prune every stream past the announced last-stream-id; streams at or below it are
        // deliberately kept, per RFC 9113 §6.8 they may still get a response.
        std::erase_if(m_streams, [stream_id](const auto& entry) {
            return entry.first > stream_id;
        });

        // Graceful shutdown leaves the socket open (still reading) so the sender can flush and
        // any in-flight partial header can still finish parsing; teardown happens later (drain
        // then hard close). A hard close drops the partial header and fires the close callback
        // now.
        if (!graceful) {
            m_safe_header.reset();
            m_closer();
        }
    }

    /**
     * @brief Checks whether every stream on this session has finished.
     * @return true if no active streams remain.
     */
    [[nodiscard]] bool is_idle() const noexcept
    {
        return m_streams.empty();
    }

    /**
     * @brief Grabs the most recently assigned client-initiated stream id. Bet, plain getter.
     * @return the last client stream id handed out by `next_client_stream()`.
     */
    [[nodiscard]] std::uint32_t get_last_client_stream_id() const noexcept
    {
        return m_last_client_stream_id;
    }

    /**
     * @brief Grabs the local settings, read-only. Lowkey just a getter.
     * @return the local `Settings`.
     */
    [[nodiscard]] const Settings& get_local_settings() const noexcept
    {
        return m_local_settings;
    }

    /**
     * @brief Grabs the local settings, mutable — used by `Handshake` to read the max frame size
     * while wiring up the initial SETTINGS exchange.
     * @return the local `Settings`.
     */
    Settings& get_local_settings() noexcept
    {
        return m_local_settings;
    }

private:
    /**
     * @brief Handles a stream whose remote side is done sending. For a stream WE initiated
     * (client) this is the awaited response — its body is handed to `m_dispatch` to resolve the
     * pending request, no reply sent. For a peer-initiated stream (server) it runs the
     * registered handler, defaulting the response to NOT_FOUND up front so an unhandled
     * route/exception still gets a real HTTP status, then encodes and sends what the handler
     * produced.
     * @warning Any exception the dispatch handler throws gets caught and downgraded to a plain
     * INTERNAL_SERVER_ERROR status — the actual exception details only make it as far as the
     * logger, never back to the peer. If you're debugging a handler that's misbehaving, check
     * the logs, not the wire response, that's where the real error went.
     * @param stream_id the stream to respond on — must already exist in `m_streams`, this uses
     * `.at()` which throws `std::out_of_range` on a bad id (uncaught here).
     */
    void response(std::uint32_t stream_id)
    {
        auto& stream = *m_streams.at(stream_id);
        auto& req = stream.get_request();
        auto& res = stream.get_response();

        try {
            // Client: this completed stream is a response we awaited — headers/status/body all
            // decoded straight into res; hand it to the dispatch hook. No handler, no reply.
            if (m_role == Role::CLIENT) {
                m_dispatch(req, res, [] {});

                mark_stream_closed(stream_id);
            } else {
                // Server: default NOT_FOUND, then run the handler (it calls send() when ready).
                res.set_status(interfaces::io::types::Status::NOT_FOUND);
                auto send = [this, stream_id,
                             called = std::make_shared<std::atomic<bool>>(false)]() {
                    if (called->exchange(true)) {
                        return;
                    }
                    send_response(stream_id);
                };
                m_dispatch(req, res, std::move(send));
            }
        } catch (const std::exception& e) {
            core::logger::error("http2/session", "handler threw: {}", e.what());
            core::events::publish("http2.session.handler_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send_response(stream_id);
        }
    }

    /**
     * @brief Encodes and ships the response for `stream_id`. Idempotent — safe to call from the
     * handler's send callback or from the exception fallback path.
     * @param stream_id the stream whose response should be framed and sent.
     */
    void send_response(std::uint32_t stream_id)
    {
        auto& stream = *m_streams.at(stream_id);
        auto& res = stream.get_response();

        // Let every extension observe (and optionally mutate) the response before it's framed.
        m_extension_registry.get().for_each([&](auto& extension) {
            extension->on_response_outgoing(stream_id, res);
        });

        // Encode whatever the handler (or the NOT_FOUND/500 fallback) produced and ship it.
        auto node =
            utils::buffering::BufferNode{res.get_size(m_local_settings.get_max_frame_size())};
        node |
            WriteHttpResponseAdaptor{res, m_encoding_table, m_local_settings.get_max_frame_size()};
        send_node(std::move(node));

        // Drive the send side to END_STREAM so the state machine lands on CLOSED (the response
        // carries END_STREAM on the wire). is_local=true — this is our own send.
        stream.advance_send(shared_layer::FrameType::DATA, shared_layer::Flags::END_STREAM, true);

        // The response is fully framed and enqueued; this stream is done — remove it so
        // `is_idle()` can tell when every connection has finished.
        mark_stream_closed(stream_id);
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
    Stream<>& next_client_stream()
    {
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
    Stream<>& get_or_create_stream(const std::uint32_t& stream_id)
    {
        // Guard — stream 0 is reserved for connection-level frames, never a real per-request
        // stream.
        if (stream_id == 0) {
            throw error::http::ConnectionError{
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"
            };
        }

        // Finished stream — reject instead of silently re-opening a closed id.
        if (m_closed_streams.contains(stream_id)) {
            throw error::http::ConnectionError{
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Stream ID {} is closed", stream_id),
                m_remote_settings.get_last_stream_id()
            };
        }

        // Already tracked — return the live stream.
        auto it = m_streams.find(stream_id);
        if (it != m_streams.end()) {
            return *(it->second);
        }

        // GOAWAY already sent — refuse to open any new stream, per RFC 9113 §6.8.
        if (m_closed) {
            throw error::http::ConnectionError{
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Stream ID {} refused — connection is shutting down", stream_id),
                m_remote_settings.get_last_stream_id()
            };
        }

        // First time seeing this id — spin up a fresh stream wired to the shared connection
        // stream, HPACK tables, and settings. Role decides the inbound decode target.
        auto stream = std::make_unique<Stream<>>(
            stream_id, m_connection_stream, m_decoding_table, m_encoding_table, m_local_settings,
            m_remote_settings, true, m_role == Role::SERVER
        );

        // Track the highest client-initiated (odd) stream id seen — spec-compliant
        // last-stream-id for GOAWAY and monotonic-open validation. max() keeps it correct on
        // the client too.
        if ((stream_id & 1U) == 1U) {
            m_last_client_stream_id = std::max(m_last_client_stream_id, stream_id);
        }

        auto [new_it, inserted] = m_streams.emplace(stream_id, std::move(stream));
        if (inserted) {
            // A brand-new stream just opened — notify every extension.
            m_extension_registry.get().for_each([&](auto& extension) {
                extension->on_stream_open(stream_id);
            });
        }
        return *(new_it->second);
    }

    /**
     * @brief Builds and sends a GOAWAY frame. Duplicate teardown is guarded at the flow level
     * (`ServerFlow::close`'s `m_closed`).
     * @param code the GOAWAY error code.
     * @param stream_id the last processed stream id.
     */
    void send_goaway(error::http::Http2ErrorCode code, std::uint32_t stream_id)
    {
        auto payload = std::views::empty<std::byte> |
                       utils::codec::WriteBigEndianAdaptor<std::uint32_t>{stream_id} |
                       utils::codec::WriteBigEndianAdaptor<std::uint32_t>{
                           static_cast<std::uint32_t>(std::to_underlying(code))
                       } |
                       std::ranges::to<std::vector<std::byte>>();

        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::GOAWAY)
                         .add_flags(0)
                         .add_stream_id(0)
                         .add_payload(payload)
                         .build();

        send_frame(frame);

        core::logger::info("http2/session", "GOAWAY sent: {} last_stream={}", code, stream_id);
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
    receive_header(utils::buffering::BufferReader& target)
    {
        // Not enough bytes buffered yet for a full fixed-size header — caller retries later.
        if (target.size() < HEADER_SIZE) {
            core::logger::debug(
                "http2/session", "incomplete header expected={} got={}", HEADER_SIZE, target.size()
            );

            return std::nullopt;
        }

        // Parse the header then advance the reader past those 9 bytes in the same pipeline.
        auto header = target | std::views::take(HEADER_SIZE) |
                      ReadFrameHeaderAdaptor{m_local_settings.get_max_frame_size()} |
                      utils::buffering::AdvanceReaderAdaptor{target, HEADER_SIZE};

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
    void mark_stream_closed(std::uint32_t stream_id)
    {
        // Guard — same reserved-id rule as get_or_create_stream().
        if (stream_id == 0) {
            throw error::http::ConnectionError{
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Stream ID 0 is reserved"
            };
        }

        // Remove the stream and record its id so a later lookup rejects it instead of silently
        // re-opening a finished stream (see get_or_create_stream).
        if (auto it = m_streams.find(stream_id); it != m_streams.end()) {
            m_streams.erase(it);
            m_closed_streams.insert(stream_id);

            // Stream reached graceful teardown — notify every extension.
            m_extension_registry.get().for_each([&](auto& extension) {
                extension->on_stream_close(stream_id);
            });

            // If we're winding down after a received GOAWAY, this may have been the last
            // survivor.
            close_transport_if_drained();
            return;
        }

        throw error::http::ConnectionError{
            error::http::Http2ErrorCode::PROTOCOL_ERROR,
            std::format(
                "Stream with ID {} was not found and therefor could not be closed / finished",
                stream_id
            ),
            m_remote_settings.get_last_stream_id()
        };
    }

    /**
     * @brief Runs the RFC 9113 §6.8 teardown for a GOAWAY the connection stream received
     * (recorded, never echoed): refuse new streams, drop locally-initiated streams above the
     * peer's Last-Stream-ID (those weren't processed — retryable on a new connection), and
     * close the transport once the survivors (<= Last-Stream-ID) finish — or immediately on an
     * error GOAWAY.
     */
    void handle_goaway()
    {
        m_goaway_received = true;
        m_closed = true; // No new streams — get_or_create_stream() enforces this.

        const auto LAST_ID = m_connection_stream.goaway_last_stream_id();
        const auto CODE = m_connection_stream.goaway_error_code();

        // Streams we opened above LAST_ID were not processed by the peer — drop them.
        std::erase_if(m_streams, [LAST_ID](const auto& entry) {
            return entry.first > LAST_ID;
        });

        // An error GOAWAY tears the connection down now; a clean (NO_ERROR) GOAWAY lets streams
        // <= LAST_ID finish first — close_transport_if_drained() closes once the last one's
        // done.
        if (CODE != error::http::Http2ErrorCode::NO_ERROR_CODE) {
            m_streams.clear();
        }
        close_transport_if_drained();
    }

    /// @brief Closes the transport exactly once, after a received GOAWAY, when no streams
    /// remain.
    void close_transport_if_drained()
    {
        if (m_goaway_received && !m_transport_closed && m_streams.empty()) {
            m_transport_closed = true;
            m_closer();
        }
    }

    bool m_closed = false;
    bool m_goaway_received = false;
    bool m_transport_closed = false;
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
    std::reference_wrapper<HttpExtensionRegistry> m_extension_registry;
    Role m_role;
};

} // namespace io::layer::http2

#ifdef CONGELADO_TEST
namespace io::layer::http2::session_tests {
using namespace boost::ut;

/// @brief Builds a BufferReader wrapping exactly `bytes` — same helper shape used across this
/// module's other partitions (see stream.cppm's make_reader).
static utils::buffering::BufferReader make_reader(const std::vector<std::byte>& bytes)
{
    auto* node = new utils::buffering::BufferNode(bytes.size());
    for (auto b: bytes) {
        node->push_back(b);
    }
    utils::buffering::BufferReader reader;
    reader.push_back(node);
    return reader;
}

/// @brief Encodes a fully-built FrameBuilder into raw on-wire bytes (header + payload).
static std::vector<std::byte>
encode_frame(FrameBuilder<shared_layer::FrameRole::SENDER> frame, std::uint32_t max_frame_size)
{
    return std::views::empty<std::byte> |
           WriteFrameBuilderAdaptor{std::move(frame), max_frame_size} |
           std::ranges::to<std::vector<std::byte>>();
}

suite<"Session ctor / getters"> session_ctor_suite = [] {
    "server ctor starts idle with last-client-stream-id 0"_test = [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode&&) {}, [] {}, registry, Role::SERVER};

        expect(session.is_idle());
        expect(session.get_last_client_stream_id() == 0U);
    };

    "client ctor starts idle with last-client-stream-id 1 (first send() lands on 3)"_test = [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode&&) {}, [] {}, registry, Role::CLIENT};

        expect(session.is_idle());
        expect(session.get_last_client_stream_id() == 1U);
    };

    "get_extension_registry returns the same registry this session was constructed with"_test = [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode&&) {}, [] {}, registry, Role::SERVER};

        expect(&session.get_extension_registry() == &registry);
    };

    "get_local_settings — mutable and const overloads resolve to the same underlying Settings"_test =
        [] {
            HttpExtensionRegistry registry;
            Session session{[](utils::buffering::BufferNode&&) {}, [] {}, registry, Role::SERVER};

            auto& mutable_settings = session.get_local_settings();
            const Session& const_session = session;
            const auto& const_settings = const_session.get_local_settings();

            expect(&mutable_settings == &const_settings);
            expect(mutable_settings.get_max_frame_size() == const_settings.get_max_frame_size());
        };
};

suite<"Session::send_node / send_frame"> session_send_suite = [] {
    "send_node hands an already-encoded buffer node straight to the transport callback"_test = [] {
        std::vector<std::byte> captured;
        int send_calls = 0;
        HttpExtensionRegistry registry;
        Session session{
            [&](utils::buffering::BufferNode&& node) {
                ++send_calls;
                captured.assign(node.get_data(), node.get_data() + node.get_written());
            },
            [] {}, registry, Role::SERVER
        };

        utils::buffering::BufferNode node{3};
        node.push_back(std::byte{0xAA});
        node.push_back(std::byte{0xBB});
        node.push_back(std::byte{0xCC});

        session.send_node(std::move(node));

        expect(send_calls == 1) << fatal;
        expect(captured.size() == 3U);
        expect(captured[0] == std::byte{0xAA});
        expect(captured[2] == std::byte{0xCC});
    };

    "send_frame encodes a FrameBuilder into a single frame and ships it"_test = [] {
        std::vector<std::byte> captured;
        HttpExtensionRegistry registry;
        Session session{
            [&](utils::buffering::BufferNode&& node) {
                captured.assign(node.get_data(), node.get_data() + node.get_written());
            },
            [] {}, registry, Role::SERVER
        };

        auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                         .add_type(shared_layer::FrameType::PING)
                         .add_flags(shared_layer::Flags::ACK)
                         .add_stream_id(0)
                         .add_payload(std::vector<std::byte>(8, std::byte{0x7A}))
                         .build();

        session.send_frame(frame);

        auto header =
            captured | ReadFrameHeaderAdaptor{session.get_local_settings().get_max_frame_size()};
        expect(header.get_type() == shared_layer::FrameType::PING);
        expect((header.get_flags() & shared_layer::Flags::ACK) != 0);
        expect(header.get_stream_id() == 0U);
    };
};

suite<"Session::close / is_idle"> session_close_suite = [] {
    "close() (hard, default) sends a GOAWAY(stream=0) and invokes the close callback exactly once"_test =
        [] {
            std::vector<std::byte> captured;
            int close_calls = 0;
            HttpExtensionRegistry registry;
            Session session{
                [&](utils::buffering::BufferNode&& node) {
                    captured.assign(node.get_data(), node.get_data() + node.get_written());
                },
                [&close_calls] {
                    ++close_calls;
                },
                registry, Role::SERVER
            };

            session.close(error::http::Http2ErrorCode::NO_ERROR_CODE);

            expect(close_calls == 1);
            auto header = captured |
                          ReadFrameHeaderAdaptor{session.get_local_settings().get_max_frame_size()};
            expect(header.get_type() == shared_layer::FrameType::GOAWAY);
            expect(header.get_stream_id() == 0U);
        };

    "close(graceful=true) sends GOAWAY but leaves the transport open (close callback not invoked)"_test =
        [] {
            int close_calls = 0;
            HttpExtensionRegistry registry;
            Session session{
                [](utils::buffering::BufferNode&&) {},
                [&close_calls] {
                    ++close_calls;
                },
                registry, Role::SERVER
            };

            session.close(error::http::Http2ErrorCode::NO_ERROR_CODE, 0, true);

            expect(close_calls == 0);
        };

    "close() (hard, default stream_id=0) drops every open client stream so is_idle() becomes true"_test =
        [] {
            int close_calls = 0;
            HttpExtensionRegistry registry;
            Session session{
                [](utils::buffering::BufferNode&&) {},
                [&close_calls] {
                    ++close_calls;
                },
                registry, Role::CLIENT
            };

            HttpRequest first{0};
            first.set_header(interfaces::io::types::Token::METHOD, "GET");
            first.set_header(interfaces::io::types::Token::PATH, "/a");

            // send() drives HPACK *encoding*, which is documented elsewhere in this codebase
            // (see req.cppm's WriteHttpRequestAdaptor comment) as having pre-existing
            // correctness gaps — guarded with nothrow rather than asserted on exact wire bytes.
            expect(nothrow([&] {
                std::ignore = session.send(first);
            })) << fatal;

            expect(not session.is_idle());

            session.close(error::http::Http2ErrorCode::NO_ERROR_CODE);

            expect(session.is_idle());
            expect(close_calls == 1);
        };
};

suite<"Session::send — client-initiated stream assignment"> session_send_request_suite = [] {
    "send() assigns sequential odd client stream ids starting at 3"_test = [] {
        HttpExtensionRegistry registry;
        Session session{[](utils::buffering::BufferNode&&) {}, [] {}, registry, Role::CLIENT};

        HttpRequest first{0};
        first.set_header(interfaces::io::types::Token::METHOD, "GET");
        first.set_header(interfaces::io::types::Token::PATH, "/a");

        std::uint32_t first_id = 0;
        expect(nothrow([&] {
            first_id = session.send(first);
        })) << fatal;
        expect(first_id == 3U);
        expect(first.get_stream_id() == 3U);
        expect(session.get_last_client_stream_id() == 3U);

        HttpRequest second{0};
        second.set_header(interfaces::io::types::Token::METHOD, "GET");
        second.set_header(interfaces::io::types::Token::PATH, "/b");

        std::uint32_t second_id = 0;
        expect(nothrow([&] {
            second_id = session.send(second);
        })) << fatal;
        expect(second_id == 5U);
        expect(session.get_last_client_stream_id() == 5U);
    };
};

suite<"Session::receive"> session_receive_suite = [] {
    "receive() decodes a client HEADERS(END_HEADERS,END_STREAM) frame, opens the stream, and "
    "dispatches it to the handler; the handler's send() then closes the stream"_test = [] {
        HttpExtensionRegistry registry;
        int dispatch_calls = 0;
        interfaces::io::IRequest* captured_req = nullptr;
        interfaces::io::IResponse* captured_res = nullptr;
        std::function<void()> stored_send;

        interfaces::io::ReceiveDispatchFn dispatch = [&](interfaces::io::IRequest& req,
                                                         interfaces::io::IResponse& res,
                                                         std::function<void()> send) {
            ++dispatch_calls;
            captured_req = &req;
            captured_res = &res;
            stored_send = std::move(send);
        };

        Session session{
            [](utils::buffering::BufferNode&&) {}, [] {}, registry, Role::SERVER, dispatch
        };

        auto frame =
            FrameBuilder<shared_layer::FrameRole::SENDER>{}
                .add_type(shared_layer::FrameType::HEADERS)
                .add_flags(shared_layer::Flags::END_HEADERS | shared_layer::Flags::END_STREAM)
                .add_stream_id(3)
                .add_payload(std::vector<std::byte>{std::byte{0x82}}) // static idx 2: GET
                .build();
        auto bytes =
            encode_frame(std::move(frame), session.get_local_settings().get_max_frame_size());

        auto reader = make_reader(bytes);
        session.receive(reader);

        expect(dispatch_calls == 1) << fatal;
        expect(captured_req->get_method() == "GET");
        expect(captured_res->get_status() == interfaces::io::types::Status::NOT_FOUND);
        expect(not session.is_idle()); // stream still open, awaiting the handler's send()

        // Crosses the HPACK *encoder* path on the way out (see the close-suite note above) —
        // guarded with nothrow rather than asserted on exact wire bytes.
        expect(nothrow([&] {
            stored_send();
        }));
        expect(session.is_idle()); // send_response() -> mark_stream_closed()
    };

    "receive() a connection-level GOAWAY runs teardown and, with no streams open, closes the transport"_test =
        [] {
            int close_calls = 0;
            HttpExtensionRegistry registry;
            Session session{
                [](utils::buffering::BufferNode&&) {},
                [&close_calls] {
                    ++close_calls;
                },
                registry, Role::CLIENT
            };

            auto payload =
                std::views::empty<std::byte> |
                utils::codec::WriteBigEndianAdaptor<std::uint32_t>{0U} |
                utils::codec::WriteBigEndianAdaptor<std::uint32_t>{static_cast<std::uint32_t>(
                    std::to_underlying(error::http::Http2ErrorCode::NO_ERROR_CODE)
                )} |
                std::ranges::to<std::vector<std::byte>>();

            auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                             .add_type(shared_layer::FrameType::GOAWAY)
                             .add_flags(0)
                             .add_stream_id(0)
                             .add_payload(payload)
                             .build();
            auto bytes =
                encode_frame(std::move(frame), session.get_local_settings().get_max_frame_size());

            auto reader = make_reader(bytes);
            session.receive(reader);

            // No streams were open, so handle_goaway()'s close_transport_if_drained() fires
            // right away.
            expect(close_calls == 1);
            expect(session.is_idle());
        };
};

} // namespace io::layer::http2::session_tests
#endif
