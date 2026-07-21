module;
// TODO: Remove if std module is fixed i can not switch to libc++ for now so this is really killing
// me
#include <ranges>

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
    // monostate means "nothing to build" — every other T actually gets constructed from args.
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
    /**
     * @brief Builds the connection-level `Stream`'s helper state, tracking whatever GOAWAY
     * error code should be reported if the connection needs to close.
     * @param connection_error_code the error code to report on GOAWAY, defaults to
     * NO_ERROR_CODE (clean shutdown).
     */
    ConnectionLevelHelper(error::http::Http2ErrorCode connection_error_code =
                              error::http::Http2ErrorCode::NO_ERROR_CODE)
        : m_connection_error_code{connection_error_code} {}

    /**
     * @brief Sets the error code to report if/when this connection sends GOAWAY.
     * @param error_code the error code to store.
     */
    void set_connection_error_code(error::http::Http2ErrorCode error_code) noexcept {
        m_connection_error_code = error_code;
    }

    /**
     * @brief Grabs the currently-tracked connection error code. Bet, plain getter.
     * @return the error code that'll be reported on GOAWAY.
     */
    [[nodiscard]] const error::http::Http2ErrorCode &get_connection_error_code() const noexcept {
        return m_connection_error_code;
    }

  private:
    error::http::Http2ErrorCode m_connection_error_code;
};

class StreamLevelHelper {
  public:
    /**
     * @brief Builds the per-stream helper state — a fresh `HttpRequest`/`HttpResponse` pair for
     * `STREAM_ID`, an HPACK codec wired to the shared decode/encode tables plus this stream's
     * own request/response, and a live (not-yet-consumed) header block buffer ready to receive
     * HEADERS/CONTINUATION bytes.
     * @param STREAM_ID the stream this helper belongs to.
     * @param connection_stream reference to the shared connection-level stream, kept around so
     * DATA/window-update accounting can bubble up to it.
     * @param decoding_table the shared HPACK dynamic table for decoding incoming headers.
     * @param encoding_table the shared HPACK dynamic table for encoding outgoing headers.
     */
    StreamLevelHelper(const std::uint32_t STREAM_ID, Stream<false> &connection_stream,
                      std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
                      std::reference_wrapper<codec::hpack::HPackTable> encoding_table)
        : m_connection_stream{connection_stream}, m_request{STREAM_ID}, m_response{STREAM_ID},
          m_hpack{decoding_table, encoding_table, m_request, m_response},
          m_header_block{std::make_optional<utils::buffering::BufferView>()} {}

    /**
     * @brief Flags whether this stream is mid-way through a HEADERS/PUSH_PROMISE block still
     * waiting on trailing CONTINUATION frames.
     * @param value true if more CONTINUATION frames are still expected.
     */
    void set_expecting_continuation(bool value) noexcept { m_expecting_continuation = value; }

    /**
     * @brief Flags whether the remote side is fully done sending on this stream (drives
     * `Session::receive()`'s decision to fire the response dispatch).
     * @param value true once the remote side has sent END_STREAM.
     */
    void set_remote_done(bool value) noexcept { m_remote_done = value; }


    /**
     * @brief Drops the header block buffer, marking it as already-consumed — this is what
     * makes `get_header_block()` throw on a second HEADERS collection attempt.
     */
    void clear_header_block() noexcept { m_header_block.reset(); }

    /**
     * @brief Grabs mutable access to the pending header block buffer, for accumulating
     * HEADERS/CONTINUATION payload bytes into before they get HPACK-decoded.
     * @warning HTTP/2 only allows one HEADERS collection per stream lifecycle — a second
     * attempt (header block already cleared via `clear_header_block()`) is a straight
     * PROTOCOL_ERROR, not silently allowed. Trailers aren't a thing this tracks separately.
     * @return the live header block buffer.
     * @throws error::http::ConnectionError if the header block was already cleared (i.e. a
     * second collection of headers is being attempted on this stream).
     */
    utils::buffering::BufferView &get_header_block() {
        // Guard — a cleared header block means headers were already fully collected once on
        // this stream; HTTP/2 doesn't allow a second collection, that's a straight L for whoever
        // tries it.
        if (!m_header_block.has_value()) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                "You are trying to receive second collection of headers on the same stream ID `{}`",
                m_request.get_stream_id());
        }
        return m_header_block.value();
    }
    /**
     * @brief Grabs the shared connection-level stream this stream's window accounting bubbles
     * up through. Lowkey the glue between per-stream and connection-wide flow control.
     * @return a reference wrapper to the connection-level stream.
     */
    std::reference_wrapper<Stream<false>> get_connection_stream() noexcept {
        return m_connection_stream;
    }
    /**
     * @brief Grabs mutable access to this stream's request.
     * @return the request.
     */
    HttpRequest &get_request() noexcept { return m_request; }
    /**
     * @brief Grabs mutable access to this stream's response.
     * @return the response.
     */
    HttpResponse &get_response() noexcept { return m_response; }
    /**
     * @brief Grabs mutable access to this stream's HPACK codec.
     * @return the HPACK codec.
     */
    codec::hpack::Hpack<> &get_hpack() noexcept { return m_hpack; }
    /**
     * @brief Checks whether this stream is still waiting on trailing CONTINUATION frames.
     * @return true if more CONTINUATION frames are expected.
     */
    [[nodiscard]] bool get_expecting_continuation() const noexcept {
        return m_expecting_continuation;
    }
    /**
     * @brief Checks whether the remote side has finished sending on this stream.
     * @return true if END_STREAM's already been received.
     */
    [[nodiscard]] bool get_is_remote_done() const noexcept { return m_remote_done; }

  private:
    std::reference_wrapper<Stream<false>> m_connection_stream;
    HttpRequest m_request;
    HttpResponse m_response;
    codec::hpack::Hpack<> m_hpack;
    std::optional<utils::buffering::BufferView> m_header_block;
    bool m_expecting_continuation{false};
    bool m_remote_done{false};
};


template <bool IsStreamBased = true>
class Stream {
  public:
    /**
     * @brief Connection-level ctor — builds the special stream-id-0 `Stream` that handles
     * connection-scoped frames (SETTINGS, PING, GOAWAY, connection-level WINDOW_UPDATE). State
     * machine starts pinned to id 0, send/recv windows seeded from the remote's advertised
     * initial window size.
     * @note Only enabled `requires(!IsStreamBased)` — the stream-level ctor right below covers
     * `IsStreamBased = true`. Mutually exclusive overloads via the requires-clause, same
     * pattern as `Handshake::process()`.
     * @param local_settings reference to the session's local settings.
     * @param remote_settings reference to the session's remote settings, used to seed the
     * initial flow-control windows.
     */
    Stream(std::reference_wrapper<Settings> local_settings,
           std::reference_wrapper<Settings> remote_settings)
        requires(!IsStreamBased)
        : m_state_machine{StreamStateMachine{0}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().get_initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().get_initial_window_size())},
          m_local_settings{local_settings}, m_remote_settings{remote_settings},
          m_stream_helper{ConnectionLevelHelper{error::http::Http2ErrorCode::NO_ERROR_CODE}} {}

    /**
     * @brief Stream-level ctor — builds a real per-request `Stream` bound to `STREAM_ID`, with
     * parity validated up front against `is_client_initiated` (RFC 9113 §5.1.1: client streams
     * are odd, server-initiated ones are even).
     * @note Only enabled `requires(IsStreamBased)` — the connection-level ctor above covers
     * `IsStreamBased = false`.
     * @warning `is_client_initiated` defaults to `true`, and the two branches throw *different*
     * error types for what's conceptually the same class of violation — a bad-parity client
     * stream id throws `ConnectionError{PROTOCOL_ERROR}`, but a bad-parity server-initiated one
     * throws `ConnectionError{INTERNAL_ERROR}`. Worth knowing if you're catching by error code
     * upstream, they're not symmetric.
     * @param STREAM_ID the stream id this instance represents.
     * @param connection_stream reference to the shared connection-level stream.
     * @param decoding_table the shared HPACK dynamic table for decoding.
     * @param encoding_table the shared HPACK dynamic table for encoding.
     * @param local_settings reference to the session's local settings.
     * @param remote_settings reference to the session's remote settings, used to seed the
     * initial flow-control windows.
     * @param is_client_initiated true if `STREAM_ID` should be validated as client-initiated
     * (odd), false to validate as server-initiated (even). Defaults to true.
     * @throws error::http::ConnectionError if `STREAM_ID`'s parity doesn't match
     * `is_client_initiated`.
     */
    Stream(const std::uint32_t STREAM_ID, Stream<false> &connection_stream,
           std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
           std::reference_wrapper<codec::hpack::HPackTable> encoding_table,
           std::reference_wrapper<Settings> local_settings,
           std::reference_wrapper<Settings> remote_settings, bool is_client_initiated = true)
        requires IsStreamBased
        : m_state_machine{StreamStateMachine{STREAM_ID}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().get_initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().get_initial_window_size())},
          m_local_settings{local_settings}, m_remote_settings{remote_settings},
          m_stream_helper{
              StreamLevelHelper{STREAM_ID, connection_stream, decoding_table, encoding_table}} {
        if (is_client_initiated) {
            // As a server, frames we receive from a client MUST be odd if non-zero
            if (STREAM_ID > 0 && (STREAM_ID % 2 == 0)) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Client-initiated stream ID must be odd");
            }
        } else {
            // Server-initiated (pushed) streams are always even — different error code than
            // the client-parity check above, not symmetric on purpose per the class docs.
            if (STREAM_ID % 2 != 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::INTERNAL_ERROR,
                                                   "Server-initiated stream ID must be even");
            }
        }
    }

    /**
     * @brief Dtor — runs the same cleanup as `cleanup_resources()`, clearing the receive
     * buffer and (stream-based only) the continuation-expected flag. No drama, clean motion.
     */
    ~Stream() { cleanup_resources(); }
    Stream(const Stream &) = default;
    Stream &operator=(const Stream &) = default;
    Stream(Stream &&) = default;
    Stream &operator=(Stream &&) = default;

    /**
     * @brief Connection-level frame receiver — handles WINDOW_UPDATE, GOAWAY, PING, SETTINGS
     * at the connection scope; DATA/HEADERS/PUSH_PROMISE/CONTINUATION/RST_STREAM are all
     * rejected outright since they're never valid on the connection-level stream.
     * @note Only enabled `requires(!IsStreamBased)`. The stream-based overload right below
     * handles `IsStreamBased = true` — same name, mutually exclusive via requires-clause.
     * @warning PRIORITY throws unconditionally here — this implementation doesn't support
     * PRIORITY frames at all (deprecated in RFC 9113), so receiving one is treated the same as
     * a genuinely malformed connection-level frame.
     * @tparam Role the sender/receiver role tag on the incoming `FrameHeader`.
     * @param header the already-parsed frame header.
     * @param reader the reader positioned at the frame's payload — gets consumed by exactly
     * `header.get_length()` bytes before returning, regardless of frame type.
     * @return a reply frame to send back (SETTINGS ACK, PING ACK, or an echoed GOAWAY), or
     * `std::nullopt` if nothing needs sending back for this frame.
     * @throws error::http::ConnectionError for PRIORITY, or for any stream-scoped frame type
     * received at connection level.
     */
    template <shared_layer::FrameRole Role>
        requires(!IsStreamBased)
    std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>>
    receive(const FrameHeader<Role> &header, utils::buffering::BufferReader &reader) {

        const auto &type = header.get_type();
        std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>> response = std::nullopt;

        core::logger::debug("http2/conn", "frame {} len={} flags={}", type, header.get_length(),
                            header.get_flags());

        // Fan out on frame type — only connection-scoped types are legal here.
        switch (type) {
        case shared_layer::FrameType::WINDOW_UPDATE: {
            // Connection-level window update — bumps our send window, no reply needed.
            auto increment = reader | ReadWindowIncrementAdaptor{};

            core::logger::debug("http2/conn", "WINDOW_UPDATE increment={}", increment);

            update_send_window(increment);
            break;
        }
        case shared_layer::FrameType::GOAWAY: {
            // Peer's telling us to wind down — record their last-stream threshold and echo
            // our own GOAWAY back with whatever error code we're currently tracking.
            core::logger::debug("http2/conn", "GOAWAY last_stream={} code={}",
                                header.get_stream_id(),
                                m_stream_helper.get_connection_error_code());

            m_remote_settings.get().set_last_stream_id(header.get_stream_id());

            auto payload =
                std::views::empty<std::byte> |
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
            core::logger::debug("http2/conn", "PING flags={}", header.get_flags());

            const auto &flags = header.get_flags();

            // ACK — a reply to a PING we sent earlier, feed it to the round-trip tracker.
            // Non-ACK — peer's pinging us, echo the same 8 bytes back with ACK set.
            if (flags & shared_layer::Flags::ACK) {
                auto payload_array =
                    reader | std::views::take(8) | std::ranges::to<std::vector<std::byte>>();
                if (!m_remote_settings.get().get_ping_tracker().on_ack(payload_array)) {
                    core::logger::warning("http2/conn", "unsolicited PING ACK, ignoring");
                }
            } else {
                m_remote_settings.get().get_ping_tracker().note_activity();

                core::logger::debug("http2/conn", "PING sending ACK");

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
            // Delegate the whole SETTINGS dance (ACK short-circuit, decode, window delta,
            // reply ACK) to the dedicated helper.
            core::logger::debug("http2/conn", "SETTINGS len={}", header.get_length());

            response = handle_settings(header, reader);
            break;
        }
        case shared_layer::FrameType::PRIORITY: {
            // Deprecated frame type, unconditionally rejected — this implementation never
            // supported it.
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                "PRIORITY frames are not supported in this implementation - deprecated "
                "in HTTP/2 (RFC9113)");
        }

        case shared_layer::FrameType::DATA:
        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION:
        case shared_layer::FrameType::RST_STREAM: {
            // Every one of these is inherently stream-scoped — none of them make sense on the
            // connection-level (stream 0) receiver.
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Type `{}` is not valid on connection-level stream", type));
        }
        default:
            break;
        }

        // Whatever branch ran, the frame's payload bytes get consumed off the reader here.
        reader.consume(header.get_length());

        return response;
    }


    /**
     * @brief Stream-level frame receiver — the workhorse for per-request frame handling: runs
     * the frame through `m_state_machine.advance()` first (so illegal transitions throw before
     * any data gets touched), then dispatches DATA into the request body, HEADERS/PUSH_PROMISE/
     * CONTINUATION into the header block (decoding via `handle_header()` once END_HEADERS
     * lands), WINDOW_UPDATE into send-window accounting, and RST_STREAM into an immediate
     * cleanup + rethrow as `StreamError`. Marks the stream remote-done once HALF_CLOSED_REMOTE
     * is reached, and runs cleanup if the state machine lands on CLOSED.
     * @note Only enabled `requires(IsStreamBased)` — see the connection-level overload above
     * for `IsStreamBased = false`.
     * @warning If `m_stream_helper.get_expecting_continuation()` is true and a non-CONTINUATION
     * frame shows up, that's an immediate PROTOCOL_ERROR before the state machine even runs —
     * HEADERS fragments can't be interleaved with other frame types on the same stream, RFC
     * 9113 §6.10 is strict about this.
     * @warning PRIORITY, SETTINGS, PING, and GOAWAY are all rejected here — this stream-level
     * path is exclusively for per-request frame types, the connection-scoped ones only belong
     * on the connection-level `receive()` overload above.
     * @tparam Role the sender/receiver role tag on the incoming `FrameHeader`.
     * @param header the already-parsed frame header.
     * @param reader the reader positioned at the frame's payload — gets consumed by exactly
     * `header.get_length()` bytes before returning.
     * @throws error::http::ConnectionError for illegal state transitions, unexpected
     * CONTINUATION interleaving, incomplete DATA/HEADERS reads, or frame types not valid at
     * stream level (PRIORITY/SETTINGS/PING/GOAWAY).
     * @throws error::http::StreamError on RST_STREAM (stream reset by the peer), or on a
     * flow-control window violation.
     */
    template <shared_layer::FrameRole Role>
        requires IsStreamBased
    void receive(const FrameHeader<Role> &header, utils::buffering::BufferReader &reader) {

        const auto &type = header.get_type();

        core::logger::debug("http2/stream", "stream {} frame {} len={} flags={}", get_stream_id(),
                            type, header.get_length(), header.get_flags());

        // Guard — mid-HEADERS-block, only CONTINUATION frames are allowed to interleave.
        if (m_stream_helper.get_expecting_continuation() &&
            type != shared_layer::FrameType::CONTINUATION) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Stream expecting CONTINUATION but received `{}`", type),
                get_stream_id());
        }

        // Run the frame through the state machine first — illegal transitions throw before any
        // payload data gets touched below.
        m_state_machine.advance(header.get_type(), header.get_flags(), false);

        switch (type) {
        case shared_layer::FrameType::DATA: {
            // Debit both flow-control windows and read the bytes straight into the request body.
            auto &view = m_stream_helper.get_request().get_body();
            if (header.get_length() > 0) {
                consume_window(header.get_length(), false);

                reader.expand_view(view, header.get_length());
                if (view.size() != header.get_length()) {
                    throw error::http::ConnectionError(
                        error::http::Http2ErrorCode::INTERNAL_ERROR,
                        std::format("Failed to read DATA payload expected `{}`, got `{}` bytes",
                                    header.get_length(), view.size()),
                        get_stream_id());
                }
            }


            const auto &flags = header.get_flags();
            if (flags & shared_layer::Flags::END_STREAM) {
                core::logger::debug("http2/stream", "stream {} data {} bytes", get_stream_id(),
                                    view.size());
            } else {
                core::logger::debug("http2/stream", "stream {} data {} bytes (partial)",
                                    get_stream_id(), view.size());
            }

            break;
        }

        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION: {
            // All three feed the same accumulating header block buffer.
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

            // END_HEADERS closes the block out and triggers HPACK decode right away; otherwise
            // flag that more CONTINUATION frames are still expected.
            const auto &flags = header.get_flags();
            if (flags & shared_layer::Flags::END_HEADERS) {
                m_stream_helper.set_expecting_continuation(false);
                handle_header(view);

                core::logger::debug("http2/stream", "stream {} {} {} bytes", get_stream_id(), type,
                                    view.size());
            } else {
                m_stream_helper.set_expecting_continuation(true);

                core::logger::debug("http2/stream", "stream {} {} {} bytes (partial)",
                                    get_stream_id(), type, view.size());
            }

            break;
        }

        case shared_layer::FrameType::WINDOW_UPDATE: {
            // Peer's topping up how much we're allowed to send them.
            auto increment = reader | ReadWindowIncrementAdaptor{};

            core::logger::debug("http2/stream", "stream {} WINDOW_UPDATE increment={}",
                                get_stream_id(), increment);

            update_send_window(increment);

            break;
        }

        case shared_layer::FrameType::RST_STREAM: {
            // Peer's killing this stream, lowkey no warning — clean up immediately and surface it
            // as a StreamError so the session can respond with a targeted teardown instead of a
            // full GOAWAY.
            auto error_code = reader | std::views::take(4) | utils::codec::ReadBigEndianAdaptor<>{};

            cleanup_resources();

            throw error::http::StreamError(get_stream_id(),
                                           error::http::get_http2_error_code(error_code),
                                           "Stream reset via RST_STREAM frame");
        }

        case shared_layer::FrameType::PRIORITY: {
            // Same deprecated-frame rejection as the connection-level receiver.
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                "PRIORITY frames are not supported in this implementation - deprecated "
                "in HTTP/2 (RFC9113)");
        }

        case shared_layer::FrameType::SETTINGS: {
            // Connection-scoped only — never valid on a real per-request stream.
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                "SETTINGS frame are not valid on stream-level stream and can only "
                "be processed after the connection preface");
        }

        case shared_layer::FrameType::PING:
        case shared_layer::FrameType::GOAWAY: {
            // Also connection-scoped only.
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                std::format("Type `{}` is not valid on stream-level stream", type));
        }

        default:
            break;
        }

        // Consume the frame's payload bytes off the reader regardless of which branch ran.
        reader.consume(header.get_length());

        // Remote side just half-closed — flag it so Session::receive() knows to fire the
        // response dispatch.
        if (m_state_machine.get_state() == shared_layer::StreamState::HALF_CLOSED_REMOTE) {
            m_stream_helper.set_remote_done(true);
        }

        // Stream fully closed as a result of this frame — release its transient state.
        if (m_state_machine.is_closed()) {
            cleanup_resources();
        }
    }

    /**
     * @brief Fires once a full header block (HEADERS + any CONTINUATION frames) has arrived —
     * HPACK-decodes `view` straight into `m_request`/`m_response` via the wired-up `Hpack<>`
     * codec, then clears the header block so a second attempt on this stream gets rejected by
     * `get_header_block()`'s guard.
     * @warning If `view` is empty when this runs (an END_HEADERS-flagged frame with zero
     * payload bytes, first frame in the block), the entire body — decode AND
     * `clear_header_block()` — gets skipped by the `if (view.size() > 0)` guard. That leaves
     * the header block un-cleared even though END_HEADERS already fired, meaning a genuinely
     * illegal second HEADERS collection on this stream would slip right past
     * `get_header_block()`'s "already consumed" check instead of getting rejected. Edge case,
     * but a real gap in the double-collection guard.
     * @param view the accumulated header block bytes to decode.
     * @throws error::http::StreamError if HPACK decoding doesn't consume the entire payload, or
     * if decoding itself fails (compression error or a bad dynamic-table index).
     */
    void handle_header(utils::buffering::BufferView &view) {
        // Only decode if there's actually something buffered (see the class-level warning about
        // the empty-view edge case leaving the header block un-cleared).
        if (!view.empty()) {
            try {
                // Full payload must decode in one go — a partial consume means the compressed
                // representation was somehow malformed.
                if (auto consumed = m_stream_helper.get_hpack().decode(view);
                    consumed < view.size()) {
                    throw error::http::StreamError(
                        get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                        std::format("HPACK decoding did not consume entire payload: "
                                    "consumed `{}` bytes but payload size is `{}` bytes",
                                    consumed, view.size()));
                }
            } catch (error::http::Http2Exception &e) {
                // Generic HPACK decode failure — clear the block before rethrowing as a
                // StreamError so this stream's state doesn't linger half-decoded.
                m_stream_helper.clear_header_block();
                throw error::http::StreamError(get_stream_id(),
                                               error::http::Http2ErrorCode::COMPRESSION_ERROR,
                                               std::format("HPACK decoding error `{}`", e.what()));
            } catch (std::out_of_range &e) {
                // Bad dynamic-table index specifically — same cleanup-then-rethrow shape.
                m_stream_helper.clear_header_block();
                throw error::http::StreamError(
                    get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                    std::format("HPACK table index out of range: `{}`", e.what()));
            }
            // Clean decode — release the header block so a genuine second HEADERS collection
            // gets rejected by get_header_block()'s guard.
            m_stream_helper.clear_header_block();
        }
    }

    /**
     * @brief Stream-level flow-control accounting — debits `size` bytes off the send or recv
     * window, and bubbles the same debit up to the connection-level stream since HTTP/2 flow
     * control is two-tiered (both a stream window AND a connection-wide window have to have
     * room).
     * @note Only enabled `requires(IsStreamBased)` — the connection-level overload right below
     * handles `IsStreamBased = false` and doesn't bubble anywhere further up (it IS the top).
     * @warning Call order matters here: the connection-level debit happens *before* this
     * stream's own send-window check. If the connection-level `consume_window()` throws (it
     * shares this same window-violation logic), this stream's window never gets touched at all
     * — the two windows can only ever be checked in that fixed order, not independently.
     * @param size the byte count to debit from the window.
     * @param is_sender true to debit the send window (our outgoing data), false to debit the
     * recv window (data received from the peer).
     * @throws error::http::StreamError if debiting the send window would exceed what's
     * available, or if debiting the recv window drives it negative.
     */
    void consume_window(std::int32_t size, bool is_sender)
        requires IsStreamBased
    {
        core::logger::debug("http2/stream", "stream {} consume window size={} sender={}",
                            get_stream_id(), size, is_sender);

        // Connection-level window gets debited first — if that throws, this stream's own
        // window never gets touched (fixed check order, not independent).
        m_stream_helper.get_connection_stream().get().consume_window(size, is_sender);

        // Sender path — reject if this would exceed what we're still allowed to send.
        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(
                    get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                    "Attempted to send DATA exceeding flow control window");
            }
            core::logger::debug("http2/stream", "stream {} send_window -{} ={}", get_stream_id(),
                                size, m_send_window - size);

            m_send_window -= size;
        } else {
            // Receiver path — debit first, then check for underflow after the fact.
            m_recv_window -= size;
            core::logger::debug("http2/stream", "stream {} recv_window -{} ={}", get_stream_id(),
                                size, m_recv_window);

            if (m_recv_window < 0) {
                throw error::http::StreamError(get_stream_id(),
                                               error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                               "Receive window underflow");
            }
        }
    }


    /**
     * @brief Connection-level flow-control accounting — same debit logic as the stream-level
     * overload above, but this is the top of the chain, nothing further to bubble up to.
     * @note Only enabled `requires(!IsStreamBased)`.
     * @param size the byte count to debit from the window.
     * @param is_sender true to debit the send window, false to debit the recv window.
     * @throws error::http::StreamError if debiting the send window would exceed what's
     * available, or if debiting the recv window drives it negative.
     */
    void consume_window(std::int32_t size, bool is_sender)
        requires(!IsStreamBased)
    {
        core::logger::debug("http2/conn", "consume window size={} sender={}", size, is_sender);

        // Same sender-vs-receiver split as the stream-level overload, just with nothing
        // further to bubble up to since this IS the top of the chain.
        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(
                    get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                    "Attempted to send DATA exceeding flow control window");
            }

            core::logger::debug("http2/conn", "send_window -{} ={}", size, m_send_window - size);

            m_send_window -= size;
        } else {
            m_recv_window -= size;

            core::logger::debug("http2/conn", "recv_window -{} ={}", size, m_recv_window);

            if (m_recv_window < 0) {
                throw error::http::StreamError(get_stream_id(),
                                               error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                               "Receive window underflow");
            }
        }
    }


    /**
     * @brief Checks both preconditions for sending `size` bytes of DATA — legal stream state
     * (per `StreamStateMachine::can_send_data()`) AND enough send-window room. Both gotta be
     * true, no half-measures.
     * @param size the byte count you're asking about sending.
     * @return true if both the state machine and the send window allow it.
     */
    [[nodiscard]] bool can_send_data_of_size(std::int32_t size) const noexcept {
        return m_state_machine.can_send_data() && (m_send_window >= size);
    }

    /**
     * @brief Applies a WINDOW_UPDATE increment to the send window, propagating it down to the
     * connection-level stream too when this is a stream-based instance.
     * @note `increment` is unsigned, so the `<= 0` check below is really just an `== 0` check —
     * there's no way for an unsigned value to go negative, that's not a bug, just worth not
     * misreading the comparison as a sign check.
     * @param increment the window increment to apply. Must be nonzero.
     * @throws error::http::ConnectionError if `increment` is 0.
     * @throws error::http::StreamError if applying `increment` would push the send window past
     * `MAX_INITIAL_WINDOW_SIZE` (2^31-1).
     */
    void update_send_window(const std::uint32_t &increment) {
        // Zero increment is explicitly illegal per spec.
        if (increment <= 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Window update increment must be positive",
                                               get_stream_id());
        }

        // Reject if applying it would blow past the max legal window size.
        if (m_send_window + increment > MAX_INITIAL_WINDOW_SIZE) {
            throw error::http::StreamError(get_stream_id(),
                                           error::http::Http2ErrorCode::FLOW_CONTROL_ERROR,
                                           "Flow control window overflow");
        }

        core::logger::debug("http2/stream", "stream {} send_window +{} ={}", get_stream_id(),
                            increment, m_send_window + increment);

        m_send_window += increment;
        // Stream-based instances also propagate the same increment down to the connection
        // stream so both tiers of flow control stay in sync.
        if constexpr (IsStreamBased) {
            m_stream_helper.get_connection_stream().get().update_send_window(increment);
        }
    }

    /**
     * @brief Appends received bytes to `m_recv_buffer`, gated on the stream actually being in a
     * state where receiving data is legal.
     * @note `m_recv_buffer` here is separate from the request body view populated in the DATA
     * case of the stream-level `receive()` above (`m_stream_helper.get_request().get_body()`)
     * — the actual DATA-frame handling path reads straight into the request body via
     * `reader.expand_view()`, it never calls this method. Grep confirms zero call sites for
     * this method anywhere in the codebase — `m_recv_buffer` looks like a secondary/legacy
     * accumulation path that isn't wired into the live receive flow at all.
     * @param data the bytes to append.
     * @throws error::http::StreamError if the current state doesn't permit receiving data (see
     * `StreamStateMachine::can_receive_data()`).
     */
    void append_received_data(std::span<const std::byte> data) {
        // Guard — only append while the state machine says receiving is still legal here.
        if (!m_state_machine.can_receive_data()) {
            throw error::http::StreamError(get_stream_id(),
                                           error::http::Http2ErrorCode::STREAM_CLOSED,
                                           "Received data on stream not in receiving state");
        }
        core::logger::debug("http2/stream", "stream {} append {} bytes", get_stream_id(),
                            data.size());

        m_recv_buffer.insert(m_recv_buffer.end(), data.begin(), data.end());
    }

    /**
     * @brief Checks whether the remote side has finished sending on this stream — thin
     * pass-through to the stream helper's flag.
     * @note Only enabled `requires(IsStreamBased)` — the connection-level stream doesn't track
     * this concept.
     * @return true if the remote side has sent END_STREAM.
     */
    [[nodiscard]] bool is_remote_done() const noexcept
        requires IsStreamBased
    {
        return m_stream_helper.get_is_remote_done();
    }

    /**
     * @brief Grabs mutable access to this stream's request.
     * @note Only enabled `requires(IsStreamBased)`.
     * @return the request.
     */
    [[nodiscard]] HttpRequest &get_request() noexcept
        requires IsStreamBased
    {
        return m_stream_helper.get_request();
    }
    /**
     * @brief Grabs mutable access to this stream's response.
     * @note Only enabled `requires(IsStreamBased)`.
     * @return the response.
     */
    [[nodiscard]] HttpResponse &get_response() noexcept
        requires IsStreamBased
    {
        return m_stream_helper.get_response();
    }

    /**
     * @brief Advances the state machine for a frame *we're* about to send, cleaning up if that
     * lands the stream in CLOSED.
     * @note Only enabled `requires(IsStreamBased)`.
     * @warning No cap, this is a real bug: `is_local` is hardcoded `false` here, but this
     * method is specifically for frames the local side is sending (that's the entire point of
     * the `_send` in the name, and the one call site — `Session::send()` — invokes this right
     * after building and shipping a HEADERS frame with END_STREAM). Per
     * `StreamStateMachine::advance()`'s OPEN-state logic, `is_local=false` on an END_STREAM
     * transition routes to HALF_CLOSED_REMOTE instead of HALF_CLOSED_LOCAL — meaning every
     * client-sent request through this path leaves the stream's tracked state saying "the
     * *peer* half-closed" when actually *we* did. That's inverted directionality baked into the
     * one call site that exists for this method. Straight L for anything downstream that reads
     * stream state to decide who's still allowed to send/receive.
     * @param type the frame type driving this transition.
     * @param flags the frame's raw flags byte.
     * @throws error::http::ConnectionError or error::http::StreamError per
     * `StreamStateMachine::advance()`'s usual rules.
     */
    void advance_send(const shared_layer::FrameType &type, const std::uint8_t &flags)
        requires IsStreamBased
    {
        // Drive the state machine forward for this outgoing frame...
        m_state_machine.advance(type, flags, false);
        // ...and release transient state immediately if that lands the stream in CLOSED.
        if (m_state_machine.is_closed()) {
            cleanup_resources();
        }
    }

    /**
     * @brief Checks whether this stream's receive window has drained past the halfway point
     * and could use a WINDOW_UPDATE to top it back up. Bet, standard flow-control housekeeping.
     * @return true if the recv window is under half the remote's initial window size.
     */
    [[nodiscard]] bool needs_recv_window_update() const {
        return m_recv_window < m_remote_settings.get().get_initial_window_size() / 2;
    }

    /**
     * @brief Computes how much to top the recv window back up to its full initial size.
     * @return the increment that would restore the recv window to the remote's initial window
     * size.
     */
    [[nodiscard]] std::uint32_t recv_window_increment() const {
        return static_cast<std::uint32_t>(m_remote_settings.get().get_initial_window_size() -
                                          m_recv_window);
    }

    /**
     * @brief Moves `m_recv_buffer` out to the caller, leaving it empty behind.
     * @note Pairs with `append_received_data()` — and shares the same "no live call sites"
     * status, grep turns up nothing calling this anywhere in the codebase either.
     * @return the accumulated received bytes, moved out.
     */
    std::vector<std::byte> take_received_data() { return std::move(m_recv_buffer); }

    /**
     * @brief Grabs the current send window.
     * @return the send window, in bytes (may be used for either DATA-sending eligibility
     * checks or just introspection).
     */
    [[nodiscard]] const std::int32_t &send_window() const noexcept { return m_send_window; }
    /**
     * @brief Grabs the current recv window.
     * @return the recv window, in bytes.
     */
    [[nodiscard]] const std::int32_t &recv_window() const noexcept { return m_recv_window; }
    /**
     * @brief Grabs this stream's id. Straightforward W of a getter.
     * @return the stream id.
     */
    [[nodiscard]] const std::uint32_t &get_stream_id() const noexcept {
        return m_state_machine.id();
    }
    /**
     * @brief Grabs the current stream state.
     * @return the stream's current state.
     */
    [[nodiscard]] const shared_layer::StreamState &get_state() const noexcept {
        return m_state_machine.get_state();
    }

  private:
    using StreamHelper =
        std::conditional_t<IsStreamBased, StreamLevelHelper, ConnectionLevelHelper>;

    /**
     * @brief Handles an incoming SETTINGS frame at the connection level — ACKs get their own
     * short-circuit path (marks local settings as acknowledged, nothing else to do), non-ACKs
     * get decoded, diffed for an INITIAL_WINDOW_SIZE change (which needs to ripple through to
     * every open stream's send window via `update_send_window()`), then get a SETTINGS ACK
     * built to send back.
     * @warning No cap, straight-up bug here: `reader | ReadSettingsAdaptor{}` decodes the
     * peer's SETTINGS values into a brand-new local `Settings settings` object — and that
     * object is never assigned back onto `m_remote_settings`, never read from again after the
     * one line that creates it. `old_window`/`new_window` are both read from
     * `m_remote_settings` before AND after that decode, and since nothing ever writes the
     * decoded values into `m_remote_settings`, `old_window == new_window` unconditionally. The
     * whole `if (old_window != new_window)` window-delta-propagation block is dead code that
     * can never fire, and — bigger picture — the peer's actual SETTINGS values (header table
     * size, max frame size, max concurrent streams, all of it) never make it into
     * `m_remote_settings` at all. Only `m_remote_settings`'s *state* flag gets updated to
     * ACKNOWLEDGED; every numeric field stays at whatever it was (spec defaults, since it's
     * never mutated elsewhere either). This is a real functional gap, not a style nit — the
     * remote's advertised settings are effectively ignored connection-wide.
     * @note Only enabled `requires(!IsStreamBased)` — SETTINGS is connection-scoped, never
     * valid on a real stream (see the stream-level `receive()` override which rejects it
     * outright).
     * @param header the SETTINGS frame's header.
     * @param reader the reader positioned at the SETTINGS payload.
     * @return a SETTINGS ACK to send back, or `std::nullopt` if the incoming frame was itself
     * already an ACK (nothing to reply to).
     * @throws error::http::ConnectionError if a non-ACK SETTINGS arrives after the initial
     * exchange already finished, or if decoding the payload fails (see `Settings::apply()`).
     */
    std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>>
    handle_settings(const FrameHeader<shared_layer::FrameRole::RECEIVER> &header,
                    utils::buffering::BufferReader &reader)
        requires(!IsStreamBased)
    {
        // ACK short-circuit — nothing to decode, just mark our own settings acknowledged.
        if ((header.get_flags() & shared_layer::Flags::ACK) != 0) {
            core::logger::debug("http2/conn", "SETTINGS ACK");

            m_local_settings.get().set_state(SettingsState::ACKNOWLEDGED);
            return std::nullopt;
        }

        // Guard — a second non-ACK SETTINGS after the initial exchange already finished is a
        // protocol violation.
        if (m_remote_settings.get().is_finished()) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Received SETTINGS after initial exchange");
        }

        std::uint32_t old_window = m_remote_settings.get().get_initial_window_size();

        // Decode the peer's SETTINGS payload (see class-level warning — the decoded values
        // never actually get written back onto m_remote_settings as written today).
        auto settings = reader | ReadSettingsAdaptor{};

        std::uint32_t new_window = m_remote_settings.get().get_initial_window_size();

        // If the initial window changed, ripple the delta through every open stream's send
        // window and record it for Session::receive() to apply once this gets ACKed downstream.
        if (old_window != new_window) {
            std::int32_t delta =
                static_cast<std::int32_t>(new_window) - static_cast<std::int32_t>(old_window);

            update_send_window(delta);

            m_remote_settings.get().set_delta_window_on_settings(delta);
        }

        // Mark acknowledged and build the SETTINGS ACK to send back.
        m_remote_settings.get().set_state(SettingsState::ACKNOWLEDGED);
        auto setting_ack = Settings::generate_ack();

        core::logger::debug("http2/conn", "SETTINGS applied window {}->{}  delta={}", old_window,
                            new_window, new_window - old_window);

        return std::make_optional(setting_ack);
    }

    /**
     * @brief Resets per-stream transient state — clears the (largely-unused, see
     * `append_received_data()`'s note) recv buffer, and for stream-based instances resets the
     * expecting-continuation flag so a fresh HEADERS block can start clean. Called from the
     * dtor and from both `receive()` overloads whenever the state machine lands on CLOSED.
     */
    void cleanup_resources() {
        core::logger::debug("http2/stream", "stream {} cleanup", get_stream_id());

        // Recv buffer clears unconditionally; the continuation flag only exists on
        // stream-based instances, so that reset is gated behind the same compile-time check.
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
