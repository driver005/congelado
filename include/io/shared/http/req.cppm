module;
#include <cassert>
export module io_shared:http_req;

import std;
import hashmap;
import shared;
import io_base_buffering;
import :consts;
import :http_header;
import :http_types;

export namespace io::shared::http {

class HttpRequest : public ::shared::Request {
  public:
    explicit HttpRequest(std::uint32_t stream_id) : m_stream_id{stream_id}, m_static_headers{}, m_headers{} {}

    template <bool IsStatic>
    void insert(std::shared_ptr<HeaderField<IsStatic>> field) {
        if (field == nullptr) {
            throw std::invalid_argument("Header field cannot be null");
        }
        if constexpr (IsStatic) {
            m_static_headers[std::to_underlying(field->get_name())] = field;
        } else {
            if (auto existing = m_headers.find(field->get_name()); existing.has_value()) {
                (*existing)->set_value((*existing)->get_value() + VALUE_SEPARATOR + field->get_value());
                return;
            }
            m_headers.insert(field->get_name(), field);
        }
    }

    void insert(std::string_view name, std::string_view value) {
        if (name.empty()) {
            throw std::invalid_argument("Header name cannot be empty");
        }

        auto token = tokenize(name);
        if (token == Token::Cookie) {
            const auto IDX = std::to_underlying(Token::Cookie);
            if (m_static_headers[IDX] == nullptr) {
                m_static_headers[IDX] = std::make_shared<HeaderField<true>>(Token::Cookie, std::string(value));
            } else if (!value.empty()) {
                m_static_headers[IDX]->set_value(m_static_headers[IDX]->get_value() + COOKIE_SEPARATOR +
                                                 std::string(value));
            }
        } else if (token == Token::Custom) {
            if (!value.empty()) {
                if (auto existing_opt = m_headers.find(name); existing_opt.has_value()) {
                    const auto &existing = *existing_opt;
                    existing->set_value(existing->get_value() + VALUE_SEPARATOR + std::string(value));
                }
                return;
            }
            m_headers.insert(name, std::make_shared<HeaderField<false>>(name, value));
        } else {
            insert(token, value);
        }
    }

    void insert(Token token, std::string_view value) {
        if (token == Token::None) {
            throw std::invalid_argument("Token cannot be None");
        }
        if (token == Token::Custom) {
            throw std::invalid_argument("Token cannot be Custom");
        }

        m_static_headers[std::to_underlying(token)] = std::make_shared<HeaderField<true>>(token, std::string(value));
    }

    template <bool IsStatic>
    std::shared_ptr<HeaderField<IsStatic>> get(std::string_view name) {
        auto token = tokenize(name);
        if constexpr (IsStatic) {
            if (token == Token::None || token == Token::Custom) {
                return nullptr;
            }
            return m_static_headers[std::to_underlying(token)];
        } else {
            if (token != Token::Custom) {
                return nullptr;
            }
            auto result = m_headers.find(name);
            return result.has_value() ? *result : nullptr;
        }
    }

    // void append_body(std::span<std::uint8_t> chunk) {
    //     m_body.insert(m_body.end(), std::make_move_iterator(chunk.begin()), std::make_move_iterator(chunk.end()));
    // }
    //
    // void set_body(std::vector<std::uint8_t> body) { m_body = std::move(body); }

    [[nodiscard]] const std::uint32_t &get_stream_id() const { return m_stream_id; }
    base::buffering::BufferView &get_body() { return m_body; }

  private:
    std::shared_ptr<HeaderField<true>> get_static(const Token &token) {
        return m_static_headers[std::to_underlying(token)];
    }

    std::uint32_t m_stream_id;
    std::array<std::shared_ptr<HeaderField<true>>, std::to_underlying(Token::Custom) + 1> m_static_headers{};
    hashmap::swiss::SwissHashMap<std::string_view, std::shared_ptr<HeaderField<false>>> m_headers;
    base::buffering::BufferView m_body;
};

} // namespace io::shared::http
