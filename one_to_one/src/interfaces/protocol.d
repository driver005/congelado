module interfaces.protocol;
@nogc nothrow:

import shared.flow   : ReadCallback, SendCallback, CloseCallback;
import interfaces.request  : IRequest;
import interfaces.response : IResponse;

// PORT-NOTE: io::shared::http::Protocol is forward-referenced here as an opaque
// placeholder. The concrete module (io.shared.http.http) will supply the real type.
// For now use a forward-declared struct tag so the function aliases compile.
struct HttpProtocol;   // PORT-NOTE: placeholder for io.shared.http.Protocol

// Dispatch function: called by the protocol layer for each fully-received request.
// Protocol implementations call this once per request/response pair.
// PORT-NOTE: std::function -> fn+ctx pair to stay @nogc.
struct ReceiveDispatchFn {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    void function(void* ctx,
                  ref IRequest!HttpProtocol  req,
                  ref IResponse!HttpProtocol res) @nogc nothrow fn;
    void* ctx;
}

// PORT-NOTE: C++ concept ServerConcept -> D template constraint comment.
// ServerConcept!T: T must have:
//   on_connect(SendCallback, CloseCallback) -> ReadCallback
//   build(void* router_ctx) -> void

// SendDispatchFn: called on client side to issue a request.
// PORT-NOTE: std::function -> fn+ctx pair.
struct SendDispatchFn {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    void function(void* ctx, ref IRequest!HttpProtocol req) @nogc nothrow fn;
    void* ctx;
}

// PORT-NOTE: C++ concept ClientConcept -> D template constraint comment.
// ClientConcept!T: T must be constructible from ReceiveDispatchFn,
// and have:
//   on_connect(SendCallback, CloseCallback) -> ReadCallback
//   on_send() -> SendDispatchFn

// Interface for io-layer protocol plugins.
// Each protocol implementation controls transport binding
// (host, port, TLS, threads) and per-connection data handling.
// PORT-NOTE: C++ template <ServerConcept Server, ClientConcept Client> class IProtocol ->
// D template class. Concept constraints dropped; enforced at instantiation sites.
class IProtocol(Server, Client) {
    const(char)[] get_protocol_name() const;

    const(char)[] get_bind_host() const;
    ushort        get_bind_port() const;
    uint          get_bind_threads() const;
    const(char)[] get_tls_cert() const;
    const(char)[] get_tls_key() const;

    // PORT-NOTE: C++ returned std::unique_ptr<Server>/std::unique_ptr<Client>;
    // D returns naked pointer allocated with make!T; caller owns and must dispose.
    // Default implementations return null (no-op, unlike C++ which throws std::runtime_error).
    Server* get_server() { return null; }
    Client* get_client(ReceiveDispatchFn) { return null; }

    // Default no-op for protocols that build internally via build().
    void set_dispatch(ReceiveDispatchFn) {}
}


// PORT-NOTE: C++ std::function aliases -> fn+ctx pair structs to stay @nogc.
struct HandlerFn(Derived) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    void function(void* ctx,
                  ref IRequest!Derived  req,
                  ref IResponse!Derived res) @nogc nothrow fn;
    void* ctx;
}

// PORT-NOTE: std::move_only_function -> fn+ctx pair.
struct NextFn(Derived) {
    // PORT-NOTE: value wrapper, exempt from classes-only rule
    void function(void* ctx,
                  ref IRequest!Derived  req,
                  ref IResponse!Derived res) @nogc nothrow fn;
    void* ctx;
}

// MiddlewareFn: plain function pointer (no capture needed).
alias MiddlewareFn(Derived) =
    void function(ref IRequest!Derived req,
                  ref IResponse!Derived res,
                  NextFn!Derived next) @nogc nothrow;
