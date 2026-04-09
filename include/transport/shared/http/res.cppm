export module transport_shared:http_res;

import std;
import hashmap;
import :consts;
import :http_header;
import :http_types;

export namespace transport::shared::http {

class HttpResponse {
  public:
    explicit HttpResponse(std::uint32_t stream_id) : m_stream_id{stream_id}, m_headers{}, m_body{} {}

    template <bool IsStatic>
    void insert(std::shared_ptr<HeaderField<IsStatic>> field) {
        if (!field)
            throw std::invalid_argument("Header field cannot be null");

        m_headers.push_back(field);
    }

    template <bool IsStatic>
    void insert(std::string_view name, std::string_view value) {
        if (name.empty() || value.empty())
            throw std::invalid_argument("Header components cannot be empty");

        m_headers.push_back(std::make_shared<HeaderField<IsStatic>>(name, value));
    }

    // void insert(Token token, std::string_view value) {
    //     if (token == Token::None)
    //         throw std::invalid_argument("Token cannot be None");
    //     if (value.empty())
    //         throw std::invalid_argument("Value cannot be empty");
    //
    //     m_static_headers[std::to_underlying(token)] = HeaderField<true>{token, std::string(value)};
    // }

    void append_body(std::span<std::uint8_t> chunk) {
        m_body.insert(m_body.end(), std::make_move_iterator(chunk.begin()), std::make_move_iterator(chunk.end()));
    }

    void set_body(std::vector<std::uint8_t> body) { m_body = std::move(body); }

    std::span<const std::uint8_t> get_body() const { return std::span{m_body}; }
    const std::uint32_t &get_stream_id() const { return m_stream_id; }

    std::span<const HeaderEntry> get_headers() const { return std::span{m_headers}; }

  private:
    std::uint32_t m_stream_id;
    std::vector<HeaderEntry> m_headers;
    std::vector<std::uint8_t> m_body;
};

} // namespace transport::shared::http
