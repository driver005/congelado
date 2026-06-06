# Client Abstraction Design

**Date:** 2026-06-06
**Status:** Approved

## Goal

Add a platform-agnostic HTTP client abstraction to `core/` that mirrors the router abstraction (`RouteHandler<Derived>` in `core/server/`). The abstraction provides typed verb methods and a CRTP connector pattern so concrete transports (HTTP/2, future gRPC, etc.) plug in without changing consumers.

## Context

`EngineClient` in `include/worker/engine_client.cppm` is a concrete HTTP/2 persistent client used by `WorkerContext` via a raw `call(method, path, body, callback)` method. There is no abstraction layer — `WorkerContext` depends directly on the concrete type.

The router already establishes the pattern: `RouteHandler<Derived>` in `core/server/router.cppm` is protocol-agnostic at class level, dispatches through `IRequest<Protocol>` / `IResponse<Protocol>`, and is CRTP-based (no vtable). The client abstraction follows the same model.

## Design

### New file: `include/core/client/client.cppm`

Module: `core_client`

```cpp
export module core_client;
import std;
import interfaces;

export namespace core::client {

template <typename Derived>
class ClientHandler {
  public:
    using ResponseFn = std::function<void(int status, std::string body)>;

    // Protocol moves to call site — class has no Protocol template parameter
    template <typename Protocol>
    void send(interfaces::IRequest<Protocol>& req, ResponseFn cb) {
        static_cast<Derived&>(*this).do_send(req, std::move(cb));
    }

    // Generic factory — used by call_engine() where method is a runtime string
    template <typename Protocol>
    auto request(std::string_view method, std::string_view path) {
        return static_cast<Derived&>(*this).template make_request<Protocol>(method, path);
    }

    template <typename Protocol>
    auto get(std::string_view path)   { return request<Protocol>("GET",    path); }
    template <typename Protocol>
    auto post(std::string_view path)  { return request<Protocol>("POST",   path); }
    template <typename Protocol>
    auto put(std::string_view path)   { return request<Protocol>("PUT",    path); }
    template <typename Protocol>
    auto del(std::string_view path)   { return request<Protocol>("DELETE", path); }
    template <typename Protocol>
    auto patch(std::string_view path) { return request<Protocol>("PATCH",  path); }
    template <typename Protocol>
    auto head(std::string_view path)  { return request<Protocol>("HEAD",   path); }
};

} // namespace core::client
```

**Key decisions:**
- CRTP (`Derived`) — zero-cost, no vtable, matches `RouteHandler<Derived>` style
- `Protocol` template on function, not class — class is protocol-agnostic; call sites specify protocol
- `Derived` provides two connectors: `do_send<Protocol>` (transport hook) and `make_request<Protocol>` (factory)

### Modified: `include/worker/engine_client.cppm`

**a) Inheritance:**
```cpp
class EngineClient : public core::client::ClientHandler<EngineClient>,
                     public shared::HandlerBase { ... };
```

**b) Constructor — host + port (owns connection lifecycle):**
```cpp
explicit EngineClient(std::string_view host, std::uint16_t port)
    : m_socket{TlsSocket::connect(host, port)}, ...
```
> Implementation note: verify `TlsSocket` has a static `connect(host, port)` factory; if not, add it or adapt the constructor to call the appropriate socket connection API.

**c) Remove `call()` — replaced by CRTP connectors:**
```cpp
// Connector 1: send to HTTP/2 session
template <typename Protocol>
void do_send(interfaces::IRequest<Protocol>& req, ResponseFn cb) {
    auto& http_req = static_cast<io::layer::http2::HttpRequest&>(req);
    m_session.send(http_req);
    m_pending.emplace(http_req.get_stream_id(), std::move(cb));
}

// Connector 2: request factory
// NOTE: private helper renamed make_http_request() → build_http_request() to avoid
// conflict with this public CRTP connector method.
template <typename Protocol>
std::unique_ptr<interfaces::IRequest<Protocol>>
make_request(std::string_view method, std::string_view path) {
    return std::make_unique<io::layer::http2::HttpRequest>(
        build_http_request(method, path));
}
```

`set_self_contract()` and `on_execute()` / `on_released()` remain unchanged — `HandlerBase` lifecycle is implementation detail, not part of `ClientHandler`.

### Modified: `include/worker/context.cppm`

`m_engine_client` stays `EngineClient*` — CRTP cannot be erased to a common base without an extra wrapper. The abstraction value is the `ClientHandler` mixin providing a clean, typed API.

`call_engine()` uses verb API:
```cpp
void call_engine(std::string_view method, std::string_view path,
                 std::string_view body, EngineResponseFn callback) noexcept {
    using P = io::shared::http::Protocol;
    auto req = m_engine_client->request<P>(method, path);
    // populate body via req->get_body()
    std::promise<std::pair<int, std::string>> promise;
    auto future = promise.get_future();
    m_engine_client->send<P>(*req,
        [p = std::move(promise)](int s, std::string b) mutable {
            p.set_value({s, std::move(b)});
        });
    try {
        auto [status, resp_body] = future.get();
        callback(status, std::move(resp_body));
    } catch (...) {
        core::logger::error("worker/context", "engine call failed");
        callback(500, "engine communication error");
    }
}
```

## File Summary

| File | Change |
|---|---|
| `include/core/client/client.cppm` | **new** — `ClientHandler<Derived>` template |
| `include/worker/engine_client.cppm` | refactor — inherit `ClientHandler`, CRTP connectors, host/port ctor |
| `include/worker/context.cppm` | update — use `send<P>` / verb API |

## Build

`core_client` module needs adding to the core CMakeLists. `worker:engine_client` gains `import core_client`.

## Pattern Comparison

| | Router | Client |
|---|---|---|
| File | `core/server/router.cppm` | `core/client/client.cppm` |
| Class | `RouteHandler<Derived>` | `ClientHandler<Derived>` |
| Protocol | class-level `Derived` | function-level `Protocol` |
| Connector | `add_route()` / `match()` | `do_send<P>` / `make_request<P>` |
| Transport hook | `DispatchFn` in Session | `do_send` in `EngineClient` |
| Concrete impl | engine handler classes | `EngineClient` |
