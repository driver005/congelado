export module interfaces:request;

import std;
import utils_buffering;

export namespace interfaces {

// CRTP base for protocol-agnostic requests (HTTP/1–3, gRPC, WebSocket, …).
// Protocols without header/body support inherit no-op defaults.
// All mutators return Derived& for builder chaining.
template <typename Protocol>
class IRequest {
    using Header = typename Protocol::Header;
    using Token = typename Protocol::Token;

  public:
    virtual ~IRequest() = default;

    template <typename Self>
    Self &&add_header(std::string_view name, std::string_view value) && noexcept {
        add_header(name, value);
        return std::forward<Self>(*this);
    }

    template <typename Self>
    Self &&remove_header(std::string_view name) && noexcept {
        remove_header(name);
        return std::forward<Self>(*this);
    }

    template <typename Self>
    [[nodiscard]] Self &&build() && {
        return std::forward<Self>(*this);
    }

    virtual void add_header(std::variant<std::string_view, Token> name, std::string_view value) & = 0;
    virtual void remove_header(std::variant<std::string_view, Token> name) & = 0;

    virtual std::string_view get_method() const noexcept = 0;
    virtual std::string_view get_target() const noexcept = 0;
    [[nodiscard]] virtual utils::buffering::BufferView &get_body() noexcept = 0;
    virtual std::vector<Header> get_header() const noexcept = 0;
};

template <typename Derived>
concept Request = std::derived_from<Derived, IRequest<Derived>>;

} // namespace interfaces
