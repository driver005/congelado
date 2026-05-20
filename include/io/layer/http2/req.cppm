module;
#include <cassert>
#include <ranges>
export module io_layer_http2:request;

import std;
import hashmap;
import shared;
import io_shared;
import io_layer_shared;
import io_codec_hpack;
import utils_buffering;
import utils_encode;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpRequest : public ::interfaces::IRequest<HttpRequest, shared::http::HeaderEntry, shared::http::Token> {
  public:
    explicit HttpRequest(std::uint32_t stream_id) : m_stream_id{stream_id}, m_static_headers{} {}

    static HttpRequest get(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::GET, path);
    }
    static HttpRequest head(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::HEAD, path);
    }
    static HttpRequest post(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::POST, path);
    }
    static HttpRequest put(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::PUT, path);
    }
    static HttpRequest del(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::DELETE, path);
    }
    static HttpRequest patch(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::PATCH, path);
    }
    static HttpRequest options(std::uint32_t stream_id, std::string_view path) {
        return HttpRequest(FactoryTag{}, stream_id, shared::http::HttpMethod::OPTIONS, path);
    }

    // --- Builder methods ---
    HttpRequest &with_method(shared::http::HttpMethod method) {
        insert(shared::http::Token::METHOD, method_str(method));
        return *this;
    }
    HttpRequest &with_method(std::string_view method) {
        insert(shared::http::Token::METHOD, method);
        return *this;
    }
    HttpRequest &with_path(std::string_view path) {
        insert(shared::http::Token::PATH, path);
        return *this;
    }
    HttpRequest &with_scheme(std::string_view schema) {
        insert(shared::http::Token::SCHEME, schema);
        return *this;
    }
    HttpRequest &with_authority(std::string_view authority) {
        insert(shared::http::Token::AUTHORITY, authority);
        return *this;
    }
    HttpRequest &with_header(std::string_view name, std::string_view value) {
        insert(name, value);
        return *this;
    }
    HttpRequest &with_header(shared::http::Token token, std::string_view value) {
        insert(token, value);
        return *this;
    }

    HttpRequest &with_query(std::string_view key, std::string_view value) {
        auto &path_field = m_static_headers[std::to_underlying(shared::http::Token::PATH)];
        std::string new_path;
        if (path_field) {
            new_path = path_field->get_value();
        }
        new_path += new_path.contains('?') ? '&' : '?';
        new_path += utils::encode::url_encode(key);
        new_path += '=';
        new_path += utils::encode::url_encode(value);
        insert(shared::http::Token::PATH, new_path);
        return *this;
    }

    HttpRequest &with_bearer_auth(std::string_view token) {
        insert(shared::http::Token::AUTHORIZATION, "Bearer " + std::string(token));
        return *this;
    }

    HttpRequest &with_basic_auth(std::string_view user, std::string_view password) {
        insert(shared::http::Token::AUTHORIZATION,
               "Basic " + utils::encode::base64_encode(std::string(user) + ":" + std::string(password)));
        return *this;
    }

    HttpRequest &with_content_type(std::string_view mime) {
        insert(shared::http::Token::CONTENT_TYPE, mime);
        return *this;
    }

    HttpRequest &with_accept(std::string_view mime) {
        insert(shared::http::Token::ACCEPT, mime);
        return *this;
    }

    HttpRequest &with_user_agent(std::string_view user) {
        insert(shared::http::Token::USER_AGENT, user);
        return *this;
    }

    HttpRequest(const HttpRequest &) = delete;
    HttpRequest &operator=(const HttpRequest &) = delete;
    constexpr HttpRequest(HttpRequest &&) noexcept = delete;
    constexpr HttpRequest &operator=(HttpRequest &&) noexcept = delete;

    void add_header(std::variant<std::string_view, shared::http::Token> name_variant, std::string_view value) &
        override {
        std::visit([&](const auto &name) { insert(name, value); }, name_variant);
    }

    void remove_header(std::variant<std::string_view, shared::http::Token> name) & override {
        std::visit(
            [&](const auto &name) {
                using T = std::decay_t<decltype(name)>;
                if constexpr (std::is_same_v<T, shared::http::Token>) {
                    m_static_headers[std::to_underlying(name)] = nullptr;
                } else {
                    auto token = shared::http::tokenize(name);
                    if (token != shared::http::Token::CUSTOM) {
                        m_static_headers[std::to_underlying(token)] = nullptr;
                    } else {
                        m_headers.erase(name);
                    }
                }
            },
            name);
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

    [[nodiscard]] const std::uint32_t &get_stream_id() const { return m_stream_id; }
    utils::buffering::BufferView &get_body() noexcept override { return m_body; }

    // --- Insert ---
    // template <bool IsStatic>
    // void insert(std::shared_ptr<shared::http::HeaderField<IsStatic>> field) {
    //     if (field == nullptr) {
    //         throw std::invalid_argument("Header field cannot be null");
    //     }
    //     if constexpr (IsStatic) {
    //         m_static_headers[std::to_underlying(field->get_name())] = field;
    //     } else {
    //         if (auto existing = m_headers.find(field->get_name()); existing.has_value()) {
    //             (*existing)->set_value((*existing)->get_value() + shared::VALUE_SEPARATOR + field->get_value());
    //             return;
    //         }
    //         m_headers.insert(field->get_name(), field);
    //     }
    // }

    // template <bool IsStatic>
    // std::shared_ptr<shared::http::HeaderField<IsStatic>> get(std::string_view name) {
    //     auto token = shared::http::tokenize(name);
    //     if constexpr (IsStatic) {
    //         if (token == shared::http::Token::NONE || token == shared::http::Token::CUSTOM) {
    //             return nullptr;
    //         }
    //         return m_static_headers[std::to_underlying(token)];
    //     } else {
    //         if (token != shared::http::Token::CUSTOM) {
    //             return nullptr;
    //         }
    //         auto result = m_headers.find(name);
    //         return result.has_value() ? *result : nullptr;
    //     }
    // }

  private:
    void insert(std::string_view name, std::string_view value) {
        if (name.empty()) {
            throw std::invalid_argument("Header name cannot be empty");
        }

        auto token = shared::http::tokenize(name);
        if (token == shared::http::Token::COOKIE) {
            const auto IDX = std::to_underlying(shared::http::Token::COOKIE);
            if (m_static_headers[IDX] == nullptr) {
                m_static_headers[IDX] =
                    std::make_shared<shared::http::HeaderField<true>>(shared::http::Token::COOKIE, std::string(value));
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

    HttpRequest(FactoryTag, std::uint32_t stream_id, shared::http::HttpMethod method, std::string_view path)
        : HttpRequest(stream_id) {
        insert(shared::http::Token::METHOD, method_str(method));
        insert(shared::http::Token::PATH, path);
    }

    std::shared_ptr<shared::http::HeaderField<true>> get_static(const shared::http::Token &token) {
        return m_static_headers[std::to_underlying(token)];
    }

    std::uint32_t m_stream_id;
    std::array<std::shared_ptr<shared::http::HeaderField<true>>, std::to_underlying(shared::http::Token::CUSTOM) + 1>
        m_static_headers{};
    hashmap::swiss::SwissHashMap<std::string_view, std::shared_ptr<shared::http::HeaderField<false>>> m_headers;
    utils::buffering::BufferView m_body;
};

struct WriteHttpRequestAdaptor : std::ranges::range_adaptor_closure<WriteHttpRequestAdaptor> {
    explicit constexpr WriteHttpRequestAdaptor(HttpRequest &req, codec::hpack::HPackTable &table,
                                               std::size_t max_frame_size, std::uint8_t flags = 0)
        : m_req{req}, m_table{table}, m_max_frame_size{max_frame_size}, m_flags{flags} {}


    template <std::ranges::viewable_range R>
    auto operator()(R &&output) const {
        auto stream_id = m_req.get().get_stream_id();
        return std::views::concat(
            std::forward<R>(output),
            m_req.get().get_header() | codec::hpack::HpackEncodeAdaptor<std::uint32_t, R>{m_table, output} |
                WriteFrameClosureAdapter{stream_id, shared_layer::FrameType::HEADERS, m_max_frame_size},
            m_req.get().get_body() |
                WriteFrameClosureAdapter{stream_id, shared_layer::FrameType::DATA, m_max_frame_size});
    }

    auto operator()() const { return (*this)(std::views::empty<std::byte>); }

    std::reference_wrapper<HttpRequest> m_req;
    std::reference_wrapper<codec::hpack::HPackTable> m_table;
    std::size_t m_max_frame_size;
    std::uint8_t m_flags;
};

} // namespace io::layer::http2
