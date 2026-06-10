module core.server.handler;
@nogc nothrow:

import interfaces.request    : IRequest;
import interfaces.response   : IResponse;
import interfaces.interfaces : HandlerFn;
import core.server.consts    : HANDLER_MASK;
import core.server.types     : Method;

// PORT-NOTE: C++ template<typename Derived, uint8_t MaxHandlerSize = 8>
// → D class Handler(Derived, ubyte MaxHandlerSize = 8)

class Handler(Derived, ubyte MaxHandlerSize = 8) {
  public:
    this() {
        m_handler_mask  = HANDLER_MASK;
        m_handler_index = 0;
    }

    void add_handler(const(Method) method, HandlerFn!Derived handler) {
        if (m_handler_index >= MaxHandlerSize)
            assert(false,
                "HandlerSize cannot be greater than MaxHandlerSize due to offerflow limitations, "
                "please check your routes");

        const ubyte shift = cast(ubyte)(cast(ubyte)method * 8);

        if (((m_handler_mask >> shift) & 0xFF) != 0xFF)
            assert(false, "Handler for method already exists");

        m_handler_mask &= ~(0xFFUL << shift);
        m_handler_mask |= (cast(ulong)m_handler_index << shift);
        m_handler[m_handler_index++] = handler;
    }

    HandlerFn!Derived find(Method method) const {
        const ubyte idx = cast(ubyte)((m_handler_mask >> (cast(ubyte)method * 8)) & 0xFF);
        return idx != 0xFF ? m_handler[idx] : null;
    }

    ubyte get_size() const  { return m_handler_index; }
    size_t get_mask() const { return m_handler_mask; }
    ref const(HandlerFn!Derived[MaxHandlerSize]) get_handler() const { return m_handler; }

  private:
    HandlerFn!Derived[MaxHandlerSize] m_handler;
    size_t m_handler_mask;
    ubyte  m_handler_index;
}

class HandlerPool(Derived, size_t MaxHandlerSize) {
  public:
    this() { m_handler_index = 0; }

    HandlerFn!Derived find(size_t idx, size_t handler_mask, Method method) const {
        const ubyte offset = cast(ubyte)((handler_mask >> (cast(ubyte)method * 8)) & 0xFF);
        return offset != 0xFF ? m_handler[idx + offset] : null;
    }

    void add_handler(HandlerFn!Derived handler) {
        if (m_handler_index >= MaxHandlerSize)
            assert(false,
                "HandlerSize cannot be greater than MaxHandlerSize due to offerflow limitations, "
                "please check your routes");
        m_handler[m_handler_index++] = handler;
    }

    ubyte get_size() { return m_handler_index; }

  private:
    HandlerFn!Derived[MaxHandlerSize] m_handler;
    ubyte m_handler_index;
}
