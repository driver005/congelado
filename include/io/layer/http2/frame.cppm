module;
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>
export module io_layer_http2:frame;

import std;
import io_layer_shared;
import io_error;
import shared;
import :consts;

export namespace io::layer::http2 {

class FrameHeader {
  public:
    FrameHeader() : m_length{0}, m_type{shared_layer::FrameType::DATA}, m_flags{0}, m_stream_id{0} {}

    FrameHeader(std::uint32_t length, shared_layer::FrameType type, std::uint8_t flags, std::uint32_t stream_id)
        : m_length(length), m_type(type), m_flags(flags) {
        set_stream_id(stream_id);
    }

    FrameHeader &&add_length(std::uint32_t len) && noexcept {
        m_length = len;
        return std::move(*this);
    }

    FrameHeader &&add_type(shared_layer::FrameType type) && noexcept {
        m_type = type;
        return std::move(*this);
    }

    FrameHeader &&add_flags(std::uint8_t flags) && noexcept {
        m_flags = flags;
        return std::move(*this);
    }

    FrameHeader &&add_stream_id(std::uint32_t stream_id) && noexcept {
        set_stream_id(stream_id);
        return std::move(*this);
    }

    template <shared::ByteRangeReader R>
    static FrameHeader from_bytes(R &&range, std::uint32_t max_frame_size) {
        if (std::ranges::size(range) < HEADER_SIZE) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Incomplete frame header");
        }

        std::uint32_t len = shared_layer::Atom<>::read_big_endian(range | std::views::take(3));

        if (len > max_frame_size) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Frame length exceeds SETTINGS_MAX_FRAME_SIZE");
        }

        auto type = static_cast<shared_layer::FrameType>(*std::ranges::next(std::ranges::begin(range), 3));
        auto flags = static_cast<std::uint8_t>(*std::ranges::next(std::ranges::begin(range), 4));
        std::uint32_t id =
            shared_layer::Atom<>::read_big_endian(range | std::views::drop(5) | std::views::take(4)) & 0x7FFFFFFF;

        return {len, type, flags, id};
    }

    template <shared::ByteIteratorReader It>
    static FrameHeader from_bytes(It &it, std::uint32_t max_frame_size) {
        auto header = from_bytes(std::views::counted(it, HEADER_SIZE), max_frame_size);
        std::advance(it, HEADER_SIZE);
        return header;
    }

    template <shared::ByteRangeWriter R>
    void to_bytes(R &&range) const {
        shared_layer::Atom<>::write_big_endian(range | std::views::take(3), m_length);

        // 2. Write Type (1 byte)
        shared_layer::Atom<std::uint8_t>::write_big_endian(range | std::views::drop(3) | std::views::take(1),
                                                           std::to_underlying(m_type));
        // 3. Write Flags (1 byte)
        shared_layer::Atom<std::uint8_t>::write_big_endian(range | std::views::drop(4) | std::views::take(1), m_flags);

        // 4. Write Stream ID (31-bit / 4 bytes)
        // Masking bit 31 as per HTTP/2 spec
        shared_layer::Atom<>::write_big_endian(range | std::views::drop(5) | std::views::take(4),
                                               m_stream_id & 0x7FFFFFFF);
    }

    template <shared::ByteIteratorWriter It>
    void to_bytes(It &it) const {
        to_bytes(std::views::counted(it, HEADER_SIZE));
        std::advance(it, HEADER_SIZE);
    }

    constexpr std::size_t get_size() const noexcept { return HEADER_SIZE; }
    const std::uint32_t &get_length() const noexcept { return m_length; }
    const shared_layer::FrameType &get_type() const noexcept { return m_type; }
    const std::uint8_t &get_flags() const noexcept { return m_flags; }
    const std::uint32_t &get_stream_id() const noexcept { return m_stream_id; }

    void set_length(std::uint32_t len) noexcept { m_length = len; }
    void set_type(shared_layer::FrameType t) noexcept { m_type = t; }
    void set_flags(std::uint8_t f) noexcept { m_flags = f; }
    void set_stream_id(std::uint32_t id) noexcept { m_stream_id = id & 0x7FFFFFFF; }

  private:
    std::uint32_t m_length;
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::uint32_t m_stream_id;
};


template <shared_layer::FrameRole Role>
class Frame {
  public:
    Frame() : m_header{}, m_payload{} {}

    Frame(FrameHeader header, std::vector<std::byte> payload)
        : m_header(std::move(header)), m_payload(std::move(payload)) {
        validate();
    }

    Frame &&add_header(FrameHeader header) && noexcept {
        m_header = std::move(header);
        return std::move(*this);
    }

    template <std::ranges::contiguous_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    Frame &&add_payload(R &&payload) && noexcept {
        m_payload.assign(std::ranges::begin(payload), std::ranges::end(payload));
        return std::move(*this);
    }

    Frame &&build() && noexcept {
        validate();
        return std::move(*this);
    }

    template <shared::ByteRangeReader R>
    void encode(R &&range) const {
        m_header.to_bytes(range);
        std::ranges::move(m_payload, std::ranges::begin(range | std::views::drop(HEADER_SIZE)));
    }

    template <shared::ByteIteratorWriter It>
    void encode(It &it) const {
        m_header.to_bytes(it);
        std::move(m_payload.begin(), m_payload.end(), it);
    }

    template <shared::ByteRangeReader R>
    static Frame decode_pre_header(R &&range, const std::uint32_t &max_frame_size) {
        auto header = FrameHeader::from_bytes(range | std::views::take(HEADER_SIZE), max_frame_size);
        const std::uint32_t payload_len = header.get_length();

        // Verify total length
        if (std::ranges::size(range) < HEADER_SIZE + payload_len) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Buffer contains truncated payload");
        }

        std::vector<std::byte> payload = range | std::views::drop(HEADER_SIZE) | std::views::take(payload_len) |
                                         std::ranges::to<std::vector<std::byte>>();
        return Frame(std::move(header), std::move(payload));
    }

    template <shared::ByteIteratorReader It>
    static Frame decode_pre_header(It &it, const std::uint32_t &max_frame_size) {
        return decode_pre_header(std::ranges::subrange(it, std::default_sentinel), max_frame_size);
    }

    template <shared::ByteRangeReader R>
    static Frame decode_post_header(R &&range, FrameHeader header) {
        const std::uint32_t payload_len = header.get_length();
        if (std::ranges::distance(range) < payload_len) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Buffer contains truncated payload");
        }
        std::vector<std::byte> payload =
            range | std::views::take(payload_len) | std::ranges::to<std::vector<std::byte>>();
        return Frame(std::move(header), std::move(payload));
    }

    template <shared::ByteIteratorReader It>
    static Frame decode_post_header(It &it, FrameHeader header) {
        return decode_post_header(std::ranges::subrange(it, std::default_sentinel), std::move(header));
    }

    std::uint32_t get_window_increament() const {
        std::uint32_t inc = shared_layer::Atom<>::read_big_endian(std::span(m_payload).subspan<0, 4>()) & 0x7FFFFFFF;
        if (inc == 0) {
            if (m_header.get_stream_id() == 0)
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "WINDOW_UPDATE inc 0 on stream 0");
            else
                throw error::http::StreamError(m_header.get_stream_id(), error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "WINDOW_UPDATE inc 0");
        }

        return inc;
    }

    const std::uint32_t &get_stream_id() const noexcept { return m_header.get_stream_id(); }
    const std::uint32_t &get_payload_size() const noexcept { return m_header.get_length(); }
    const FrameHeader &get_header() const noexcept { return m_header; }
    std::size_t get_size() const noexcept { return get_payload_size() + get_header().get_size(); }
    std::span<const std::byte> get_payload() const noexcept { return m_payload; }

    // TODO: This is a bit hacky, but it avoids unnecessary copying when we know the payload is actually bytes (e.g.
    // for HPACK).
    std::span<const std::uint8_t> get_payload_as_u8() const noexcept {
        return std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t *>(m_payload.data()),
                                             m_payload.size()};
    }

  private:
    void validate() {
        const auto id = m_header.get_stream_id();
        const auto type = m_header.get_type();

        if constexpr (Role == shared_layer::FrameRole::Sender) {
            if (id % 2 != 0) {
                if (type == shared_layer::FrameType::PUSH_PROMISE) {
                    throw error::http::ConnectionError(error::http::Http2ErrorCode::INTERNAL_ERROR,
                                                       "Server-initiated PUSH_PROMISE must use even stream ID");
                }
            }
        }

        switch (type) {
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

        if (m_payload.size() != m_header.get_length()) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::INTERNAL_ERROR,
                std::format("Payload size mismatch for frame type {}: expected {}, got {}", std::to_underlying(type),
                            m_header.get_length(), m_payload.size()),
                get_stream_id());
        }
    }

    void validate_data() {
        if (m_header.get_stream_id() == 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "DATA on stream 0");
        if (m_header.get_flags() & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::PADDED))
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for DATA",
                                               get_stream_id());

        if (m_header.get_flags() & shared_layer::Flags::PADDED) {
            if (m_header.get_length() < 1)
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "DATA too short for padding", get_stream_id());
            if (std::to_integer<std::uint32_t>(m_payload[0]) >= m_header.get_length())
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Padding exceeds frame",
                                                   get_stream_id());
        }
    }

    void validate_headers() {
        if (m_header.get_stream_id() == 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "HEADERS on stream 0");

        if (m_header.get_flags() & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::END_HEADERS |
                                     shared_layer::Flags::PADDED | shared_layer::Flags::PRIORITY))
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for HEADERS",
                                               get_stream_id());

        std::uint32_t min_len = 0;
        if (m_header.get_flags() & shared_layer::Flags::PADDED)
            min_len += 1;
        if (m_header.get_flags() & shared_layer::Flags::PRIORITY)
            min_len += 5;

        if (m_header.get_length() < min_len)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "HEADERS length too short", get_stream_id());

        if (m_header.get_flags() & shared_layer::Flags::PADDED) {
            if (std::to_integer<std::uint32_t>(m_payload[0]) >= m_header.get_length())
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Padding exceeds payload size", get_stream_id());
        }
    }

    void validate_priority() {
        if (m_header.get_stream_id() == 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "PRIORITY on stream 0");
        if (m_header.get_flags() != 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "PRIORITY flags must be 0",
                                               get_stream_id());
        if (m_header.get_length() != 5)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "PRIORITY length must be 5", get_stream_id());
    }

    void validate_rst_stream() {
        if (m_header.get_stream_id() == 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "RST_STREAM on stream 0");
        if (m_header.get_flags() != 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "RST_STREAM flags must be 0", get_stream_id());
        if (m_header.get_length() != 4)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "RST_STREAM length must be 4", get_stream_id());
    }

    void validate_settings() {
        if (m_header.get_stream_id() != 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "SETTINGS not on stream 0");
        if (m_header.get_flags() & ~shared_layer::Flags::ACK)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for SETTINGS");

        if (m_header.get_flags() & shared_layer::Flags::ACK) {
            if (m_header.get_length() != 0)
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "SETTINGS ACK with payload");
        } else if (m_header.get_length() % 6 != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "SETTINGS size mismatch");
        }
    }

    void validate_push_promise() {
        if constexpr (Role == shared_layer::FrameRole::Receiver) {
            // §6.6: A receiver (server) MUST treat receipt of PUSH_PROMISE as a connection error
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PUSH_PROMISE received by server");
        } else {
            // Sender validation for server PUSH_PROMISE
            if (m_header.get_stream_id() == 0)
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "PUSH_PROMISE on stream 0");
            if (m_header.get_flags() & ~(shared_layer::Flags::END_HEADERS | shared_layer::Flags::PADDED))
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Invalid flags for PUSH_PROMISE", get_stream_id());
        }
    }

    void validate_ping() {
        if (m_header.get_stream_id() != 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "PING not on stream 0");
        if (m_header.get_flags() & ~shared_layer::Flags::ACK)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for PING");
        if (m_header.get_length() != 8)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "PING length must be 8");
    }

    void validate_goaway() {
        if (m_header.get_stream_id() != 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "GOAWAY not on stream 0");
        if (m_header.get_flags() != 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "GOAWAY flags must be 0");
        if (m_header.get_length() < 8)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "GOAWAY length < 8");
    }

    void validate_window_update() {
        if (m_header.get_flags() != 0) {
            auto stream_id = m_header.get_stream_id() == 0 ? MAX_CONNECTED_STREAMS : get_stream_id();
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "WINDOW_UPDATE flags must be 0", stream_id);
        }
        if (m_header.get_length() != 4) {
            auto stream_id = m_header.get_stream_id() == 0 ? MAX_CONNECTED_STREAMS : get_stream_id();
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "WINDOW_UPDATE length != 4", stream_id);
        }
    }

    void validate_continuation() {
        if (m_header.get_stream_id() == 0)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "CONTINUATION on stream 0");
        if (m_header.get_flags() & ~shared_layer::Flags::END_HEADERS)
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for CONTINUATION", get_stream_id());
    }

    FrameHeader m_header;
    std::vector<std::byte> m_payload;
};

} // namespace io::layer::http2
