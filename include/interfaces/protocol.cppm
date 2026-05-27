export module interfaces:protocol;

import std;
import shared;
import :request;
import :response;

export namespace interfaces {

// Interface for io-layer protocol plugins.
// Each protocol implementation controls transport binding
// (host, port, TLS, threads) and per-connection data handling.
class IProtocol {
  public:
    virtual ~IProtocol() = default;

    [[nodiscard]] virtual std::string_view get_protocol_name() const noexcept = 0;

    [[nodiscard]] virtual std::string_view get_bind_host() const noexcept = 0;
    [[nodiscard]] virtual std::uint16_t get_bind_port() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t get_bind_threads() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_tls_cert() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_tls_key() const noexcept = 0;

    [[nodiscard]] virtual shared::ReadCallback on_connect(shared::SendCallback send, shared::CloseCallback close) = 0;
};


template <typename Derived>
using HandlerFn = void (*)(IRequest<Derived> &, IResponse<Derived> &) noexcept;


template <typename Derived>
using NextFn = std::move_only_function<void(IRequest<Derived> &, IResponse<Derived> &) noexcept>;


template <typename Derived>
using MiddlewareFn = void (*)(IRequest<Derived> &, IResponse<Derived> &, NextFn<Derived> &&);

} // namespace interfaces
