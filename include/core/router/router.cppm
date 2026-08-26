export module core_router:router;

import std;
import interfaces;
import core_otel;
import core_events;
import :utils;
import :handler;
import :consts;
import :middleware;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::router {

class RouterNode
{
public:
    /// @brief Default-constructs a zeroed-out node — no path, no offsets, `EdgeKind::PATH`.
    RouterNode() = default;

    /**
     * @brief Constructs a fully-populated trie node.
     * @param kind the edge kind this node matches as (`Path`, `Param`, or `Wild`).
     * @param path the literal path segment (only meaningful for `Path`-kind nodes).
     * @param children_offset offset into the node table where this node's children start.
     * @param children_length how many children this node has.
     * @param middleware_offset offset into the middleware pool for this node's middleware
     * slice.
     * @param middleware_size how many middlewares are attached to this node.
     * @param handler_offset offset into the handler pool for this node's handlers.
     * @param handler_mask the packed method-to-slot mask for this node's handlers.
     */
    RouterNode(
        EdgeKind kind,
        std::string_view path,
        std::uint16_t children_offset,
        std::uint8_t children_length,
        std::uint16_t middleware_offset,
        std::uint8_t middleware_size,
        std::uint16_t handler_offset,
        std::size_t handler_mask
    ) :
        m_kind{kind},
        m_path{path},
        m_children_offset{children_offset},
        m_children_length{children_length},
        m_middleware_offset{middleware_offset},
        m_middleware_length{middleware_size},
        m_handler_offset{handler_offset},
        m_handler_mask{handler_mask}
    {
    }

    /**
     * @brief Finds the child of this node matching path segment `seg`, hash-probing among
     * siblings when there's more than one child. The real workhorse of route matching.
     * @param seg the path segment to match against this node's children.
     * @param table the full flat node table children are stored in.
     * @param current_index in/out — on entry, this node's own index (used as the search
     * origin); on return, the matched child's index in `table` if found.
     * @note Match precedence, in order: an exact `Path` literal match wins immediately; a
     * `Param` child matches immediately too (first param found, no backtracking against later
     * siblings); a `Wild` child is only remembered and returned as a last resort if nothing
     * else in the probe matched, no cap. Reorder this and you silently change which route wins
     * on overlapping paths.
     * @warning `current_index` gets `+= m_children_offset` applied unconditionally before the
     * search even starts, in the multi-child branch — so on a full miss (`std::nullopt` return)
     * `current_index` is still left mutated, not restored to its original value. Callers
     * checking `current_index` after a failed match need to know it's not a "no-op on failure"
     * out-param.
     * @return the matched child node, or `std::nullopt` if nothing in this node's children
     * matches `seg` (not even a wildcard).
     */
    // Not noexcept: the body calls std::println (formatting/IO may throw). Its sole caller,
    // RouteHandler::match() below, isn't noexcept either (it already throws std::runtime_error
    // on a routing miss), so dropping noexcept here doesn't tighten any contract.
    constexpr std::optional<RouterNode>
    find_child(std::string_view seg, std::span<RouterNode> table, std::uint8_t& current_index) const
    {
        // no children at all — nothing to probe, bail immediately
        if (m_children_length == 0) {
            return std::nullopt;
        }

        // shift into this node's slice of the flat table — note this mutates the caller's
        // out-param even if the probe below ends up finding nothing
        current_index += m_children_offset;

        std::optional<std::size_t> wild_index{std::nullopt};

        if (m_children_length > 1) {
            // multiple siblings — hash-probe starting from a bucket derived from the segment,
            // walking the whole sibling range so every child gets checked exactly once
            const std::uint32_t HASH = fnv1a(seg) % m_children_length;

            for (std::uint32_t i = 0; i < m_children_length; ++i) {
                std::uint8_t idx = current_index + ((HASH + i) % m_children_length);

                auto& node = table[idx]; // FIXME(clang-tidy): unchecked operator[], consider .at()
                std::println(
                    "Checking node at index {}: kind={}, literal={}", idx,
                    std::to_underlying(node.get_kind()), node.get_literal()
                );
                std::println(
                    "wild_index: {}",
                    wild_index.has_value() ? std::to_string(*wild_index) : "nullopt"
                );

                // precedence: exact literal match wins on the spot, no cap; a param child also
                // matches immediately (first one found, no backtracking); a wildcard is only
                // remembered as a fallback in case nothing better turns up in the probe
                switch (node.get_kind()) {
                    case EdgeKind::PATH:
                        if (node.get_literal() == seg) {
                            current_index = idx;
                            return std::make_optional(node);
                        }
                        break;
                    case EdgeKind::PARAM:
                        current_index = idx;
                        return std::make_optional(node);
                    case EdgeKind::WILD:
                        wild_index = std::make_optional(idx);
                        continue;
                }
            }
        } else {
            // single child — no hashing needed, just check the one node directly
            auto& node =
                table[current_index]; // FIXME(clang-tidy): unchecked operator[], consider .at()

            switch (node.get_kind()) {
                case EdgeKind::PATH:
                    if (node.get_literal() == seg) {
                        return std::make_optional(node);
                    }
                    break;
                case EdgeKind::PARAM:
                    return std::make_optional(node);
                case EdgeKind::WILD:
                    wild_index = std::make_optional(current_index);
            }
        }

        // nothing matched outright during the probe — fall back to the wildcard we saw, if any
        if (wild_index) {
            std::println("Returning wildcard node at index {}", *wild_index);
            current_index = *wild_index;
            return std::make_optional(
                table[*wild_index]
            ); // FIXME(clang-tidy): unchecked operator[], consider .at()
        }

        // truly nothing matched, not even a wildcard
        return std::nullopt;
    }

    /**
     * @brief Gets this node's children-offset field.
     * @return mutable reference to the children offset.
     */
    constexpr std::uint16_t& get_data_offset() noexcept
    {
        return m_children_offset;
    }

    /**
     * @brief Gets this node's child count.
     * @return mutable reference to the children length.
     */
    constexpr std::uint8_t& get_children_length() noexcept
    {
        return m_children_length;
    }

    /**
     * @brief Gets this node's middleware-pool offset.
     * @return mutable reference to the middleware offset.
     */
    constexpr std::uint16_t& get_middleware_offset() noexcept
    {
        return m_middleware_offset;
    }

    /**
     * @brief Gets this node's middleware count.
     * @return mutable reference to the middleware length.
     */
    constexpr std::uint8_t& get_middleware_length() noexcept
    {
        return m_middleware_length;
    }

    /**
     * @brief Gets this node's handler-pool offset.
     * @return mutable reference to the handler offset.
     */
    constexpr std::uint16_t& get_handler_offset() noexcept
    {
        return m_handler_offset;
    }

    /**
     * @brief Gets this node's packed handler mask.
     * @warning Returns a mutable reference despite the "get_" name — bet you could mutate route
     * dispatch state through what looks like a read-only accessor. Handle with care.
     * @return mutable reference to the handler mask.
     */
    constexpr std::size_t& get_handler_mask() noexcept
    {
        return m_handler_mask;
    }

    /**
     * @brief Gets this node's edge kind (`Path`, `Param`, or `Wild`).
     * @return mutable reference to the edge kind.
     */
    constexpr EdgeKind& get_kind() noexcept
    {
        return m_kind;
    }

    /**
     * @brief Gets this node's literal path segment.
     * @return the literal path view (empty for `Param`/`Wild` nodes).
     */
    constexpr std::string_view get_literal() noexcept
    {
        return m_path;
    }

private:
    EdgeKind m_kind{};
    std::string m_path;
    std::uint16_t m_children_offset{};
    std::uint8_t m_children_length{};
    std::uint16_t m_middleware_offset{};
    std::uint8_t m_middleware_length{};
    std::uint16_t m_handler_offset{};
    std::size_t m_handler_mask{};
};

template<
    std::size_t RouterSize = 256,
    std::size_t HandlerSize = 64,
    std::size_t MiddlewareSize = 10>
class RouteHandler
{
public:
    /**
     * @brief Sets up an empty, ready-to-populate route table.
     */
    constexpr explicit RouteHandler() = default;

    /// @brief Defaulted move constructor — steals the table/pool state, no copies.
    constexpr RouteHandler(RouteHandler&&) noexcept = default;
    /// @brief Defaulted move assignment — steals the table/pool state, no copies.
    constexpr RouteHandler& operator=(RouteHandler&&) noexcept = default;

    /// @brief Deleted — route tables are built once and moved out, not duplicated.
    RouteHandler(const RouteHandler&) = delete;
    /// @brief Deleted — route tables are built once and moved out, not duplicated.
    RouteHandler& operator=(const RouteHandler&) = delete;

    /// @brief Defaulted destructor.
    ~RouteHandler() = default;

    /**
     * @brief Dispatches an incoming request: walks the segment trie from the root, running each
     * matched node's middleware along the way, then invokes the handler for `method` on the
     * final node if one's registered.
     * @param method the HTTP method of the incoming request.
     * @param path the request path to match against the trie.
     * @param req the request object, forwarded to middleware and the eventual handler.
     * @param res the response object, forwarded to middleware and the eventual handler.
     * @warning Middleware for every matched node along the walk runs *before* the final
     * method-vs-handler check — so if the path matches but the method doesn't have a handler,
     * you've already run all that middleware before the "Wrong method for route" throw fires.
     * No rollback, no skip. That's a real ordering footgun if any middleware has side effects
     * (auth headers, logging, whatever) — it fires even on requests that ultimately 404/405
     * out. Straight cooked if you assumed middleware only runs on a clean match. Flag it loud:
     * this isn't a "maybe," it's baked into the control flow as written.
     * @throws std::runtime_error with "Wrong method for route" if the path fully resolves to a
     * node but that node has no handler for `method`.
     * @throws std::runtime_error with "Route not found" if the path doesn't fully resolve to a
     * node (segments run out early, or a segment has no matching child).
     */
    constexpr void match(
        interfaces::io::types::Method method,
        std::string_view path,
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        const std::function<void()>& send
    )
    {
        // Global middleware runs first, on every request, before the trie is even walked. Each
        // gets a `next` that just flags "keep going"; a middleware continues by calling it and
        // a rejecting one short-circuits by replying + `send` and NOT calling it. If any one
        // doesn't continue, the whole trie walk + handler below is skipped and the response it
        // already sent stands.
        for (const auto& global_middleware: m_global_middleware) {
            bool proceed = false;
            global_middleware(
                req, res,
                [&proceed](
                    interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>
                ) noexcept {
                    proceed = true;
                },
                send
            );
            if (!proceed) {
                return;
            }
        }

        // break the request path into its `/`-separated segments up front
        auto segments = split_path(path);
        auto it = segments.begin();
        auto end = segments.end();

        {
            // walk the trie from the root, one path segment at a time
            RouterNode current =
                m_table[0]; // FIXME(clang-tidy): unchecked operator[], consider .at()
            std::uint8_t current_index = 0;

            for (; it != end; ++it) {
                auto node = current.find_child(*it, m_table, current_index);
                // segment has no matching child anywhere in the trie — walk stops short,
                // `it != end` below is what flags this as an incomplete match
                if (!node) {
                    break;
                }
                // every matched node along the way gets its middleware run, before we even
                // know whether the full path resolves or whether the final method has a
                // handler — that's intentional per the node's own contract, not a bug fix here
                if (node->get_middleware_length() > 0) {
                    m_middleware.execute(
                        req, res, send, node->get_middleware_offset(), node->get_middleware_length()
                    );
                }
                current = *node;
            }

            // path fully resolved to a node — now it's just a question of whether that node
            // has a handler for this specific method
            if (it == end) {
                // HANDLER_MASK unchanged means no handlers were ever registered on this node
                if (current.get_handler_mask() != HANDLER_MASK) {
                    const auto HANDLER_FN = m_handler.find(
                        current.get_handler_offset(), current.get_handler_mask(), method
                    );
                    // W — got a handler for this method, run it and we're done. Every dispatch
                    // through here automatically runs inside a SERVER span — same "just works,
                    // no per-route boilerplate" deal as OpenAPI metadata capture (ApiRoute/
                    // ApiRouter), just at dispatch time instead of registration time.
                    // Propagates an inbound `traceparent` header as the parent if present.
                    if (HANDLER_FN) {
                        auto method_str = interfaces::io::types::method_str(method);
                        auto span_name = std::format("{} {}", method_str, path);
                        auto incoming =
                            core::otel::parse_traceparent(req.find_header("traceparent"));
                        auto span =
                            incoming.has_value()
                                ? core::otel::start_span(
                                      span_name, interfaces::SpanKind::SERVER, *incoming
                                  )
                                : core::otel::start_span(span_name, interfaces::SpanKind::SERVER);
                        // Same choke point as the span above — one change here covers every
                        // dispatched request, mirroring src/worker_main.cc's poll_cycle metrics
                        // (task.completed/task.duration_ms) so the server has its own metrics
                        // signal instead of only ever coming from the worker.
                        const auto start_time = std::chrono::steady_clock::now();
                        const std::array<interfaces::Attribute, 2> metric_attrs{
                            interfaces::Attribute{"method", method_str},
                            interfaces::Attribute{"path", path},
                        };
                        try {
                            HANDLER_FN(req, res, send);
                            span.set_status(interfaces::SpanStatus::OK, "");
                            core::otel::counter_add("http.server.requests", 1.0, metric_attrs);
                            core::events::publish(
                                "router.request.completed",
                                {{"method", std::string{method_str}}, {"path", std::string{path}}}
                            );
                        } catch (const std::exception& e) {
                            span.set_status(interfaces::SpanStatus::ERROR, e.what());
                            const std::array<interfaces::Attribute, 3> error_attrs{
                                interfaces::Attribute{"method", method_str},
                                interfaces::Attribute{"path", path},
                                interfaces::Attribute{"result", "error"},
                            };
                            core::otel::counter_add("http.server.requests", 1.0, error_attrs);
                            const auto elapsed_ms =
                                std::chrono::duration<double, std::milli>(
                                    std::chrono::steady_clock::now() - start_time
                                )
                                    .count();
                            core::otel::histogram_record(
                                "http.server.request.duration_ms", elapsed_ms, metric_attrs
                            );
                            core::events::publish(
                                "router.request.failed", {{"method", std::string{method_str}},
                                                          {"path", std::string{path}},
                                                          {"error", e.what()}}
                            );
                            throw;
                        }
                        const auto elapsed_ms = std::chrono::duration<double, std::milli>(
                                                    std::chrono::steady_clock::now() - start_time
                        )
                                                    .count();
                        core::otel::histogram_record(
                            "http.server.request.duration_ms", elapsed_ms, metric_attrs
                        );
                        return;
                    }
                    // node exists but nothing registered for this method — 405-style L
                    core::events::publish(
                        "router.request.method_not_allowed",
                        {{"path", std::string{path}},
                         {"method", std::string{interfaces::io::types::method_str(method)}}}
                    );
                    throw std::runtime_error(
                        std::format("Wrong method for route: {}", std::to_underlying(method))
                    );
                }
            }
        }

        // either the segment walk broke early or the node had no handlers at all — 404
        core::events::publish("router.request.not_found", {{"path", std::string{path}}});
        throw std::runtime_error("Route not found");
    }

    /**
     * @brief Appends one node to the flat route table, copying `handler`'s and `middleware`'s
     * entries into the shared pools and recording their offsets on the new node.
     * @param kind the edge kind for the new node.
     * @param path the literal path segment (only meaningful for `Path`-kind nodes).
     * @param children_offset offset to this node's children in the table.
     * @param handler the handler table to copy entries from into the shared handler pool.
     * @param middleware the middleware chain to copy entries from into the shared middleware
     * pool.
     * @param children_size how many children this node has.
     * @note Takes `const Handler<8> &` specifically — a fixed `MaxHandlerSize` of 8,
     * independent of this class's own `HandlerSize` template parameter. Don't confuse the two
     * when reasoning about capacity; they're not the same knob, and mixing them up is an easy
     * L.
     */
    // Not noexcept: the body calls std::println (formatting/IO may throw). Its only caller,
    // RouteBuilder::build() in builder.cppm, isn't noexcept either (and itself calls
    // std::println too), so dropping noexcept here doesn't tighten any contract.
    constexpr void add_route(
        EdgeKind kind,
        std::string_view path,
        std::size_t children_offset,
        const Handler<8>& handler,
        const Middleware<MiddlewareSize>& middleware,
        std::uint8_t children_size
    )
    {
        // stamp out the node up front — offsets into the shared pools are wherever the pools'
        // current size already sits, since we're about to append right after this
        RouterNode node{
            kind,
            path,
            static_cast<std::uint16_t>(children_offset),
            children_size,
            m_middleware.get_size(),
            middleware.get_size(),
            m_handler.get_size(),
            handler.get_mask(),
        };

        std::println(
            "Adding route: {}, kind: {}, children offset: {}, children size: {}, "
            "middleware offset: {}, "
            "middleware size: {}, handler offset: {}, handler mask: {:016X}",
            path, std::to_underlying(kind), node.get_data_offset(), node.get_children_length(),
            node.get_middleware_offset(), node.get_middleware_length(), node.get_handler_offset(),
            node.get_handler_mask()
        );

        // copy this node's middleware entries into the shared pool, in order
        for (std::uint8_t i = 0; i < middleware.get_size(); ++i) {
            m_middleware.add_middleware(
                middleware.get_middlewares()[i]
            ); // FIXME(clang-tidy): unchecked operator[],
               // consider .at(); non-constant array index
        }

        // same deal for handlers — copy into the shared handler pool
        for (std::uint8_t i = 0; i < handler.get_size(); ++i) {
            m_handler.add_handler(
                handler.get_handler()[i]
            ); // FIXME(clang-tidy): unchecked operator[], consider
               // .at(); non-constant array index
        }

        // finally, drop the node itself into the flat table
        m_table[m_table_index++] = node; // FIXME(clang-tidy): unchecked operator[], consider
                                         // .at(); non-constant array index
    }

    /**
     * @brief Appends a global middleware — one that runs on every request, before the trie walk
     * and any per-node middleware, regardless of which route (if any) the path resolves to.
     * Carried over from the builder's own global list by RouterContext::build().
     * @param middleware the global middleware to append.
     */
    constexpr void add_global_middleware(interfaces::MiddlewareFn middleware)
    {
        m_global_middleware.add_middleware(middleware);
    }

private:
    std::array<RouterNode, RouterSize> m_table{};
    HandlerPool<HandlerSize> m_handler{};
    Middleware<MiddlewareSize> m_middleware{};
    // Runs on every request before the trie walk — separate from m_middleware, which is the
    // per-node offset pool the routes carve slices out of.
    Middleware<MiddlewareSize> m_global_middleware{};
    std::uint8_t m_table_index{0};
};

} // namespace core::router

// RouteHandler::match() needs a live interfaces::io::IRequest/IResponse pair (the only concrete
// implementation lives in io::layer::http2, which needs a real socket/session) — only RouterNode's
// own segment-matching logic and RouteHandler's non-dispatch bookkeeping are covered below.
#ifdef CONGELADO_TEST
namespace core::router::tests {
using namespace boost::ut;

// static: internal linkage so this doesn't collide with the same-named test helper other
// core_router partition files (handler.cppm, builder.cppm) define in this same tests namespace.
static interfaces::MiddlewareFn noop_middleware()
{
    return [](interfaces::io::IRequest&, interfaces::io::IResponse&, interfaces::NextFn&&,
              std::function<void()>) noexcept {};
}

// static: mirrors handler.cppm's own noop_handler() helper — internal linkage keeps the two
// from colliding even though they share a name in this same tests namespace.
static interfaces::HandlerFn noop_handler()
{
    return [](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) {};
}

// Base IRequest::find_header() aborts unless overridden (see interfaces/io/request.cppm) —
// match()'s successful-dispatch branch calls it to look for an inbound traceparent header. The
// wraparound test below expects to hit the *unsuccessful* "Wrong method for route" branch
// instead (which never reaches find_header at all), but this override is here as a defensive
// backstop in case that expectation is ever wrong — better a stub value than an abort() taking
// down the whole shared test binary.
class SafeFakeRequest : public interfaces::io::IRequest
{
public:
    explicit SafeFakeRequest(std::uint32_t stream_id) :
        interfaces::io::IRequest(stream_id)
    {
    }

    [[nodiscard]] std::string_view find_header(
        std::variant<std::string_view, interfaces::io::types::Token>
    ) const noexcept override
    {
        return {};
    }
};

suite<"RouterNode"> router_node_suite = [] {
    "default-constructed node has no children and Path kind"_test = [] {
        RouterNode node;
        expect(node.get_kind() == EdgeKind::PATH);
        expect(node.get_children_length() == 0);
    };

    "populated constructor stores every field"_test = [] {
        RouterNode node{EdgeKind::PARAM, "id", 3, 2, 1, 4, 5, 0xFF};
        expect(node.get_kind() == EdgeKind::PARAM);
        expect(node.get_literal() == "id");
        expect(node.get_data_offset() == 3);
        expect(node.get_children_length() == 2);
        expect(node.get_middleware_offset() == 1);
        expect(node.get_middleware_length() == 4);
        expect(node.get_handler_offset() == 5);
        expect(node.get_handler_mask() == 0xFF);
    };

    "find_child returns nullopt when the node has no children"_test = [] {
        RouterNode node{EdgeKind::PATH, "root", 0, 0, 0, 0, 0, HANDLER_MASK};
        std::array<RouterNode, 1> table{};
        std::uint8_t index = 0;
        expect(node.find_child("anything", table, index) == std::nullopt);
    };

    "find_child (single child): exact literal match succeeds, mismatch fails"_test = [] {
        std::array<RouterNode, 1> table{
            RouterNode{EdgeKind::PATH, "users", 0, 0, 0, 0, 0, HANDLER_MASK}
        };
        RouterNode parent{EdgeKind::PATH, "root", 0, 1, 0, 0, 0, HANDLER_MASK};

        std::uint8_t index = 0;
        auto matched = parent.find_child("users", table, index);
        expect(matched.has_value());
        expect(matched->get_literal() == "users");

        index = 0;
        expect(parent.find_child("other", table, index) == std::nullopt);
    };

    "find_child (single child): a param child matches any segment"_test = [] {
        std::array<RouterNode, 1> table{
            RouterNode{EdgeKind::PARAM, "", 0, 0, 0, 0, 0, HANDLER_MASK}
        };
        RouterNode parent{EdgeKind::PATH, "root", 0, 1, 0, 0, 0, HANDLER_MASK};

        std::uint8_t index = 0;
        auto matched = parent.find_child("42", table, index);
        expect(matched.has_value());
        expect(matched->get_kind() == EdgeKind::PARAM);
    };

    "find_child (single child): a wildcard child matches as a fallback"_test = [] {
        std::array<RouterNode, 1> table{
            RouterNode{EdgeKind::WILD, "", 0, 0, 0, 0, 0, HANDLER_MASK}
        };
        RouterNode parent{EdgeKind::PATH, "root", 0, 1, 0, 0, 0, HANDLER_MASK};

        std::uint8_t index = 0;
        auto matched = parent.find_child("whatever", table, index);
        expect(matched.has_value());
        expect(matched->get_kind() == EdgeKind::WILD);
    };

    "find_child (multiple children): exact literal wins over a sibling wildcard"_test = [] {
        std::array<RouterNode, 2> table{
            RouterNode{EdgeKind::PATH, "users", 0, 0, 0, 0, 0, HANDLER_MASK},
            RouterNode{EdgeKind::WILD, "", 0, 0, 0, 0, 0, HANDLER_MASK},
        };
        RouterNode parent{EdgeKind::PATH, "root", 0, 2, 0, 0, 0, HANDLER_MASK};

        std::uint8_t index = 0;
        auto matched = parent.find_child("users", table, index);
        expect(matched.has_value());
        expect(matched->get_kind() == EdgeKind::PATH);
        expect(matched->get_literal() == "users");
    };

    "find_child (multiple children): no literal match falls back to the wildcard sibling"_test =
        [] {
            std::array<RouterNode, 2> table{
                RouterNode{EdgeKind::PATH, "users", 0, 0, 0, 0, 0, HANDLER_MASK},
                RouterNode{EdgeKind::WILD, "", 0, 0, 0, 0, 0, HANDLER_MASK},
            };
            RouterNode parent{EdgeKind::PATH, "root", 0, 2, 0, 0, 0, HANDLER_MASK};

            std::uint8_t index = 0;
            auto matched = parent.find_child("nope", table, index);
            expect(matched.has_value());
            expect(matched->get_kind() == EdgeKind::WILD);
        };
};

suite<"RouteHandler"> route_handler_suite = [] {
    "add_route stores a node without throwing, for an empty handler/middleware set"_test = [] {
        RouteHandler<> handler{};
        expect(not throws<std::runtime_error>([&] {
            handler.add_route(EdgeKind::PATH, "foo", 0, Handler<8>{}, Middleware<10>{}, 0);
        }));
    };

    "add_global_middleware doesn't throw"_test = [] {
        RouteHandler<> handler{};
        expect(not throws<std::runtime_error>([&] {
            handler.add_global_middleware(noop_middleware());
        }));
    };

    // add_route()'s m_table_index is a plain uint8_t counter with no capacity check anywhere —
    // it just wraps like any other uint8_t past 255. RouterSize defaults to 256 (see m_table's
    // declaration), i.e. exactly uint8_t's full range, so every write below stays perfectly
    // in-bounds (indices 0-255 in a 256-element std::array) — this is NOT a memory-safety bug,
    // just silent semantic corruption: the 257th add_route() call wraps the counter back to 0
    // and overwrites whatever was already stored at index 0, same as writing over any other
    // live slot.
    "add_route silently overwrites route 0 once more than 256 routes are registered"_test = [] {
        RouteHandler<> table{};

        // Route 0 gets a real GET-only handler — this becomes the node sitting at m_table[0],
        // which is exactly the slot the 257th call below is going to land back on top of.
        Handler<8> get_only;
        get_only.add_handler(interfaces::io::types::Method::GET, noop_handler());

        // Route 256 (the 257th distinct route registered) gets a POST-only handler instead, so
        // its arrival at index 0 is unmistakable from route 0's original GET-only registration.
        Handler<8> post_only;
        post_only.add_handler(interfaces::io::types::Method::POST, noop_handler());

        for (std::uint32_t i = 0; i < 257; ++i) {
            const auto PATH = std::format("/route{}", i);
            if (i == 0) {
                table.add_route(EdgeKind::PATH, PATH, 0, get_only, Middleware<10>{}, 0);
            } else if (i == 256) {
                // m_table_index has now wrapped from 255 -> 0 (256 prior calls incremented it
                // clean off the top of uint8_t's range) — this write silently clobbers route
                // 0's node instead of landing in a fresh slot.
                table.add_route(EdgeKind::PATH, PATH, 0, post_only, Middleware<10>{}, 0);
            } else {
                table.add_route(EdgeKind::PATH, PATH, 0, Handler<8>{}, Middleware<10>{}, 0);
            }
        }

        // An empty path splits into zero segments, so match() never calls find_child() at all —
        // `current` stays exactly m_table[0], untouched by the trie walk. Had m_table_index not
        // wrapped, index 0 would still be route 0's GET-only node and this lookup would resolve
        // straight to its handler. Instead index 0 now holds route 256's POST-only node, so
        // looking up GET on it lands on the "node exists but no handler for this method" branch
        // — observable proof that route 0's original registration was silently replaced.
        SafeFakeRequest req{0};
        interfaces::io::IResponse res{0};
        expect(throws<std::runtime_error>([&] {
            table.match(interfaces::io::types::Method::GET, "", req, res, [] {});
        }));
    };
};

} // namespace core::router::tests
#endif
