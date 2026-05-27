export module interfaces:response;

import std;
import :status;

export namespace interfaces {

// CRTP base for protocol-agnostic responses (HTTP/1–3, gRPC, WebSocket, …).
// All mutators return Derived& for builder chaining.
template <typename Protocol>
class IResponse {
    using Header = typename Protocol::Header;
    using Token = typename Protocol::Token;

  public:
    virtual ~IResponse() = default;

    template <typename Self>
    Self &add_header(std::string_view name, std::string_view value) && noexcept {
        add_header(name, value);
        return std::forward<Self>(*this);
    }

    template <typename Self>
    Self &remove_header(std::string_view name) && noexcept {
        remove_header(name);
        return std::forward<Self>(*this);
    }

    template <typename Self>
    Self &with_status(Status status) noexcept {
        set_status(status);
        return std::forward<Self>(*this);
    }

    template <typename Self>
    [[nodiscard]] Self &&build() && {
        return std::forward<Self>(*this);
    }

    virtual void add_header(std::variant<std::string_view, Token> name, std::string_view value) & = 0;
    virtual void remove_header(std::variant<std::string_view, Token> name) & = 0;
    virtual void set_status(Status status) & = 0;
    virtual void set_body(std::vector<std::byte> body) & = 0;

    virtual std::vector<Header> get_header() const noexcept = 0;
    [[nodiscard]] virtual std::span<const std::byte> get_body() const noexcept = 0;
};

template <typename Derived>
concept Response = std::derived_from<Derived, IResponse<Derived>>;

} // namespace interfaces
