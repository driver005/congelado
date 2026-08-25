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
import core_events;
import core_logger;
import interfaces;
import utils_buffering;
import :extension;
import :settings;
import :frame;
import :helper;
import :request;
import :response;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

    /**
     * @brief Records that a GOAWAY was received off the wire, along with the peer's Last-Stream-ID
     * and error code parsed from its payload. The session reads this after receive() to run the
     * connection-level teardown (RFC 9113 §6.8).
     * @param last_stream_id the peer's highest possibly-processed stream id.
     * @param error_code the GOAWAY error code.
     */
    void set_goaway_received(std::uint32_t last_stream_id,
                             error::http::Http2ErrorCode error_code) noexcept {
        m_goaway_received = true;
        m_goaway_last_stream_id = last_stream_id;
        m_goaway_error_code = error_code;
    }

    /// @brief Whether a GOAWAY has been received on this connection.
    [[nodiscard]] bool got_goaway() const noexcept { return m_goaway_received; }
    /// @brief The Last-Stream-ID from the received GOAWAY's payload.
    [[nodiscard]] std::uint32_t goaway_last_stream_id() const noexcept {
        return m_goaway_last_stream_id;
    }
    /// @brief The error code from the received GOAWAY's payload.
    [[nodiscard]] error::http::Http2ErrorCode goaway_error_code() const noexcept {
        return m_goaway_error_code;
    }

  private:
    error::http::Http2ErrorCode m_connection_error_code;
    bool m_goaway_received{false};
    std::uint32_t m_goaway_last_stream_id{0};
    error::http::Http2ErrorCode m_goaway_error_code{error::http::Http2ErrorCode::NO_ERROR_CODE};
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
     * @param is_server true on a server session (inbound decodes into the request), false on a
     * client session (inbound decodes into the response).
     */
    StreamLevelHelper(const std::uint32_t STREAM_ID, Stream<false> &connection_stream,
                      std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
                      std::reference_wrapper<codec::hpack::HPackTable> encoding_table, bool is_server)
        : m_connection_stream{connection_stream}, m_request{STREAM_ID}, m_response{STREAM_ID},
          m_hpack{decoding_table, encoding_table, m_request, m_response, is_server},
          m_header_block{std::make_optional<utils::buffering::BufferView>()}, m_is_server{is_server} {
    }

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
     * @brief The trailers-aware counterpart to `get_header_block()`. The first HEADERS block
     * (before `m_primary_headers_done`) routes straight through to `get_header_block()` (same
     * single-collection guard). Once the primary headers are done, a further HEADERS block is
     * trailers, which accumulate into a separate lazily-created buffer — a legitimate second
     * collection, not a protocol error.
     * @return the live buffer HEADERS/CONTINUATION payload bytes should accumulate into.
     * @throws error::http::ConnectionError only via `get_header_block()` if the primary block
     * was already cleared but `m_primary_headers_done` wasn't set (the empty-primary edge case).
     */
    utils::buffering::BufferView &get_header_block_for_write() {
        if (m_primary_headers_done) {
            if (!m_trailer_block.has_value()) {
                m_trailer_block = std::make_optional<utils::buffering::BufferView>();
            }
            return m_trailer_block.value();
        }
        return get_header_block();
    }
    /**
     * @brief Drops the trailer block buffer, marking it consumed — mirrors
     * `clear_header_block()`'s role but for the trailers path.
     */
    void clear_trailer_block() noexcept { m_trailer_block.reset(); }
    /**
     * @brief Marks the primary (request/response) HEADERS block as decoded, so any further
     * HEADERS block on this stream is treated as trailers.
     */
    void mark_primary_headers_done() noexcept { m_primary_headers_done = true; }
    /**
     * @brief Whether the primary HEADERS block has already been decoded on this stream.
     * @return true once `mark_primary_headers_done()` has run.
     */
    [[nodiscard]] bool primary_headers_done() const noexcept { return m_primary_headers_done; }
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
    std::optional<utils::buffering::BufferView> m_trailer_block;
    bool m_expecting_continuation{false};
    bool m_remote_done{false};
    bool m_primary_headers_done{false};
    bool m_is_server;

  public:
    /**
     * @brief Whether this stream's session is server-side (drives inbound decode/body targets).
     * @return true on a server session, false on a client session.
     */
    [[nodiscard]] bool is_server() const noexcept { return m_is_server; }
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
     * @param is_server true on a server session (inbound decodes into the request), false on a
     * client session (inbound decodes into the response). Defaults to true.
     * @throws error::http::ConnectionError if `STREAM_ID`'s parity doesn't match
     * `is_client_initiated`.
     */
    Stream(const std::uint32_t STREAM_ID, Stream<false> &connection_stream,
           std::reference_wrapper<codec::hpack::HPackTable> decoding_table,
           std::reference_wrapper<codec::hpack::HPackTable> encoding_table,
           std::reference_wrapper<Settings> local_settings,
           std::reference_wrapper<Settings> remote_settings, bool is_client_initiated = true,
           bool is_server = true)
        requires IsStreamBased
        : m_state_machine{StreamStateMachine{STREAM_ID}},
          m_send_window{static_cast<std::int32_t>(remote_settings.get().get_initial_window_size())},
          m_recv_window{static_cast<std::int32_t>(remote_settings.get().get_initial_window_size())},
          m_local_settings{local_settings}, m_remote_settings{remote_settings},
          m_stream_helper{StreamLevelHelper{STREAM_ID, connection_stream, decoding_table,
                                            encoding_table, is_server}} {
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
    receive(const FrameHeader<Role> &header, utils::buffering::BufferReader &reader,
            HttpExtensionRegistry &extension_registry) {

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

            extension_registry.for_each([&](auto &extension) { extension->on_window_update(0, increment); });
            break;
        }
        case shared_layer::FrameType::GOAWAY: {
            // Validate before trusting the payload (stream 0, flags 0, length >= 8). A malformed
            // GOAWAY throws FRAME_SIZE_ERROR here → the session's catch closes the connection.
            header.validate();

            // Parse the payload: 4-byte Last-Stream-ID (reserved top bit masked off) + 4-byte
            // error code. Read 8 bytes out without consuming — the consume at the end of receive()
            // drops the whole payload (incl. any optional debug data).
            auto bytes = reader | std::views::take(8) | std::ranges::to<std::vector<std::byte>>();
            const std::uint32_t LAST_ID =
                (bytes | std::views::take(4) |
                 utils::codec::ReadBigEndianAdaptor<std::uint32_t>{}) &
                0x7FFFFFFFU;
            const auto CODE = static_cast<error::http::Http2ErrorCode>(
                bytes | std::views::drop(4) | std::views::take(4) |
                utils::codec::ReadBigEndianAdaptor<std::uint32_t>{});

            core::logger::debug("http2/conn", "GOAWAY last_stream={} code={}", LAST_ID,
                                std::to_underlying(CODE));

            // Record only — GOAWAY is one-way (RFC 9113 §6.8), so NO echo. The session reads this
            // after receive() to run the connection teardown (no new streams, prune streams past
            // LAST_ID, close once the survivors finish).
            m_remote_settings.get().set_last_stream_id(LAST_ID);
            m_stream_helper.set_goaway_received(LAST_ID, CODE);
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
                    core::events::publish("http2.stream.unsolicited_ping_ack");
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

            extension_registry.for_each(
                [&](auto &extension) { extension->on_ping((flags & shared_layer::Flags::ACK) != 0); });
            break;
        }

        case shared_layer::FrameType::SETTINGS: {
            // Delegate the whole SETTINGS dance (ACK short-circuit, decode, window delta,
            // reply ACK) to the dedicated helper.
            response = handle_settings(header, reader, extension_registry);
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
        default: {
            // Not one of the 10 RFC 9113 types — per §4.1, unknown frame types MUST be ignored.
            // Before discarding, hand the raw frame to every extension's on_frame_complete().
            if (extension_registry.has_extensions()) {
                utils::buffering::BufferView view;
                if (header.get_length() > 0) {
                    reader.grow_view(view, header.get_length());
                }
                auto payload = view | std::ranges::to<std::vector<std::byte>>();
                bool end_stream = (header.get_flags() & shared_layer::Flags::END_STREAM) != 0;
                extension_registry.for_each([&](auto &extension) {
                    extension->on_frame_complete(get_stream_id(), std::to_underlying(type),
                                                header.get_flags(), payload, end_stream);
                });
            }
            break;
        }
        }

        // grow_view() (if any branch used it) only referenced the payload; this is the single
        // point that actually removes it from the reader.
        reader.consume(header.get_length());

        return response;
    }

    /// @brief Whether a GOAWAY has been received on this connection (connection-level stream only).
    /// The session polls this after receive() to run RFC 9113 §6.8 teardown.
    [[nodiscard]] bool got_goaway() const noexcept
        requires(!IsStreamBased)
    {
        return m_stream_helper.got_goaway();
    }
    /// @brief The Last-Stream-ID from the received GOAWAY's payload.
    [[nodiscard]] std::uint32_t goaway_last_stream_id() const noexcept
        requires(!IsStreamBased)
    {
        return m_stream_helper.goaway_last_stream_id();
    }
    /// @brief The error code from the received GOAWAY's payload.
    [[nodiscard]] error::http::Http2ErrorCode goaway_error_code() const noexcept
        requires(!IsStreamBased)
    {
        return m_stream_helper.goaway_error_code();
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
    void receive(const FrameHeader<Role> &header, utils::buffering::BufferReader &reader,
                HttpExtensionRegistry &extension_registry) {

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
            // Debit both flow-control windows, then read the payload into the role-appropriate
            // body — request on a server, response on a client.
            auto &view = m_stream_helper.is_server() ? m_stream_helper.get_request().get_body()
                                                     : m_stream_helper.get_response().get_body();
            std::size_t size_before = view.size();
            if (header.get_length() > 0) {
                consume_window(header.get_length(), false);

                reader.grow_view(view, header.get_length());
                if (view.size() != size_before + header.get_length()) {
                    throw error::http::ConnectionError(
                        error::http::Http2ErrorCode::INTERNAL_ERROR,
                        std::format("Failed to read DATA payload expected `{}`, got `{}` bytes",
                                    header.get_length(), view.size() - size_before),
                        get_stream_id());
                }
            }

            const auto &flags = header.get_flags();
            bool end_stream = (flags & shared_layer::Flags::END_STREAM) != 0;

            // Hand this DATA frame's bytes to every extension. `size_before` isolates just this
            // frame's tail from the accumulated body.
            if (extension_registry.has_extensions()) {
                auto payload = view | std::views::drop(size_before) |
                               std::ranges::to<std::vector<std::byte>>();
                extension_registry.for_each([&](auto &extension) {
                    extension->on_frame_complete(get_stream_id(),
                                                std::to_underlying(shared_layer::FrameType::DATA),
                                                header.get_flags(), payload, end_stream);
                });
            }

            break;
        }

        case shared_layer::FrameType::HEADERS:
        case shared_layer::FrameType::PUSH_PROMISE:
        case shared_layer::FrameType::CONTINUATION: {
            // All three feed the same accumulating header block buffer — trailers-aware, so a
            // second HEADERS block (trailers) lands in a separate buffer instead of hitting the
            // ordinary single-collection guard.
            auto &view = m_stream_helper.get_header_block_for_write();
            if (header.get_length() > 0) {
                reader.grow_view(view, header.get_length());
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
                handle_header(view, extension_registry);

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

            extension_registry.for_each(
                [&](auto &extension) { extension->on_window_update(get_stream_id(), increment); });
            break;
        }

        case shared_layer::FrameType::RST_STREAM: {
            // Peer's killing this stream, lowkey no warning — clean up immediately and surface it
            // as a StreamError so the session can respond with a targeted teardown instead of a
            // full GOAWAY.
            auto error_code = reader | std::views::take(4) | utils::codec::ReadBigEndianAdaptor<>{};

            extension_registry.for_each(
                [&](auto &extension) { extension->on_stream_reset(get_stream_id(), error_code); });

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

        default: {
            // Not one of the 10 RFC 9113 types — per §4.1, unknown frame types MUST be ignored.
            // Before discarding, hand the raw frame to every extension's on_frame_complete().
            if (extension_registry.has_extensions()) {
                utils::buffering::BufferView view;
                if (header.get_length() > 0) {
                    reader.grow_view(view, header.get_length());
                }
                auto payload = view | std::ranges::to<std::vector<std::byte>>();
                bool end_stream = (header.get_flags() & shared_layer::Flags::END_STREAM) != 0;
                extension_registry.for_each([&](auto &extension) {
                    extension->on_frame_complete(get_stream_id(), std::to_underlying(type),
                                                header.get_flags(), payload, end_stream);
                });
            }
            break;
        }
        }

        // grow_view() (if any branch used it) only referenced the payload; this is the single
        // point that actually removes it from the reader.
        reader.consume(header.get_length());

        // Remote side just finished sending — flag it so Session::receive() fires the response
        // dispatch. Covers both HALF_CLOSED_REMOTE and CLOSED: a client that already half-closed
        // its local side (sent its request with END_STREAM) jumps straight to CLOSED on the
        // response's END_STREAM, never passing through HALF_CLOSED_REMOTE.
        if (m_state_machine.get_state() == shared_layer::StreamState::HALF_CLOSED_REMOTE ||
            m_state_machine.is_closed()) {
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
     * @note A second HEADERS block on the same stream (after `mark_primary_headers_done()`) is
     * treated as trailers: decoded into a throwaway request via the shared HPACK decoding table
     * (so dynamic-table state stays in sync regardless of which object the fields land in) and
     * handed to `IHttpExtension::on_trailers()`. The primary block decodes into `m_request` and
     * fires `on_request_incoming()`. Both paths fire `on_header_added()` per decoded field.
     * @param view the accumulated header block bytes to decode.
     * @param extension_registry the registry whose extensions get the per-field / request /
     * trailers hooks.
     * @throws error::http::StreamError if HPACK decoding doesn't consume the entire payload, or
     * if decoding itself fails (compression error or a bad dynamic-table index).
     */
    void handle_header(utils::buffering::BufferView &view,
                       HttpExtensionRegistry &extension_registry) {
        // Fire on_header_added() for every decoded field of `req` — visits the static/dynamic
        // HeaderEntry variant, resolving a static field's Token name back to its string.
        auto fire_headers = [&](interfaces::io::IRequest &req) {
            if (!extension_registry.has_extensions()) {
                return;
            }
            for (const auto &entry : req.get_headers()) {
                std::string_view header_name;
                std::string_view header_value;
                std::visit(
                    [&](const auto &field) {
                        using NameType = std::decay_t<decltype(field->get_name())>;
                        if constexpr (std::is_same_v<NameType, interfaces::io::types::Token>) {
                            header_name = interfaces::io::types::token_to_string(field->get_name());
                        } else {
                            header_name = field->get_name();
                        }
                        header_value = field->get_value();
                    },
                    entry);
                extension_registry.for_each([&](auto &extension) {
                    extension->on_header_added(get_stream_id(), header_name, header_value);
                });
            }
        };

        if (m_stream_helper.primary_headers_done()) {
            // Trailers — decode into a throwaway request instead of m_request, reusing the same
            // shared decoding table so HPACK dynamic-table state stays in sync with the peer.
            if (!view.empty()) {
                HttpRequest trailers{get_stream_id()};
                try {
                    if (auto consumed = m_stream_helper.get_hpack().decode_into(trailers, view);
                        consumed < view.size()) {
                        throw error::http::StreamError(
                            get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                            std::format("HPACK decoding did not consume entire trailers payload: "
                                        "consumed `{}` bytes but payload size is `{}` bytes",
                                        consumed, view.size()));
                    }
                } catch (error::http::Http2Exception &e) {
                    m_stream_helper.clear_trailer_block();
                    throw error::http::StreamError(
                        get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                        std::format("HPACK decoding error `{}`", e.what()));
                } catch (std::out_of_range &e) {
                    m_stream_helper.clear_trailer_block();
                    throw error::http::StreamError(
                        get_stream_id(), error::http::Http2ErrorCode::COMPRESSION_ERROR,
                        std::format("HPACK table index out of range: `{}`", e.what()));
                }
                m_stream_helper.clear_trailer_block();

                fire_headers(trailers);
                extension_registry.for_each(
                    [&](auto &extension) { extension->on_trailers(get_stream_id(), trailers); });
            }
            return;
        }

        // Primary HEADERS block. Only decode if there's actually something buffered (see the
        // class-level warning about the empty-view edge case leaving the header block uncleared).
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
            // Clean decode — release the header block, and mark the primary block done so any
            // further HEADERS block routes to the trailers path above.
            m_stream_helper.clear_header_block();
            m_stream_helper.mark_primary_headers_done();

            // Request-side hooks only fire on a server — a client decoded the block into its
            // response, so there's no incoming request to announce.
            if (m_stream_helper.is_server()) {
                // Fire per-field, then the assembled-request hook.
                fire_headers(m_stream_helper.get_request());
                extension_registry.for_each([&](auto &extension) {
                    extension->on_request_incoming(get_stream_id(), m_stream_helper.get_request());
                });
            }
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
            m_send_window -= size;
        } else {
            // Receiver path — debit first, then check for underflow after the fact.
            m_recv_window -= size;
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
        // Same sender-vs-receiver split as the stream-level overload, just with nothing
        // further to bubble up to since this IS the top of the chain.
        if (is_sender) {
            if (m_send_window < size) {
                throw error::http::StreamError(
                    get_stream_id(), error::http::Http2ErrorCode::INTERNAL_ERROR,
                    "Attempted to send DATA exceeding flow control window");
            }

            m_send_window -= size;
        } else {
            m_recv_window -= size;

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
     * `reader.grow_view()`, it never calls this method. Grep confirms zero call sites for
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
    void advance_send(const shared_layer::FrameType &type, const std::uint8_t &flags,
                      bool is_local = false)
        requires IsStreamBased
    {
        // Drive the state machine forward for this outgoing frame...
        m_state_machine.advance(type, flags, is_local);
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
     * get decoded, written back onto `m_remote_settings` via `Settings::apply_all()`, diffed
     * for an INITIAL_WINDOW_SIZE change (which needs to ripple through to every open stream's
     * send window via `update_send_window()`), then get a SETTINGS ACK built to send back.
     * @note Only enabled `requires(!IsStreamBased)` — SETTINGS is connection-scoped, never
     * valid on a real stream (see the stream-level `receive()` override which rejects it
     * outright).
     * @param header the SETTINGS frame's header.
     * @param reader the reader positioned at the SETTINGS payload.
     * @param extension_registry the registry whose extensions get `on_settings_ack()` /
     * `on_remote_settings()`.
     * @return a SETTINGS ACK to send back, or `std::nullopt` if the incoming frame was itself
     * already an ACK (nothing to reply to).
     * @throws error::http::ConnectionError if a non-ACK SETTINGS arrives after the initial
     * exchange already finished, or if decoding the payload fails (see `Settings::apply()`).
     */
    std::optional<FrameBuilder<shared_layer::FrameRole::SENDER>>
    handle_settings(const FrameHeader<shared_layer::FrameRole::RECEIVER> &header,
                    utils::buffering::BufferReader &reader,
                    HttpExtensionRegistry &extension_registry)
        requires(!IsStreamBased)
    {
        // ACK short-circuit — nothing to decode, just mark our own settings acknowledged.
        if ((header.get_flags() & shared_layer::Flags::ACK) != 0) {
            core::logger::debug("http2/conn", "SETTINGS ACK");

            m_local_settings.get().set_state(SettingsState::ACKNOWLEDGED);

            extension_registry.for_each([](auto &extension) { extension->on_settings_ack(); });
            return std::nullopt;
        }

        // Guard — a second non-ACK SETTINGS after the initial exchange already finished is a
        // protocol violation.
        if (m_remote_settings.get().is_finished()) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Received SETTINGS after initial exchange");
        }

        std::uint32_t old_window = m_remote_settings.get().get_initial_window_size();

        // Decode the peer's SETTINGS payload and write the six negotiable fields back onto
        // m_remote_settings — apply_all() leaves lifecycle state (m_state, m_last_stream_id,
        // ping tracker, pending delta) untouched.
        auto settings = reader | ReadSettingsAdaptor{};
        m_remote_settings.get().apply_all(settings);

        // Let every extension observe the peer's applied settings — apply_all() carried the
        // vendor ids across too, so remote.get_vendor_settings() is populated.
        extension_registry.for_each(
            [&](auto &extension) { extension->on_remote_settings(m_remote_settings.get()); });

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

#ifdef CONGELADO_TEST
namespace io::layer::http2::stream_tests {
using namespace boost::ut;
using shared_layer::FrameRole;
using shared_layer::FrameType;
using shared_layer::Flags;
using shared_layer::StreamState;
using RxHeader = FrameHeader<FrameRole::RECEIVER>;

/// @brief Builds a BufferReader wrapping exactly `bytes` — the payload-only reader `receive()`
/// expects (it's positioned at the frame's payload, the 9-byte header is parsed elsewhere).
static utils::buffering::BufferReader make_reader(const std::vector<std::byte> &bytes) {
    auto *node = new utils::buffering::BufferNode(bytes.size());
    for (auto b : bytes) {
        node->push_back(b);
    }
    utils::buffering::BufferReader reader;
    reader.push_back(node);
    return reader;
}

/// @brief Big-endian-encodes a single uint32 into its own 4-byte payload — WINDOW_UPDATE/GOAWAY
/// shape.
static std::vector<std::byte> be32(std::uint32_t value) {
    return std::views::empty<std::byte> | utils::codec::WriteBigEndianAdaptor<std::uint32_t>{value} |
           std::ranges::to<std::vector<std::byte>>();
}

suite<"ConnectionLevelHelper"> connection_level_helper_suite = [] {
    "default ctor starts at NO_ERROR_CODE"_test = [] {
        ConnectionLevelHelper helper;
        expect(helper.get_connection_error_code() == error::http::Http2ErrorCode::NO_ERROR_CODE);
    };
    "explicit ctor stores the given error code"_test = [] {
        ConnectionLevelHelper helper{error::http::Http2ErrorCode::INTERNAL_ERROR};
        expect(helper.get_connection_error_code() == error::http::Http2ErrorCode::INTERNAL_ERROR);
    };
    "set_connection_error_code/get_connection_error_code round-trip"_test = [] {
        ConnectionLevelHelper helper;
        helper.set_connection_error_code(error::http::Http2ErrorCode::FLOW_CONTROL_ERROR);
        expect(helper.get_connection_error_code() == error::http::Http2ErrorCode::FLOW_CONTROL_ERROR);
    };
    "got_goaway starts false"_test = [] {
        ConnectionLevelHelper helper;
        expect(not helper.got_goaway());
    };
    "set_goaway_received populates last-stream-id/error-code and flips got_goaway"_test = [] {
        ConnectionLevelHelper helper;
        helper.set_goaway_received(7, error::http::Http2ErrorCode::CANCEL);

        expect(helper.got_goaway());
        expect(helper.goaway_last_stream_id() == 7U);
        expect(helper.goaway_error_code() == error::http::Http2ErrorCode::CANCEL);
    };
};

suite<"StreamLevelHelper"> stream_level_helper_suite = [] {
    "ctor wires the request/response to STREAM_ID and starts with every flag clear"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;

        StreamLevelHelper helper{9, conn, decoding, encoding, true};

        expect(helper.get_request().get_stream_id() == 9U);
        expect(helper.get_response().get_stream_id() == 9U);
        expect(helper.is_server());
        expect(not helper.get_expecting_continuation());
        expect(not helper.get_is_remote_done());
        expect(not helper.primary_headers_done());
    };
    "set_expecting_continuation/get_expecting_continuation round-trip"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        helper.set_expecting_continuation(true);
        expect(helper.get_expecting_continuation());
    };
    "set_remote_done/get_is_remote_done round-trip"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        helper.set_remote_done(true);
        expect(helper.get_is_remote_done());
    };
    "get_header_block is usable until clear_header_block, then throws on the next access"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        expect(nothrow([&] { std::ignore = helper.get_header_block(); }));
        helper.clear_header_block();
        expect(throws<error::http::ConnectionError>([&] { std::ignore = helper.get_header_block(); }));
    };
    "get_header_block_for_write routes to the primary block until marked done, then a fresh trailer block"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        auto &primary = helper.get_header_block_for_write();
        expect(primary.empty());

        helper.mark_primary_headers_done();
        expect(helper.primary_headers_done());

        auto &trailer = helper.get_header_block_for_write();
        expect(trailer.empty());

        helper.clear_trailer_block();
        auto &fresh_trailer = helper.get_header_block_for_write();
        expect(fresh_trailer.empty());
    };
    "get_connection_stream returns a reference to the same connection-level stream"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        expect(&helper.get_connection_stream().get() == &conn);
    };
    "get_request/get_response give mutable access reflected on the same object"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        helper.get_request().set_header(interfaces::io::types::Token::METHOD, "POST");
        expect(helper.get_request().get_method() == "POST");
    };
    "get_hpack returns a usable codec bound to this stream's request/response"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper helper{1, conn, decoding, encoding, true};

        std::vector<std::byte> empty_block;
        expect(helper.get_hpack().decode(empty_block) == 0U);
    };
    "is_server reflects the constructor argument"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        StreamLevelHelper client_side{1, conn, decoding, encoding, false};

        expect(not client_side.is_server());
    };
};

suite<"Stream<false> (connection-level)"> connection_stream_suite = [] {
    "ctor pins stream id 0, starts IDLE, windows seeded from remote's initial window"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        expect(conn.get_stream_id() == 0U);
        expect(conn.get_state() == StreamState::IDLE);
        expect(conn.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE));
        expect(conn.recv_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE));
    };
    "receive() WINDOW_UPDATE bumps the send window and replies with nothing"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        auto reader = make_reader(be32(1000));
        RxHeader header{4, FrameType::WINDOW_UPDATE, 0, 0};
        auto response = conn.receive(header, reader, registry);

        expect(not response.has_value());
        expect(conn.send_window() ==
               static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 1000);
    };
    "receive() GOAWAY records last-stream-id/error-code and never echoes a reply"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        std::vector<std::byte> payload = be32(5);
        auto code_bytes = be32(static_cast<std::uint32_t>(std::to_underlying(
            error::http::Http2ErrorCode::NO_ERROR_CODE)));
        payload.insert(payload.end(), code_bytes.begin(), code_bytes.end());

        auto reader = make_reader(payload);
        RxHeader header{8, FrameType::GOAWAY, 0, 0};
        auto response = conn.receive(header, reader, registry);

        expect(not response.has_value());
        expect(conn.got_goaway());
        expect(conn.goaway_last_stream_id() == 5U);
        expect(conn.goaway_error_code() == error::http::Http2ErrorCode::NO_ERROR_CODE);
    };
    "receive() non-ACK PING replies with an ACK carrying the same payload"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        std::vector<std::byte> payload(8, std::byte{0xAB});
        auto reader = make_reader(payload);
        RxHeader header{8, FrameType::PING, 0, 0};
        auto response = conn.receive(header, reader, registry);

        expect(response.has_value()) << fatal;
        expect(response->get_type() == FrameType::PING);
        expect((response->get_flags() & Flags::ACK) != 0);
    };
    "receive() an unsolicited PING ACK does not throw and replies with nothing"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        std::vector<std::byte> payload(8, std::byte{0x00});
        auto reader = make_reader(payload);
        RxHeader header{8, FrameType::PING, Flags::ACK, 0};

        std::optional<FrameBuilder<FrameRole::SENDER>> response;
        expect(nothrow([&] { response = conn.receive(header, reader, registry); }));
        expect(not response.has_value());
    };
    "receive() non-ACK SETTINGS applies them and replies with a SETTINGS ACK"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        Settings incoming;
        incoming.apply(0x4, 100000); // INITIAL_WINDOW_SIZE
        auto payload =
            std::views::empty<std::byte> | WriteSettingsAdaptor{incoming} |
            std::ranges::to<std::vector<std::byte>>();

        auto reader = make_reader(payload);
        RxHeader header{static_cast<std::uint32_t>(payload.size()), FrameType::SETTINGS, 0, 0};
        auto response = conn.receive(header, reader, registry);

        expect(response.has_value()) << fatal;
        expect(response->get_type() == FrameType::SETTINGS);
        expect((response->get_flags() & Flags::ACK) != 0);
    };
    "receive() a SETTINGS ACK replies with nothing"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        utils::buffering::BufferReader reader;
        RxHeader header{0, FrameType::SETTINGS, Flags::ACK, 0};
        auto response = conn.receive(header, reader, registry);

        expect(not response.has_value());
    };
    "receive() PRIORITY at connection level throws (unsupported, deprecated in RFC 9113)"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        utils::buffering::BufferReader reader;
        RxHeader header{0, FrameType::PRIORITY, 0, 0};

        expect(throws<error::http::ConnectionError>(
            [&] { std::ignore = conn.receive(header, reader, registry); }));
    };
    "receive() a stream-scoped frame type (DATA) at connection level throws"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        utils::buffering::BufferReader reader;
        RxHeader header{0, FrameType::DATA, 0, 0};

        expect(throws<error::http::ConnectionError>(
            [&] { std::ignore = conn.receive(header, reader, registry); }));
    };
    "receive() an unrecognized frame type is ignored per RFC 9113 §4.1"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        HttpExtensionRegistry registry;

        utils::buffering::BufferReader reader;
        RxHeader header{0, static_cast<FrameType>(0xFF), 0, 0};

        std::optional<FrameBuilder<FrameRole::SENDER>> response;
        expect(nothrow([&] { response = conn.receive(header, reader, registry); }));
        expect(not response.has_value());
    };
    "can_send_data_of_size is always false — the connection-level state machine never leaves IDLE"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        expect(not conn.can_send_data_of_size(0));
    };
    "update_send_window adds the increment; zero increment throws; overflow throws"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        conn.update_send_window(500);
        expect(conn.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 500);

        expect(throws<error::http::ConnectionError>([&] { conn.update_send_window(0); }));
        expect(throws<error::http::StreamError>(
            [&] { conn.update_send_window(MAX_INITIAL_WINDOW_SIZE); }));
    };
    "append_received_data always throws on the connection-level stream — advance() never touches it"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        std::array<std::byte, 2> data{std::byte{1}, std::byte{2}};
        expect(throws<error::http::StreamError>([&] { conn.append_received_data(data); }));
    };
    "needs_recv_window_update/recv_window_increment reflect the current recv window"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        expect(not conn.needs_recv_window_update());
        expect(conn.recv_window_increment() == 0U);

        conn.consume_window(static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE / 2) + 2, false);
        expect(conn.needs_recv_window_update());
        expect(conn.recv_window_increment() == DEFAULT_INITIAL_WINDOW_SIZE / 2 + 2);
    };
    "take_received_data moves the (always-empty) recv buffer out"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        auto data = conn.take_received_data();
        expect(data.empty());
    };
};

suite<"Stream<> ctor — parity validation"> stream_ctor_suite = [] {
    "a client-initiated odd stream id constructs fine"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;

        Stream<> stream{3, conn, decoding, encoding, local, remote};

        expect(stream.get_stream_id() == 3U);
        expect(stream.get_state() == StreamState::IDLE);
        expect(stream.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE));
        expect(stream.recv_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE));
    };
    "a client-initiated even (nonzero) stream id throws PROTOCOL_ERROR"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;

        expect(throws<error::http::ConnectionError>([&] {
            Stream<> stream{4, conn, decoding, encoding, local, remote};
        }));
    };
    "stream id 0 with is_client_initiated=true is exempt from the parity check"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;

        expect(nothrow([&] {
            Stream<> stream{0, conn, decoding, encoding, local, remote};
        }));
    };
    "a server-initiated (pushed) odd stream id throws INTERNAL_ERROR"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;

        expect(throws<error::http::ConnectionError>([&] {
            Stream<> stream{5, conn, decoding, encoding, local, remote, false};
        }));
    };
    "a server-initiated (pushed) even stream id constructs fine"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;

        expect(nothrow([&] {
            Stream<> stream{6, conn, decoding, encoding, local, remote, false};
        }));
    };
};

suite<"Stream<> receive() — HEADERS/DATA/state transitions"> stream_receive_suite = [] {
    "HEADERS with END_HEADERS (no END_STREAM) decodes the request and opens the stream"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        auto reader = make_reader({std::byte{0x82}}); // Indexed static entry 2: :method GET
        RxHeader header{1, FrameType::HEADERS, Flags::END_HEADERS, 3};
        stream.receive(header, reader, registry);

        expect(stream.get_state() == StreamState::OPEN);
        expect(not stream.is_remote_done());
        expect(stream.get_request().get_method() == "GET");
    };
    "HEADERS with END_HEADERS+END_STREAM half-closes remote and marks the stream remote-done"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        auto reader = make_reader({std::byte{0x82}});
        RxHeader header{1, FrameType::HEADERS, Flags::END_HEADERS | Flags::END_STREAM, 3};
        stream.receive(header, reader, registry);

        expect(stream.get_state() == StreamState::HALF_CLOSED_REMOTE);
        expect(stream.is_remote_done());
    };
    "DATA on an OPEN stream appends to the request body and debits the recv window"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        auto headers_reader = make_reader({std::byte{0x82}});
        RxHeader headers{1, FrameType::HEADERS, Flags::END_HEADERS, 3};
        stream.receive(headers, headers_reader, registry);

        auto data_reader = make_reader({std::byte{'A'}, std::byte{'B'}, std::byte{'C'}});
        RxHeader data_header{3, FrameType::DATA, 0, 3};
        stream.receive(data_header, data_reader, registry);

        expect(stream.get_request().get_body().size() == 3U);
        expect(stream.recv_window() ==
               static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) - 3);
        expect(stream.get_state() == StreamState::OPEN);
    };
    "RST_STREAM on a still-IDLE stream is a ConnectionError, not a StreamError"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        auto reader = make_reader(be32(0));
        RxHeader header{4, FrameType::RST_STREAM, 0, 3};

        expect(throws<error::http::ConnectionError>(
            [&] { stream.receive(header, reader, registry); }));
    };
    "RST_STREAM on an OPEN stream throws StreamError and closes it"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        auto headers_reader = make_reader({std::byte{0x82}});
        RxHeader headers{1, FrameType::HEADERS, Flags::END_HEADERS, 3};
        stream.receive(headers, headers_reader, registry);

        auto reader = make_reader(be32(8)); // CANCEL
        RxHeader header{4, FrameType::RST_STREAM, 0, 3};

        expect(throws<error::http::StreamError>([&] { stream.receive(header, reader, registry); }));
        expect(stream.get_state() == StreamState::CLOSED);
    };
    "WINDOW_UPDATE bumps this stream's AND the connection stream's send window"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        // WINDOW_UPDATE is only valid on a stream already past IDLE (RFC 9113 §5.1) — open it
        // via HEADERS first, same as every other non-HEADERS-frame test in this suite.
        auto open_reader = make_reader({std::byte{0x82}});
        RxHeader open_header{1, FrameType::HEADERS, Flags::END_HEADERS, 3};
        stream.receive(open_header, open_reader, registry);

        auto reader = make_reader(be32(1000));
        RxHeader header{4, FrameType::WINDOW_UPDATE, 0, 3};
        stream.receive(header, reader, registry);

        expect(stream.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 1000);
        expect(conn.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 1000);
    };
    "PRIORITY/SETTINGS/PING/GOAWAY are all rejected at stream level"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        HttpExtensionRegistry registry;

        for (auto type : {FrameType::PRIORITY, FrameType::SETTINGS, FrameType::PING, FrameType::GOAWAY}) {
            Stream<> stream{3, conn, decoding, encoding, local, remote};
            utils::buffering::BufferReader reader;
            RxHeader header{0, type, 0, 3};

            expect(throws<error::http::ConnectionError>(
                [&] { stream.receive(header, reader, registry); }));
        }
    };
    "an unrecognized frame type at stream level is ignored, not rejected"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        // An unrecognized frame type is only "ignored" once past IDLE — handle_idle() (RFC 9113
        // §5.1) rejects everything but HEADERS/PUSH_PROMISE regardless of recognition, same
        // reasoning as the WINDOW_UPDATE test above.
        auto open_reader = make_reader({std::byte{0x82}});
        RxHeader open_header{1, FrameType::HEADERS, Flags::END_HEADERS, 3};
        stream.receive(open_header, open_reader, registry);

        utils::buffering::BufferReader reader;
        RxHeader header{0, static_cast<FrameType>(0xFF), 0, 3};

        expect(nothrow([&] { stream.receive(header, reader, registry); }));
    };
    "a non-CONTINUATION frame while expecting CONTINUATION is a ConnectionError"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        utils::buffering::BufferReader partial_reader;
        RxHeader partial{0, FrameType::HEADERS, 0, 3}; // no END_HEADERS
        stream.receive(partial, partial_reader, registry);

        utils::buffering::BufferReader reader = make_reader(be32(1));
        RxHeader header{4, FrameType::WINDOW_UPDATE, 0, 3};

        expect(throws<error::http::ConnectionError>(
            [&] { stream.receive(header, reader, registry); }));
    };
    "a HEADERS block split across CONTINUATION frames decodes once END_HEADERS lands"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        utils::buffering::BufferReader partial_reader;
        RxHeader partial{0, FrameType::HEADERS, 0, 3}; // no END_HEADERS, empty block so far
        stream.receive(partial, partial_reader, registry);

        auto continuation_reader = make_reader({std::byte{0x82}});
        RxHeader continuation{1, FrameType::CONTINUATION, Flags::END_HEADERS, 3};
        stream.receive(continuation, continuation_reader, registry);

        expect(stream.get_request().get_method() == "GET");
        expect(stream.get_state() == StreamState::OPEN);
    };
};

suite<"Stream<> flow control"> stream_flow_control_suite = [] {
    "can_send_data_of_size requires both an OPEN-ish state and enough send window"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        // Still IDLE — the state machine forbids sending regardless of window size.
        expect(not stream.can_send_data_of_size(0));

        stream.advance_send(FrameType::HEADERS, 0, true); // -> OPEN
        expect(stream.can_send_data_of_size(100));
        expect(not stream.can_send_data_of_size(
            static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 1));
    };
    "update_send_window propagates the increment down to the connection stream too"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        stream.update_send_window(250);

        expect(stream.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 250);
        expect(conn.send_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 250);
    };
    "update_send_window rejects a zero increment and an overflowing one"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        expect(throws<error::http::ConnectionError>([&] { stream.update_send_window(0); }));
        expect(throws<error::http::StreamError>(
            [&] { stream.update_send_window(MAX_INITIAL_WINDOW_SIZE); }));
    };
    "consume_window(sender) throws with the stream's own id once ITS window (not the connection's) is exceeded"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        // Give the connection far more headroom than the stream's own send window, so it's the
        // stream-level check — not the connection-level one, which runs first — that actually
        // trips here.
        conn.update_send_window(1'000'000);

        try {
            stream.consume_window(static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 1, true);
            expect(false) << "expected a StreamError";
        } catch (const error::http::StreamError &e) {
            expect(e.get_stream_id() == 3U);
            expect(e.get_code() == error::http::Http2ErrorCode::INTERNAL_ERROR);
        }
    };
    "consume_window(receiver) debits both this stream's and the connection's recv window in lockstep, and throws on underflow"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        stream.consume_window(100, false);
        expect(stream.recv_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) - 100);
        expect(conn.recv_window() == static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) - 100);

        // The connection-level debit always runs first (see the class docs' "call order matters"
        // warning), so once the shared window underflows it's the connection-level check that
        // trips — same FLOW_CONTROL_ERROR shape the security suite's connection-level pin covers
        // below, just reached this time via the stream-level entry point.
        expect(throws<error::http::StreamError>([&] {
            stream.consume_window(static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE), false);
        }));
    };
    "needs_recv_window_update/recv_window_increment reflect the current recv window"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        expect(not stream.needs_recv_window_update());

        stream.consume_window(static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE / 2) + 2, false);
        expect(stream.needs_recv_window_update());
        expect(stream.recv_window_increment() == DEFAULT_INITIAL_WINDOW_SIZE / 2 + 2);
    };
};

suite<"Stream<> received-data buffer + cleanup"> stream_recv_buffer_suite = [] {
    "append_received_data throws while IDLE, succeeds once OPEN, and take_received_data drains it"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        std::array<std::byte, 2> data{std::byte{1}, std::byte{2}};
        expect(throws<error::http::StreamError>([&] { stream.append_received_data(data); }));

        stream.advance_send(FrameType::HEADERS, 0, true); // -> OPEN
        expect(nothrow([&] { stream.append_received_data(data); }));

        auto drained = stream.take_received_data();
        expect(drained.size() == 2U);
        expect(stream.take_received_data().empty());
    };
    "closing the stream via receive() runs cleanup_resources(), clearing any buffered recv data"_test =
        [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        stream.advance_send(FrameType::HEADERS, 0, true); // -> OPEN
        std::array<std::byte, 2> data{std::byte{9}, std::byte{8}};
        stream.append_received_data(data);

        stream.advance_send(FrameType::DATA, Flags::END_STREAM, true); // -> HALF_CLOSED_LOCAL

        utils::buffering::BufferReader reader;
        RxHeader header{0, FrameType::DATA, Flags::END_STREAM, 3};
        stream.receive(header, reader, registry); // peer ends too -> CLOSED, cleanup runs

        expect(stream.get_state() == StreamState::CLOSED);
        expect(stream.take_received_data().empty());
    };
};

suite<"Stream<> advance_send"> stream_advance_send_suite = [] {
    "advance_send(is_local=true) with END_STREAM from IDLE half-closes LOCAL"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        stream.advance_send(FrameType::HEADERS, Flags::END_STREAM, true);
        expect(stream.get_state() == StreamState::HALF_CLOSED_LOCAL);
    };
    "advance_send with its default is_local=false and END_STREAM half-closes REMOTE"_test = [] {
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};

        stream.advance_send(FrameType::HEADERS, Flags::END_STREAM);
        expect(stream.get_state() == StreamState::HALF_CLOSED_REMOTE);
    };
};

suite<"Stream<> security/regression pins — document current (not-yet-fixed) behavior"> stream_security_suite =
    [] {
    "1) a malformed HPACK header block throws DecodeError straight out of receive(), uncaught"_test =
        [] {
        // stream.cppm's own catches (see handle_header()) only trap error::http::Http2Exception
        // and std::out_of_range — DecodeError is a SIBLING hierarchy (both derive std::runtime_error
        // independently), so it is never caught here and propagates all the way out of receive().
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        // 0x80 = Indexed Header Field representation with index 0 — always invalid (RFC 7541
        // §6.1), decoded via push_helper() which throws InvalidIndexError<std::uint32_t>.
        auto reader = make_reader({std::byte{0x80}});
        RxHeader header{1, FrameType::HEADERS, Flags::END_HEADERS, 3};

        expect(throws<error::http::InvalidIndexError<std::uint32_t>>(
            [&] { stream.receive(header, reader, registry); }));
        // Also true via the shared base — same exception, checked through its DecodeError base.
        auto reader2 = make_reader({std::byte{0x80}});
        Stream<> stream2{5, conn, decoding, encoding, local, remote};
        expect(throws<error::http::DecodeError>(
            [&] { stream2.receive(header, reader2, registry); }));
    };
    "2) PADDED-flag DATA is never unpadded — validate_padding() is dead code, pad byte + padding "
    "leak straight into the body"_test = [] {
        // Confirmed by grep: FrameHeader::validate_padding() has exactly one call site in the whole
        // codebase (its own unit test in frame.cppm) — receive()'s DATA case below never calls it.
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};
        codec::hpack::HPackTable decoding;
        codec::hpack::HPackTable encoding;
        Stream<> stream{3, conn, decoding, encoding, local, remote};
        HttpExtensionRegistry registry;

        auto headers_reader = make_reader({std::byte{0x82}});
        RxHeader headers{1, FrameType::HEADERS, Flags::END_HEADERS, 3};
        stream.receive(headers, headers_reader, registry);

        // PADDED DATA shape (frame.cppm's validate_padding()): [pad-length][payload...][padding...].
        // pad-length=2, payload="AB", 2 zero padding bytes -> 5 total declared bytes.
        auto reader = make_reader(
            {std::byte{0x02}, std::byte{'A'}, std::byte{'B'}, std::byte{0x00}, std::byte{0x00}});
        RxHeader data_header{5, FrameType::DATA, Flags::PADDED, 3};
        stream.receive(data_header, reader, registry);

        auto &body = stream.get_request().get_body();
        // Current (buggy) behavior: all 5 bytes, pad-length prefix and padding included, land
        // straight in the body — nothing strips them.
        expect(body.size() == 5U) << fatal;
        auto it = body.begin();
        expect(*it == std::byte{0x02}); // the pad-length byte itself, unstripped
    };
    "3) a connection-level recv-window violation throws StreamError{stream_id=0}, not ConnectionError "
    "(RFC 9113 §6.9.1 gap)"_test = [] {
        // consume_window()'s !IsStreamBased overload (the connection-level top of the chain) reuses
        // the exact same StreamError-raising logic as the per-stream overload — even though a
        // connection-level flow-control violation is conceptually connection-wide and RFC 9113
        // §6.9.1 says it SHOULD be reported as a ConnectionError instead.
        Settings local;
        Settings remote;
        Stream<false> conn{local, remote};

        try {
            conn.consume_window(static_cast<std::int32_t>(DEFAULT_INITIAL_WINDOW_SIZE) + 1, false);
            expect(false) << "expected a StreamError (documenting the current, non-conformant type)";
        } catch (const error::http::StreamError &e) {
            expect(e.get_stream_id() == 0U);
            expect(e.get_code() == error::http::Http2ErrorCode::FLOW_CONTROL_ERROR);
        } catch (...) {
            expect(false) << "threw something other than StreamError";
        }
    };
};

} // namespace io::layer::http2::stream_tests
#endif
