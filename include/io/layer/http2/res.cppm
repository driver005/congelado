export module io_layer_http2:response;

import std;
import hashmap;
import shared;
import io_shared;
import interfaces;
import :frame;

export namespace io::layer::http2 {

class HttpResponse : public ::interfaces::IResponse<HttpResponse> {
  public:
    explicit HttpResponse(std::uint32_t stream_id) : m_stream_id{stream_id}, m_static_headers{} {}

    HttpResponse &with_header(std::string_view name, std::string_view value) {
        insert(name, value);
        return *this;
    }
    HttpResponse &with_header(shared::http::Token token, std::string_view value) {
        insert(token, value);
        return *this;
    }
    HttpResponse &with_body(std::vector<std::uint8_t> body) {
        m_body = std::move(body);
        return *this;
    }

    // --- insert ---
    template <bool IsStatic>
    void insert(std::shared_ptr<shared::http::HeaderField<IsStatic>> field) {
        if (field == nullptr) {
            throw std::invalid_argument("Header field cannot be null");
        }
        if constexpr (IsStatic) {
            m_static_headers[std::to_underlying(field->get_name())] = field;
        } else {
            if (auto existing = m_headers.find(field->get_name()); existing.has_value()) {
                (*existing)->set_value((*existing)->get_value() + shared::VALUE_SEPARATOR + field->get_value());
                return;
            }
            m_headers.insert(field->get_name(), field);
        }
    }

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

    // --- getters ---
    template <bool IsStatic>
    std::shared_ptr<shared::http::HeaderField<IsStatic>> get(std::string_view name) {
        auto token = shared::http::tokenize(name);
        if constexpr (IsStatic) {
            if (token == shared::http::Token::NONE || token == shared::http::Token::CUSTOM) {
                return nullptr;
            }
            return m_static_headers[std::to_underlying(token)];
        } else {
            if (token != shared::http::Token::CUSTOM) {
                return nullptr;
            }
            auto result = m_headers.find(name);
            return result.has_value() ? *result : nullptr;
        }
    }

    [[nodiscard]] std::vector<shared::http::HeaderEntry> get_headers() const {
        std::vector<shared::http::HeaderEntry> result;
        for (const auto &field : m_static_headers) {
            if (field != nullptr) {
                result.push_back(field);
            }
        }
        for (const auto &entry : m_headers) {
            result.push_back(entry.value());
        }
        return result;
    }

    [[nodiscard]] const std::uint32_t &get_stream_id() const { return m_stream_id; }
    [[nodiscard]] std::span<const std::uint8_t> get_body() const noexcept override { return m_body; }
    std::vector<std::uint8_t> &get_body_mut() { return m_body; }

  protected:
    void on_add_header(std::string_view name, std::string_view value) noexcept override { insert(name, value); }

    void on_remove_header(std::string_view name) noexcept override {
        auto token = shared::http::tokenize(name);
        if (token != shared::http::Token::CUSTOM) {
            m_static_headers[std::to_underlying(token)] = nullptr;
        } else {
            m_headers.erase(name);
        }
    }

    void on_set_body(std::span<const std::uint8_t> body) noexcept override { m_body.assign(body.begin(), body.end()); }

  private:
    std::uint32_t m_stream_id;
    std::array<std::shared_ptr<shared::http::HeaderField<true>>, std::to_underlying(shared::http::Token::CUSTOM) + 1>
        m_static_headers{};
    hashmap::swiss::SwissHashMap<std::string_view, std::shared_ptr<shared::http::HeaderField<false>>> m_headers;
    std::vector<std::uint8_t> m_body;
};

} // namespace io::layer::http2
