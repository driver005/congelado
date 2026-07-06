module;
#include <stdexcept>
export module interfaces:protocol;

import std;
import shared;
import io_shared;
import :client;
import :io;

export namespace interfaces {

// Dispatch function: called by the protocol layer for each fully-received request.
// Protocol implementations call this once per request/response pair.

template <typename T>
concept ServerConcept = requires(T server, shared::SendCallback &&send,
                                 shared::CloseCallback &&close, void *router_ctx) {
    { server.on_connect(std::move(send), std::move(close)) } -> std::same_as<shared::ReadCallback>;
    { server.build(router_ctx) } -> std::same_as<void>;
};

template <typename T>
concept ClientConcept = std::constructible_from<T, io::ReceiveDispatchFn &&> &&
                        requires(T client, shared::SendCallback &&send,
                                 shared::CloseCallback &&close, void *router_ctx) {
                            {
                                client.on_connect(std::move(send), std::move(close))
                            } -> std::same_as<shared::ReadCallback>;
                            { client.on_send() } -> std::same_as<io::SendDispatchFn>;
                        };

// Interface for io-layer protocol plugins.
// Each protocol implementation controls transport binding
// (host, port, TLS, threads) and per-connection data handling.
template <ServerConcept Server>
class IProtocol {
  public:
    virtual ~IProtocol() = default;

    [[nodiscard]] virtual std::string_view get_protocol_name() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_bind_host() const noexcept = 0;
    [[nodiscard]] virtual std::uint16_t get_bind_port() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t get_bind_threads() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_tls_cert() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_tls_key() const noexcept = 0;

    [[nodiscard]] virtual std::unique_ptr<Server> get_server() {
        throw std::runtime_error("IServer not implemented for this protocol");
    };
    [[nodiscard]] virtual std::unique_ptr<IClient> get_client(io::ReceiveDispatchFn &&) {
        throw std::runtime_error("IClient not implemented for this protocol");
    };

    // Default no-op for protocols that build internally via build().
    virtual void set_dispatch(io::ReceiveDispatchFn &&) {}
};


using HandlerFn = std::function<void(io::IRequest &, io::IResponse &)>;

using NextFn = std::move_only_function<void(io::IRequest &, io::IResponse &) noexcept>;

using MiddlewareFn = void (*)(io::IRequest &, io::IResponse &, NextFn &&);

} // namespace interfaces
