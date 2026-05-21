export module interfaces:response;

import std;
import :status;

export namespace interfaces {

// CRTP base for protocol-agnostic responses (HTTP/1–3, gRPC, WebSocket, …).
// All mutators return Derived& for builder chaining.
template <typename Derived, typename Header, typename Token>
class IResponse {
  public:
    virtual ~IResponse() = default;

    Derived &add_header(std::string_view name, std::string_view value) && noexcept {
        add_header(name, value);
        return static_cast<Derived &>(*this);
    }

    Derived &remove_header(std::string_view name) && noexcept {
        remove_header(name);
        return static_cast<Derived &>(*this);
    }

    Derived &with_status(Status status) noexcept {
        set_status(status);
        return static_cast<Derived &>(*this);
    }

    [[nodiscard]] Derived build() && { return std::move(static_cast<Derived &>(*this)); }

    virtual void add_header(std::variant<std::string_view, Token> name, std::string_view value) & = 0;
    virtual void remove_header(std::variant<std::string_view, Token> name) & = 0;
    virtual void set_status(Status status) & = 0;

    virtual std::vector<Header> get_header() const noexcept = 0;
    [[nodiscard]] virtual std::span<const std::byte> get_body() const noexcept = 0;
};

template <typename Derived, typename Header, typename Token>
concept Response = std::derived_from<Derived, IResponse<Derived, Header, Token>>;

} // namespace interfaces
