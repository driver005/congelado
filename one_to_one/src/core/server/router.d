module core.server.router;
@nogc nothrow:

import interfaces.request    : IRequest;
import interfaces.response   : IResponse;
import interfaces.interfaces : HandlerFn;
import core.server.types     : Method, EdgeKind;
import core.server.handler   : Handler, HandlerPool;
import core.server.consts    : HANDLER_MASK;
import core.server.middleware : Middleware;

// PORT-NOTE: split_path / fnv1a are module-level helpers (no namespace needed)

private auto split_path(const(char)[] path, void delegate(const(char)[]) @nogc nothrow fn) {
    // Iterate over '/' delimited segments, skip empty
    size_t start = 0;
    for (size_t i = 0; i <= path.length; ++i) {
        if (i == path.length || path[i] == '/') {
            if (i > start)
                fn(path[start .. i]);
            start = i + 1;
        }
    }
}

// constexpr uint fnv1a(std::string_view s) — djb2 variant
private uint fnv1a(const(char)[] s) {
    uint h = 5381u;
    foreach (c; s)
        h = h * 33u ^ cast(ubyte)c;
    return h;
}

class RouterNode {
  public:
    this() {}

    this(EdgeKind kind, const(char)[] path, ushort children_offset,
         ubyte children_length, ushort middleware_offset,
         ubyte middleware_size, ushort handler_offset, size_t handler_mask) {
        m_kind              = kind;
        m_path              = path;
        m_children_offset   = children_offset;
        m_children_length   = children_length;
        m_middleware_offset = middleware_offset;
        m_middleware_length = middleware_size;
        m_handler_offset    = handler_offset;
        m_handler_mask      = handler_mask;
    }

    // PORT-NOTE: std::optional<RouterNode> → bool + out param
    bool find_child(const(char)[] seg, RouterNode[] table,
                    ref ubyte current_index, out RouterNode found_node) const {
        if (m_children_length == 0)
            return false;

        current_index += m_children_offset;

        bool     has_wild  = false;
        uint     wild_index = 0;

        if (m_children_length > 1) {
            const uint hash = fnv1a(seg) % m_children_length;

            for (uint i = 0; i < m_children_length; ++i) {
                ubyte idx = cast(ubyte)(current_index + (hash + i) % m_children_length);
                auto  node = table[idx];
                import core.stdc.stdio : printf;
                printf("Checking node at index %u: kind=%u, literal=%.*s\n",
                       idx, node.m_kind, cast(int)node.m_path.length, node.m_path.ptr);
                printf("wild_index: %s\n", has_wild ? "set" : "nullopt");

                switch (node.m_kind) {
                case EdgeKind.Path:
                    if (node.m_path == seg) {
                        printf("Found matching path node at index %u: %.*s\n",
                               idx, cast(int)node.m_path.length, node.m_path.ptr);
                        found_node = node;
                        return true;
                    }
                    break;
                case EdgeKind.Param:
                    found_node = node;
                    return true;
                case EdgeKind.Wild:
                    has_wild   = true;
                    wild_index = idx;
                    continue;
                default:
                    break;
                }
            }
        } else {
            auto node = table[current_index];

            switch (node.m_kind) {
            case EdgeKind.Path:
                if (node.m_path == seg) {
                    found_node = node;
                    return true;
                }
                break;
            case EdgeKind.Param:
                found_node = node;
                return true;
            case EdgeKind.Wild:
                has_wild   = true;
                wild_index = current_index;
                break;
            default:
                break;
            }
        }

        if (has_wild) {
            import core.stdc.stdio : printf;
            printf("Returning wildcard node at index %u\n", wild_index);
            found_node = table[wild_index];
            return true;
        }

        return false;
    }

    ref ushort get_data_offset()       { return m_children_offset; }
    ref ubyte  get_children_length()   { return m_children_length; }
    ref ushort get_middleware_offset()  { return m_middleware_offset; }
    ref ubyte  get_middleware_length()  { return m_middleware_length; }
    ref ushort get_handler_offset()     { return m_handler_offset; }
    ref size_t get_handler_mask()       { return m_handler_mask; }
    ref EdgeKind get_kind()             { return m_kind; }
    const(char)[] get_literal()         { return m_path; }

  private:
    EdgeKind      m_kind;
    const(char)[] m_path;
    ushort m_children_offset;
    ubyte  m_children_length;
    ushort m_middleware_offset;
    ubyte  m_middleware_length;
    ushort m_handler_offset;
    size_t m_handler_mask;
}

class RouteHandler(Derived,
                   size_t RouterSize     = 256,
                   size_t HandlerSize    = 64,
                   size_t MiddlewareSize = 10) {
  public:
    this() { m_table_index = 0; }

    void match(Method method, const(char)[] path,
               ref IRequest!Derived req, ref IResponse!Derived res) {
        // Split path into segments and traverse
        auto current       = m_table[0];
        ubyte current_index = 0;
        bool  matched       = true;

        split_path(path, (const(char)[] seg) {
            import core.stdc.stdio : printf;
            printf("Current node index: %u, looking for segment: %.*s\n",
                   current_index, cast(int)seg.length, seg.ptr);
            RouterNode node_found;
            if (!current.find_child(seg, m_table[], current_index, node_found)) {
                matched = false;
                return;
            }
            if (node_found.get_middleware_length() > 0) {
                m_middleware.execute(req, res, node_found.get_middleware_offset(),
                                     node_found.get_middleware_length());
            }
            current = node_found;
        });

        import core.stdc.stdio : printf;
        printf("Edge kind: %u, literal: %.*s, children length: %u, middleware length: %u, "
               "handler offset: %u, handler mask: %016llX\n",
               cast(ubyte)current.get_kind(),
               cast(int)current.get_literal().length, current.get_literal().ptr,
               current.get_children_length(), current.get_middleware_length(),
               current.get_handler_offset(), cast(ulong)current.get_handler_mask());

        printf("Handler mask: %016llX, looking for method: %u\n",
               cast(ulong)current.get_handler_mask(), cast(ubyte)method);

        printf("Gert handler offset: %u, middleware offset: %u, handler mask: %016llX\n",
               current.get_handler_offset(), current.get_middleware_offset(),
               cast(ulong)current.get_handler_mask());

        if (matched) {
            if (current.get_handler_mask() != HANDLER_MASK) {
                const handler_fn = m_handler.find(current.get_handler_offset(),
                                                   current.get_handler_mask(), method);
                if (handler_fn !is null) {
                    handler_fn(req, res);
                    return;
                } else {
                    // PORT-NOTE: std::runtime_error → assert for @nogc
                    assert(false, "Wrong method for route");
                }
            }
        }

        assert(false, "Route not found");
    }

    void add_route(EdgeKind kind, const(char)[] path, size_t children_offset,
                   const(Handler!(Derived, 8)) handler,
                   const(Middleware!(Derived, MiddlewareSize)) middleware,
                   ubyte children_size) {
        import util.alloc : make;
        RouterNode node = make!RouterNode(
            kind,
            path,
            cast(ushort)children_offset,
            children_size,
            cast(ushort)m_middleware.get_size(),
            middleware.get_size(),
            cast(ushort)m_handler.get_size(),
            handler.get_mask(),
        );

        import core.stdc.stdio : printf;
        printf("Adding route: %.*s, kind: %u, children offset: %u, children size: %u, "
               "middleware offset: %u, middleware size: %u, handler offset: %u, handler mask: %016llX\n",
               cast(int)path.length, path.ptr, cast(ubyte)kind,
               node.get_data_offset(), node.get_children_length(),
               node.get_middleware_offset(), node.get_middleware_length(),
               node.get_handler_offset(), cast(ulong)node.get_handler_mask());

        for (ubyte i = 0; i < middleware.get_size(); ++i)
            m_middleware.add_middleware(middleware.get_middlewares()[i]);

        for (ubyte i = 0; i < handler.get_size(); ++i)
            m_handler.add_handler(handler.get_handler()[i]);

        m_table[m_table_index++] = node;
    }

  private:
    RouterNode[RouterSize]                     m_table;
    HandlerPool!(Derived, HandlerSize)         m_handler;
    Middleware!(Derived, MiddlewareSize)       m_middleware;
    ubyte m_table_index;
}
