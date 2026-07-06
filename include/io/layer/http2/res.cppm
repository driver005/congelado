module;
#include <cassert>
#include <ranges>
export module io_layer_http2:response;

import std;
import hashmap;
import shared;
import interfaces;
import io_layer_shared;
import io_codec_hpack;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpResponse : public interfaces::io::IResponse {
  public:
    explicit HttpResponse(std::uint32_t stream_id)
        : interfaces::io::IResponse{stream_id}, m_static_headers{} {}

    HttpResponse(const HttpResponse &) = delete;
    HttpResponse &operator=(const HttpResponse &) = delete;
    constexpr HttpResponse(HttpResponse &&) noexcept = default;
    constexpr HttpResponse &operator=(HttpResponse &&) noexcept = default;

    void set_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token,
                    std::string_view value) &
        override {
        std::visit(
            [this, value](auto &&name) {
                using T = std::decay_t<decltype(name)>;

                if constexpr (std::is_same_v<T, std::string_view>) {
                    if (name.empty()) {
                        throw std::invalid_argument("Header name cannot be empty");
                    }

                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (!token_opt.has_value()) {
                        if (!value.empty()) {
                            if (auto existing_opt = m_headers.find(name);
                                existing_opt.has_value()) {
                                const auto &existing = *existing_opt;
                                existing->set_value(existing->get_value() +
                                                    interfaces::consts::VALUE_SEPARATOR +
                                                    std::string(value));
                            }
                            return;
                        }
                        m_headers.insert(name, std::make_shared<interfaces::io::HeaderField<false>>(
                                                   name, value));
                    }

                    auto token = token_opt.value();

                    if (token == interfaces::io::types::Token::COOKIE) {
                        const auto IDX = std::to_underlying(interfaces::io::types::Token::COOKIE);
                        if (m_static_headers[IDX] == nullptr) {
                            m_static_headers[IDX] =
                                std::make_shared<interfaces::io::HeaderField<true>>(
                                    interfaces::io::types::Token::COOKIE, std::string(value));
                        } else if (!value.empty()) {
                            m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() +
                                                             interfaces::consts::COOKIE_SEPARATOR +
                                                             std::string(value));
                        }
                    } else {
                        set_header(token, value);
                    }

                } else if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    if (name == interfaces::io::types::Token::NONE) {
                        throw std::invalid_argument("interfaces::io::types::Token cannot be None");
                    }

                    m_static_headers[std::to_underlying(name)] =
                        std::make_shared<interfaces::io::HeaderField<true>>(name,
                                                                            std::string(value));
                }
            },
            name_or_token);
    }

    void remove_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token) &
        override {
        std::visit(
            [&](const auto &name) {
                using T = std::decay_t<decltype(name)>;
                if constexpr (std::is_same_v<T, interfaces::io::types::Token>) {
                    m_static_headers[std::to_underlying(name)] = nullptr;
                } else {
                    auto token_opt = interfaces::io::types::tokenize(name);
                    if (token_opt.has_value()) {
                        m_static_headers[std::to_underlying(token_opt.value())] = nullptr;
                    } else {
                        m_headers.erase(name);
                    }
                }
            },
            name_or_token);
    }

    void set_status(interfaces::io::types::Status status) & override {
        set_header(interfaces::io::types::Token::STATUS,
                   std::to_string(interfaces::io::types::status_code(status)));
    }

    [[nodiscard]] std::vector<interfaces::io::HeaderEntry> get_headers() const noexcept override {
        std::vector<interfaces::io::HeaderEntry> result;
        for (const auto &field : m_static_headers) {
            if (field != nullptr) {
                result.emplace_back(field);
            }
        }
        for (const auto &entry : m_headers) {
            result.emplace_back(entry.value());
        }
        return result;
    }

    /**
     * Returns an approximation of the total on-wire size of the complete HTTP/2 response
     * (header frames + data frames + payload).
     */
    [[nodiscard]] std::size_t get_size(const std::size_t &max_frame_payload) const noexcept {
        std::size_t total = 0;

        std::size_t header_block = std::ranges::fold_left(
            m_static_headers |
                std::views::filter([](const auto &field) noexcept { return field != nullptr; }),
            std::size_t{0},
            [](std::size_t acc, const auto &field) noexcept { return acc + field->size(); });

        // TODO: add ranges support to my swiss hashmap
        //  header_block = std::ranges::fold_left(m_headers, header_block, [](std::size_t acc, const
        //  auto &entry) noexcept {
        //      return acc + entry.value()->size();
        //  });
        for (const auto &entry : m_headers) {
            header_block += entry.value()->size();
        }

        std::size_t num_header_frames =
            (header_block == 0) ? 1 : (header_block + max_frame_payload - 1) / max_frame_payload;

        total += (num_header_frames * 9) + header_block;

        // --- DATA frames ---
        const std::size_t BODY_SIZE = m_body.size();

        if (BODY_SIZE > 0) {
            std::size_t num_data_frames = (BODY_SIZE + max_frame_payload - 1) / max_frame_payload;
            total += (num_data_frames * 9) + BODY_SIZE;
        } else {
            total += 9; // empty DATA frame with END_STREAM
        }

        return total;
    }

    void set_body(std::vector<std::byte> body) & noexcept override { m_body = std::move(body); }

    [[nodiscard]] std::span<const std::byte> get_body() const noexcept override { return m_body; }

  private:
    HttpResponse(interfaces::io::types::Status status) {
        set_header(interfaces::io::types::Token::STATUS,
                   std::to_string(interfaces::io::types::status_code(status)));
    }

    std::shared_ptr<interfaces::io::HeaderField<true>>
    get_static(const interfaces::io::types::Token &token) {
        return m_static_headers[std::to_underlying(token)];
    }

    // TODO: make length a constant
    std::array<std::shared_ptr<interfaces::io::HeaderField<true>>,
               std::to_underlying(interfaces::io::types::Token::WWW_AUTHENTICATE) + 1>
        m_static_headers{};
    hashmap::swiss::SwissHashMap<std::string_view,
                                 std::shared_ptr<interfaces::io::HeaderField<false>>>
        m_headers;
    std::vector<std::byte> m_body;
};

struct WriteHttpResponseAdaptor : std::ranges::range_adaptor_closure<WriteHttpResponseAdaptor> {
    explicit constexpr WriteHttpResponseAdaptor(HttpResponse &res, codec::hpack::HPackTable &table,
                                                std::size_t max_frame_size, std::uint8_t flags = 0)
        : m_res{res}, m_table{table}, m_max_frame_size{max_frame_size}, m_flags{flags} {}

    template <std::ranges::viewable_range R>
    auto operator()(R &&output) const {
        const auto STREAM_ID = m_res.get().get_stream_id();
        auto header_entries = m_res.get().get_headers();

        bool first_frame = true;

        codec::hpack::HpackEncoder<std::uint32_t>{
            m_table.get(), std::span<const interfaces::io::HeaderEntry>(header_entries),
            m_max_frame_size,
            [&](std::span<const std::byte> data, codec::hpack::HpackFlushReason reason) {
                const auto TYPE = first_frame ? shared_layer::FrameType::HEADERS
                                              : shared_layer::FrameType::CONTINUATION;
                const std::uint8_t FLAGS =
                    (reason == codec::hpack::HpackFlushReason::END)
                        ? static_cast<std::uint8_t>(shared_layer::Flags::END_HEADERS)
                        : std::uint8_t{0};
                output.append_range(
                    std::views::empty<std::byte> |
                    FrameHeaderClosureAdaptor{static_cast<std::uint32_t>(data.size()), TYPE, FLAGS,
                                              STREAM_ID});
                output.append_range(data);
                first_frame = false;
            }}();

        if (m_res.get().get_body().empty()) {
            std::uint8_t data_flags = m_flags | shared_layer::Flags::END_STREAM;

            auto frame = FrameBuilder<shared_layer::FrameRole::SENDER>{}
                             .add_type(shared_layer::FrameType::DATA)
                             .add_flags(data_flags)
                             .add_stream_id(STREAM_ID)
                             .build();

            output.append_range(std::views::empty<std::byte> |
                                WriteFrameBuilderAdaptor{std::move(frame), m_max_frame_size});
        } else {
            output.append_range(m_res.get().get_body() |
                                WriteFrameClosureAdapter{STREAM_ID, shared_layer::FrameType::DATA,
                                                         m_flags, m_max_frame_size});
        }
    }

    std::reference_wrapper<HttpResponse> m_res;
    std::reference_wrapper<codec::hpack::HPackTable> m_table;
    std::size_t m_max_frame_size;
    std::uint8_t m_flags;
};

} // namespace io::layer::http2
