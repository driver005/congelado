# gRPC extension

**gRPC-over-HTTP/2 as an `IHttpExtension` — ride the existing h2 transport, no new protocol layer, no `conan::grpc`.**

gRPC is not its own wire protocol. It is a calling convention layered on plain
HTTP/2: a `POST` to `/Service/Method`, `content-type: application/grpc`, one or
more length-prefixed protobuf messages carried in DATA frames, and a status code
delivered in trailers. Everything below the calling convention is already
implemented in this repo's HTTP/2 layer. This extension observes those pieces
through the `IHttpExtension` hooks (`../../layer/http2/extension.cppm`) and hands
the decoded message off — it does **not** re-implement HTTP/2.

## Specs

- **gRPC over HTTP/2** — the [gRPC HTTP2 protocol](https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md).
  Defines the request/response header set, `application/grpc[+proto]`
  content-type, the **Length-Prefixed-Message** framing, and the
  `grpc-status`/`grpc-message` **trailers**. This is the contract you implement.
- **RFC 9113 (HTTP/2)** — the transport. §8.1 request/response, §8.1 trailing
  HEADERS (trailers), §6.1 DATA frames. gRPC's LPM stream lives inside DATA.
- **Protobuf** — the default message codec. `conan::protobuf/6.33.5` is available
  in this build (`xmake.lua:79`). The gRPC library itself is **not** — it is
  intentionally commented out (`xmake.lua:80`), see the packaging note below.

## The gRPC calling convention (what actually goes on the wire)

Request from client:

```
:method = POST
:scheme = https
:path = /helloworld.Greeter/SayHello        # /<fully-qualified-service>/<method>
content-type = application/grpc              # or application/grpc+proto
te = trailers
<DATA frames: one or more Length-Prefixed-Messages>
```

Response from server:

```
:status = 200                                # HTTP status is always 200 for a gRPC call
content-type = application/grpc
<DATA frames: one or more Length-Prefixed-Messages>
grpc-status = 0                              # TRAILERS — the real gRPC result code
grpc-message = ...                           # TRAILERS — optional human message
```

**Length-Prefixed-Message** (the payload framing inside DATA):

```
+---------------+-----------------------+----------------------------+
| Compressed?   | Message length        | Message bytes              |
| 1 byte        | 4 bytes, big-endian   | <length> bytes of protobuf |
+---------------+-----------------------+----------------------------+
```

`Compressed?` is `0` (identity) or `1` (compressed per `grpc-encoding`). One
DATA frame can carry several LPMs, and one LPM can span several DATA frames — the
extension must buffer and reassemble across `on_frame_complete` calls.

## How it hooks in (wire order)

Everything the extension needs is a hook on `IHttpExtension`
(`../../layer/http2/extension.cppm`). Override only these; the rest stay no-ops.

1. `on_request_incoming(stream_id, IRequest&)` (`extension.cppm:124`) — first
   HEADERS block decoded. Detect a gRPC call: `content-type` starts with
   `application/grpc` and `:path` is `/Service/Method`. Record that this
   `stream_id` is a gRPC stream so the DATA hook knows to parse it.
2. `on_header_added(stream_id, name, value)` (`extension.cppm:115`) — fired per
   decoded header field. Use it to pick up `grpc-encoding`, `grpc-timeout`,
   `te: trailers` before the request object is finalized, if you need them
   earlier than `on_request_incoming`.
3. `on_frame_complete(stream_id, type, flags, payload, end_stream)`
   (`extension.cppm:159`) — DATA frames arrive here as raw bytes. Append
   `payload` to this stream's reassembly buffer, then pull off complete LPMs
   (1-byte flag + 4-byte BE length + body). `end_stream` marks the last DATA.
   `payload` is valid only for the duration of the call — copy what you keep.
4. `on_trailers(stream_id, IRequest&)` (`extension.cppm:146`) — the second
   HEADERS block. On a client-side extension this is where `grpc-status` /
   `grpc-message` land; read the result here.
5. `on_response_outgoing(stream_id, IResponse&)` (`extension.cppm:138`) /
   `on_request_outgoing` (`extension.cppm:131`) — observe (and optionally
   annotate) what is about to be framed on the way out.

## The write-back limit — read this before you design

`IHttpExtension` is documented (`extension.cppm:9-17`) as **pure
observe/intercept**: extensions "do NOT own a write-back channel and do NOT take
over streams." The only thing a hook can change is the reference it is handed
(`IRequest&`, `IResponse&`, `Settings&`).

Consequence for gRPC: **this extension cannot send a response.** It can decode
the incoming protobuf, validate the LPM framing, enforce timeouts/limits, and
annotate the `IRequest`/`IResponse` — but the actual method dispatch and the
reply go through the normal route handler and `IResponse` path
(`../../layer/http2/plugin.cppm` `Server::build` wires request→route→response).
Design the extension as the decode/observe half; serve the method as an ordinary
route registered against the router, keyed on the gRPC `:path`.

## Registration & packaging

An extension becomes live only when something registers it. Two seams:

- `Server::register_extension(std::shared_ptr<IHttpExtension>)`
  (`../../layer/http2/plugin.cppm:201`) — server side.
- `Client::register_extension(...)` (`../../layer/http2/plugin.cppm:38`) —
  client side, before `on_connect()` builds the flow.

The runtime path (`plugin.cppm:194-197`): package the extension as a
`congelado::Plugin`, resolve the http2 protocol plugin's published `Server*`
through the capability dispatch (`plugins/protocol/http2/src/http2.cc:208`
`protocol_get()`), and call `register_extension(...)` during `on_load`, before
the protocol plugin starts accepting connections in `on_ready`.

The packaging template to copy is `plugins/extensions/otel_otlp/` — a
`shared`-kind target using `apply_common_layer_settings`, `add_deps("congelado_sdk")`,
ending in `CONGELADO_PLUGIN(...)`.

**Do not add `conan::grpc`.** It is commented out on purpose (`xmake.lua:80`);
its from-source build (abseil/re2/c-ares/protobuf codegen) dominated build time
(rationale at `plugins/extensions/otel_otlp/src/otel_otlp_plugin.cc:62-66`).
Depend on `protobuf` only, and encode/decode the LPM framing yourself — it is a
1-byte flag plus a 4-byte length, not worth pulling in the whole stack.

If/when a code module lands here, name it `io_extension_grpc` (mirrors the
`io_extension_<folder>` convention). No module exists yet — this is docs only.

## Skeleton (illustrative — not a build file)

```cpp
export module io_extension_grpc;   // when code lands; folder is docs-only for now

import std;
import io_layer_http2;   // IHttpExtension lives in :extension, re-exported here
import interfaces;

export namespace io::extension::grpc {

class GrpcExtension final : public io::layer::http2::IHttpExtension {
  public:
    [[nodiscard]] std::string_view name() const noexcept override { return "grpc"; }

    void on_request_incoming(std::uint32_t stream_id,
                             interfaces::io::IRequest &request) override {
        // content-type: application/grpc[...] and :path == /Service/Method
        // -> mark stream_id as a gRPC stream for the DATA hook below.
    }

    void on_frame_complete(std::uint32_t stream_id, std::uint8_t type, std::uint8_t flags,
                           std::span<const std::byte> payload, bool end_stream) override {
        // Only DATA frames on a marked stream. Append payload to this stream's
        // buffer, pop complete Length-Prefixed-Messages (1B flag + 4B BE len + body),
        // hand each decoded protobuf message to the handler. Copy anything kept —
        // payload is valid only for this call.
    }

    void on_trailers(std::uint32_t stream_id, interfaces::io::IRequest &trailers) override {
        // client side: read grpc-status / grpc-message here.
    }

  private:
    // per-stream reassembly buffers keyed by stream_id
};

} // namespace io::extension::grpc
```
