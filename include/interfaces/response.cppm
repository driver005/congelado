export module interfaces:response;

import std;

export namespace interfaces {

// CRTP base for protocol-agnostic responses (HTTP/1–3, gRPC, WebSocket, …).
// Protocols without header/body support inherit no-op defaults.
// All mutators return Derived& for builder chaining.
template <typename Derived>
class IResponse {
  public:
    virtual ~IResponse() = default;

    Derived &add_header(std::string_view name, std::string_view value) noexcept {
        on_add_header(name, value);
        return static_cast<Derived &>(*this);
    }

    Derived &remove_header(std::string_view name) noexcept {
        on_remove_header(name);
        return static_cast<Derived &>(*this);
    }

    Derived &with_body(std::span<const std::uint8_t> body) noexcept {
        on_set_body(body);
        return static_cast<Derived &>(*this);
    }

    [[nodiscard]] virtual std::span<const std::uint8_t> get_body() const noexcept { return {}; }

  protected:
    virtual void on_add_header(std::string_view /*name*/, std::string_view /*value*/) noexcept {}
    virtual void on_remove_header(std::string_view /*name*/) noexcept {}
    virtual void on_set_body(std::span<const std::uint8_t> /*body*/) noexcept {}
};

template <typename T>
concept Response = std::derived_from<T, IResponse<T>>;

} // namespace interfaces
