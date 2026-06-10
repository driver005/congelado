module core.server.middleware;
@nogc nothrow:

import interfaces.request  : IRequest;
import interfaces.response : IResponse;
import interfaces.interfaces : MiddlewareFn;
import core.server.types   : Method;

// PORT-NOTE: C++ template<typename Derived, uint8_t MiddlewareSize> → D class template
// PORT-NOTE: std::runtime_error → assert/abort for @nogc

class Middleware(Derived, ubyte MiddlewareSize) {
  public:
    this() { m_middleware_index = 0; }

    void add_middleware(MiddlewareFn!Derived middleware) {
        if (m_middleware_index >= MiddlewareSize) {
            // PORT-NOTE: std::runtime_error → assert
            assert(false,
                "MiddlewareSize cannot be greater than MaxHandlerSize due to offerflow limitations, "
                "please check your routes");
        }
        m_middleware[m_middleware_index++] = middleware;
    }

    // constexpr void
    // push_middleware(const std::array<MiddlewareFn<Derived>, MiddlewareSize> &middleware) noexcept { ... }

    void execute(ref IRequest!Derived req, ref IResponse!Derived res,
                 ubyte offset, ubyte length) const {
        run_step(req, res, offset, cast(ubyte)(offset + length));
    }

    ubyte get_size() const { return m_middleware_index; }
    ref const(MiddlewareFn!Derived[MiddlewareSize]) get_middlewares() const { return m_middleware; }

  private:
    void run_step(ref IRequest!Derived req, ref IResponse!Derived res,
                  ubyte offset, ubyte border) const {
        if (offset < border) {
            const current_mw = m_middleware[offset];
            current_mw(req, res, (ref IRequest!Derived rq, ref IResponse!Derived rs) {
                this.run_step(rq, rs, cast(ubyte)(offset + 1), border);
            });
        }
    }

    MiddlewareFn!Derived[MiddlewareSize] m_middleware;
    ubyte m_middleware_index;
}
