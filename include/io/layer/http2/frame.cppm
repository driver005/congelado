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

export namespace io::layer::http2 {

template <shared_layer::FrameRole Role>
class FrameHeader {
  public:
    FrameHeader() : m_length{0}, m_type{shared_layer::FrameType::DATA}, m_flags{0}, m_stream_id{0} {
    }

    FrameHeader(std::uint32_t length, shared_layer::FrameType type, std::uint8_t flags,
                std::uint32_t stream_id)
        : m_length{length}, m_type{type}, m_flags{flags}, m_stream_id{0} {
        set_stream_id(stream_id);
    }

    FrameHeader &&add_length(std::uint32_t len) && noexcept {
        core::logger::debug("FrameHeader", "len={}", len);

        m_length = len;
        return std::move(*this);
    }

    FrameHeader &&add_type(shared_layer::FrameType type) && noexcept {
        core::logger::debug("FrameHeader", "type={}", type);

        m_type = type;
        return std::move(*this);
    }

    FrameHeader &&add_flags(std::uint8_t flags) && noexcept {
        core::logger::debug("FrameHeader", "flags={}", flags);

        m_flags = flags;
        return std::move(*this);
    }

    FrameHeader &&add_stream_id(std::uint32_t stream_id) && noexcept {
        core::logger::debug("FrameHeader", "stream_id={}", stream_id);

        set_stream_id(stream_id);
        return std::move(*this);
    }


    void validate() {
        core::logger::debug("FrameBuilder", "validate type={} stream={}", m_type, m_stream_id);

        if constexpr (Role == shared_layer::FrameRole::SENDER) {
            if (m_stream_id % 2 != 0) {
                if (m_type == shared_layer::FrameType::PUSH_PROMISE) {
                    throw error::http::ConnectionError(
                        error::http::Http2ErrorCode::INTERNAL_ERROR,
                        "Server-initiated PUSH_PROMISE must use even stream ID");
                }
            }
        }

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

    void validate_payload_size(std::size_t actual_size) const {
        if (actual_size != m_length) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::INTERNAL_ERROR,
                std::format("Payload size mismatch for frame type {}: expected {}, got {}",
                            std::to_underlying(m_type), m_length, actual_size),
                get_stream_id());
        }
    }

    void validate_padding(std::uint32_t actual_size) const {
        if (actual_size >= m_length) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Padding exceeds frame ", get_stream_id());
        }
    }

    [[nodiscard]] bool is_end_stream() const noexcept {
        return (m_flags & shared_layer::Flags::END_STREAM) != 0;
    }
    [[nodiscard]] bool is_padded() const noexcept {
        return (m_flags & shared_layer::Flags::PADDED) != 0;
    }

    [[nodiscard]] constexpr std::size_t get_size() const noexcept { return HEADER_SIZE; }
    [[nodiscard]] const std::uint32_t &get_length() const noexcept { return m_length; }
    [[nodiscard]] const shared_layer::FrameType &get_type() const noexcept { return m_type; }
    [[nodiscard]] const std::uint8_t &get_flags() const noexcept { return m_flags; }
    [[nodiscard]] const std::uint32_t &get_stream_id() const noexcept { return m_stream_id; }

    void set_length(std::uint32_t len) noexcept { m_length = len; }
    void set_type(shared_layer::FrameType type) noexcept { m_type = type; }
    void set_flags(std::uint8_t flags) noexcept { m_flags = flags; }
    void set_stream_id(std::uint32_t new_id) noexcept { m_stream_id = new_id & 0x7FFFFFFF; }

  private:
    void validate_data() const {
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "DATA on stream 0");
        }
        if ((m_flags & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::PADDED)) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for DATA", m_stream_id);
        }

        if ((m_flags & shared_layer::Flags::PADDED) != 0) {
            if (m_length < 1) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "DATA too short for padding", m_stream_id);
            }
        }
    }

    void validate_headers() const {
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "HEADERS on stream 0");
        }

        if ((m_flags & ~(shared_layer::Flags::END_STREAM | shared_layer::Flags::END_HEADERS |
                         shared_layer::Flags::PADDED | shared_layer::Flags::PRIORITY)) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for HEADERS", m_stream_id);
        }

        std::uint32_t min_len = 0;
        if ((m_flags & shared_layer::Flags::PADDED) != 0) {
            min_len += 1;
        }
        if ((m_flags & shared_layer::Flags::PRIORITY) != 0) {
            min_len += 5;
        }

        if (m_length < min_len) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "HEADERS length too short", m_stream_id);
        }

        if ((m_flags & shared_layer::Flags::PADDED) != 0) {
            if (m_length < 1) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "DATA too short for padding", m_stream_id);
            }
        }
    }

    void validate_priority() const {
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PRIORITY on stream 0");
        }
        if (m_flags != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PRIORITY flags must be 0", m_stream_id);
        }
        if (m_length != 5) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "PRIORITY length must be 5", m_stream_id);
        }
    }

    void validate_rst_stream() const {
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "RST_STREAM on stream 0");
        }
        if (m_flags != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "RST_STREAM flags must be 0", m_stream_id);
        }
        if (m_length != 4) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "RST_STREAM length must be 4", m_stream_id);
        }
    }

    void validate_settings() const {
        if (m_stream_id != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "SETTINGS not on stream 0");
        }
        if ((m_flags & ~shared_layer::Flags::ACK) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for SETTINGS");
        }

        if ((m_flags & shared_layer::Flags::ACK) != 0) {
            if (m_length != 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                                   "SETTINGS ACK with payload");
            }
        } else if (m_length % 6 != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "SETTINGS size mismatch");
        }
    }

    void validate_push_promise() const {
        if constexpr (Role == shared_layer::FrameRole::RECEIVER) {
            // A receiver (server) MUST treat receipt of PUSH_PROMISE as a connection error
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PUSH_PROMISE received by server");
        } else {
            // Sender validation for server PUSH_PROMISE
            if (m_stream_id == 0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "PUSH_PROMISE on stream 0");
            }
            if ((m_flags & ~(shared_layer::Flags::END_HEADERS | shared_layer::Flags::PADDED)) !=
                0) {
                throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                                   "Invalid flags for PUSH_PROMISE", m_stream_id);
            }
        }
    }

    void validate_ping() const {
        if (m_stream_id != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "PING not on stream 0");
        }
        if ((m_flags & ~shared_layer::Flags::ACK) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for PING");
        }
        if (m_length != 8) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "PING length must be 8");
        }
    }

    void validate_goaway() const {
        if (m_stream_id != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "GOAWAY not on stream 0");
        }
        if (m_flags != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "GOAWAY flags must be 0");
        }
        if (m_length < 8) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "GOAWAY length < 8");
        }
    }

    void validate_window_update() const {
        if (m_flags != 0) {
            auto stream_id = m_stream_id == 0 ? MAX_CONNECTED_STREAMS : m_stream_id;
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "WINDOW_UPDATE flags must be 0", stream_id);
        }
        if (m_length != 4) {
            auto stream_id = m_stream_id == 0 ? MAX_CONNECTED_STREAMS : m_stream_id;
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "WINDOW_UPDATE length != 4", stream_id);
        }
    }

    void validate_continuation() const {
        if (m_stream_id == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "CONTINUATION on stream 0");
        }
        if ((m_flags & ~shared_layer::Flags::END_HEADERS) != 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "Invalid flags for CONTINUATION", m_stream_id);
        }
    }

    std::uint32_t m_length;
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::uint32_t m_stream_id;
};

class ReadFrameHeaderAdaptor : public std::ranges::range_adaptor_closure<ReadFrameHeaderAdaptor> {
  public:
    explicit constexpr ReadFrameHeaderAdaptor(std::uint32_t max_frame_size)
        : m_max_frame_size{max_frame_size} {}

    template <std::ranges::viewable_range R>
    FrameHeader<shared_layer::FrameRole::RECEIVER> operator()(R &&data) const {
        auto range = std::forward<R>(data);

        if (std::ranges::size(range) < HEADER_SIZE) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                                               "Incomplete frame header");
        }

        std::uint32_t len =
            range | std::views::take(3) | utils::codec::ReadBigEndianAdaptor<std::uint32_t>{};

        if (len > m_max_frame_size) {
            throw error::http::ConnectionError(
                error::http::Http2ErrorCode::FRAME_SIZE_ERROR,
                "FrameBuilder length exceeds SETTINGS_MAX_FRAME_SIZE");
        }

        auto type = static_cast<shared_layer::FrameType>(
            range | std::views::drop(3) | std::views::take(1) |
            utils::codec::ReadBigEndianAdaptor<std::uint8_t>{});

        auto flags = range | std::views::drop(4) | std::views::take(1) |
                     utils::codec::ReadBigEndianAdaptor<std::uint8_t>{};

        std::uint32_t id = 0x7FFF'FFFFU & (range | std::views::drop(5) | std::views::take(4) |
                                           utils::codec::ReadBigEndianAdaptor<std::uint32_t>{});

        return {len, type, flags, id};
    }

  private:
    std::uint32_t m_max_frame_size;
};

// TODO: to be removed
// struct WriteFrameHeaderAdaptor : std::ranges::range_adaptor_closure<WriteFrameHeaderAdaptor> {
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


class FrameHeaderClosureAdaptor
    : public std::ranges::range_adaptor_closure<FrameHeaderClosureAdaptor> {
  public:
    explicit constexpr FrameHeaderClosureAdaptor(std::uint32_t length, shared_layer::FrameType type,
                                                 std::uint8_t flags,
                                                 std::uint32_t stream_id) noexcept
        : m_length{length}, m_type{type}, m_flags{flags},
          m_clean_stream_id{stream_id & 0x7FFFFFFF} {}

    template <std::ranges::viewable_range R>
    [[nodiscard]] constexpr auto operator()(R &&range) const {
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

class WriteFrameClosureAdapter
    : public std::ranges::range_adaptor_closure<WriteFrameClosureAdapter> {
  public:
    explicit constexpr WriteFrameClosureAdapter(std::uint32_t stream_id,
                                                shared_layer::FrameType type, std::uint8_t flags,
                                                std::size_t max_frame_size,
                                                bool end_stream_after_data = false,
                                                bool no_data = false)
        : m_stream_id{stream_id}, m_type{type}, m_flags{flags}, m_max_frame_size{max_frame_size},
          m_end_stream_after_data{end_stream_after_data}, m_no_data{no_data} {}

    template <std::ranges::viewable_range R>
    auto operator()(R &&range) const {
        auto data = std::views::all(std::forward<R>(range));

        const std::size_t total_len = std::ranges::distance(data);
        const std::size_t slice_size = std::min(m_max_frame_size, total_len);

        const std::size_t total_chunks =
            total_len == 0 ? 1 : (total_len + slice_size - 1) / slice_size;

        auto chunked = std::views::iota(0uz, total_chunks) |
                       std::views::transform([data, slice_size](std::size_t chunk_idx) {
                           return data | std::views::drop(chunk_idx * slice_size) |
                                  std::views::take(slice_size);
                       });

        // Unified pipeline
        return chunked | std::views::enumerate |
               std::views::transform([self = *this, total_chunks](auto &&entry) {
                   auto [idx, chunk] = entry;
                   const std::size_t chunk_idx = static_cast<std::size_t>(idx);

                   auto type = self.m_type;
                   std::uint8_t flags = self.m_flags;

                   const bool is_last = (chunk_idx == total_chunks - 1);

                   if (type == shared_layer::FrameType::HEADERS) {
                       if (is_last) {
                           flags |= shared_layer::Flags::END_HEADERS;
                           if (self.m_no_data) {
                               flags |= shared_layer::Flags::END_STREAM;
                           }
                       }
                       if (chunk_idx != 0)
                           type = shared_layer::FrameType::CONTINUATION;
                   } else if (type == shared_layer::FrameType::DATA && is_last &&
                              !self.m_end_stream_after_data) {
                       flags |= shared_layer::Flags::END_STREAM;
                   }

                   return std::views::concat(
                       std::views::empty<std::byte> |
                           FrameHeaderClosureAdaptor{
                               static_cast<std::uint32_t>(std::ranges::distance(chunk)), type,
                               flags, self.m_stream_id},
                       chunk);
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

struct ReadWindowIncrementAdaptor : std::ranges::range_adaptor_closure<ReadWindowIncrementAdaptor> {
    template <std::ranges::viewable_range R>
    std::uint32_t operator()(R &&range) const {
        std::uint32_t inc =
            (std::forward<R>(range) | std::views::take(4) | utils::codec::ReadBigEndianAdaptor{}) &
            0x7FFFFFFF;

        if (inc == 0) {
            throw error::http::ConnectionError(error::http::Http2ErrorCode::PROTOCOL_ERROR,
                                               "WINDOW_UPDATE increment must be non-zero");
        }

        return inc;
    }
};

template <shared_layer::FrameRole Role>
class FrameBuilder {
  public:
    FrameBuilder() : m_type{shared_layer::FrameType::DATA}, m_flags{0}, m_stream_id{0} {
    }

    FrameBuilder &&add_type(shared_layer::FrameType type) && noexcept {
        core::logger::debug("FrameBuilder", "type={}", type);

        m_type = type;
        return std::move(*this);
    }

    FrameBuilder &&add_flags(std::uint8_t flags) && noexcept {
        core::logger::debug("FrameBuilder", "flags={}", flags);

        m_flags = flags;
        return std::move(*this);
    }

    FrameBuilder &&add_stream_id(std::uint32_t stream_id) && noexcept {
        core::logger::debug("FrameBuilder", "stream_id={}", stream_id);

        m_stream_id = stream_id & 0x7FFFFFFF;
        return std::move(*this);
    }

    template <std::ranges::forward_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    FrameBuilder &&add_payload(R &&payload) && noexcept {
        m_payload.append_range(std::forward<R>(payload));
        core::logger::debug("FrameBuilder", "payload size={}", m_payload.size());
        return std::move(*this);
    }

    FrameBuilder &&build() && {
        core::logger::debug("FrameBuilder", "build");
        return std::move(*this);
    }

    template <std::ranges::forward_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    void expand_payload(R &&payload) noexcept {
        m_payload.append_range(std::forward<R>(payload));
        core::logger::debug("FrameBuilder", "payload total={}", m_payload.size());
    }

    [[nodiscard]] std::size_t get_size() const noexcept { return HEADER_SIZE + m_payload.size(); }
    [[nodiscard]] std::span<const std::byte> get_payload() const noexcept { return m_payload; }
    [[nodiscard]] std::size_t get_length() const noexcept { return m_payload.size(); }
    [[nodiscard]] shared_layer::FrameType get_type() const noexcept { return m_type; }
    [[nodiscard]] std::uint8_t get_flags() const noexcept { return m_flags; }
    [[nodiscard]] std::uint32_t get_stream_id() const noexcept { return m_stream_id; }

  private:
    shared_layer::FrameType m_type;
    std::uint8_t m_flags;
    std::uint32_t m_stream_id;
    std::vector<std::byte> m_payload;
};


class WriteFrameBuilderAdaptor
    : public std::ranges::range_adaptor_closure<WriteFrameBuilderAdaptor> {
  public:
    explicit constexpr WriteFrameBuilderAdaptor(FrameBuilder<shared_layer::FrameRole::SENDER> frame,
                                                std::size_t max_frame_size,
                                                bool end_stream_after_data = false,
                                                bool no_data = false)
        : m_frame{std::move(frame)}, m_max_frame_size{max_frame_size},
          m_end_stream_after_data{end_stream_after_data}, m_no_data{no_data} {}

    template <std::ranges::viewable_range R>
    auto operator()(R &&range) const {
        return std::views::concat(
            std::forward<R>(range),
            m_frame.get_payload() |
                WriteFrameClosureAdapter{m_frame.get_stream_id(), m_frame.get_type(),
                                         m_frame.get_flags(), m_max_frame_size,
                                         m_end_stream_after_data, m_no_data});
    }

    auto operator()() const { return (*this)(std::views::empty<std::byte>); }

  private:
    FrameBuilder<shared_layer::FrameRole::SENDER> m_frame;
    std::size_t m_max_frame_size;
    bool m_end_stream_after_data;
    bool m_no_data;
};


} // namespace io::layer::http2
