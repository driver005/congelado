export module interfaces:request;

import std;
import utils_buffering;

export namespace interfaces {

// CRTP base for protocol-agnostic requests (HTTP/1–3, gRPC, WebSocket, …).
// Protocols without header/body support inherit no-op defaults.
// All mutators return Derived& for builder chaining.
template <typename Derived, typename Header, typename Token>
class IRequest {
  public:
    virtual ~IRequest() = default;

    Derived &add_header(std::string_view name, std::string_view value) && noexcept {
        add_header(name, value);
        return static_cast<Derived &>(*this);
    }

    Derived &remove_header(std::string_view name) && noexcept {
        remove_header(name);
        return static_cast<Derived &>(*this);
    }

    virtual void add_header(std::variant<std::string_view, Token> name, std::string_view value) & = 0;
    virtual void remove_header(std::variant<std::string_view, Token> name) & = 0;

    [[nodiscard]] virtual utils::buffering::BufferView &get_body() noexcept = 0;
    virtual std::vector<Header> get_header() const noexcept = 0;
};

template <typename Derived, typename Header, typename Token>
concept Request = std::derived_from<Derived, IRequest<Derived, Header, Token>>;

} // namespace interfaces
