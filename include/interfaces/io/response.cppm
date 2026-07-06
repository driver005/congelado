export module interfaces:io_response;

import std;
import :io_header;
import :io_types;

export namespace interfaces::io {

// CRTP base for protocol-agnostic responses (HTTP/1–3, gRPC, WebSocket, …).
// All mutators return Derived& for builder chaining.
class IResponse {
  public:
    IResponse(std::uint32_t stream_id) : m_stream_id{stream_id} {}
    IResponse() : IResponse{0} {};

    virtual ~IResponse() = default;

    [[nodiscard]] static std::unique_ptr<IResponse> ok(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::OK);
        return res;
    }

    [[nodiscard]] static std::unique_ptr<IResponse> created(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::CREATED);
        return res;
    }

    [[nodiscard]] static std::unique_ptr<IResponse> no_content(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::NO_CONTENT);
        return res;
    }

    [[nodiscard]] static std::unique_ptr<IResponse> bad_request(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::BAD_REQUEST);
        return res;
    }

    [[nodiscard]] static std::unique_ptr<IResponse> not_found(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::NOT_FOUND);
        return res;
    }

    [[nodiscard]] static std::unique_ptr<IResponse> internal_error(std::uint32_t stream_id) {
        auto res = std::make_unique<IResponse>(stream_id);
        std::move(*res).with_status(types::Status::INTERNAL_SERVER_ERROR);
        return res;
    }

    IResponse &&with_header(std::string_view name, std::string_view value) && noexcept {
        set_header(name, value);
        return std::move(*this);
    }

    IResponse &&with_header(types::Token token, std::string_view value) && noexcept {
        set_header(token, value);
        return std::move(*this);
    }

    IResponse &&without_header(std::string_view name) && noexcept {
        remove_header(name);
        return std::move(*this);
    }

    IResponse &&with_status(types::Status status) && noexcept {
        set_status(status);
        return std::move(*this);
    }

    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    IResponse &&with_body(R &&body_range) && noexcept {
        set_body(std::forward<R>(body_range));
        return std::move(*this);
    }

    IResponse &&with_stream_id(std::uint32_t id) && noexcept {
        set_stream_id(id);
        return std::move(*this);
    }

    IResponse &&with_method(types::Method method) && noexcept {
        set_header(types::Token::METHOD, method_str(method));
        return std::move(*this);
    }

    IResponse &&with_content_type(std::string_view content_type) && noexcept {
        set_header(types::Token::CONTENT_TYPE, content_type);
        return std::move(*this);
    }

    IResponse &&with_content_length(std::size_t length) && noexcept {
        set_header(types::Token::CONTENT_LENGTH, std::to_string(length));
        return std::move(*this);
    }

    IResponse &&with_location(std::string_view location) && noexcept {
        set_header(types::Token::LOCATION, location);
        return std::move(*this);
    }

    IResponse &&with_e_tag(std::string_view etag) && noexcept {
        set_header(types::Token::E_TAG, etag);
        return std::move(*this);
    }

    IResponse &&with_date(std::string_view date) && noexcept {
        set_header(types::Token::DATE, date);
        return std::move(*this);
    }

    IResponse &&with_server(std::string_view server) && noexcept {
        set_header(types::Token::SERVER, server);
        return std::move(*this);
    }

    IResponse &&with_cache_control(std::string_view cache_control) && noexcept {
        set_header(types::Token::CACHE_CONTROL, cache_control);
        return std::move(*this);
    }

    IResponse &&with_last_modified(std::string_view last_modified) && noexcept {
        set_header(types::Token::LAST_MODIFIED, last_modified);
        return std::move(*this);
    }

    IResponse &&with_set_cookie(std::string_view cookie) && noexcept {
        set_header(types::Token::SET_COOKIE, cookie);
        return std::move(*this);
    }

    IResponse &&with_keep_alive(bool keep_alive) && noexcept {
        set_keep_alive(keep_alive);
        return std::move(*this);
    }

    [[nodiscard]] IResponse &&build() && noexcept { return std::move(*this); }

    void add_method(types::Method method) & noexcept {
        set_header(types::Token::METHOD, method_str(method));
    }

    void add_content_type(std::string_view type) & noexcept {
        set_header(types::Token::CONTENT_TYPE, type);
    }

    void add_content_length(std::size_t length) & noexcept {
        set_header(types::Token::CONTENT_LENGTH, std::to_string(length));
    }

    void add_location(std::string_view location) & noexcept {
        set_header(types::Token::LOCATION, location);
    }

    void add_e_tag(std::string_view etag) & noexcept { set_header(types::Token::E_TAG, etag); }

    void add_date(std::string_view date) & noexcept { set_header(types::Token::DATE, date); }

    void add_server(std::string_view server) & noexcept {
        set_header(types::Token::SERVER, server);
    }

    void add_cache_control(std::string_view cache_control) & noexcept {
        set_header(types::Token::CACHE_CONTROL, cache_control);
    }

    void add_last_modified(std::string_view last_modified) & noexcept {
        set_header(types::Token::LAST_MODIFIED, last_modified);
    }

    void add_set_cookie(std::string_view cookie) & noexcept {
        add_header(types::Token::SET_COOKIE, cookie);
    }

    void add_status(types::Status status) & noexcept { set_status(status); }

    void add_keep_alive(bool keep_alive) & noexcept { set_keep_alive(keep_alive); }

    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte>
    void add_body(R &&body_range) & noexcept {
        set_body(std::forward<R>(body_range));
    }

    void add_stream_id(std::uint32_t stream_id) & noexcept { set_stream_id(stream_id); }

    void add_header(std::variant<std::string_view, types::Token> name_or_token,
                    std::string_view value) & noexcept {
        set_header(name_or_token, value);
    }

    void pop_header(std::variant<std::string_view, types::Token> name) & noexcept {
        remove_header(name);
    }

    void replace_header(std::variant<std::string_view, types::Token> name,
                        std::string_view value) & noexcept {
        remove_header(name);
        set_header(name, value);
    }


    void set_stream_id(std::uint32_t stream_id) & noexcept { m_stream_id = stream_id; }
    [[nodiscard]] std::uint32_t get_stream_id() const noexcept { return m_stream_id; }

    virtual void set_body(std::vector<std::byte> body) & { std::abort(); }

    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, std::byte> &&
                 (!std::same_as<std::remove_cvref_t<R>,
                                std::vector<std::byte>>) // Prevents ambiguous overload resolution
    void set_body(R &&body_range) & {
        set_body(std::ranges::to<std::vector<std::byte>>(std::forward<R>(body_range)));
    }

    virtual void set_status(types::Status status) & { std::abort(); }
    virtual void set_keep_alive(bool keep_alive) & { std::abort(); }

    virtual void set_header(std::variant<std::string_view, types::Token> name_or_token,
                            std::string_view value) & {
        std::abort();
    }

    virtual void remove_header(std::variant<std::string_view, types::Token> name) & {
        std::abort();
    }

    [[nodiscard]] virtual std::string_view
    find_header(std::variant<std::string_view, types::Token> name_or_token) const noexcept {
        std::abort();
    }

    [[nodiscard]] virtual types::Status get_status() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::span<const std::byte> get_body() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::vector<HeaderEntry> get_headers() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::vector<HeaderEntry> get_set_cookies() const noexcept {
        std::abort();
    }
    [[nodiscard]] virtual std::string_view get_status_text() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_content_type() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::size_t get_content_length() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_location() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_etag() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_date() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_server() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_cache_control() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_last_modified() const noexcept { std::abort(); }
    [[nodiscard]] virtual bool is_keep_alive() const noexcept { std::abort(); }
    [[nodiscard]] virtual bool is_informational() const noexcept { std::abort(); }
    [[nodiscard]] virtual bool is_success() const noexcept { std::abort(); }
    [[nodiscard]] virtual bool is_redirection() const noexcept { std::abort(); }
    [[nodiscard]] virtual bool is_client_error() const noexcept { std::abort(); }
    [[nodiscard]] virtual bool is_server_error() const noexcept { std::abort(); }

  protected:
    std::uint32_t m_stream_id;
};

} // namespace interfaces::io
