export module shared:transport;

import std;

export namespace shared {

// class Request {
//   public:
//     virtual ~Request() = default;
// };
//
// class Response {
//   public:
//     virtual ~Response() = default;
//     // virtual void send(std::span<const std::uint8_t> data) = 0;
//     // virtual void set_status(int code) = 0;
// };

// template <typename T>
// concept IsRequest = std::derived_from<T, Request>;
//
// template <typename T>
// concept IsResponse = std::derived_from<T, Response>;


// using HandlerFn = void (*)(IRequest &, IResponse &) noexcept;
//
// using NextFn = std::move_only_function<void(IRequest &, IResponse &) noexcept>;
//
// using MiddlewareFn = void (*)(Req &, Res &, NextFn<IRequest, IResponse> &&);

} // namespace shared
