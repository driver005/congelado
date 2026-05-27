export module core_server:middleware;

import std;
import interfaces;
import :types;

export namespace core::server {

template <typename Derived, std::uint8_t MiddlewareSize>
class Middleware {
  public:
    constexpr Middleware() : m_middleware{}, m_middleware_index{0} {}

    constexpr void push(interfaces::MiddlewareFn<Derived> middleware) {
        if (m_middleware_index >= MiddlewareSize)
            throw std::runtime_error(
                "MiddlewareSize cannot be greater than MaxHandlerSize due to offerflow limitations, please check "
                "your routes");

        m_middleware[m_middleware_index++] = middleware;
    }

    // constexpr void
    // push_middleware(const std::array<interfaces::MiddlewareFn<Derived>, MiddlewareSize> &middleware) noexcept {
    //     if (m_middleware_index + middleware.size() <= MiddlewareSize) {
    //         std::ranges::copy(m_middleware, end());
    //         m_middleware_index += middleware.size();
    //     }
    // }

    void execute(interfaces::IRequest<Derived> &req, interfaces::IResponse<Derived> &res, std::uint8_t offset,
                 std::uint8_t length) const noexcept {
        run_step(req, res, offset, offset + length);
    }

    constexpr auto begin() noexcept { return m_middleware.begin(); }
    constexpr auto end() noexcept { return m_middleware.begin() + m_middleware_index; }

    constexpr auto begin() const noexcept { return m_middleware.begin(); }
    constexpr auto end() const noexcept { return m_middleware.begin() + m_middleware_index; }

    constexpr const std::uint8_t &get_size() const noexcept { return m_middleware_index; }
    constexpr const std::array<interfaces::MiddlewareFn<Derived>, MiddlewareSize> &get_middlewares() const noexcept {
        return m_middleware;
    }

  private:
    void run_step(interfaces::IRequest<Derived> &req, interfaces::IResponse<Derived> &res, std::uint8_t offset,
                  std::uint8_t border) const noexcept {
        if (offset < border) {
            const auto current_mw = m_middleware[offset];
            current_mw(req, res, [&](interfaces::IRequest<Derived> &rq, interfaces::IResponse<Derived> &rs) noexcept {
                this->run_step(rq, rs, offset + 1, border);
            });
        }
    }

    std::array<interfaces::MiddlewareFn<Derived>, MiddlewareSize> m_middleware;
    std::uint8_t m_middleware_index;
};

} // namespace core::server
