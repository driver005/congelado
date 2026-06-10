module interfaces.response;
@nogc nothrow:

import interfaces.status;

// CRTP base for protocol-agnostic responses (HTTP/1-3, gRPC, WebSocket, ...).
// All mutators return Derived& for builder chaining.
// PORT-NOTE: C++ CRTP class -> D template class; builder Self&& helpers dropped (no deducing-this in D).
class IResponse(Protocol) {
    alias Header = Protocol.Header;
    alias Token = Protocol.Token;

    // PORT-NOTE: C++ had templated rvalue-ref builder helpers (add_header/remove_header/with_status/build).
    // D does not support deducing-this; builder chaining must be done at the concrete class level.

    abstract void add_header(scope const(char)[] name, scope const(char)[] value);
    abstract void remove_header(scope const(char)[] name);
    abstract void set_status(Status status);
    abstract void set_body(ubyte[] body);

    abstract Header[] get_header() const;
    abstract const(ubyte)[] get_body() const;
}
