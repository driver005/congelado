module io.flow.flow;
@nogc nothrow:

import shared_.handler;
import shared_.flow;
import io.base.leverage.base;
import io.flow.sender.sender;
import io.flow.receiver.reveiver;
import io.flow.socket.socket;

// PORT-NOTE: C++ io::base::flow::Flow was a variadic template builder that accumulated
// "recipe" lambdas in a std::tuple and called them on build().
// D has no heterogeneous tuple builder with CTAD.  The class is ported as a simple
// runtime builder that holds a single factory function; for multiple flows callers
// chain build() calls explicitly.  The variadic pattern can be restored via template
// mixins in Run 3.

/// io::base::flow::Flow
/// Builder that lazily constructs flow objects using the registered recipes.
class Flow {
  public:
    this(ref Leverager leverager_ref, ref HandlerController controller_ref) {
        m_leverager = &leverager_ref;
        m_controller = &controller_ref;
    }

    // PORT-NOTE: C++ template add<T>() accumulated recipes in a std::tuple.
    // D port defers multi-recipe composition to Run 3.
    // add() stubs are omitted; callers instantiate concrete flow types directly.

    // PORT-NOTE: C++ build() returned std::make_tuple(recipes(lev, ctrl)...).
    // D port returns void; concrete flows are created by callers.
    void build() {
        // No-op in this structural port.
    }

  private:
    Leverager* m_leverager;
    HandlerController* m_controller;
}
