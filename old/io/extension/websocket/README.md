# WebSocket extension

**WebSockets over HTTP/2 (RFC 8441 extended CONNECT) as an `IHttpExtension` — flip one SETTINGS bit, parse RFC 6455 frames off DATA, reuse the h2 transport.**

Classic WebSockets (RFC 6455) bootstrap off an HTTP/1.1 `Upgrade`. Over HTTP/2
there is no `Upgrade`; instead RFC 8441 defines an **extended CONNECT** — a
single h2 stream opened with `:method = CONNECT` and `:protocol = websocket`,
gated by a SETTINGS bit the server advertises. Once that stream is open, RFC 6455
data frames flow as the payload of h2 DATA frames on that stream. This extension
advertises the SETTINGS bit, recognises the extended-CONNECT request, and parses
the RFC 6455 framing — all through the `IHttpExtension` hooks
(`../../layer/http2/extension.cppm`). No `Upgrade`, no separate socket, no new
protocol layer.

## Specs

- **RFC 6455 (The WebSocket Protocol)** — the frame format (§5.2: FIN, opcode,
  MASK, payload length, masking key), opcodes (§5.6 text/binary, §5.5
  close/ping/pong), masking (§5.3), and close codes (§7.4). This is the byte
  layout you parse out of the DATA payload.
- **RFC 8441 (Bootstrapping WebSockets with HTTP/2)** — §3 defines
  `SETTINGS_ENABLE_CONNECT_PROTOCOL` (id `0x8`); §4 defines the extended CONNECT
  request (`:method = CONNECT`, `:protocol = websocket`, `:scheme`/`:path`
  required, no `Upgrade`/`Connection`/`Sec-WebSocket-Key` headers). This is the
  handshake.
- **RFC 9113 (HTTP/2)** — the transport. §6.1 DATA frames carry the RFC 6455
  frames; §8.5 is the extended CONNECT method itself.

Out of scope: HTTP/1.1 `Upgrade` and h2c (cleartext h2 via Upgrade). This repo's
h2 is TLS/ALPN only and `session.cppm:405` notes h2c is not wired.

## Handshake / negotiation (RFC 8441)

A server that accepts WebSockets over h2 **must** advertise
`SETTINGS_ENABLE_CONNECT_PROTOCOL = 0x8` with value `1` in its initial SETTINGS
frame. The handshake path already fans this seam out to every extension before
the SETTINGS frame is serialized: `Handshake::send_handshake()`
(`../../layer/http2/handshake.cppm:126-143`) calls
`on_local_settings(local)` on each registered extension, once per connection
(guarded by `m_sent_settings`). The comment at `handshake.cppm:137-140` names
this exact use case.

So the extension's `on_local_settings` override is:

```cpp
void on_local_settings(io::layer::http2::Settings &local) override {
    local.add_local_setting_override(0x8, 1);   // SETTINGS_ENABLE_CONNECT_PROTOCOL
}
```

`settings.cppm:173` already references `SETTINGS_ENABLE_CONNECT_PROTOCOL = 0x8`.
Confirm the peer enabled it (client side) via `on_remote_settings(const
Settings&)` (`extension.cppm:82`) and `remote.get_vendor_settings()` before
attempting an extended CONNECT.

## How it hooks in (wire order)

Override only these hooks on `IHttpExtension`
(`../../layer/http2/extension.cppm`); the rest stay no-ops.

1. `on_local_settings(Settings&)` (`extension.cppm:75`) — advertise the CONNECT
   protocol bit, as above. Fired once per connection at handshake.
2. `on_request_incoming(stream_id, IRequest&)` (`extension.cppm:124`) — the
   extended-CONNECT stream's HEADERS finished decoding. Detect it:
   `:method == CONNECT` and `:protocol == websocket`. Record `stream_id` as a
   WebSocket stream so the DATA hook parses it as RFC 6455 frames.
3. `on_frame_complete(stream_id, type, flags, payload, end_stream)`
   (`extension.cppm:159`) — DATA frames on that stream carry RFC 6455 frames.
   Append `payload` to the stream's buffer, then pull complete WS frames
   (FIN/opcode byte, MASK + 7/16/64-bit length, masking key, payload).
   Unmask (§5.3), reassemble fragments (§5.4), dispatch by opcode. `payload` is
   valid only during the call — copy anything you retain.
4. `on_stream_reset` / `on_stream_close` (`extension.cppm:97,103`) — tear down
   the per-stream WS state when the h2 stream ends.

## The write-back limit — read this before you design

`IHttpExtension` is documented (`extension.cppm:9-17`) as **pure
observe/intercept**: extensions "do NOT own a write-back channel and do NOT take
over streams." A hook can only mutate the reference it is handed.

Consequence for WebSockets: **this extension cannot send WS frames.** It can
enable the SETTINGS bit, recognise the CONNECT, decode and validate inbound RFC
6455 frames (including responding-to-ping *logic*), and enforce limits — but the
outbound half (pong replies, server-pushed text/binary, close frames, an echo)
must go through the normal response/DATA path
(`../../layer/http2/plugin.cppm`), not the extension. Design the extension as the
inbound decode/observe half; drive outbound frames from the handler that owns the
stream's `IResponse`/send path.

## Registration & packaging

An extension is live only once registered:

- `Server::register_extension(std::shared_ptr<IHttpExtension>)`
  (`../../layer/http2/plugin.cppm:201`) — server side.
- `Client::register_extension(...)` (`../../layer/http2/plugin.cppm:38`) —
  client side, before `on_connect()` builds the flow.

Runtime path (`plugin.cppm:194-197`): package as a `congelado::Plugin`, resolve
the http2 plugin's published `Server*` via capability dispatch
(`plugins/protocol/http2/src/http2.cc:208` `protocol_get()`), and
`register_extension(...)` during `on_load` before serving starts in `on_ready`.
Copy the packaging shape from `plugins/extensions/otel_otlp/`
(`shared` target, `apply_common_layer_settings`, `add_deps("congelado_sdk")`,
`CONGELADO_PLUGIN(...)`).

RFC 6455 needs no third-party dependency — the frame parser is a few bytes of
bit-twiddling. If/when a code module lands here, name it `io_extension_websocket`
(mirrors the `io_extension_<folder>` convention). No module exists yet — docs only.

## Skeleton (illustrative — not a build file)

```cpp
export module io_extension_websocket;   // when code lands; folder is docs-only for now

import std;
import io_layer_http2;   // IHttpExtension + Settings live in http2, re-exported
import interfaces;

export namespace io::extension::websocket {

class WebSocketExtension final : public io::layer::http2::IHttpExtension {
  public:
    [[nodiscard]] std::string_view name() const noexcept override { return "websocket"; }

    void on_local_settings(io::layer::http2::Settings &local) override {
        local.add_local_setting_override(0x8, 1);   // RFC 8441 SETTINGS_ENABLE_CONNECT_PROTOCOL
    }

    void on_request_incoming(std::uint32_t stream_id,
                             interfaces::io::IRequest &request) override {
        // :method == CONNECT && :protocol == websocket
        // -> mark stream_id as a WebSocket stream for the DATA hook below.
    }

    void on_frame_complete(std::uint32_t stream_id, std::uint8_t type, std::uint8_t flags,
                           std::span<const std::byte> payload, bool end_stream) override {
        // Only DATA frames on a marked stream. Append payload to this stream's
        // buffer, pop complete RFC 6455 frames (FIN/opcode, MASK+len, masking key,
        // payload), unmask, reassemble fragments, dispatch by opcode. Copy anything
        // kept — payload is valid only for this call. Outbound frames are NOT sent
        // from here (no write-back channel) — drive them from the stream's handler.
    }

    void on_stream_close(std::uint32_t stream_id) override {
        // drop per-stream WebSocket state
    }

  private:
    // per-stream frame-reassembly state keyed by stream_id
};

} // namespace io::extension::websocket
```
