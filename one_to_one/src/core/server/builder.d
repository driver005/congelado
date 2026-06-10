module core.server.builder;
@nogc nothrow:

import interfaces.request    : IRequest;
import interfaces.response   : IResponse;
import interfaces.interfaces : HandlerFn, MiddlewareFn;
import core.server.types     : Method, EdgeKind;
import core.server.handler   : Handler;
import core.server.middleware : Middleware;
import core.server.router    : RouteHandler;
import core.server.server    : Server;
import util.alloc : make, dispose;

// Forward declarations
class Route(Derived, size_t MaxHandlerSize = 8, size_t MaxMiddlewareSize = 10);
class Router(Derived, size_t MaxMiddlewareSize = 10);

// PORT-NOTE: fnv1a also used in builder for sort hashing — declared here too
private uint fnv1a(const(char)[] s) {
    uint h = 5381u;
    foreach (c; s)
        h = h * 33u ^ cast(ubyte)c;
    return h;
}

class RouterContext(Derived,
                    size_t RouterSize     = 256,
                    size_t MaxHandlerSize = 8,
                    size_t MaxMiddlewareSize = 10) {
  public:
    this() {
        m_router_size             = 2;
        m_highest_router_number   = 1;
        m_base_router_children    = 0;
    }

    size_t add_route(Route!(Derived) route) {
        const auto idx = m_router_size++;
        m_routes[idx] = route;
        if (route.get_base_router() == 0)
            ++m_base_router_children;
        return idx;
    }

    Route!Derived opIndex(size_t index) {
        if (index >= m_router_size)
            assert(false, "RouterContext index out of bounds");
        return m_routes[index];
    }

    void opIndexAssign(Route!Derived val, size_t index) {
        m_routes[index] = val;
    }

    // TODO: figue out a way to make this function consteval
    RouteHandler!Derived build() {
        // PORT-NOTE: std::vector<std::vector<RouterBuildingHelper>> table
        // → fixed-size RouterBuildingHelper[128][16] to avoid GC.
        RouterBuildingHelper[128][16] table_buf;
        size_t[16] table_counts;
        size_t table_len = 0;
        size_t idx = 0;

        // Sort is skipped under @nogc for the one-to-one pass
        // (std::ranges::sort → deferred to Run 3 with a @nogc sort)

        auto handler = make!(RouteHandler!Derived)();

        while (idx < m_router_size) {
            insert_route(m_routes[idx], table_buf, table_counts, table_len);
            ++idx;
        }

        foreach (ri; 0 .. table_len) {
            size_t cumulative_children = 0;
            foreach (i; 0 .. table_counts[ri]) {
                auto kind       = table_buf[ri][i].kind;
                auto literal    = table_buf[ri][i].literal;
                auto children   = table_buf[ri][i].children;
                auto handlers   = table_buf[ri][i].handlers;
                auto middlewares = table_buf[ri][i].middlewares;
                size_t offset = 0;
                if (children > 0)
                    offset = table_counts[ri] - i + cumulative_children;

                cumulative_children += children;

                handler.add_route(kind, literal, offset,
                                   handlers, middlewares,
                                   cast(ubyte)children);
            }
        }

        import core.stdc.stdio : printf;
        printf("Finished building\n");

        return handler;
    }

    size_t get_highest_router_number() { return m_highest_router_number++; }
    size_t get_router_size()           const { return m_router_size; }
    size_t get_base_router_children()  const { return m_base_router_children; }

    void decrement_base_router_children() {
        if (m_base_router_children > 0)
            --m_base_router_children;
    }

  private:
    struct RouterBuildingHelper {
        EdgeKind   kind          = EdgeKind.Path;
        const(char)[] literal;
        Handler!(Derived, MaxHandlerSize) handlers;
        Middleware!(Derived, MaxMiddlewareSize) middlewares;
        size_t    children       = 0;
        size_t    router_number  = 0;
    }

    static private (EdgeKind, const(char)[]) determine_node_kind(const(char)[] path) {
        if (path.length == 0)
            assert(false, "Path cannot be empty");

        if (path == "*")
            return (EdgeKind.Wild, "");

        if (path.length > 0 && path[0] == ':')
            return (EdgeKind.Param, "");

        return (EdgeKind.Path, path);
    }

    // PORT-NOTE: C++ uses std::vector<std::vector<RouterBuildingHelper>>;
    // D uses fixed-size RouterBuildingHelper[128][16] to avoid GC.
    private void insert_route(Route!Derived route,
                               ref RouterBuildingHelper[128][16] table_buf,
                               ref size_t[16] table_counts,
                               ref size_t table_len) {
        if (route.get_child_routes() != 0) {
            if (route.is_build()) return;

            auto t = determine_node_kind(route.get_path());
            EdgeKind      kind    = t[0];
            const(char)[] literal = t[1];

            switch (kind) {
            case EdgeKind.Path:  break;
            case EdgeKind.Param: assert(false, "Router path cannot be parameterized");
            case EdgeKind.Wild:  assert(false, "Router path cannot be wildcard");
            default:             break;
            }

            size_t new_idx = 0;

            if (route.get_base_router() != size_t.max) {
                bool found = false;
                foreach (ri; 0 .. table_len) {
                    foreach (ci; 0 .. table_counts[ri]) {
                        if (table_buf[ri][ci].router_number == route.get_base_router()) {
                            new_idx = ri + 1;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found)
                    assert(false, "Parent router not found");
            }

            if (new_idx >= table_len)
                table_len = new_idx + 1;

            assert(table_counts[new_idx] < 128, "insert_route: row overflow");
            // PORT-NOTE: C++ uses std::vector; D uses fixed-size T[N] to avoid GC.
            table_buf[new_idx][table_counts[new_idx]++] = RouterBuildingHelper(
                kind, literal, route.get_handlers(), route.get_middlewares(),
                route.get_child_routes(), route.get_router_number());

            route.set_build();
        } else {
            if (route.is_build()) return;

            auto t = determine_node_kind(route.get_path());
            EdgeKind      kind    = t[0];
            const(char)[] literal = t[1];

            size_t new_idx = 0;

            if (route.get_base_router() != size_t.max) {
                bool found = false;
                foreach (ri; 0 .. table_len) {
                    foreach (ci; 0 .. table_counts[ri]) {
                        if (table_buf[ri][ci].router_number == route.get_base_router()) {
                            new_idx = ri + 1;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found)
                    assert(false, "Parent router not found");
            }

            if (new_idx >= table_len)
                table_len = new_idx + 1;

            assert(table_counts[new_idx] < 128, "insert_route: row overflow");
            // PORT-NOTE: C++ uses std::vector; D uses fixed-size T[N] to avoid GC.
            table_buf[new_idx][table_counts[new_idx]++] = RouterBuildingHelper(
                kind, literal, route.get_handlers(), route.get_middlewares(),
                route.get_child_routes(), 0);

            route.set_build();
        }
    }

    // Middleware<MaxMiddlewareSize> m_middlewares;
    Route!Derived[RouterSize] m_routes;
    size_t m_router_size;
    size_t m_highest_router_number;
    size_t m_base_router_children;
}

class Route(Derived, size_t MaxHandlerSize = 8, size_t MaxMiddlewareSize = 10) {
  public:
    this() {
        m_path            = "";
        m_is_build        = false;
        m_router_number   = 0;
        m_base_router     = 0;
        m_child_routes    = 0;
        m_handler         = make!(Handler!(Derived, MaxHandlerSize))();
        m_local_middleware = make!(Middleware!(Derived, MaxMiddlewareSize))();
    }

    ~this() {
        dispose(m_handler);
        dispose(m_local_middleware);
    }

    this(const(char)[] path) {
        m_path = (path.length > 1 && path[0] == '/') ? path[1 .. $] : path;
        m_is_build        = false;
        m_router_number   = 0;
        m_base_router     = 0;
        m_child_routes    = 0;
        m_handler         = make!(Handler!(Derived, MaxHandlerSize))();
        m_local_middleware = make!(Middleware!(Derived, MaxMiddlewareSize))();
        if (path != "*") {
            bool has_slash = false;
            foreach (c; path) { if (c == '/') { has_slash = true; break; } }
            if (!has_slash) assert(false, "Route path must start '/'");
        }
    }

    Route!Derived use(MiddlewareFn!Derived mw) {
        m_local_middleware.add_middleware(mw);
        return this;
    }

    Route!Derived get(HandlerFn!Derived handler) {
        return add_handler(Method.GET, handler);
    }

    Route!Derived post(HandlerFn!Derived handler) {
        return add_handler(Method.POST, handler);
    }

    Route!Derived put(HandlerFn!Derived handler) {
        return add_handler(Method.PUT, handler);
    }

    Route!Derived patch(HandlerFn!Derived handler) {
        return add_handler(Method.PATCH, handler);
    }

    Route!Derived delt(HandlerFn!Derived handler) {
        return add_handler(Method.DELETE, handler);
    }

    Route!Derived head(HandlerFn!Derived handler) {
        return add_handler(Method.HEAD, handler);
    }

    Route!Derived options(HandlerFn!Derived handler) {
        return add_handler(Method.OPTIONS, handler);
    }

    Route!Derived add_handler(Method method, HandlerFn!Derived handler) {
        auto handler_fnc = m_handler.find(method);
        if (handler_fnc is null) {
            m_handler.add_handler(method, handler);
            return this;
        }
        assert(false, "Duplicate handler found for method");
    }

    void add_handler_in_place(Method method, HandlerFn!Derived handler) {
        if (m_handler.find(method) !is null)
            assert(false, "Handler for method already exists");
        m_handler.add_handler(method, handler);
    }

    Route!Derived set_base_router(size_t router_number) {
        m_base_router = router_number;
        return this;
    }

    Route!Derived set_router_number(size_t router_number) {
        m_router_number = router_number;
        return this;
    }

    void add_middleware(MiddlewareFn!Derived middleware) {
        m_local_middleware.add_middleware(middleware);
    }

    const(char)[] get_path()         const { return m_path; }
    bool   is_build()                const { return m_is_build; }
    size_t get_child_routes()        const { return m_child_routes; }
    size_t get_base_router()         const { return m_base_router; }
    size_t get_router_number()       const { return m_router_number; }
    Handler!(Derived, MaxHandlerSize) get_handlers()     const { return m_handler; }
    Middleware!(Derived, MaxMiddlewareSize) get_middlewares() const { return m_local_middleware; }

    void update_base_router(size_t number)   { m_base_router = number; }
    void update_child_routes(size_t number)  { m_child_routes = number; }
    void update_router_number(size_t number) { m_router_number = number; }
    void set_build()                         { m_is_build = true; }
    void increment_child_routes()            { ++m_child_routes; }

  private:
    const(char)[]  m_path;
    bool   m_is_build;
    size_t m_router_number;
    size_t m_base_router;
    ubyte  m_child_routes;
    Handler!(Derived, MaxHandlerSize)     m_handler;
    Middleware!(Derived, MaxMiddlewareSize) m_local_middleware;
}

class Router(Derived, size_t MaxMiddlewareSize = 10) {
  public:
    this() {
        m_ctx          = null;
        m_router_number = 0;
        m_router_index  = 0;
    }

    this(RouterContext!Derived ctx, const(char)[] path) {
        m_ctx           = ctx;
        m_router_number = ctx.get_highest_router_number();
        m_router_index  = m_ctx.add_route(
            (make!(Route!Derived)(path)).set_router_number(m_router_number));
    }

    Router!Derived use(MiddlewareFn!Derived middleware) {
        m_ctx[m_router_index].add_middleware(middleware);
        return this;
    }

    Router!Derived get(HandlerFn!Derived handler) {
        m_ctx[m_router_index].add_handler_in_place(Method.GET, handler);
        return this;
    }

    Router!Derived post(HandlerFn!Derived handler) {
        m_ctx[m_router_index].add_handler_in_place(Method.POST, handler);
        return this;
    }

    Router!Derived put(HandlerFn!Derived handler) {
        m_ctx[m_router_index].add_handler_in_place(Method.PUT, handler);
        return this;
    }

    Router!Derived delt(HandlerFn!Derived handler) {
        m_ctx[m_router_index].add_handler_in_place(Method.DELETE, handler);
        return this;
    }

    Router!Derived patch(HandlerFn!Derived handler) {
        m_ctx[m_router_index].add_handler_in_place(Method.PATCH, handler);
        return this;
    }

    Router!Derived add_router(Router!Derived sub) {
        m_ctx[sub.get_router_index()].update_base_router(m_router_number);
        m_ctx.decrement_base_router_children();
        m_ctx[m_router_index].increment_child_routes();
        return this;
    }

    Router!Derived add_route(Route!Derived sub) {
        sub.update_base_router(m_router_number);
        m_ctx.add_route(sub);
        m_ctx[m_router_index].increment_child_routes();
        return this;
    }

    size_t get_router_number() const { return m_router_number; }
    size_t get_router_index()  const { return m_router_index; }

  private:
    RouterContext!Derived m_ctx;
    size_t m_router_number;
    size_t m_router_index;
}

class RouteBuilder(Derived) {
  public:
    this() {
        m_port = 0;
        m_root_route     = (make!(Route!Derived)("/")).set_base_router(size_t.max);
        m_fallback_route = (make!(Route!Derived)("*")).set_base_router(size_t.max);
    }

    ~this() {
        dispose(m_root_route);
        dispose(m_fallback_route);
    }

    RouteBuilder!Derived address(const(char)[] addr) {
        m_address = addr;
        return this;
    }

    RouteBuilder!Derived port(int p) {
        m_port = cast(ubyte)p;
        return this;
    }

    RouteBuilder!Derived name(const(char)[] n) {
        m_name = n;
        return this;
    }

    // TODO: figue out a way to make this function consteval
    RouteHandler!Derived build(RouterContext!Derived ctx) {
        m_root_route.update_child_routes(ctx.get_base_router_children());
        ctx[0] = m_root_route;
        ctx[1] = m_fallback_route;
        return ctx.build();
    }

  private:
    const(char)[] m_address;
    ubyte         m_port;
    const(char)[] m_name;
    Route!Derived m_root_route;
    Route!Derived m_fallback_route;
}
