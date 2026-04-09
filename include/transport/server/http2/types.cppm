export module http2:types;

import std;

export namespace transport::server::http2 {

using HandlerFn = void (*)(void *) noexcept;
using NextFn = void (*)(void *) noexcept;
using MiddlewareFn = void (*)(void *, NextFn) noexcept;

} // namespace transport::server::http2
