module;
#include <cstddef>
#include <iterator>
#include <print>
#include <ranges>
#include <utility>
export module io_layer_http2:frame;

import std;
import io_layer_shared;
import io_error;
import core_logger;
import shared;
import utils_codec;
import :consts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace io::layer::http2 {

template<shared_layer::FrameRole Role>
class FrameHeader
{
public:
    /**
     * @brief Default ctor — everything zeroed, type defaults to DATA. Not a valid header on its
     * own, just a starting point for the builder-style `add_*` chain.
     */
    FrameHeader() :
        m_length{0},
        m_type{shared_layer::FrameType::DATA},
        m_flags{0},
        m_stream_id{0}
    {
    }

    /**
     * @brief Builds a fully-populated header in one shot.
     * @param length the frame payload length.
     * @param type the frame type.
     * @param flags the frame flags byte.
     * @param stream_id the stream id — gets run through set_stream_id() so the reserved top bit
     * is masked off no matter what garbage bit you pass in here.
     */
    FrameHeader(
        std::uint32_t length,
        shared_layer::FrameType type,
        std::uint8_t flags,
        std::uint32_t stream_id
    ) :
        m_length{length},
        m_type{type},
        m_flags{flags},
        m_stream_id{0}
    {
        set_stream_id(stream_id);
    }

    /**
     * @brief Builder chain — sets the payload length. Bet, nothing fancy going on here.
     * @param len the length to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameHeader&& add_length(std::uint32_t len) && noexcept
    {
        core::logger::debug("FrameHeader", "len={}", len);

        m_length = len;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the frame type.
     * @param type the type to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameHeader&& add_type(shared_layer::FrameType type) && noexcept
    {
        core::logger::debug("FrameHeader", "type={}", type);

        m_type = type;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the raw flags byte.
     * @param flags the flags to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameHeader&& add_flags(std::uint8_t flags) && noexcept
    {
        core::logger::debug("FrameHeader", "flags={}", flags);

        m_flags = flags;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the stream id, masked through set_stream_id().
     * @param stream_id the stream id to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameHeader&& add_stream_id(std::uint32_t stream_id) && noexcept
    {
        core::logger::debug("FrameHeader", "stream_id={}", stream_id);

        set_stream_id(stream_id);
        return std::move(*this);
    }

    /**
     * @brief Dispatches to the right per-type `validate_*()` based on `m_type`, plus one
     * cross-cutting check up front: a SENDER building a PUSH_PROMISE must use an even stream id
     * (server push convention), checked before the switch even runs.
     * @warning This is the RFC 9113 conformance gate — every frame type has its own stream-id-
     * zero rule, flag-mask rule, and length rule, and they're all enforced in the private
     * `validate_*()` methods this fans out to. Skip calling this and you're trusting untrusted
     * wire data raw, which is a straight L waiting to happen.
     * @warning No cap, this method has zero call sites anywhere in the codebase as of this
     * comment pass — grep confirms it. Every private `validate_*()` here has real, correct
     * logic behind it, but none of it currently runs against real wire input through this
     * entry point. Wire-format validation may be happening some other way (or not at all)
     * upstream in the actual receive path — flagging it since that's a genuinely surprising gap
     * for HTTP/2 framing code to have, not something to quietly assume is fine.
     * @throws error::http::ConnectionError if a SENDER PUSH_PROMISE has an odd stream id, or if
     * the dispatched-to validator throws (see each `validate_*()` for its specific rules).
     */
    void validate() const
    {
        core::logger::debug("FrameBuilder", "validate type={} stream={}", m_type, m_stream_id);

        // Cross-cutting check that lives outside the per-type switch below — server push over
        // an odd stream id is illegal regardless of which validate_* would otherwise run.
        if constexpr (Role == shared_layer::FrameRole::SENDER) {
            if (m_stream_id % 2 != 0) {
                if (m_type == shared_layer::FrameType::PUSH_PROMISE) {
                    throw error::http::ConnectionError(
                        error::http::Http2ErrorCode::INTERNAL_ERROR,
                        "Server-initiated PUSH_PROMISE must use even stream ID"
                    );
                }
            }
        }

        // Fan out to the type-specific validator, bet — everything else is a no-op default.
        switch (m_type) {
            case shared_layer::FrameType::DATA:
                validate_data();
                break;
            case shared_layer::FrameType::HEADERS:
                validate_headers();
                break;
            case shared_layer::FrameType::PRIORITY:
                validate_priority();
                break;
            case shared_layer::FrameType::RST_STREAM:
                validate_rst_stream();
                break;
            case shared_layer::FrameType::SETTINGS:
                validate_settings();
                break;
            case shared_layer::FrameType::PUSH_PROMISE:
                validate_push_promise();
                break;
            case shared_layer::FrameType::PING:
                validate_ping();
                break;
            case shared_layer::FrameType::GOAWAY:
                validate_goaway();
                break;
            case shared_layer::FrameType::WINDOW_UPDATE:
                validate_window_update();
                break;
            case shared_layer::FrameType::CONTINUATION:
                validate_continuation();
                break;
            default:
                break;
        }
    }

    /**
     * @brief Confirms the bytes actually read off the wire match this header's declared
     * `m_length`. Straight sanity check between what the header promised and what showed up.
     * @param actual_size the number of payload bytes actually present.
     * @throws error::http::ConnectionError if `actual_size` doesn't equal `m_length`.
     */
    void validate_payload_size(std::size_t actual_size) const
    {
        if (actual_size != m_length) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::INTERNAL_ERROR,
                std::format(
                    "Payload size mismatch for frame type {}: expected {}, got {}",
                    std::to_underlying(m_type), m_length, actual_size
                ),
                get_stream_id()
            );
        }
    }

    /**
     * @brief Guards against a padding length that's >= the entire frame length — that would
     * leave zero or negative room for actual payload, which is cooked, not a valid frame.
     * @param actual_size the declared pad length pulled off the wire.
     * @throws error::http::ConnectionError if `actual_size` is not strictly less than
     * `m_length`.
     */
    void validate_padding(std::uint32_t actual_size) const
    {
        if (actual_size >= m_length) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Padding exceeds frame ",
                get_stream_id()
            );
        }
    }

    /**
     * @brief Checks the END_STREAM flag bit.
     * @return true if END_STREAM is set on this frame.
     */
    [[nodiscard]] bool is_end_stream() const noexcept
    {
        return (m_flags & shared_layer::Flags::END_STREAM) != 0;
    }

    /**
     * @brief Checks the PADDED flag bit.
     * @return true if PADDED is set on this frame.
     */
    [[nodiscard]] bool is_padded() const noexcept
    {
        return (m_flags & shared_layer::Flags::PADDED) != 0;
    }

    /**
     * @brief Fixed HTTP/2 frame header wire size — always 9 bytes, lowkey doesn't depend on
     * frame type or payload length at all.
     * @return `HEADER_SIZE` (9).
     */
    [[nodiscard]] constexpr std::size_t get_header_length() const noexcept
    {
        return HEADER_SIZE;
    }

    /**
     * @brief Full on-wire size of this frame — the fixed 9-byte header plus however much
     * payload it declares. Combines `get_header_length()` and `get_length()`.
     * @return `get_header_length() + get_length()`.
     */
    [[nodiscard]] std::size_t get_size() const noexcept
    {
        return get_header_length() + m_length;
    }

    /**
     * @brief Grabs the declared payload length.
     * @return the payload length in bytes.
     */
    [[nodiscard]] const std::uint32_t& get_length() const noexcept
    {
        return m_length;
    }

    /**
     * @brief Grabs the frame type.
     * @return the frame type.
     */
    [[nodiscard]] const shared_layer::FrameType& get_type() const noexcept
    {
        return m_type;
    }

    /**
     * @brief Grabs the raw flags byte.
     * @return the flags byte.
     */
    [[nodiscard]] const std::uint8_t& get_flags() const noexcept
    {
        return m_flags;
    }

    /**
     * @brief Grabs the stream id.
     * @return the stream id, already masked to 31 bits.
     */
    [[nodiscard]] const std::uint32_t& get_stream_id() const noexcept
    {
        return m_stream_id;
    }

    /**
     * @brief Sets the payload length directly on the member, no validation run here — that's
     * validate()'s job, not this.
     * @param len the length to store.
     */
    void set_length(std::uint32_t len) noexcept
    {
        m_length = len;
    }

    /**
     * @brief Sets the frame type directly on the member.
     * @param type the type to store.
     */
    void set_type(shared_layer::FrameType type) noexcept
    {
        m_type = type;
    }

    /**
     * @brief Sets the raw flags byte directly on the member.
     * @param flags the flags to store.
     */
    void set_flags(std::uint8_t flags) noexcept
    {
        m_flags = flags;
    }

    /**
     * @brief Sets the stream id, masking off the reserved high bit (RFC 9113 §4.1 — that bit
     * must be ignored on receipt and MUST NOT be set on send). Whatever you pass in, only the
     * low 31 bits survive.
     * @param new_id the raw stream id to store, high bit gets clipped regardless of value.
     */
    void set_stream_id(std::uint32_t new_id) noexcept
    {
        m_stream_id = new_id & 0x7F'FF'FF'FF;
    }

private:
    /**
     * @brief Enforces DATA frame rules: never on stream 0, flags limited to
     * END_STREAM|PADDED, and if PADDED is set there's gotta be at least 1 byte of payload
     * (the pad-length prefix octet itself needs somewhere to live).
     * @throws error::http::ConnectionError if stream id is 0 or unexpected flag bits are set.
     * @throws error::http::ConnectionError if PADDED is set but `m_length` is too short.
     */
    void validate_data() const
    {
        // DATA always belongs to a real stream, never the connection-level stream 0.
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "DATA on stream 0"
            );
        }
        // Only END_STREAM/PADDED are legal bits for this frame type.
        if ((m_flags & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::PADDED)) != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for DATA", m_stream_id
            );
        }

        // PADDED needs room for at least the pad-length prefix octet itself.
        if ((m_flags & shared_layer::Flags::PADDED) != 0) {
            if (m_length < 1) {
                throw error::http::ConnectionError(
                    error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "DATA too short for padding",
                    m_stream_id
                );
            }
        }
    }

    /**
     * @brief Enforces HEADERS frame rules: never on stream 0, flags limited to
     * END_STREAM|END_HEADERS|PADDED|PRIORITY, and a minimum length that scales with which of
     * those optional-field flags are actually set (1 byte for the pad-length prefix if PADDED,
     * 5 more for the priority fields if PRIORITY).
     * @warning There's a duplicate padding-too-short check in here copy-pasted straight from
     * validate_data() — same logic, same error message ("DATA too short for padding") even
     * though this is the HEADERS validator. Message is misleading but the check itself is
     * still doing real work, not touching it since this pass is comment-only.
     * @throws error::http::ConnectionError if stream id is 0, unexpected flags are set, or
     * `m_length` is below the computed minimum for the flags present.
     */
    void validate_headers() const
    {
        // HEADERS always belongs to a real stream, never stream 0.
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "HEADERS on stream 0"
            );
        }

        // Only these four bits are legal for HEADERS.
        if ((m_flags & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::END_HEADERS |
                         shared_layer::Flags::PADDED | shared_layer::Flags::PRIORITY)) != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for HEADERS",
                m_stream_id
            );
        }

        // Minimum length scales with which optional fields are actually present.
        std::uint32_t min_len = 0;
        if ((m_flags & shared_layer::Flags::PADDED) != 0) {
            min_len += 1;
        }
        if ((m_flags & shared_layer::Flags::PRIORITY) != 0) {
            min_len += 5;
        }

        if (m_length < min_len) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "HEADERS length too short",
                m_stream_id
            );
        }

        // Same padding-room check DATA does — needs at least the pad-length prefix octet.
        if ((m_flags & shared_layer::Flags::PADDED) != 0) {
            if (m_length < 1) {
                throw error::http::ConnectionError(
                    error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "DATA too short for padding",
                    m_stream_id
                );
            }
        }
    }

    /**
     * @brief Enforces PRIORITY frame rules: never on stream 0, zero flags, exactly 5 bytes of
     * payload (32-bit stream dependency + 8-bit weight).
     * @note Heads up though — `StreamStateMachine::advance()` over in helper.cppm treats
     * PRIORITY as a total no-op that doesn't even reach frame handling in practice, and the
     * connection/stream `receive()` paths in stream.cppm actively reject PRIORITY with a
     * "deprecated, not supported" error before this validator would ever run on real traffic.
     * This validation exists but may be effectively dead code on the actual receive path.
     * @throws error::http::ConnectionError if stream id is 0, flags are nonzero, or length
     * isn't exactly 5.
     */
    void validate_priority() const
    {
        // Stream id, flags, and length all get their own dedicated check — no shortcuts here.
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "PRIORITY on stream 0"
            );
        }
        if (m_flags != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "PRIORITY flags must be 0", m_stream_id
            );
        }
        if (m_length != 5) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "PRIORITY length must be 5",
                m_stream_id
            );
        }
    }

    /**
     * @brief Enforces RST_STREAM frame rules: never on stream 0, zero flags, exactly 4 bytes
     * of payload (32-bit error code). Straightforward, no cap.
     * @throws error::http::ConnectionError if stream id is 0, flags are nonzero, or length
     * isn't exactly 4.
     */
    void validate_rst_stream() const
    {
        // Same three-check shape as PRIORITY — stream, flags, length.
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "RST_STREAM on stream 0"
            );
        }
        if (m_flags != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "RST_STREAM flags must be 0",
                m_stream_id
            );
        }
        if (m_length != 4) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "RST_STREAM length must be 4",
                m_stream_id
            );
        }
    }

    /**
     * @brief Enforces SETTINGS frame rules: must be on stream 0, only the ACK flag is legal,
     * and length has to match the flag — an ACK carries zero payload (RFC 9113 §6.5.3, ACK is
     * empty by definition) while a non-ACK settings frame must be a multiple of 6 bytes (each
     * setting is a 2-byte id + 4-byte value pair).
     * @throws error::http::ConnectionError if not on stream 0, an invalid flag bit is set, an
     * ACK carries payload, or a non-ACK length isn't a multiple of 6.
     */
    void validate_settings() const
    {
        // SETTINGS is always connection-level, never tied to a specific stream.
        if (m_stream_id != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "SETTINGS not on stream 0"
            );
        }
        // Only ACK is a legal flag here.
        if ((m_flags & ~shared_layer::Flags::ACK) != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for SETTINGS"
            );
        }

        // ACK carries no payload at all; a real settings frame must be a whole number of
        // 6-byte (id + value) pairs.
        if ((m_flags & shared_layer::Flags::ACK) != 0) {
            if (m_length != 0) {
                throw error::http::ConnectionError(
                    error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "SETTINGS ACK with payload"
                );
            }
        } else if (m_length % 6 != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "SETTINGS size mismatch"
            );
        }
    }

    /**
     * @brief Enforces PUSH_PROMISE frame rules — and they're role-split. As a RECEIVER (i.e.
     * server side, since only servers push), getting a PUSH_PROMISE at all is an automatic
     * PROTOCOL_ERROR — clients don't push. As a SENDER, the usual deal applies: never on
     * stream 0, flags limited to END_HEADERS|PADDED.
     * @throws error::http::ConnectionError unconditionally if `Role` is RECEIVER.
     * @throws error::http::ConnectionError if `Role` is SENDER and stream id is 0 or unexpected
     * flags are set.
     */
    void validate_push_promise() const
    {
        if constexpr (Role == shared_layer::FrameRole::RECEIVER) {
            // A receiver (server) MUST treat receipt of PUSH_PROMISE as a connection error
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "PUSH_PROMISE received by server"
            );
        } else {
            // Sender validation for server PUSH_PROMISE
            if (m_stream_id == 0) {
                throw error::http::ConnectionError(
                    error::http::Http2ErrorCode::PROTOCOL_ERROR, "PUSH_PROMISE on stream 0"
                );
            }
            if ((m_flags & ~(shared_layer::Flags::END_HEADERS | shared_layer::Flags::PADDED)) !=
                0) {
                throw error::http::ConnectionError(
                    error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for PUSH_PROMISE",
                    m_stream_id
                );
            }
        }
    }

    /**
     * @brief Enforces PING frame rules: must be on stream 0, only the ACK flag is legal, and
     * length is always exactly 8 bytes (opaque payload echoed back on ACK).
     * @throws error::http::ConnectionError if not on stream 0, an invalid flag bit is set, or
     * length isn't exactly 8.
     */
    void validate_ping() const
    {
        // Connection-level frame, no stream id.
        if (m_stream_id != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "PING not on stream 0"
            );
        }
        // Only ACK is legal, same rule as SETTINGS.
        if ((m_flags & ~shared_layer::Flags::ACK) != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for PING"
            );
        }
        // Fixed 8-byte opaque payload, no exceptions either direction.
        if (m_length != 8) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "PING length must be 8"
            );
        }
    }

    /**
     * @brief Enforces GOAWAY frame rules: must be on stream 0, zero flags, minimum 8 bytes
     * (32-bit last-stream-id + 32-bit error code — debug data past that is optional and
     * variable length, ngl, hence the "at least" instead of "exactly").
     * @throws error::http::ConnectionError if not on stream 0, flags are nonzero, or length is
     * under 8.
     */
    void validate_goaway() const
    {
        // Connection-level, no stream id.
        if (m_stream_id != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "GOAWAY not on stream 0"
            );
        }
        if (m_flags != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "GOAWAY flags must be 0"
            );
        }
        // At least 8 bytes (last-stream-id + error code); optional debug data can push it
        // higher.
        if (m_length < 8) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "GOAWAY length < 8"
            );
        }
    }

    /**
     * @brief Enforces WINDOW_UPDATE frame rules: zero flags, exactly 4 bytes (32-bit
     * increment). Legal on stream 0 (connection-level) or any stream, so there's no stream-id-
     * zero rejection here unlike most of its siblings.
     * @note The error's `last_stream_id` gets set to `MAX_CONNECTED_STREAMS` when this frame is
     * itself connection-level (stream 0) — since there's no "offending stream" to point at, it
     * falls back to the max possible id instead of 0, so downstream GOAWAY handling doesn't
     * misread it as "abort everything from stream 0 onward."
     * @throws error::http::ConnectionError if flags are nonzero or length isn't exactly 4.
     */
    void validate_window_update() const
    {
        // No stream-id-zero rejection here — WINDOW_UPDATE is legal at both connection and
        // stream level, so only flags/length get validated.
        if (m_flags != 0) {
            // Stream 0 has no "offending stream" to name, so fall back to the max possible id
            // instead of 0 — keeps downstream GOAWAY handling from misreading it.
            auto stream_id = m_stream_id == 0 ? MAX_CONNECTED_STREAMS : m_stream_id;
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "WINDOW_UPDATE flags must be 0",
                stream_id
            );
        }
        if (m_length != 4) {
            auto stream_id = m_stream_id == 0 ? MAX_CONNECTED_STREAMS : m_stream_id;
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "WINDOW_UPDATE length != 4",
                stream_id
            );
        }
    }

    /**
     * @brief Enforces CONTINUATION frame rules: never on stream 0, flags limited to
     * END_HEADERS only — no END_STREAM/PADDED/PRIORITY here since those belong to the HEADERS
     * frame that started the block.
     * @throws error::http::ConnectionError if stream id is 0 or unexpected flags are set.
     */
    void validate_continuation() const
    {
        // Always belongs to the stream whose HEADERS block it's continuing.
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "CONTINUATION on stream 0"
            );
        }
        // Only END_HEADERS applies here — END_STREAM/PADDED/PRIORITY belong to the HEADERS
        // frame that started the block, not its continuations.
        if ((m_flags & ~shared_layer::Flags::END_HEADERS) != 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for CONTINUATION",
                m_stream_id
            );
        }
    }

    std::uint32_t m_length;
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::uint32_t m_stream_id;
};

class ReadFrameHeaderAdaptor : public std::ranges::range_adaptor_closure<ReadFrameHeaderAdaptor>
{
public:
    /**
     * @brief Range adaptor closure ctor — just stashes the locally-configured max frame size so
     * operator() can reject oversized incoming frames against it.
     * @param max_frame_size the local SETTINGS_MAX_FRAME_SIZE, incoming frame length gets
     * checked against this.
     */
    explicit constexpr ReadFrameHeaderAdaptor(std::uint32_t max_frame_size) :
        m_max_frame_size{max_frame_size}
    {
    }

    /**
     * @brief Parses the fixed 9-byte HTTP/2 frame header (length, type, flags, stream id) off
     * the front of `data`, big-endian, per RFC 9113 §4.1 wire layout.
     * @warning Doesn't consume/advance `data` itself — this just reads and returns, caller's on
     * the hook for advancing the underlying reader past the 9 bytes (see how session.cppm's
     * `receive_header()` chains this into `AdvanceReaderAdaptor` right after).
     * @tparam R a viewable range of bytes to read the header from.
     * @param data the range to parse — needs at least `HEADER_SIZE` (9) bytes available.
     * @return the parsed header, stream id already masked to 31 bits.
     * @throws error::http::ConnectionError if `data` is shorter than `HEADER_SIZE`, or if the
     * declared length exceeds `m_max_frame_size`.
     */
    template<std::ranges::viewable_range R>
    FrameHeader<shared_layer::FrameRole::RECEIVER> operator()(R&& data) const
    {
        auto range = std::forward<R>(data);

        // Guard — need the full 9 fixed bytes before there's anything to parse at all.
        if (std::ranges::size(range) < HEADER_SIZE) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "Incomplete frame header"
            );
        }

        // First 3 bytes are the payload length.
        std::uint32_t len =
            range | std::views::take(3) | utils::codec::ReadBigEndianAdaptor<std::uint32_t>{};

        // Reject up front if the peer's declaring a frame bigger than we're willing to accept.
        if (len > m_max_frame_size) {
            // WIREDUMP (temporary): dump the 9 header bytes the parser choked on + the bad
            // length.
            {
                std::string hex;
                std::size_t n = 0;
                for (auto b: range | std::views::take(16)) {
                    hex += std::format(
                        "{:02x} ", static_cast<unsigned>(std::to_integer<std::uint8_t>(b))
                    );
                    ++n;
                }
                core::logger::warning(
                    "WIREDUMP", "read-throw len={} max={} bytes[{}]: {}", len, m_max_frame_size, n,
                    hex
                );
            }
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                "FrameBuilder length exceeds SETTINGS_MAX_FRAME_SIZE"
            );
        }

        // Byte 3 is the type, byte 4 is the flags — straightforward single-byte reads.
        auto type = static_cast<shared_layer::FrameType>(
            range | std::views::drop(3) | std::views::take(1) |
            utils::codec::ReadBigEndianAdaptor<std::uint8_t>{}
        );

        auto flags = range | std::views::drop(4) | std::views::take(1) |
                     utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};

        // Last 4 bytes are the stream id — mask off the reserved high bit same as everywhere
        // else.
        std::uint32_t id = 0x7F'FF'FF'FFU & (range | std::views::drop(5) | std::views::take(4) |
                                             utils::codec::ReadBigEndianAdaptor<std::uint32_t>{});

        return {len, type, flags, id};
    }

private:
    std::uint32_t m_max_frame_size;
};

// TODO: to be removed
// struct WriteFrameHeaderAdaptor : std::ranges::range_adaptor_closure<WriteFrameHeaderAdaptor>
// {
//     explicit constexpr WriteFrameHeaderAdaptor(FrameHeader<shared_layer::FrameRole::SENDER>
//     header)
//         : m_header{header} {}
//
//     template <std::ranges::viewable_range R>
//     constexpr auto operator()(R &&range) const {
//         return std::forward<R>(range) |
//                (utils::codec::WriteBigEndianAdaptor<std::uint32_t>{m_header.get_length()} |
//                std::views::take(3)) |
//                utils::codec::WriteBigEndianAdaptor<std::uint8_t>{std::to_underlying(m_header.get_type())}
//                | utils::codec::WriteBigEndianAdaptor<std::uint8_t>{m_header.get_flags()} |
//                utils::codec::WriteBigEndianAdaptor<std::uint32_t>{m_header.get_stream_id() &
//                0x7FFFFFFF};
//     }
//
//     FrameHeader<shared_layer::FrameRole::SENDER> m_header;
// };


class FrameHeaderClosureAdaptor :
    public std::ranges::range_adaptor_closure<FrameHeaderClosureAdaptor>
{
public:
    /**
     * @brief Range adaptor closure ctor — stashes the four header fields to write, masking the
     * stream id up front, lowkey saving operator() from redoing it later.
     * @param length the payload length to encode.
     * @param type the frame type to encode.
     * @param flags the flags byte to encode.
     * @param stream_id the stream id to encode, masked to 31 bits right here in the ctor.
     */
    explicit constexpr FrameHeaderClosureAdaptor(
        std::uint32_t length,
        shared_layer::FrameType type,
        std::uint8_t flags,
        std::uint32_t stream_id
    ) noexcept :
        m_length{length},
        m_type{type},
        m_flags{flags},
        m_clean_stream_id{stream_id & 0x7F'FF'FF'FF}
    {
    }

    /**
     * @brief Writes the 9-byte HTTP/2 frame header onto the end of `range`, big-endian, per RFC
     * 9113 §4.1 — length (3 bytes), type (1), flags (1), stream id (4, masked).
     * @tparam R a viewable range this appends the header bytes onto.
     * @param range the range to append the encoded header onto.
     * @return the range with the 9 header bytes appended.
     */
    template<std::ranges::viewable_range R>
    [[nodiscard]] constexpr auto operator()(R&& range) const
    {
        return std::forward<R>(range) |
               (utils::codec::WriteBigEndianAdaptor<std::uint32_t>{m_length} |
                std::views::drop(1)) |
               utils::codec::WriteBigEndianAdaptor<std::uint8_t>{std::to_underlying(m_type)} |
               utils::codec::WriteBigEndianAdaptor<std::uint8_t>{m_flags} |
               utils::codec::WriteBigEndianAdaptor<std::uint32_t>{m_clean_stream_id};
    }

private:
    std::uint32_t m_length;
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::uint32_t m_clean_stream_id;
};

class WriteFrameClosureAdapter : public std::ranges::range_adaptor_closure<WriteFrameClosureAdapter>
{
public:
    /**
     * @brief Range adaptor closure ctor — stashes everything needed to chunk a payload into
     * max-frame-size slices and prepend a header onto each one.
     * @param stream_id the stream id every emitted frame header carries.
     * @param type the frame type for the first chunk — HEADERS chunks past the first
     * automatically flip to CONTINUATION in operator(), everything else keeps this type as-is.
     * @param flags base flags applied to every chunk before the last-chunk logic layers on
     * END_HEADERS/END_STREAM.
     * @param max_frame_size the slice size each payload chunk gets capped to.
     * @param end_stream_after_data when true, suppresses auto-setting END_STREAM on the last
     * DATA chunk — for when a separate empty DATA frame is going to carry END_STREAM instead.
     * @param no_data when true and `type` is HEADERS, also sets END_STREAM on the last HEADERS/
     * CONTINUATION chunk — for header-only responses/requests with no body at all.
     */
    explicit constexpr WriteFrameClosureAdapter(
        std::uint32_t stream_id,
        shared_layer::FrameType type,
        std::uint8_t flags,
        std::size_t max_frame_size,
        bool end_stream_after_data = false,
        bool no_data = false
    ) :
        m_stream_id{stream_id},
        m_type{type},
        m_flags{flags},
        m_max_frame_size{max_frame_size},
        m_end_stream_after_data{end_stream_after_data},
        m_no_data{no_data}
    {
    }

    /**
     * @brief Chunks `range` into `m_max_frame_size`-sized slices and writes a framed HTTP/2
     * frame (header + payload) for each one — this is the workhorse that turns a raw byte
     * range into wire-ready frames, HEADERS/CONTINUATION splitting and END_HEADERS/END_STREAM
     * flag placement all handled per chunk.
     * @warning Empty input still produces exactly one frame (an empty DATA or HEADERS frame),
     * never zero frames — `total_chunks` is clamped to at least 1 specifically so an empty
     * body/header-block still gets a frame with END_STREAM/END_HEADERS set on it. Skip that
     * clamp and the peer never sees stream closure, that's an L for the whole exchange.
     * @tparam R a viewable range of bytes to chunk and frame.
     * @param range the payload bytes to split across one or more frames.
     * @return a lazily-joined view of every emitted frame's bytes (headers + chunk payload,
     * back to back).
     */
    template<std::ranges::viewable_range R>
    auto operator()(R&& range) const
    {
        auto data = std::views::all(std::forward<R>(range));

        // Figure out how many max_frame_size-sized slices this payload splits into.
        const std::size_t TOTAL_LENGTH = std::ranges::distance(data);
        const std::size_t SLICE_SIZE = std::min(m_max_frame_size, TOTAL_LENGTH);

        // Clamped to at least 1 — an empty body/header-block still needs one frame to carry
        // END_STREAM/END_HEADERS, otherwise the peer never sees stream closure.
        const std::size_t TOTAL_CHUNKS =
            TOTAL_LENGTH == 0 ? 1 : (TOTAL_LENGTH + SLICE_SIZE - 1) / SLICE_SIZE;

        // Lazily slice the payload into `total_chunks` windows of `slice_size` bytes each.
        auto chunked = std::views::iota(0UZ, TOTAL_CHUNKS) |
                       std::views::transform([data, SLICE_SIZE](std::size_t chunk_idx) {
                           return data | std::views::drop(chunk_idx * SLICE_SIZE) |
                                  std::views::take(SLICE_SIZE);
                       });

        // Unified pipeline
        return chunked | std::views::enumerate |
               std::views::transform([self = *this, TOTAL_CHUNKS](auto&& entry) {
                   auto [idx, chunk] = entry;
                   const auto CHUNK_IDX = static_cast<std::size_t>(idx);

                   auto type = self.m_type;
                   std::uint8_t flags = self.m_flags;

                   const bool IS_LAST = (CHUNK_IDX == TOTAL_CHUNKS - 1);

                   // HEADERS chunks past the first flip to CONTINUATION; only the last chunk of
                   // either gets END_HEADERS (and END_STREAM too if there's no body coming).
                   if (type == shared_layer::FrameType::HEADERS) {
                       if (IS_LAST) {
                           flags |= shared_layer::Flags::END_HEADERS;
                           if (self.m_no_data) {
                               flags |= shared_layer::Flags::END_STREAM;
                           }
                       }
                       if (CHUNK_IDX != 0) {
                           type = shared_layer::FrameType::CONTINUATION;
                       }
                   } else if (
                       type == shared_layer::FrameType::DATA && IS_LAST &&
                       !self.m_end_stream_after_data
                   ) {
                       // Last DATA chunk closes the stream, unless a separate empty DATA frame
                       // is going to carry END_STREAM instead.
                       flags |= shared_layer::Flags::END_STREAM;
                   }

                   // Prepend the header for this chunk, then the chunk's own bytes.
                   return std::views::concat(
                       std::views::empty<std::byte> |
                           FrameHeaderClosureAdaptor{
                               static_cast<std::uint32_t>(std::ranges::distance(chunk)), type,
                               flags, self.m_stream_id
                           },
                       chunk
                   );
               }) |
               std::views::join;
    }

private:
    std::uint32_t m_stream_id;
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::size_t m_max_frame_size;
    bool m_end_stream_after_data;
    bool m_no_data;
};

struct ReadWindowIncrementAdaptor : std::ranges::range_adaptor_closure<ReadWindowIncrementAdaptor>
{
    /**
     * @brief Reads the 4-byte big-endian increment off a WINDOW_UPDATE payload, masking off
     * the reserved high bit same as stream ids get (RFC 9113 §6.9 — that bit's reserved, MUST
     * be ignored on receipt).
     * @tparam R a viewable range of at least 4 bytes.
     * @param range the WINDOW_UPDATE payload to read from.
     * @return the window increment, masked to 31 bits.
     * @throws error::http::ConnectionError if the decoded increment is zero — a zero increment
     * is explicitly illegal per spec, would mean "don't actually update the window" which
     * makes no sense for this frame type.
     */
    template<std::ranges::viewable_range R>
    std::uint32_t operator()(R&& range) const
    {
        // Decode the 4-byte increment, masking off the reserved high bit same as stream ids.
        std::uint32_t inc =
            (std::forward<R>(range) | std::views::take(4) | utils::codec::ReadBigEndianAdaptor{}) &
            0x7F'FF'FF'FF;

        // A zero increment is explicitly illegal per spec — "don't actually update the window"
        // makes no sense for this frame type.
        if (inc == 0) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::PROTOCOL_ERROR,
                "WINDOW_UPDATE increment must be non-zero"
            );
        }

        return inc;
    }
};

template<shared_layer::FrameRole Role>
class FrameBuilder
{
public:
    /**
     * @brief Default ctor — type defaults to DATA, everything else zeroed. Not a real frame
     * yet, lowkey just the builder's starting state.
     */
    FrameBuilder() = default;

    /**
     * @brief Builder chain — sets the frame type.
     * @param type the type to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameBuilder&& add_type(shared_layer::FrameType type) && noexcept
    {
        core::logger::debug("FrameBuilder", "type={}", type);

        m_type = type;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the raw flags byte.
     * @param flags the flags to set.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameBuilder&& add_flags(std::uint8_t flags) && noexcept
    {
        core::logger::debug("FrameBuilder", "flags={}", flags);

        m_flags = flags;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — sets the stream id, masking off the reserved high bit inline.
     * @param stream_id the stream id to set, high bit gets clipped regardless of value.
     * @return `*this`, moved, so the chain keeps going.
     */
    FrameBuilder&& add_stream_id(std::uint32_t stream_id) && noexcept
    {
        core::logger::debug("FrameBuilder", "stream_id={}", stream_id);

        m_stream_id = stream_id & 0x7F'FF'FF'FF;
        return std::move(*this);
    }

    /**
     * @brief Builder chain — appends bytes onto the payload. Can be called more than once,
     * each call just tacks more bytes on the end, doesn't replace what's already there.
     * @tparam R a forward range whose value type is `std::byte`.
     * @param payload the bytes to append.
     * @return `*this`, moved, so the chain keeps going.
     */
    template<std::ranges::forward_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    // FIXME(clang-tidy): bugprone-exception-escape — append_range may throw (bad_alloc) inside
    // a noexcept function; leaving noexcept as-is since removing it would change this builder's
    // exception-safety contract without a confirmed-safe rework.
    FrameBuilder&& add_payload(R&& payload) && noexcept
    {
        m_payload.append_range(std::forward<R>(payload));
        core::logger::debug("FrameBuilder", "payload size={}", m_payload.size());
        return std::move(*this);
    }

    /**
     * @brief Terminal builder call — closes out the chain, no extra logic, just hands back the
     * finished builder.
     * @return `*this`, moved out to the caller as the finished builder.
     */
    FrameBuilder&& build() &&
    {
        core::logger::debug("FrameBuilder", "build");
        return std::move(*this);
    }

    /**
     * @brief In-place (lvalue) version of add_payload() — appends more bytes onto an existing
     * builder instead of chaining through a moved rvalue.
     * @tparam R a forward range whose value type is `std::byte`.
     * @param payload the bytes to append.
     */
    template<std::ranges::forward_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    void expand_payload(R&& payload) noexcept
    {
        m_payload.append_range(std::forward<R>(payload));
        core::logger::debug("FrameBuilder", "payload total={}", m_payload.size());
    }

    /**
     * @brief Total wire size this frame will take up once framed — header plus payload. Simple
     * math, bet.
     * @return `HEADER_SIZE` (9) plus the current payload size.
     */
    [[nodiscard]] std::size_t get_size() const noexcept
    {
        return HEADER_SIZE + m_payload.size();
    }

    /**
     * @brief Grabs a read-only view over the accumulated payload bytes.
     * @return the payload bytes.
     */
    [[nodiscard]] std::span<const std::byte> get_payload() const noexcept
    {
        return m_payload;
    }

    /**
     * @brief Grabs the payload length.
     * @return the current payload size in bytes.
     */
    [[nodiscard]] std::size_t get_length() const noexcept
    {
        return m_payload.size();
    }

    /**
     * @brief Grabs the frame type.
     * @return the frame type.
     */
    [[nodiscard]] shared_layer::FrameType get_type() const noexcept
    {
        return m_type;
    }

    /**
     * @brief Grabs the raw flags byte.
     * @return the flags byte.
     */
    [[nodiscard]] std::uint8_t get_flags() const noexcept
    {
        return m_flags;
    }

    /**
     * @brief Grabs the stream id.
     * @return the stream id, already masked to 31 bits.
     */
    [[nodiscard]] std::uint32_t get_stream_id() const noexcept
    {
        return m_stream_id;
    }

private:
    shared_layer::FrameType m_type{shared_layer::FrameType::DATA};
    std::uint8_t m_flags{0};
    std::uint32_t m_stream_id{0};
    std::vector<std::byte> m_payload;
};

class WriteFrameBuilderAdaptor : public std::ranges::range_adaptor_closure<WriteFrameBuilderAdaptor>
{
public:
    /**
     * @brief Range adaptor closure ctor — takes ownership of a completed `FrameBuilder` and
     * the chunking config needed to frame it.
     * @param frame the finished builder to encode. Taken by value, moved in.
     * @param max_frame_size the slice size the builder's payload gets chunked to.
     * @param end_stream_after_data forwarded straight through to `WriteFrameClosureAdapter`.
     * @param no_data forwarded straight through to `WriteFrameClosureAdapter`.
     */
    explicit constexpr WriteFrameBuilderAdaptor(
        FrameBuilder<shared_layer::FrameRole::SENDER> frame,
        std::size_t max_frame_size,
        bool end_stream_after_data = false,
        bool no_data = false
    ) :
        m_frame{std::move(frame)},
        m_max_frame_size{max_frame_size},
        m_end_stream_after_data{end_stream_after_data},
        m_no_data{no_data}
    {
    }

    /**
     * @brief Concats `range` with the fully-framed bytes of `m_frame`'s payload (chunked and
     * headered via `WriteFrameClosureAdapter`).
     * @tparam R a viewable range this appends the encoded frame bytes onto.
     * @param range the range to append the encoded frame onto.
     * @return `range` followed by the encoded frame bytes, lazily joined.
     */
    template<std::ranges::viewable_range R>
    auto operator()(R&& range) const
    {
        return std::views::concat(
            std::forward<R>(range),
            m_frame.get_payload() | WriteFrameClosureAdapter{
                                        m_frame.get_stream_id(), m_frame.get_type(),
                                        m_frame.get_flags(), m_max_frame_size,
                                        m_end_stream_after_data, m_no_data
                                    }
        );
    }

    /**
     * @brief No-arg convenience overload — same as calling operator() on an empty byte range.
     * Handy motion for kicking off a frame write when there's nothing to prepend it onto yet.
     * @return the encoded frame bytes, standalone.
     */
    auto operator()() const
    {
        return (*this)(std::views::empty<std::byte>);
    }

private:
    FrameBuilder<shared_layer::FrameRole::SENDER> m_frame;
    std::size_t m_max_frame_size;
    bool m_end_stream_after_data;
    bool m_no_data;
};


} // namespace io::layer::http2

#ifdef CONGELADO_TEST
namespace io::layer::http2::tests {
using namespace boost::ut;
using shared_layer::Flags;
using shared_layer::FrameRole;
using shared_layer::FrameType;

suite<"FrameHeader"> frame_header_suite = [] {
    "builder chain accumulates every field"_test = [] {
        auto header = FrameHeader<FrameRole::SENDER>{}
                          .add_length(42)
                          .add_type(FrameType::HEADERS)
                          .add_flags(Flags::END_HEADERS)
                          .add_stream_id(3);

        expect(header.get_length() == 42);
        expect(header.get_type() == FrameType::HEADERS);
        expect(header.get_flags() == Flags::END_HEADERS);
        expect(header.get_stream_id() == 3);
    };

    "set_stream_id masks off the reserved high bit"_test = [] {
        FrameHeader<FrameRole::SENDER> header;
        header.set_stream_id(0x80'00'00'05U);

        expect(header.get_stream_id() == 5U);
    };

    "get_size is header (9) plus payload length"_test = [] {
        FrameHeader<FrameRole::SENDER> header{100, FrameType::DATA, 0, 1};

        expect(header.get_header_length() == HEADER_SIZE);
        expect(header.get_size() == HEADER_SIZE + 100);
    };

    "is_end_stream/is_padded read the matching flag bits"_test = [] {
        FrameHeader<FrameRole::SENDER> header{
            0, FrameType::DATA, Flags::END_STREAM | Flags::PADDED, 1
        };

        expect(header.is_end_stream());
        expect(header.is_padded());

        FrameHeader<FrameRole::SENDER> plain{0, FrameType::DATA, 0, 1};
        expect(not plain.is_end_stream());
        expect(not plain.is_padded());
    };

    "validate_payload_size throws on a length mismatch, passes on a match"_test = [] {
        FrameHeader<FrameRole::SENDER> header{5, FrameType::DATA, 0, 1};

        expect(throws<error::http::ConnectionError>([&] {
            header.validate_payload_size(4);
        }));
        expect(nothrow([&] {
            header.validate_payload_size(5);
        }));
    };

    "validate_padding throws when the pad length isn't strictly less than the frame length"_test =
        [] {
            FrameHeader<FrameRole::SENDER> header{5, FrameType::DATA, Flags::PADDED, 1};

            expect(throws<error::http::ConnectionError>([&] {
                header.validate_padding(5);
            }));
            expect(nothrow([&] {
                header.validate_padding(4);
            }));
        };

    "validate() rejects DATA on stream 0, bad flags, or short padded payload"_test = [] {
        FrameHeader<FrameRole::RECEIVER> stream_zero{5, FrameType::DATA, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            stream_zero.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> bad_flags{5, FrameType::DATA, Flags::PRIORITY, 1};
        expect(throws<error::http::ConnectionError>([&] {
            bad_flags.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> short_padded{0, FrameType::DATA, Flags::PADDED, 1};
        expect(throws<error::http::ConnectionError>([&] {
            short_padded.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{5, FrameType::DATA, Flags::END_STREAM, 1};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() rejects HEADERS on stream 0 and enforces the flag-scaled minimum length"_test = [] {
        FrameHeader<FrameRole::RECEIVER> stream_zero{0, FrameType::HEADERS, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            stream_zero.validate();
        }));

        // PRIORITY flag demands 5 extra bytes; 2 is too short.
        FrameHeader<FrameRole::RECEIVER> too_short{2, FrameType::HEADERS, Flags::PRIORITY, 1};
        expect(throws<error::http::ConnectionError>([&] {
            too_short.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{5, FrameType::HEADERS, Flags::PRIORITY, 1};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() requires PRIORITY to be exactly 5 bytes, zero flags, off stream 0"_test = [] {
        FrameHeader<FrameRole::RECEIVER> stream_zero{5, FrameType::PRIORITY, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            stream_zero.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> bad_length{4, FrameType::PRIORITY, 0, 1};
        expect(throws<error::http::ConnectionError>([&] {
            bad_length.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{5, FrameType::PRIORITY, 0, 1};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() requires RST_STREAM to be exactly 4 bytes"_test = [] {
        FrameHeader<FrameRole::RECEIVER> bad_length{5, FrameType::RST_STREAM, 0, 1};
        expect(throws<error::http::ConnectionError>([&] {
            bad_length.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{4, FrameType::RST_STREAM, 0, 1};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() enforces SETTINGS stream-0, ACK-empty, and multiple-of-6 rules"_test = [] {
        FrameHeader<FrameRole::RECEIVER> off_stream{0, FrameType::SETTINGS, 0, 1};
        expect(throws<error::http::ConnectionError>([&] {
            off_stream.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ack_with_payload{6, FrameType::SETTINGS, Flags::ACK, 0};
        expect(throws<error::http::ConnectionError>([&] {
            ack_with_payload.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> bad_size{7, FrameType::SETTINGS, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            bad_size.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ack{0, FrameType::SETTINGS, Flags::ACK, 0};
        expect(nothrow([&] {
            ack.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> twelve{12, FrameType::SETTINGS, 0, 0};
        expect(nothrow([&] {
            twelve.validate();
        }));
    };

    "validate() rejects PUSH_PROMISE received by a server unconditionally"_test = [] {
        FrameHeader<FrameRole::RECEIVER> received{5, FrameType::PUSH_PROMISE, 0, 2};
        expect(throws<error::http::ConnectionError>([&] {
            received.validate();
        }));
    };

    "validate() enforces PUSH_PROMISE sender rules (stream, flags)"_test = [] {
        FrameHeader<FrameRole::SENDER> stream_zero{5, FrameType::PUSH_PROMISE, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            stream_zero.validate();
        }));

        FrameHeader<FrameRole::SENDER> bad_flags{5, FrameType::PUSH_PROMISE, Flags::PRIORITY, 2};
        expect(throws<error::http::ConnectionError>([&] {
            bad_flags.validate();
        }));

        FrameHeader<FrameRole::SENDER> ok{5, FrameType::PUSH_PROMISE, Flags::END_HEADERS, 2};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() requires PING to be on stream 0 with an 8-byte payload"_test = [] {
        FrameHeader<FrameRole::RECEIVER> off_stream{8, FrameType::PING, 0, 1};
        expect(throws<error::http::ConnectionError>([&] {
            off_stream.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> bad_length{4, FrameType::PING, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            bad_length.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{8, FrameType::PING, Flags::ACK, 0};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() requires GOAWAY on stream 0, no flags, at least 8 bytes"_test = [] {
        FrameHeader<FrameRole::RECEIVER> off_stream{8, FrameType::GOAWAY, 0, 1};
        expect(throws<error::http::ConnectionError>([&] {
            off_stream.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> too_short{7, FrameType::GOAWAY, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            too_short.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{8, FrameType::GOAWAY, 0, 0};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() requires WINDOW_UPDATE to be exactly 4 bytes, no flags, any stream"_test = [] {
        FrameHeader<FrameRole::RECEIVER> bad_flags{4, FrameType::WINDOW_UPDATE, Flags::ACK, 0};
        expect(throws<error::http::ConnectionError>([&] {
            bad_flags.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> bad_length{5, FrameType::WINDOW_UPDATE, 0, 1};
        expect(throws<error::http::ConnectionError>([&] {
            bad_length.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> on_conn{4, FrameType::WINDOW_UPDATE, 0, 0};
        expect(nothrow([&] {
            on_conn.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> on_stream{4, FrameType::WINDOW_UPDATE, 0, 3};
        expect(nothrow([&] {
            on_stream.validate();
        }));
    };

    "validate() rejects CONTINUATION on stream 0 or with flags other than END_HEADERS"_test = [] {
        FrameHeader<FrameRole::RECEIVER> stream_zero{0, FrameType::CONTINUATION, 0, 0};
        expect(throws<error::http::ConnectionError>([&] {
            stream_zero.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> bad_flags{0, FrameType::CONTINUATION, Flags::PADDED, 1};
        expect(throws<error::http::ConnectionError>([&] {
            bad_flags.validate();
        }));

        FrameHeader<FrameRole::RECEIVER> ok{0, FrameType::CONTINUATION, Flags::END_HEADERS, 1};
        expect(nothrow([&] {
            ok.validate();
        }));
    };

    "validate() rejects a SENDER PUSH_PROMISE on an odd stream id"_test = [] {
        FrameHeader<FrameRole::SENDER> odd{5, FrameType::PUSH_PROMISE, Flags::END_HEADERS, 3};
        expect(throws<error::http::ConnectionError>([&] {
            odd.validate();
        }));
    };
};

suite<"ReadFrameHeaderAdaptor / FrameHeaderClosureAdaptor"> frame_header_codec_suite = [] {
    "round-trips a header through write then read"_test = [] {
        auto bytes = std::views::empty<std::byte> |
                     FrameHeaderClosureAdaptor{100, FrameType::HEADERS, Flags::END_HEADERS, 5} |
                     std::ranges::to<std::vector<std::byte>>();

        expect(bytes.size() == HEADER_SIZE);

        auto header = bytes | ReadFrameHeaderAdaptor{MIN_FRAME_SIZE};

        expect(header.get_length() == 100U);
        expect(header.get_type() == FrameType::HEADERS);
        expect(header.get_flags() == Flags::END_HEADERS);
        expect(header.get_stream_id() == 5U);
    };

    "throws on a header shorter than HEADER_SIZE"_test = [] {
        std::vector<std::byte> too_short(5);

        expect(throws<error::http::ConnectionError>([&] {
            std::ignore = (too_short | ReadFrameHeaderAdaptor{MIN_FRAME_SIZE});
        }));
    };

    "throws when the declared length exceeds the local max frame size"_test = [] {
        auto bytes = std::views::empty<std::byte> |
                     FrameHeaderClosureAdaptor{100, FrameType::DATA, 0, 1} |
                     std::ranges::to<std::vector<std::byte>>();

        expect(throws<error::http::ConnectionError>([&] {
            std::ignore = (bytes | ReadFrameHeaderAdaptor{10});
        }));
    };

    "masks the reserved high bit off the stream id on read"_test = [] {
        auto bytes = std::views::empty<std::byte> |
                     FrameHeaderClosureAdaptor{0, FrameType::WINDOW_UPDATE, 0, 3} |
                     std::ranges::to<std::vector<std::byte>>();

        auto header = bytes | ReadFrameHeaderAdaptor{MIN_FRAME_SIZE};
        expect(header.get_stream_id() == 3U);
    };
};

suite<"WriteFrameClosureAdapter"> write_frame_closure_suite = [] {
    "a payload under max_frame_size becomes one DATA frame with END_STREAM"_test = [] {
        std::vector<std::byte> payload(10, std::byte{0xAB});

        auto framed = payload | WriteFrameClosureAdapter{7, FrameType::DATA, 0, 1'024} |
                      std::ranges::to<std::vector<std::byte>>();

        expect(framed.size() == HEADER_SIZE + 10);

        auto header = framed | ReadFrameHeaderAdaptor{1'024};
        expect(header.get_length() == 10U);
        expect(header.get_type() == FrameType::DATA);
        expect(header.get_flags() == Flags::END_STREAM);
        expect(header.get_stream_id() == 7U);
    };

    "an empty payload still emits exactly one frame carrying END_STREAM"_test = [] {
        std::vector<std::byte> empty_payload;

        auto framed = empty_payload | WriteFrameClosureAdapter{3, FrameType::DATA, 0, 1'024} |
                      std::ranges::to<std::vector<std::byte>>();

        expect(framed.size() == HEADER_SIZE);

        auto header = framed | ReadFrameHeaderAdaptor{1'024};
        expect(header.get_length() == 0U);
        expect(header.get_flags() == Flags::END_STREAM);
    };

    "a payload larger than max_frame_size chunks with END_STREAM only on the last DATA frame"_test =
        [] {
            std::vector<std::byte> payload(10, std::byte{0x11});

            auto framed = payload | WriteFrameClosureAdapter{1, FrameType::DATA, 0, 4} |
                          std::ranges::to<std::vector<std::byte>>();

            // 3 chunks of 4/4/2 bytes, each with its own 9-byte header.
            expect(framed.size() == 3 * HEADER_SIZE + 10);

            std::span<const std::byte> remaining{framed};
            auto first = remaining | ReadFrameHeaderAdaptor{4};
            expect(first.get_length() == 4U);
            expect(first.get_flags() == 0U);
            remaining = remaining.subspan(HEADER_SIZE + first.get_length());

            auto second = remaining | ReadFrameHeaderAdaptor{4};
            expect(second.get_length() == 4U);
            expect(second.get_flags() == 0U);
            remaining = remaining.subspan(HEADER_SIZE + second.get_length());

            auto third = remaining | ReadFrameHeaderAdaptor{4};
            expect(third.get_length() == 2U);
            expect(third.get_flags() == Flags::END_STREAM);
        };

    "HEADERS chunks past the first flip to CONTINUATION, with END_HEADERS on the last"_test = [] {
        std::vector<std::byte> payload(6, std::byte{0x22});

        auto framed = payload | WriteFrameClosureAdapter{9, FrameType::HEADERS, 0, 4, false, true} |
                      std::ranges::to<std::vector<std::byte>>();

        std::span<const std::byte> remaining{framed};
        auto first = remaining | ReadFrameHeaderAdaptor{4};
        expect(first.get_type() == FrameType::HEADERS);
        expect(first.get_flags() == 0U);
        remaining = remaining.subspan(HEADER_SIZE + first.get_length());

        auto second = remaining | ReadFrameHeaderAdaptor{4};
        expect(second.get_type() == FrameType::CONTINUATION);
        // no_data=true, so the last chunk also carries END_STREAM alongside END_HEADERS.
        expect(second.get_flags() == (Flags::END_HEADERS | Flags::END_STREAM));
    };
};

suite<"ReadWindowIncrementAdaptor"> read_window_increment_suite = [] {
    "decodes a positive 4-byte big-endian increment"_test = [] {
        auto bytes = std::views::empty<std::byte> |
                     utils::codec::WriteBigEndianAdaptor<std::uint32_t>{1'000} |
                     std::ranges::to<std::vector<std::byte>>();

        auto increment = bytes | ReadWindowIncrementAdaptor{};
        expect(increment == 1'000U);
    };

    "masks off the reserved high bit"_test = [] {
        auto bytes = std::views::empty<std::byte> |
                     utils::codec::WriteBigEndianAdaptor<std::uint32_t>{0x80'00'00'64U} |
                     std::ranges::to<std::vector<std::byte>>();

        auto increment = bytes | ReadWindowIncrementAdaptor{};
        expect(increment == 100U);
    };

    "throws on a zero increment"_test = [] {
        auto bytes = std::views::empty<std::byte> |
                     utils::codec::WriteBigEndianAdaptor<std::uint32_t>{0U} |
                     std::ranges::to<std::vector<std::byte>>();

        expect(throws<error::http::ConnectionError>([&] {
            std::ignore = (bytes | ReadWindowIncrementAdaptor{});
        }));
    };
};

suite<"FrameBuilder"> frame_builder_suite = [] {
    "builder chain accumulates type/flags/stream id/payload"_test = [] {
        std::vector<std::byte> payload{std::byte{1}, std::byte{2}, std::byte{3}};

        auto frame = FrameBuilder<FrameRole::SENDER>{}
                         .add_type(FrameType::PING)
                         .add_flags(Flags::ACK)
                         .add_stream_id(0)
                         .add_payload(payload)
                         .build();

        expect(frame.get_type() == FrameType::PING);
        expect(frame.get_flags() == Flags::ACK);
        expect(frame.get_stream_id() == 0U);
        expect(frame.get_length() == 3U);
        expect(frame.get_size() == HEADER_SIZE + 3);
        expect(frame.get_payload().size() == 3U);
    };

    "add_stream_id masks off the reserved high bit"_test = [] {
        auto frame = FrameBuilder<FrameRole::SENDER>{}.add_stream_id(0x80'00'00'05U);
        expect(frame.get_stream_id() == 5U);
    };

    "add_payload appends across multiple calls instead of replacing"_test = [] {
        auto frame = FrameBuilder<FrameRole::SENDER>{}
                         .add_payload(std::vector<std::byte>{std::byte{1}})
                         .add_payload(std::vector<std::byte>{std::byte{2}, std::byte{3}});

        expect(frame.get_length() == 3U);
    };

    "expand_payload appends onto an lvalue builder in place"_test = [] {
        FrameBuilder<FrameRole::SENDER> frame;
        frame.expand_payload(std::vector<std::byte>{std::byte{9}});

        expect(frame.get_length() == 1U);
    };
};

suite<"WriteFrameBuilderAdaptor"> write_frame_builder_suite = [] {
    "encodes a FrameBuilder into a header followed by its payload"_test = [] {
        auto frame = FrameBuilder<FrameRole::SENDER>{}
                         .add_type(FrameType::PING)
                         .add_flags(Flags::ACK)
                         .add_stream_id(0)
                         .add_payload(std::vector<std::byte>(8, std::byte{0}))
                         .build();

        auto bytes = WriteFrameBuilderAdaptor{std::move(frame), MIN_FRAME_SIZE}() |
                     std::ranges::to<std::vector<std::byte>>();

        expect(bytes.size() == HEADER_SIZE + 8);

        auto header = bytes | ReadFrameHeaderAdaptor{MIN_FRAME_SIZE};
        expect(header.get_type() == FrameType::PING);
        expect(header.get_flags() == Flags::ACK);
        expect(header.get_length() == 8U);
    };

    "appends onto an existing range instead of replacing it"_test = [] {
        std::vector<std::byte> prefix{std::byte{0xFF}};

        auto frame = FrameBuilder<FrameRole::SENDER>{}
                         .add_type(FrameType::WINDOW_UPDATE)
                         .add_stream_id(0)
                         .build();

        auto bytes = (prefix | WriteFrameBuilderAdaptor{std::move(frame), MIN_FRAME_SIZE}) |
                     std::ranges::to<std::vector<std::byte>>();

        expect(bytes.size() == 1 + HEADER_SIZE);
        expect(bytes[0] == std::byte{0xFF});
    };
};

} // namespace io::layer::http2::tests
#endif
