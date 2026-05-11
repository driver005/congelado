module;
#include <ranges>
export module io_layer_http2:frame;

import std;
import io_layer_shared;
import io_error;
import core_logger;
import shared;
import utils_codec;
import :consts;

export namespace io::layer::http2 {

class FrameHeader {
  public:
    FrameHeader() : m_length{0}, m_type{shared_layer::FrameType::DATA}, m_flags{0}, m_stream_id{0} {
        core::logger::debug(
            "FrameHeader",
            "Default constructor called, initialized with length `0`, type `DATA`, flags `0`, stream_id `0`");
    }

    FrameHeader(std::uint32_t length, shared_layer::FrameType type, std::uint8_t flags, std::uint32_t stream_id)
        : m_length(length), m_type(type), m_flags(flags) {
        set_stream_id(stream_id);
        core::logger::debug("FrameHeader",
                            "Constructor called, initialized with length `{}`, type `{}`, flags `{}`, stream_id `{}`",
                            length, type, flags, stream_id);
    }

    FrameHeader &&add_length(std::uint32_t len) && noexcept {
        m_length = len;
        core::logger::debug("FrameHeader", "add_length called, set length to `{}`", len);
        return std::move(*this);
    }

    FrameHeader &&add_type(shared_layer::FrameType type) && noexcept {
        m_type = type;
        core::logger::debug("FrameHeader", "add_type called, set type to `{}`", type);
        return std::move(*this);
    }

    FrameHeader &&add_flags(std::uint8_t flags) && noexcept {
        m_flags = flags;
        core::logger::debug("FrameHeader", "add_flags called, set flags to `{}`", flags);
        return std::move(*this);
    }

    FrameHeader &&add_stream_id(std::uint32_t stream_id) && noexcept {
        set_stream_id(stream_id);
        core::logger::debug("FrameHeader", "add_stream_id called, set stream_id to `{}`", stream_id);
        return std::move(*this);
    }

    constexpr std::size_t get_size() const noexcept { return HEADER_SIZE; }
    const std::uint32_t &get_length() const noexcept { return m_length; }
    const shared_layer::FrameType &get_type() const noexcept { return m_type; }
    const std::uint8_t &get_flags() const noexcept { return m_flags; }
    const std::uint32_t &get_stream_id() const noexcept { return m_stream_id; }

    void set_length(std::uint32_t len) noexcept { m_length = len; }
    void set_type(shared_layer::FrameType t) noexcept { m_type = t; }
    void set_flags(std::uint8_t flags) noexcept { m_flags = flags; }
    void set_stream_id(std::uint32_t new_id) noexcept { m_stream_id = new_id & 0x7FFFFFFF; }

  private:
    std::uint32_t m_length;
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::uint32_t m_stream_id;
};

struct ReadFrameHeaderAdaptor : std::ranges::range_adaptor_closure<ReadFrameHeaderAdaptor> {
    explicit constexpr ReadFrameHeaderAdaptor(std::uint32_t max_frame_size) : m_max_frame_size{max_frame_size} {}

    template <std::ranges::viewable_range R>
    FrameHeader operator()(R &&range) const {
        if (std::ranges::size(range) < HEADER_SIZE) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Incomplete frame header");
        }

        std::uint32_t len = range | std::views::take(3) | utils::codec::ReadBigEndianAdaptor<std::uint32_t>{};

        if (len > m_max_frame_size) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Frame length exceeds SETTINGS_MAX_FRAME_SIZE");
        }

        auto type = static_cast<shared_layer::FrameType>(range | std::views::drop(3) | std::views::take(1) |
                                                         utils::codec::ReadBigEndianAdaptor<std::uint8_t>{});

        auto flags =
            range | std::views::drop(4) | std::views::take(1) | utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};

        std::uint32_t id = 0x7FFF'FFFFU & (range | std::views::drop(5) | std::views::take(4) |
                                           utils::codec::ReadBigEndianAdaptor<std::uint32_t>{});

        return {len, type, flags, id};
    }

    std::uint32_t m_max_frame_size;
};

struct WriteFrameHeaderAdaptor : std::ranges::range_adaptor_closure<WriteFrameHeaderAdaptor> {
    explicit constexpr WriteFrameHeaderAdaptor(FrameHeader header) : m_header{header} {}

    template <std::ranges::viewable_range R>
    constexpr auto operator()(R &&range) const {
        return range |
               (utils::codec::WriteBigEndianAdaptor<std::uint32_t>{m_header.get_length()} | std::views::take(3)) |
               utils::codec::WriteBigEndianAdaptor<std::uint8_t>{std::to_underlying(m_header.get_type())} |
               utils::codec::WriteBigEndianAdaptor<std::uint8_t>{m_header.get_flags()} |
               utils::codec::WriteBigEndianAdaptor<std::uint32_t>{m_header.get_stream_id() & 0x7FFFFFFF};
    }

    FrameHeader m_header;
};

template <shared_layer::FrameRole Role>
class Frame {
  public:
    Frame() {
        core::logger::debug("Frame", "Default constructor called, initialized with default header and empty payload");
    }

    Frame(FrameHeader header, std::vector<std::byte> payload) : m_header(header), m_payload(std::move(payload)) {
        core::logger::debug("Frame", "Constructor called with header and payload, validating frame");
        validate();
    }

    Frame &&add_header(FrameHeader header) && noexcept {
        m_header = header;
        core::logger::debug("Frame",
                            "add_header called, set header with type `{}`, length `{}`, flags `{}`, and stream_id `{}`",
                            m_header.get_type(), m_header.get_length(), m_header.get_flags(), m_header.get_stream_id());
        return std::move(*this);
    }

    template <std::ranges::forward_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    Frame &&add_payload(R &&payload) && noexcept {
        m_payload.assign(std::ranges::begin(payload), std::ranges::end(payload));
        core::logger::debug("Frame", "add_payload called, set payload size to `{}`", m_payload.size());
        return std::move(*this);
    }

    Frame &&build() && {
        core::logger::debug("Frame", "build called, validating frame");
        validate();
        return std::move(*this);
    }

    [[nodiscard]] std::uint32_t get_window_increament() const {
        std::uint32_t inc = shared_layer::Atom<>::read_big_endian(std::span(m_payload).subspan<0, 4>()) & 0x7FFFFFFF;
        if (inc == 0) {
            if (m_header.get_stream_id() == 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "WINDOW_UPDATE inc 0 on stream 0");
            }
            throw error::http::StreamError(m_header.get_stream_id(), error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                           "WINDOW_UPDATE inc 0");
        }

        return inc;
    }

    [[nodiscard]] const std::uint32_t &get_stream_id() const noexcept { return m_header.get_stream_id(); }
    [[nodiscard]] const std::uint32_t &get_payload_size() const noexcept { return m_header.get_length(); }
    [[nodiscard]] const FrameHeader &get_header() const noexcept { return m_header; }
    [[nodiscard]] std::size_t get_size() const noexcept { return get_payload_size() + get_header().get_size(); }
    [[nodiscard]] std::span<const std::byte> get_payload() const noexcept { return m_payload; }

    // TODO: This is a bit hacky, but it avoids unnecessary copying when we know the payload is actually bytes (e.g.
    //  for HPACK).
    [[nodiscard]] std::span<const std::uint8_t> get_payload_as_u8() const noexcept {
        return std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t *>(m_payload.data()),
                                             m_payload.size()};
    }

  private:
    void validate() {
        const auto STREAM_ID = m_header.get_stream_id();
        const auto TYPE = m_header.get_type();

        core::logger::debug("Frame", "Validating frame with type {} and stream_id {}", TYPE, STREAM_ID);

        if constexpr (Role == shared_layer::FrameRole::Sender) {
            if (STREAM_ID % 2 != 0) {
                if (TYPE == shared_layer::FrameType::PUSH_PROMISE) {
                    throw error::http::ConnectionError(error::http::Http2ErrorCode::INTERNAL_ERROR,
                                                       "Server-initiated PUSH_PROMISE must use even stream ID");
                }
            }
        }

        switch (TYPE) {
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
                std::format("Payload size mismatch for frame type {}: expected {}, got {}", std::to_underlying(TYPE),
                            m_header.get_length(), m_payload.size()),
                get_stream_id());
        }
    }

    void validate_data() {
        if (m_header.get_stream_id() == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "DATA on stream 0");
        }
        if ((m_header.get_flags() & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::PADDED)) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for DATA",
                                               get_stream_id());
        }

        if ((m_header.get_flags() & shared_layer::Flags::PADDED) != 0) {
            if (m_header.get_length() < 1) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "DATA too short for padding", get_stream_id());
            }
            if (std::to_integer<std::uint32_t>(m_payload[0]) >= m_header.get_length()) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Padding exceeds frame",
                                                   get_stream_id());
            }
        }
    }

    void validate_headers() {
        if (m_header.get_stream_id() == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "HEADERS on stream 0");
        }

        if ((m_header.get_flags() & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::END_HEADERS |
                                      shared_layer::Flags::PADDED | shared_layer::Flags::PRIORITY)) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for HEADERS",
                                               get_stream_id());
        }

        std::uint32_t min_len = 0;
        if ((m_header.get_flags() & shared_layer::Flags::PADDED) != 0) {
            min_len += 1;
        }
        if ((m_header.get_flags() & shared_layer::Flags::PRIORITY) != 0) {
            min_len += 5;
        }

        if (m_header.get_length() < min_len) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "HEADERS length too short", get_stream_id());
        }

        if ((m_header.get_flags() & shared_layer::Flags::PADDED) != 0) {
            if (std::to_integer<std::uint32_t>(m_payload[0]) >= m_header.get_length()) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Padding exceeds payload size", get_stream_id());
            }
        }
    }

    void validate_priority() {
        if (m_header.get_stream_id() == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "PRIORITY on stream 0");
        }
        if (m_header.get_flags() != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "PRIORITY flags must be 0",
                                               get_stream_id());
        }
        if (m_header.get_length() != 5) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "PRIORITY length must be 5", get_stream_id());
        }
    }

    void validate_rst_stream() {
        if (m_header.get_stream_id() == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "RST_STREAM on stream 0");
        }
        if (m_header.get_flags() != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "RST_STREAM flags must be 0", get_stream_id());
        }
        if (m_header.get_length() != 4) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "RST_STREAM length must be 4", get_stream_id());
        }
    }

    void validate_settings() {
        if (m_header.get_stream_id() != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "SETTINGS not on stream 0");
        }
        if ((m_header.get_flags() & ~shared_layer::Flags::ACK) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for SETTINGS");
        }

        if ((m_header.get_flags() & shared_layer::Flags::ACK) != 0) {
            if (m_header.get_length() != 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "SETTINGS ACK with payload");
            }
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
            if (m_header.get_stream_id() == 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "PUSH_PROMISE on stream 0");
            }
            if ((m_header.get_flags() & ~(shared_layer::Flags::END_HEADERS | shared_layer::Flags::PADDED)) != 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Invalid flags for PUSH_PROMISE", get_stream_id());
            }
        }
    }

    void validate_ping() {
        if (m_header.get_stream_id() != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "PING not on stream 0");
        }
        if ((m_header.get_flags() & ~shared_layer::Flags::ACK) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "Invalid flags for PING");
        }
        if (m_header.get_length() != 8) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "PING length must be 8");
        }
    }

    void validate_goaway() {
        if (m_header.get_stream_id() != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "GOAWAY not on stream 0");
        }
        if (m_header.get_flags() != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "GOAWAY flags must be 0");
        }
        if (m_header.get_length() < 8) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR, "GOAWAY length < 8");
        }
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
        if (m_header.get_stream_id() == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR, "CONTINUATION on stream 0");
        }
        if ((m_header.get_flags() & ~shared_layer::Flags::END_HEADERS) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for CONTINUATION", get_stream_id());
        }
    }

    FrameHeader m_header;
    std::vector<std::byte> m_payload;
};

struct ReadFrameAdaptor : std::ranges::range_adaptor_closure<ReadFrameAdaptor> {
    explicit constexpr ReadFrameAdaptor(std::uint32_t max_frame_size) : m_max_frame_size{max_frame_size} {}

    template <shared::ByteRangeReader R, shared_layer::FrameRole Role>
    Frame<Role> operator()(R &&range) const {
        auto header = range | std::views::take(HEADER_SIZE) | ReadFrameHeaderAdaptor{m_max_frame_size};

        const std::uint32_t PAYLOAD_LEN = header.get_length();

        if (std::ranges::size(range) < HEADER_SIZE + PAYLOAD_LEN) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Buffer contains truncated payload");
        }

        std::vector<std::byte> payload = range | std::views::drop(HEADER_SIZE) | std::views::take(PAYLOAD_LEN) |
                                         std::ranges::to<std::vector<std::byte>>();

        return Frame{std::move(header), std::move(payload)};
    }

    std::uint32_t m_max_frame_size;
};

template <shared_layer::FrameRole Role>
struct ReadFramePostHeaderAdaptor : std::ranges::range_adaptor_closure<ReadFramePostHeaderAdaptor<Role>> {
    explicit constexpr ReadFramePostHeaderAdaptor(FrameHeader header) : m_header{header} {}

    template <shared::ByteRangeReader R>
    Frame<Role> operator()(R &&range) const {
        const std::uint32_t PAYLOAD_LEN = m_header.get_length();

        if (std::ranges::distance(range) < static_cast<std::ranges::range_difference_t<R>>(PAYLOAD_LEN)) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Buffer contains truncated payload");
        }

        std::vector<std::byte> payload =
            range | std::views::take(PAYLOAD_LEN) | std::ranges::to<std::vector<std::byte>>();

        return Frame<Role>{std::move(m_header), std::move(payload)};
    }

    FrameHeader m_header;
};

template <shared_layer::FrameRole Role>
struct WriteFrameAdaptor : std::ranges::range_adaptor_closure<WriteFrameAdaptor<Role>> {
    explicit constexpr WriteFrameAdaptor(Frame<Role> frame) : m_frame{std::move(frame)} {}

    auto operator()() const {
        return std::views::concat(std::views::empty<std::byte> | WriteFrameHeaderAdaptor{m_frame.get_header()},
                                  m_frame.get_payload());
    }

    template <shared::ByteRangeReader R>
    auto operator()(R &&range) const {
        return std::views::concat(range | WriteFrameHeaderAdaptor{m_frame.get_header()}, m_frame.get_payload());
    }

    Frame<Role> m_frame;
};

} // namespace io::layer::http2
