export module shared:transport;

import std;

export namespace shared {

class Request {
  public:
    Request() : m_body{} {}
    virtual ~Request() = default;

    // virtual std::string_view get_method() const noexcept = 0;
    //
    // virtual std::string_view get_path() const noexcept = 0;

    std::span<const std::uint8_t> get_body() const noexcept { return m_body; }

  protected:
    std::vector<std::uint8_t> m_body;
};

class Response {
  public:
    virtual ~Response() = default;
    // virtual void send(std::span<const std::uint8_t> data) = 0;
    // virtual void set_status(int code) = 0;
};

template <typename T>
concept IsRequest = std::derived_from<T, Request>;

template <typename T>
concept IsResponse = std::derived_from<T, Response>;


template <IsRequest Req, IsResponse Res>
using HandlerFn = void (*)(Req &, Res &) noexcept;

template <IsRequest Req, IsResponse Res>
using NextFn = std::move_only_function<void(Req &, Res &) noexcept>;

template <IsRequest Req, IsResponse Res>
using MiddlewareFn = void (*)(Req &, Res &, NextFn<Req, Res> &&);

} // namespace shared
