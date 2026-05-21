module;
#include <cassert>
#include <ranges>
export module io_layer_http2:response;

import std;
import hashmap;
import shared;
import io_shared;
import io_layer_shared;
import io_codec_hpack;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpResponse : public ::interfaces::IResponse<HttpResponse, shared::http::HeaderEntry, shared::http::Token> {
  public:
    explicit HttpResponse(std::uint32_t stream_id) : m_stream_id{stream_id}, m_static_headers{} {}

    static HttpResponse ok(std::uint32_t stream_id) {
        return HttpResponse(FactoryTag{}, stream_id, interfaces::Status::OK);
    }
    static HttpResponse created(std::uint32_t stream_id) {
        return HttpResponse(FactoryTag{}, stream_id, interfaces::Status::CREATED);
    }
    static HttpResponse no_content(std::uint32_t stream_id) {
        return HttpResponse(FactoryTag{}, stream_id, interfaces::Status::NO_CONTENT);
    }
    static HttpResponse bad_request(std::uint32_t stream_id) {
        return HttpResponse(FactoryTag{}, stream_id, interfaces::Status::BAD_REQUEST);
    }
    static HttpResponse not_found(std::uint32_t stream_id) {
        return HttpResponse(FactoryTag{}, stream_id, interfaces::Status::NOT_FOUND);
    }
    static HttpResponse internal_error(std::uint32_t stream_id) {
        return HttpResponse(FactoryTag{}, stream_id, interfaces::Status::INTERNAL_SERVER_ERROR);
    }

    // --- Builder methods ---
    HttpResponse &&with_header(std::string_view name, std::string_view value) && {
        insert(name, value);
        return std::move(*this);
    }
    HttpResponse &&with_header(shared::http::Token token, std::string_view value) && {
        insert(token, value);
        return std::move(*this);
    }
    HttpResponse &&with_content_type(std::string_view mime) && {
        insert(shared::http::Token::CONTENT_TYPE, mime);
        return std::move(*this);
    }
    HttpResponse &&with_body(std::vector<std::byte> body) && {
        m_body = std::move(body);
        return std::move(*this);
    }

    [[nodiscard]] HttpResponse build() && { return std::move(*this); }

    HttpResponse(const HttpResponse &) = delete;
    HttpResponse &operator=(const HttpResponse &) = delete;
    constexpr HttpResponse(HttpResponse &&) noexcept = default;
    constexpr HttpResponse &operator=(HttpResponse &&) noexcept = default;

    void add_header(std::variant<std::string_view, shared::http::Token> name_variant, std::string_view value) &
        override {
        std::visit([&](const auto &name) { insert(name, value); }, name_variant);
    }

    void remove_header(std::variant<std::string_view, shared::http::Token> name) & override {
        std::visit(
            [&](const auto &n) {
                using T = std::decay_t<decltype(n)>;
                if constexpr (std::is_same_v<T, shared::http::Token>) {
                    m_static_headers[std::to_underlying(n)] = nullptr;
                } else {
                    auto token = shared::http::tokenize(n);
                    if (token != shared::http::Token::CUSTOM) {
                        m_static_headers[std::to_underlying(token)] = nullptr;
                    } else {
                        m_headers.erase(n);
                    }
                }
            },
            name);
    }

    void set_status(interfaces::Status status) & override {
        insert(shared::http::Token::STATUS, std::to_string(interfaces::status_code(status)));
    }

    [[nodiscard]] std::vector<shared::http::HeaderEntry> get_header() const noexcept override {
        std::vector<shared::http::HeaderEntry> result;
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
            m_static_headers | std::views::filter([](const auto &field) noexcept { return field != nullptr; }),
            std::size_t{0}, [](std::size_t acc, const auto &field) noexcept { return acc + field->size(); });

        // TODO: add ranges support to my swiss hashmap
        //  header_block = std::ranges::fold_left(m_headers, header_block, [](std::size_t acc, const auto &entry)
        //  noexcept {
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

    [[nodiscard]] const std::uint32_t &get_stream_id() const { return m_stream_id; }
    [[nodiscard]] std::span<const std::byte> get_body() const noexcept override { return m_body; }

  private:
    void insert(std::string_view name, std::string_view value) {
        if (name.empty()) {
            throw std::invalid_argument("Header name cannot be empty");
        }

        auto token = shared::http::tokenize(name);
        if (token == shared::http::Token::SET_COOKIE) {
            const auto IDX = std::to_underlying(shared::http::Token::SET_COOKIE);
            if (m_static_headers[IDX] == nullptr) {
                m_static_headers[IDX] = std::make_shared<shared::http::HeaderField<true>>(
                    shared::http::Token::SET_COOKIE, std::string(value));
            } else if (!value.empty()) {
                m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() + shared::COOKIE_SEPARATOR +
                                                 std::string(value));
            }
        } else if (token == shared::http::Token::CUSTOM) {
            if (!value.empty()) {
                if (auto existing_opt = m_headers.find(name); existing_opt.has_value()) {
                    const auto &existing = *existing_opt;
                    existing->set_value(existing->get_value() + shared::VALUE_SEPARATOR + std::string(value));
                }
                return;
            }
            m_headers.insert(name, std::make_shared<shared::http::HeaderField<false>>(name, value));
        } else {
            insert(token, value);
        }
    }

    void insert(shared::http::Token token, std::string_view value) {
        if (token == shared::http::Token::NONE) {
            throw std::invalid_argument("shared::http::Token cannot be None");
        }
        if (token == shared::http::Token::CUSTOM) {
            throw std::invalid_argument("shared::http::Token cannot be Custom");
        }

        m_static_headers[std::to_underlying(token)] =
            std::make_shared<shared::http::HeaderField<true>>(token, std::string(value));
    }

    struct FactoryTag {};

    HttpResponse(FactoryTag, std::uint32_t stream_id, interfaces::Status status) : HttpResponse(stream_id) {
        insert(shared::http::Token::STATUS, std::to_string(interfaces::status_code(status)));
    }

    std::shared_ptr<shared::http::HeaderField<true>> get_static(const shared::http::Token &token) {
        return m_static_headers[std::to_underlying(token)];
    }

    std::uint32_t m_stream_id;
    std::array<std::shared_ptr<shared::http::HeaderField<true>>, std::to_underlying(shared::http::Token::CUSTOM) + 1>
        m_static_headers{};
    hashmap::swiss::SwissHashMap<std::string_view, std::shared_ptr<shared::http::HeaderField<false>>> m_headers;
    std::vector<std::byte> m_body;
};

struct WriteHttpResponseAdaptor : std::ranges::range_adaptor_closure<WriteHttpResponseAdaptor> {
    explicit constexpr WriteHttpResponseAdaptor(HttpResponse &res, codec::hpack::HPackTable &table,
                                                std::size_t max_frame_size, std::uint8_t flags = 0)
        : m_res{res}, m_table{table}, m_max_frame_size{max_frame_size}, m_flags{flags} {}

    template <std::ranges::viewable_range R>
    auto operator()(R &&output) const {
        auto stream_id = m_res.get().get_stream_id();
        auto headers = m_res.get().get_header();

        return std::views::concat(
            std::forward<R>(output) |
                codec::hpack::HpackEncodeAdaptor<std::uint32_t>{m_table,
                                                                std::span<shared::http::HeaderEntry>(headers)} |
                WriteFrameClosureAdapter{stream_id, shared_layer::FrameType::HEADERS, m_flags, m_max_frame_size},
            m_res.get().get_body() |
                WriteFrameClosureAdapter{stream_id, shared_layer::FrameType::DATA, m_flags, m_max_frame_size});
    }

    std::reference_wrapper<HttpResponse> m_res;
    std::reference_wrapper<codec::hpack::HPackTable> m_table;
    std::size_t m_max_frame_size;
    std::uint8_t m_flags;
};

} // namespace io::layer::http2
