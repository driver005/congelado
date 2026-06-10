module interfaces.request;
@nogc nothrow:

import utils.buffering.view;

// CRTP base for protocol-agnostic requests (HTTP/1-3, gRPC, WebSocket, ...).
// Protocols without header/body support inherit no-op defaults.
// All mutators return Derived& for builder chaining.
// PORT-NOTE: C++ CRTP class -> D template class; no equivalent of C++ deducing this / Self&&.
class IRequest(Protocol) {
    alias Header = Protocol.Header;
    alias Token = Protocol.Token;

    // PORT-NOTE: C++ had templated rvalue-ref builder helpers (add_header/remove_header/build).
    // D does not support deducing-this; builder chaining must be done at the concrete class level.

    // PORT-NOTE: C++ accepts std::variant<string_view, Token>; D uses overloads for string and TokenType separately
    abstract void add_header(scope const(char)[] name, scope const(char)[] value);
    abstract void add_header(Token name, scope const(char)[] value);
    abstract void remove_header(scope const(char)[] name);
    abstract void remove_header(Token name);

    abstract const(char)[] get_method() const;
    abstract const(char)[] get_target() const;
    abstract ref BufferView get_body();
    abstract Header[] get_header() const;

    // Look up a single header value by name (lowercase HTTP/2 style, e.g. "accept").
    // Returns empty string_view when not found.
    // Protocol implementations override this; the default is a no-op fallback.
    const(char)[] find_header(scope const(char)[] /*name*/) const {
        return [];
    }
}
