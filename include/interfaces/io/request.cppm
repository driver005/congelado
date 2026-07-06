export module interfaces:io_request;

import std;
import utils_buffering;
import utils_encode;
import :io_types;
import :io_header;

export namespace interfaces::io {

// CRTP base for protocol-agnostic requests (HTTP/1–3, gRPC, WebSocket, …).
// Protocols without header/body support inherit no-op defaults.
// All mutators return Derived& for builder chaining.
class IRequest {
  public:
    IRequest(std::uint32_t stream_id)
        : m_stream_id{stream_id}, m_timeout{std::chrono::milliseconds::zero()} {}
    IRequest() : IRequest{0} {};

    virtual ~IRequest() = default;

    [[nodiscard]] std::unique_ptr<IRequest> get(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::GET).with_path(path);
        return req;
    }
    [[nodiscard]] std::unique_ptr<IRequest> head(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::HEAD).with_path(path);
        return req;
    }
    [[nodiscard]] std::unique_ptr<IRequest> post(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::POST).with_path(path);
        return req;
    }
    [[nodiscard]] std::unique_ptr<IRequest> put(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::PUT).with_path(path);
        return req;
    }
    [[nodiscard]] std::unique_ptr<IRequest> del(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::DELETE).with_path(path);
        return req;
    }
    [[nodiscard]] std::unique_ptr<IRequest> patch(std::uint32_t stream_id, std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::PATCH).with_path(path);
        return req;
    }
    [[nodiscard]] std::unique_ptr<IRequest> options(std::uint32_t stream_id,
                                                    std::string_view path) {
        auto req = std::make_unique<IRequest>(stream_id);
        std::move(*req).with_method(types::Method::OPTIONS).with_path(path);
        return req;
    }

    IRequest &&with_method(types::Method method) && {
        set_header(types::Token::METHOD, method_str(method));
        return std::move(*this);
    }

    IRequest &&with_method(std::string_view method) && {
        set_header(types::Token::METHOD, method);
        return std::move(*this);
    }

    IRequest &&with_path(std::string_view path) && {
        set_header(types::Token::PATH, path);
        return std::move(*this);
    }

    IRequest &&with_scheme(std::string_view schema) && {
        set_header(types::Token::SCHEME, schema);
        return std::move(*this);
    }

    IRequest &&with_authority(std::string_view authority) && {
        set_header(types::Token::AUTHORITY, authority);
        return std::move(*this);
    }

    IRequest &&with_header(std::string_view name, std::string_view value) && {
        set_header(name, value);
        return std::move(*this);
    }

    IRequest &&with_header(types::Token token, std::string_view value) && {
        set_header(token, value);
        return std::move(*this);
    }

    IRequest &&without_header(std::string_view name) && noexcept {
        remove_header(name);
        return std::move(*this);
    }

    IRequest &&replace_header(std::string_view name, std::string_view value) && noexcept {
        remove_header(name);
        set_header(name, value);
        return std::move(*this);
    }

    IRequest &&replace_header(types::Token token, std::string_view value) && noexcept {
        remove_header(token);
        set_header(token, value);
        return std::move(*this);
    }

    IRequest &&clear_headers() && noexcept {
        clear_headers();
        return std::move(*this);
    }

    IRequest &&with_query(std::string_view key, std::string_view value) && {
        add_query(key, value);
        return std::move(*this);
    }

    IRequest &&with_bearer_auth(std::string_view token) && {
        set_header(types::Token::AUTHORIZATION, "Bearer " + std::string(token));
        return std::move(*this);
    }

    IRequest &&with_basic_auth(std::string_view user, std::string_view password) && {
        set_header(types::Token::AUTHORIZATION,
                   "Basic " + utils::encode::base64_encode(std::string(user) + ":" +
                                                           std::string(password)));
        return std::move(*this);
    }

    IRequest &&with_content_type(std::string_view mime) && {
        set_header(types::Token::CONTENT_TYPE, mime);
        return std::move(*this);
    }

    IRequest &&with_accept(std::string_view mime) && {
        set_header(types::Token::ACCEPT, mime);
        return std::move(*this);
    }

    IRequest &&with_user_agent(std::string_view user) && {
        set_header(types::Token::USER_AGENT, user);
        return std::move(*this);
    }

    IRequest &&with_timeout(std::chrono::milliseconds ms) && noexcept {
        set_timeout(ms);
        return std::move(*this);
    }

    IRequest &&with_no_decompress(bool disable = true) && noexcept {
        set_no_decompress(disable);
        return std::move(*this);
    }

    IRequest &&with_addr(std::string_view addr) && noexcept {
        set_addr(addr);
        return std::move(*this);
    }

    IRequest &&with_stream_id(std::uint32_t stream_id) && noexcept {
        set_stream_id(stream_id);
        return std::move(*this);
    }

    [[nodiscard]] IRequest &&build() && { return std::move(*this); }

    void add_method(types::Method method) & noexcept {
        set_header(types::Token::METHOD, method_str(method));
    }

    void add_method(std::string_view method) & noexcept {
        set_header(types::Token::METHOD, method);
    }

    void add_path(std::string_view path) & noexcept { set_header(types::Token::PATH, path); }

    void add_scheme(std::string_view schema) & noexcept {
        set_header(types::Token::SCHEME, schema);
    }

    void add_authority(std::string_view authority) & noexcept {
        set_header(types::Token::AUTHORITY, authority);
    }

    void add_query(std::string_view key, std::string_view value) & noexcept {
        auto path_field = get_path();
        std::string new_path;

        if (path_field.empty()) {
            new_path = "/";
        } else {
            new_path = std::string(path_field);
        }

        new_path += new_path.contains('?') ? '&' : '?';
        new_path += utils::encode::url_encode(key);
        new_path += '=';
        new_path += utils::encode::url_encode(value);

        set_header(types::Token::PATH, new_path);
    }

    void add_bearer_auth(std::string_view token) & noexcept {
        set_header(types::Token::AUTHORIZATION, "Bearer " + std::string(token));
    }

    void add_basic_auth(std::string_view user, std::string_view password) & noexcept {
        set_header(types::Token::AUTHORIZATION,
                   "Basic " + utils::encode::base64_encode(std::string(user) + ":" +
                                                           std::string(password)));
    }

    void add_content_type(std::string_view mime) & noexcept {
        set_header(types::Token::CONTENT_TYPE, mime);
    }

    void add_accept(std::string_view mime) & noexcept { set_header(types::Token::ACCEPT, mime); }

    void add_user_agent(std::string_view user) & noexcept {
        set_header(types::Token::USER_AGENT, user);
    }

    void add_timeout(std::chrono::milliseconds ms) & noexcept { set_timeout(ms); }

    void add_no_decompress(bool disable = true) & noexcept { set_no_decompress(disable); }

    void add_addr(std::string_view addr) & noexcept {
        set_header(types::Token::AUTHORIZATION, addr);
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
    void set_timeout(std::chrono::milliseconds ms) & noexcept { m_timeout = ms; }

    [[nodiscard]] std::uint32_t get_stream_id() const noexcept { return m_stream_id; }
    [[nodiscard]] const std::chrono::milliseconds &get_timeout() const noexcept {
        return m_timeout;
    }

    // --- Virtual Interface ---

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

    virtual void clear_headers() & noexcept { std::abort(); }


    virtual void set_no_decompress(bool disable) & noexcept { std::abort(); }
    virtual void set_addr(std::string_view addr) & noexcept { std::abort(); }

    [[nodiscard]] virtual std::string_view get_method() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_path() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_host() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_scheme() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_authority() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_content_type() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_accept() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_user_agent() const noexcept { std::abort(); }
    [[nodiscard]] virtual std::string_view get_authorization() const noexcept { std::abort(); }
    [[nodiscard]] virtual utils::buffering::BufferView &get_body() noexcept { std::abort(); }
    [[nodiscard]] virtual const utils::buffering::BufferView &get_body() const noexcept {
        std::abort();
    }
    [[nodiscard]] virtual std::vector<HeaderEntry> get_headers() const noexcept { std::abort(); }

  protected:
    std::uint32_t m_stream_id;
    std::chrono::milliseconds m_timeout;
}; // namespace interfaces::io

} // namespace interfaces::io
