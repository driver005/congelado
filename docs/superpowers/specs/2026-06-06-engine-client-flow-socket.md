# EngineClient — ClientFlowSocket Integration Design

**Date:** 2026-06-06
**Status:** Approved

## Goal

Replace `EngineClient`'s manual sync I/O (`TlsSocket` + `BufferWriter` + `flush_node()` + `receive_once()`) with `ClientFlowSocket`. Wire `WorkerContext` as the independent owner of the `ContractGroup<>` and `EngineClient`, replacing the external `set_engine_client()` setter with `set_engine_endpoint(host, port)`.

## Context

`EngineClient` currently:
- Owns `Socket<TLS>` and calls `sync_connect()` in constructor
- Owns `BufferWriter m_pool` for manual receive buffering
- Owns `Handshake<false>` for HTTP/2 client preface
- Is a `HandlerBase` — its `on_execute()` drives a manual loop: handshake → drain outbound queue → `receive_once()` → reschedule
- Uses a mutex-protected `m_outbound` queue for cross-thread request enqueue

`WorkerContext` currently:
- Holds `EngineClient*` set externally via `set_engine_client(EngineClient&)`
- Has no ownership of the ContractGroup the engine client runs on

## Design

### `WorkerContext` — `include/worker/context.cppm`

**Add** `core::contract::ContractGroup<> m_group` as a value member (owns the worker's contract pool).

**Replace** `set_engine_client(EngineClient&)` with `set_engine_endpoint(host, port)`:

```cpp
void set_engine_endpoint(std::string_view host, std::uint16_t port) {
    m_engine_client.emplace(host, port, m_group);
    m_engine_client->build();
}
```

**Change** `m_engine_client` from `EngineClient*` to `std::optional<EngineClient>`.

**Add** import: `core_contract` (for `ContractGroup<>`).

`call_engine()` is unchanged — same `promise/future` blocking pattern.

### `EngineClient` — `include/worker/engine_client.cppm`

#### What changes

| Before | After |
|---|---|
| `Socket<TLS> m_socket` | removed |
| `BufferWriter m_pool` | removed |
| `Handshake<false> m_handshake` | removed |
| `bool m_handshake_done` | removed |
| `queue<OutboundRequest> m_outbound` | removed |
| `mutex m_mutex` | becomes `m_session_mutex` (guards session access) |
| `Session m_session` | `optional<Session> m_session` |
| inherits `HandlerBase` | removed — no longer a handler |
| `on_execute()` / `on_released()` / `get_name()` | removed |
| `set_self_contract()` | removed |
| `flush_node()` / `receive_once()` / `drain_outbound()` | removed |

**Added:**
- `leverage::Leverager<leverage::Context> m_leverager{}` — dummy instance for `ClientFlowSocket` API
- `ClientFlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> m_flow`

#### Constructor

```cpp
explicit EngineClient(std::string_view host, std::uint16_t port,
                      core::contract::ContractGroup<> &group)
    : m_leverager{},
      m_flow{socket::Endpoint{host, port}, m_leverager, group},
      m_session{},
      m_pending{},
      m_discard_next{false} {}
```

No `sync_connect()` — connection is established asynchronously in `build()`.

#### `build()`

Sets `ConnectionEstablishedCallback` then calls `m_flow.build()`:

```cpp
void build() {
    m_flow.add_on_accept(
        [this](shared::SendCallback send_cb,
               shared::CloseCallback close_cb) -> shared::ReadCallback {
            // Wrap SendCallback with discard-next logic
            // (Session sends a response frame after dispatch; we drop it on the client side)
            auto wrapped_send = [this, inner = std::move(send_cb)]
                                (utils::buffering::BufferNode &&node) mutable {
                if (m_discard_next) { m_discard_next = false; return; }
                inner(std::move(node));
            };

            {
                std::lock_guard lock{m_session_mutex};
                m_session.emplace(
                    std::move(wrapped_send),
                    std::move(close_cb),
                    [this](interfaces::IRequest<io::shared::http::Protocol> &req,
                           interfaces::IResponse<io::shared::http::Protocol> &) {
                        dispatch_response(req);
                    });

                // Send HTTP/2 client preface immediately
                utils::buffering::BufferReader empty{};
                io::layer::http2::Handshake<false>{
                    m_session->get_local_settings(),
                    [this](utils::buffering::BufferNode &&node) {
                        m_session->send_node(std::move(node));
                    }
                }.process(empty);
            }

            return [this](utils::buffering::BufferReader &reader) {
                std::lock_guard lock{m_session_mutex};
                m_session->receive(reader);
            };
        });

    m_flow.build();
}
```

#### `do_send()` (CRTP connector 1)

No outbound queue. Directly acquires session mutex and calls `m_session->send()`:

```cpp
template <typename Protocol>
void do_send(std::unique_ptr<interfaces::IRequest<Protocol>> req, ResponseFn cb) {
    auto http_req = std::unique_ptr<io::layer::http2::HttpRequest>(
        static_cast<io::layer::http2::HttpRequest *>(req.release()));

    std::lock_guard lock{m_session_mutex};
    if (!m_session.has_value()) {
        core::logger::error("engine_client", "send before connection established");
        cb(503, "engine not connected");
        return;
    }

    m_session->send(*http_req);
    m_pending.emplace(http_req->get_stream_id(), std::move(cb));
}
```

#### `dispatch_response()`

Unchanged in logic — called from within `m_session->receive()` (already under `m_session_mutex`), resolves pending callback:

Unchanged from current implementation — logic is already correct:

```cpp
void dispatch_response(interfaces::IRequest<io::shared::http::Protocol> &req) {
    m_discard_next = true;
    auto &http_req = static_cast<io::layer::http2::HttpRequest &>(req);
    auto stream_id = http_req.get_stream_id();
    auto status_sv = req.find_header(":status");
    int status_code = 500;
    if (!status_sv.empty())
        std::from_chars(status_sv.data(), status_sv.data() + status_sv.size(), status_code);
    std::string body;
    for (auto byte : req.get_body())
        body.push_back(static_cast<char>(byte));
    auto it = m_pending.find(stream_id);
    if (it != m_pending.end()) {
        auto cb = std::move(it->second);
        m_pending.erase(it);
        cb(status_code, std::move(body));
    } else {
        core::logger::warning("engine_client", "no callback for stream {}", stream_id);
    }
}
```

#### Threading model

| Access | Thread | Lock |
|---|---|---|
| `do_send()` | any (caller of `call_engine()`) | `m_session_mutex` |
| ReadCallback → `m_session->receive()` | Receiver contract thread | `m_session_mutex` |
| `dispatch_response()` | inside `receive()` call | already held |
| `m_pending` read/write | inside `m_session_mutex` scope | held |
| `m_discard_next` | inside `m_session_mutex` scope | held |

#### HTTP/2 stream ID rollover

When `m_session` stream IDs approach the HTTP/2 client-stream limit (2^31−1), close and reconnect. Detection in `do_send()` before sending:

```cpp
// Client-initiated streams are odd; last valid is 2^31 - 1 = 2147483647.
// Reconnect with 1000 streams of headroom so in-flight requests can complete.
constexpr std::uint32_t STREAM_ID_RECONNECT_THRESHOLD = 2147482647u;

if (m_session->get_last_client_stream_id() >= STREAM_ID_RECONNECT_THRESHOLD) {
    m_session->close(error::http::Http2ErrorCode::NO_ERROR);
    m_session.reset();
    m_flow.build();  // initiates new async connect + TLS handshake
}
```

#### Imports added

- `io_flow_socket` (for `ClientFlowSocket`)
- `io_base_leverage` (for `Leverager<Context>`)

`utils_buffering` and `io_base_socket` are retained — `BufferNode`/`BufferReader` are still used in callbacks, and `socket::Endpoint` is used in the constructor.

## File Summary

| File | Change |
|---|---|
| `include/worker/context.cppm` | add `ContractGroup<>` member, `set_engine_endpoint()`, `optional<EngineClient>` |
| `include/worker/engine_client.cppm` | major refactor — use `ClientFlowSocket`, remove `HandlerBase`, async I/O |

## Pattern Comparison

| | Before | After |
|---|---|---|
| I/O | manual `sync_connect` + `sync_send` + `sync_receive` | `ClientFlowSocket` (Sender/Receiver contracts) |
| HTTP/2 preface | in `on_execute()` first iteration | in `ConnectionEstablishedCallback` |
| Send path | outbound queue → contract → `flush_node()` | direct `m_session->send()` under mutex |
| Receive path | `receive_once()` polled in `on_execute()` | Receiver fires ReadCallback |
| EngineClient lifecycle | `HandlerBase` registered externally | plain class owned by `WorkerContext` |
| ContractGroup owner | external (wherever `set_engine_client()` was called) | `WorkerContext` |
