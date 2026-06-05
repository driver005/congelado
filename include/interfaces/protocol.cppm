export module interfaces:protocol;

import std;
import shared;
import io_shared;
import :request;
import :response;

export namespace interfaces {

// Dispatch function: called by the protocol layer for each fully-received request.
// Protocol implementations call this once per request/response pair.
using DispatchFn = std::function<void(IRequest<io::shared::http::Protocol> &,
                                      IResponse<io::shared::http::Protocol> &)>;

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

    // Called after all plugins load so the protocol can build its router into a server.
    // router_ctx is a core::server::RouterContext<Protocol>*. Default no-op.
    virtual void build(void * /*router_ctx*/) {}

    // Default no-op for protocols that build internally via build().
    virtual void set_dispatch(DispatchFn) {}
};


template <typename Derived>
using HandlerFn = std::function<void(IRequest<Derived> &, IResponse<Derived> &)>;

template <typename Derived>
using NextFn = std::move_only_function<void(IRequest<Derived> &, IResponse<Derived> &) noexcept>;

template <typename Derived>
using MiddlewareFn = void (*)(IRequest<Derived> &, IResponse<Derived> &, NextFn<Derived> &&);

} // namespace interfaces
