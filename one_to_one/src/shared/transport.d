module shared.transport;
@nogc nothrow:

// Shared transport declarations, mirroring the C++ shared:transport partition.
// The C++ source is entirely commented out (Request, Response classes, IsRequest /
// IsResponse concepts, HandlerFn, NextFn, MiddlewareFn aliases).
// The D port preserves that state.

// class Request {
//   public:
//     virtual ~Request() = default;
// }
//
// class Response {
//   public:
//     virtual ~Response() = default;
//     // virtual void send(ubyte[] data) = 0;
//     // virtual void set_status(int code) = 0;
// }

// template IsRequest(T) { enum bool IsRequest = is(T : Request); }
// template IsResponse(T) { enum bool IsResponse = is(T : Response); }

// alias HandlerFn = void function(IRequest, IResponse) @nogc nothrow;
//
// alias NextFn = void delegate(IRequest, IResponse) @nogc nothrow;
//
// alias MiddlewareFn = void function(Req, Res, NextFn) @nogc nothrow;
