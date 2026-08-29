module;
#include <cstddef>
export module core_router:builder;

import std;
import :utils;
import :router;

export namespace core::router {


template <std::size_t MaxHandlerSize = 8, std::size_t MaxMiddlewareSize = 10>
class Route;
template <std::size_t MaxMiddlewareSize = 10>
class Router;


template <std::size_t RouterSize = 256, std::size_t MaxHandlerSize = 8,
          std::size_t MaxMiddlewareSize = 10>
class RouterContext {
  public:
    /**
     * @brief Sets up an empty context — slots 0 and 1 are reserved for the root and fallback
     * routes, so `m_router_size` starts at 2, no cap.
     */
    constexpr RouterContext() = default;

    /**
     * @brief Appends a route to the flat route array and bumps the base-router child count if it
     * hangs straight off the root.
     * @param route the route to store, moved into the backing array.
     * @return the index the route landed at — that's your handle for future lookups via
     * `operator[]`.
     */
    constexpr std::size_t add_route(Route<MaxHandlerSize, MaxMiddlewareSize> route) {
        // claim the next free slot and stash the route there
        const std::size_t SLOT_INDEX = m_router_size++;
        const std::size_t BASE_ROUTER = route.get_base_router();
        m_routes[SLOT_INDEX] = std::move(route);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
        // base router (0) is the root — routes hanging straight off it get counted so the
        // root node's child count can be synced up later at build() time
        if (BASE_ROUTER == 0) {
            ++m_base_router_children;
        }
        return SLOT_INDEX;
    }

    /**
     * @brief Grabs a mutable reference to the route stored at `index`.
     * @param index position into the internal route array.
     * @return reference to the route at that slot.
     * @throws std::out_of_range if `index` is past the currently used range — no silent clamping,
     * you get bounced hard.
     */
    constexpr Route<MaxHandlerSize, MaxMiddlewareSize> &operator[](std::size_t index) {
        // hard guard — out-of-range reads bounce instead of silently clamping
        if (index >= m_router_size) {
            throw std::out_of_range("RouterContext index out of bounds");
        }
        return m_routes[index];  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
    }

    // TODO: figue out a way to make this function consteval
    /**
     * @brief Compiles every registered route into a flat, matchable `RouteHandler` trie. Groups
     * routes by their base router, hash-buckets each group by path so lookups are ~O(1), then
     * walks the resulting table to compute per-node child offsets before handing everything off
     * to `RouteHandler::add_route()`.
     * @note Ordering is load-bearing here, not vibes — wildcard (`"*"`) routes always sort last
     * within their group, and routes with the sentinel base router (`SIZE_MAX`) sort ahead of
     * everything else in the group. Mess with the sort predicate and you'll desync match
     * precedence at runtime with zero compile-time warning.
     * @warning Consumes `m_routes` via `insert_route()`'s `is_build()` guard — call this twice and
     * the second pass just skips every already-built route, silently returning a handler missing
     * whatever you expected to be freshly rebuilt. No cap, that's an easy L if you don't know it's
     * one-shot.
     * @return the fully built `RouteHandler`, moved out.
     */
    RouteHandler<> build() {
        std::vector<std::vector<RouterBuildingHelper>> table;
        std::size_t idx{0};
        const auto SENTINEL = std::numeric_limits<std::size_t>::max();

        // only the live prefix of m_routes is real data — everything past m_router_size is
        // unused backing storage
        auto active_routes = std::span(m_routes.data(), m_router_size);
        auto group_start = active_routes.begin();

        // walk the routes in runs that share the same base router, sorting each run in place
        // so match precedence at runtime falls out of array order alone
        while (group_start != active_routes.end()) {
            // find where this base-router group ends and the next one starts
            auto group_end =
                std::ranges::find_if(group_start, active_routes.end(), [&](const auto &current_route) {
                    return current_route.get_base_router() != group_start->get_base_router();
                });

            std::size_t group_size = std::distance(group_start, group_end);
            // ordering rules, checked in priority order: wildcard ("*") always sorts last in
            // its group; sentinel-parented routes (root/fallback) sort ahead of everything
            // else; anything left over falls back to a hash-bucket ordering — mess with this
            // predicate and match precedence desyncs at runtime, no cap
            std::ranges::sort(group_start, group_end, [&](const auto &left, const auto &right) {
                if (left.get_path() == "*") {
                    return false;
                }
                if (right.get_path() == "*") {
                    return true;
                }
                if (left.get_base_router() == SENTINEL && right.get_base_router() != SENTINEL) {
                    return true;
                }
                if (right.get_base_router() == SENTINEL && left.get_base_router() != SENTINEL) {
                    return false;
                }
                return fnv1a(left.get_path()) % group_size < fnv1a(right.get_path()) % group_size;
            });


            group_start = group_end;
        }

        RouteHandler<> handler{};

        // fold every stored route into the intermediate table, grouped by parent router —
        // insert_route() itself skips anything already flagged built
        while (idx < m_router_size) {
            insert_route(m_routes[idx], table);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
            ++idx;
        }

        // walk the table row by row (each row = one router's children) to compute each
        // node's forward offset to its own children before handing it to the real handler
        for (const auto &row : table) {
            std::size_t cumulative_children = 0;
            for (const auto &[index, node] : row | std::views::enumerate) {
                std::size_t offset{0};

                // only nodes with children need a real offset — leaves stay at 0
                if (node.m_children > 0) {
                    offset = row.size() - index + cumulative_children;
                }

                cumulative_children += node.m_children;

                handler.add_route(node.m_kind, node.m_literal, offset, node.m_handlers,
                                  node.m_middlewares, node.m_children);
            }
        }

        std::println("Finished building");

        return std::move(handler);
    }

    /**
     * @brief Hands out the next free router number and advances the counter.
     * @warning Reads like a getter but it's not — every call mutates a process-wide counter via
     * fetch_add. Call it twice expecting the same value back and you're cooked, you'll get two
     * different numbers.
     * @note Drawn from a `static inline` counter shared across every `RouterContext` instance in
     * the process (for this template instantiation), not a per-instance one — `utils::openapi::
     * Registry` is itself a process-wide singleton that more than one `RouterContext` can feed
     * (e.g. engine's live router plus a throwaway one built purely to register another module's
     * routes for documentation, see `engine.cc`'s `on_load`), and Generator reconstructs full
     * paths by walking `router_number`/`base_router` across that ENTIRE Registry, not scoped to
     * whichever `RouterContext` produced a given entry. A per-instance counter that always
     * restarts at 1 collides across instances — two unrelated nodes end up sharing the same
     * `router_number`, and the path-reconstruction walk picks up the wrong parent chain for
     * whichever entry it finds first. A shared counter makes every number unique process-wide,
     * matching what Registry's own flat, cross-instance walk actually needs.
     * @return the router number to claim for this call.
     */
    std::size_t get_highest_router_number() noexcept { return s_global_router_number.fetch_add(1, std::memory_order_relaxed); }
    /**
     * @brief Gets how many routes (including the reserved root/fallback slots) are currently
     * stored.
     * @return the live route count.
     */
    [[nodiscard]] constexpr std::size_t get_router_size() const noexcept { return m_router_size; }
    /**
     * @brief Gets how many routes hang directly off the base (root) router.
     * @return the base-router child count.
     */
    [[nodiscard]] constexpr std::size_t get_base_router_children() const noexcept {
        return m_base_router_children;
    }

    /**
     * @brief Decrements the base-router child count by one, guarded against underflow.
     * @note No-ops at zero instead of wrapping — bet, that's the safe call for an unsigned
     * counter.
     */
    constexpr void decrement_base_router_children() noexcept {
        // stay clamped at zero instead of wrapping — unsigned underflow would be way worse
        if (m_base_router_children > 0) {
            --m_base_router_children;
        }
    }


  private:
    class RouterBuildingHelper {
      public:
        EdgeKind m_kind{EdgeKind::PATH};
        std::string m_literal;
        Handler<MaxHandlerSize> m_handlers{};
        Middleware<MaxMiddlewareSize> m_middlewares{};
        std::size_t m_children{0};
        std::size_t m_router_number{0};
    };

    /**
     * @brief Classifies a raw path segment into its edge kind and, for literal paths, the literal
     * text to match on.
     * @param path the router-level path to classify — `"*"` for wildcard, a leading `:` for a
     * param, anything else is a literal path segment.
     * @return a pair of the detected `EdgeKind` and the literal view (empty for `Wild`/`Param`).
     * @throws std::runtime_error if `path` is empty.
     */
    static constexpr std::pair<EdgeKind, std::string_view>
    determine_node_kind(std::string_view path) {
        // empty path has nothing to classify — bail loud
        if (path.empty()) {
            throw std::runtime_error("Path cannot be empty");
        }

        // exact "*" segment is the wildcard edge, no literal text attached
        if (path == "*") {
            return std::make_pair(EdgeKind::WILD, std::string_view{});
        }

        // leading ":" marks a param segment, also no literal text needed
        if (path.front() == ':') {
            return std::make_pair(EdgeKind::PARAM, std::string_view{});
        }

        // anything else is a plain literal path segment
        return std::make_pair(EdgeKind::PATH, path);
    }

    /**
     * @brief Finds the row in `table` that a route with the given base router should land in.
     * @param table the in-progress table of rows, indexed by parent router position.
     * @param route the route being placed — its base router selects the target row.
     * @return `0` for a sentinel-parented route (root/fallback), otherwise one past the parent
     * router's row.
     * @throws std::runtime_error if the parent router referenced by `route.get_base_router()`
     * isn't found in `table` yet.
     */
    static std::size_t
    resolve_target_row(const std::vector<std::vector<RouterBuildingHelper>> &table,
                       const Route<MaxHandlerSize, MaxMiddlewareSize> &route) {
        // sentinel base router (root/fallback) always lands in row 0
        if (route.get_base_router() == std::numeric_limits<std::size_t>::max()) {
            return 0;
        }

        // find the parent router's row so this node can be placed right after it
        auto row_iterator = std::ranges::find_if(table, [&](const auto &row) {
            return std::ranges::any_of(row, [&](const auto &node) {
                return node.m_router_number == route.get_base_router();
            });
        });

        // parent has to already be in the table by the time a child shows up here
        if (row_iterator == table.end()) {
            throw std::runtime_error(
                std::format("Parent router with number {} not found for route {}",
                            route.get_base_router(), route.get_path()));
        }

        return static_cast<std::size_t>(std::distance(table.begin(), row_iterator)) + 1;
    }

    /**
     * @brief Places one route into the intermediate build `table`, either as a router (branch)
     * node under its parent's slot or as a leaf route hanging off whichever router it belongs to.
     * @param route the route to place — router nodes are identified by having child routes
     * (`get_child_routes() != 0`), everything else is treated as a leaf.
     * @param table the in-progress table of rows, indexed by parent router position; grown with
     * `resize()` as needed.
     * @note Branch nodes get tagged with their own `router_number` so children can find their
     * parent's row later; leaf routes always get tagged `0` since nothing needs to parent off a
     * leaf.
     * @warning Marks `route.is_build()` as a side channel — `build()` calls this once per stored
     * route, and a route already flagged built is silently skipped, no error. Skip re-running
     * `build()` on the same context or this quietly drops routes.
     * @throws std::runtime_error if the router path is empty, parameterized, or wildcard (routers
     * can't be any of those — only leaf routes can), or if the parent router referenced by
     * `route.get_base_router()` isn't found in `table` yet.
     */
    constexpr void insert_route(const Route<MaxHandlerSize, MaxMiddlewareSize> &route,
                                std::vector<std::vector<RouterBuildingHelper>> &table) {  // FIXME(clang-tidy): readability-function-cognitive-complexity — needs decomposition
        // already folded into the table by an earlier build() pass — skip, don't double-add,
        // lowkey that's what keeps a second build() call from silently dropping routes
        if (route.is_build()) {
            return;
        }

        auto [kind, literal] = determine_node_kind(route.get_path());
        std::size_t router_number = 0;

        // child count > 0 means this route is actually a router/branch node — it gets its
        // own router_number tag so its future children can find it; a leaf route tags 0
        // instead since nothing parents off a leaf
        if (route.get_child_routes() != 0) {
            // routers can only be plain literal segments — param/wildcard routers make no sense
            switch (kind) {
            case EdgeKind::PATH:
                break;
            case EdgeKind::PARAM:
                throw std::runtime_error("Router path cannot be parameterized");
            case EdgeKind::WILD:
                throw std::runtime_error("Router path cannot be wildcard");
            }

            router_number = route.get_router_number();
        }

        const std::size_t NEW_IDX = resolve_target_row(table, route);

        // grow the table if this row doesn't exist yet
        if (NEW_IDX >= table.size()) {
            table.resize(NEW_IDX + 1);
        }

        // stash the node in its row, tagged with its own router_number (0 for a leaf)
        table[NEW_IDX].push_back(RouterBuildingHelper{  // FIXME(clang-tidy): unchecked operator[], consider .at()
            kind,
            std::string{literal},
            route.get_handlers(),
            route.get_middlewares(),
            route.get_child_routes(),
            router_number,
        });

        route.is_build();  // FIXME(clang-tidy): clang-diagnostic-unused-result — likely meant to call the mutating set_build(), but route is a const reference here, so fixing it properly needs a signature change; not guessing
    }

    // Middleware<MaxMiddlewareSize> m_middlewares;
    std::array<Route<MaxHandlerSize, MaxMiddlewareSize>, RouterSize> m_routes{};
    std::size_t m_router_size{2};
    // Shared across every RouterContext<RouterSize, MaxHandlerSize, MaxMiddlewareSize> instance
    // in the process — see get_highest_router_number()'s own doc comment for why this can't be
    // a per-instance counter. Starts at 1, same as the old per-instance default.
    static inline std::atomic<std::size_t> s_global_router_number{1};
    std::size_t m_base_router_children{0};
};

template <std::size_t MaxHandlerSize = 8, std::size_t MaxMiddlewareSize = 10>
class Route {
  public:
    /**
     * @brief Default-constructs an empty, unbuilt route with no path, no handlers, no
     * middleware — a blank slate.
     */
    constexpr Route()
        : m_is_build{false}, m_router_number{0}, m_base_router{0}, m_child_routes{0}, m_handler{},
          m_local_middleware{} {}

    /**
     * @brief Constructs a route for `path`, stripping a single leading `/` off into `m_path`.
     * @param path the route path. Must be `"*"` (wildcard) or contain a `/` somewhere in it.
     * @warning The thrown message says "must start '/'" but the actual check is
     * `path.contains('/')` — it only verifies a slash shows up *anywhere*, not at the front. A
     * path like `abc/def` sails through validation despite not starting with `/`. Lowkey
     * misleading error text, don't trust the message over the code.
     * @throws std::runtime_error if `path` isn't `"*"` and contains no `/` at all.
     */
    constexpr Route<>(std::string_view path)
        // strip exactly one leading '/' when it's there, otherwise keep the path as-is
        : m_path{path.size() > 1 && path.starts_with('/') ? path.substr(1) : path},
          m_is_build{false}, m_router_number{0}, m_base_router{0}, m_child_routes{0}, m_handler{},
          m_local_middleware{} {
        // wildcard is always fine — everything else just needs a '/' somewhere in it
        if (path != "*") {
            if (!path.contains('/')) {
                throw std::runtime_error("Route path must start '/'");
            }
        }
    }

    /**
     * @brief Constructs a route for `path` and registers each of `middlewares` against it in
     * order.
     * @tparam Args pack of middleware-convertible callables — each must convert to
     * `interfaces::MiddlewareFn`.
     * @param path the route path, validated the same way as the single-arg constructor.
     * @param middlewares the middleware functions to attach, added in the order given — that
     * order is also the execution order at match time.
     * @warning Same footgun as the single-arg constructor: the error message claims the path
     * "must start '/'" but only a `contains('/')` check actually runs.
     * @throws std::runtime_error if `path` isn't `"*"` and contains no `/` at all.
     */
    template <typename... Args>
        requires(std::is_convertible_v<Args, interfaces::MiddlewareFn> && ...)
    constexpr Route(std::string_view path, Args &&...middlewares)
        : m_path{path.size() > 1 && path.starts_with('/') ? path.substr(1) : path},
          m_is_build{false}, m_router_number{0}, m_base_router{0}, m_child_routes{0}, m_handler{} {
        // same path validation as the single-arg constructor
        if (path != "*") {
            if (!path.contains('/')) {
                throw std::runtime_error("Route path must start '/'");
            }
        }
        // register each middleware in the pack, in the order they were passed — that order
        // becomes execution order at match time
        (m_local_middleware.add_middleware(std::forward<Args>(middlewares)), ...);
    }

    /**
     * @brief Builder-style: appends one middleware to this route's local chain.
     * @param mw the middleware function to append.
     * @note Rvalue-qualified (`&&`) — chain it off a temporary/moved route, not a named lvalue.
     * @return `*this`, moved out, ready for the next chained call.
     */
    constexpr Route<> use(interfaces::MiddlewareFn middleware) && {
        m_local_middleware.add_middleware(std::move(middleware));
        return std::move(*this);
    }

    /**
     * @brief Builder-style: registers `handler` for `GET` on this route.
     * @param handler the handler function to run for `GET` requests.
     * @return `*this`, moved out.
     */
    Route<> get(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::GET, std::move(handler));
    }


    /**
     * @brief Builder-style: registers `handler` for `POST` on this route.
     * @param handler the handler function to run for `POST` requests.
     * @return `*this`, moved out.
     */
    Route<> post(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::POST, std::move(handler));
    }

    /**
     * @brief Builder-style: registers `handler` for `PUT` on this route.
     * @param handler the handler function to run for `PUT` requests.
     * @return `*this`, moved out.
     */
    Route<> put(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::PUT, std::move(handler));
    }

    /**
     * @brief Builder-style: registers `handler` for `PATCH` on this route.
     * @param handler the handler function to run for `PATCH` requests.
     * @return `*this`, moved out.
     */
    constexpr Route<> patch(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::PATCH, std::move(handler));
    }

    /**
     * @brief Builder-style: registers `handler` for `DELETE` on this route.
     * @param handler the handler function to run for `DELETE` requests.
     * @note Named `delt` and not `delete` for the obvious reason — `delete` is a keyword, so this
     * is the workaround. No cap, kind of a weird one to type at first.
     * @return `*this`, moved out.
     */
    constexpr Route<> delt(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::DELETE, std::move(handler));
    }

    /**
     * @brief Builder-style: registers `handler` for `HEAD` on this route.
     * @param handler the handler function to run for `HEAD` requests.
     * @return `*this`, moved out.
     */
    constexpr Route<> head(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::HEAD, std::move(handler));
    }

    /**
     * @brief Builder-style: registers `handler` for `OPTIONS` on this route.
     * @param handler the handler function to run for `OPTIONS` requests.
     * @return `*this`, moved out.
     */
    constexpr Route<> options(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::OPTIONS, std::move(handler));
    }

    /**
     * @brief Registers `handler` for `method` on this route, the shared path all the verb-named
     * builder methods (`get()`, `post()`, ...) funnel through.
     * @param method the HTTP method to bind the handler to.
     * @param handler the handler function to run when `method` matches.
     * @warning At compile time (`if consteval`), a duplicate handler for `method` is silently
     * swallowed — the `static_assert` that should catch it is commented out, so it just returns
     * `*this` unchanged with zero diagnostic. At runtime it throws instead. That asymmetry is a
     * real footgun: a dupe route that would blow up at runtime can slide right through a constexpr
     * build. Straight up cooked if you're relying on compile-time checks to catch this.
     * @throws std::runtime_error at runtime if a handler for `method` is already registered on
     * this path.
     * @return `*this`, moved out.
     */
    constexpr Route<> add_handler(interfaces::io::types::Method method,
                                  interfaces::HandlerFn handler) {
        // no existing handler for this method — register it and we're done
        auto handler_fnc = m_handler.find(method);
        if (!handler_fnc) {
            m_handler.add_handler(method, std::move(handler));
            return std::move(*this);
        }


        // duplicate handler found — compile-time and runtime disagree on what happens here.
        // at compile time the static_assert that should catch this is commented out, so it
        // just quietly returns unchanged; only the runtime path actually throws
        if consteval {
            // static_assert(false, "Duplicate handler found for method please check your routes");
            return std::move(*this);
        } else {
            throw std::runtime_error(std::format("Handler for method {} already exists on path {}",
                                                 std::to_underlying(method), m_path));
        }
    }

    /**
     * @brief Registers `handler` for `method` on this route in place, without the builder-style
     * move/return.
     * @param method the HTTP method to bind the handler to.
     * @param handler the handler function to run when `method` matches.
     * @throws std::runtime_error unconditionally (both constexpr and runtime contexts) if a
     * handler for `method` already exists on this path — no consteval carve-out here, unlike
     * `add_handler()`.
     */
    void add_handler_in_place(interfaces::io::types::Method method, interfaces::HandlerFn handler) {
        // no consteval carve-out here, unlike add_handler() — dupes always throw
        if (m_handler.find(method)) {
            throw std::runtime_error(std::format("Handler for method {} already exists on path {}",
                                                 std::to_underlying(method), m_path));
        }
        m_handler.add_handler(method, std::move(handler));
    }

    /**
     * @brief Builder-style: assigns which router this route sits under.
     * @param router_number the parent router's number.
     * @note Rvalue-qualified (`&&`).
     * @return `*this`, moved out.
     */
    constexpr Route<> set_base_router(std::size_t router_number) && {
        m_base_router = router_number;
        return std::move(*this);
    }

    /**
     * @brief Builder-style: assigns this route's own router number.
     * @param router_number the number to identify this route/router by.
     * @note Rvalue-qualified (`&&`).
     * @return `*this`, moved out.
     */
    constexpr Route<> set_router_number(std::size_t router_number) && {
        m_router_number = router_number;
        return std::move(*this);
    }

    /**
     * @brief Appends one middleware to this route's local chain, in place (no move-return).
     * @param middleware the middleware function to append.
     */
    constexpr void add_middleware(interfaces::MiddlewareFn middleware) {
        m_local_middleware.add_middleware(std::move(middleware));
    }

    /**
     * @brief Gets this route's path (post leading-slash-stripping).
     * @return the stored path view.
     */
    [[nodiscard]] constexpr std::string_view get_path() const noexcept { return m_path; }
    /**
     * @brief Checks whether this route has already been folded into a built `RouteHandler`.
     * @return `true` if `set_build()` has been called on this route.
     */
    [[nodiscard]] constexpr bool is_build() const noexcept { return m_is_build; }
    /**
     * @brief Gets the number of child routes/routers hanging off this one.
     * @return the child route count.
     */
    [[nodiscard]] constexpr std::size_t get_child_routes() const noexcept { return m_child_routes; }
    /**
     * @brief Gets the router number this route is parented under.
     * @return the base router number.
     */
    [[nodiscard]] constexpr std::size_t get_base_router() const noexcept { return m_base_router; }
    /**
     * @brief Gets this route's own router number (only meaningful for router/branch nodes).
     * @return the router number.
     */
    [[nodiscard]] constexpr std::size_t get_router_number() const noexcept {
        return m_router_number;
    }
    /**
     * @brief Gets the handler table registered on this route.
     * @return reference to the per-method handler table.
     */
    [[nodiscard]] constexpr const Handler<MaxHandlerSize> &get_handlers() const noexcept {
        return m_handler;
    }
    /**
     * @brief Gets this route's local middleware chain.
     * @return reference to the middleware chain.
     */
    [[nodiscard]] constexpr const Middleware<MaxMiddlewareSize> &get_middlewares() const noexcept {
        return m_local_middleware;
    }

    /**
     * @brief Overwrites which router this route is parented under.
     * @param number the new base router number.
     */
    constexpr void update_base_router(const std::size_t &number) noexcept {
        m_base_router = number;
    }
    /**
     * @brief Overwrites this route's child-route count.
     * @param number the new child-route count.
     * @warning `m_child_routes` is a `std::uint8_t` but this takes a `std::size_t` — feed it
     * anything above 255 and it truncates on assignment, silently, no warning, no throw. Straight
     * L waiting to happen on a big fan-out router.
     */
    constexpr void update_child_routes(const std::size_t &number) noexcept {
        m_child_routes = number;
    }
    /**
     * @brief Overwrites this route's own router number.
     * @param number the new router number.
     */
    constexpr void update_router_number(const std::size_t &number) noexcept {
        m_router_number = number;
    }
    /**
     * @brief Marks this route as folded into a built `RouteHandler`, so future `build()` passes
     * skip it.
     */
    constexpr void set_build() noexcept { m_is_build = true; }
    /**
     * @brief Bumps the child-route count by one.
     * @note Same narrow-type territory as `update_child_routes()` — `m_child_routes` is a
     * `std::uint8_t`, so this wraps at 256 rather than throwing.
     */
    constexpr void increment_child_routes() noexcept { ++m_child_routes; }

  private:
    std::string_view m_path;
    bool m_is_build;
    std::size_t m_router_number;
    std::size_t m_base_router;
    std::uint8_t m_child_routes;
    Handler<MaxHandlerSize> m_handler;
    Middleware<MaxMiddlewareSize> m_local_middleware;
};

template <std::size_t MaxMiddlewareSize = 10>
class Router {
  public:
    // constexpr Router() : m_ctx{}, m_router_number{0}, m_router_index{0} {}

    /**
     * @brief Registers a new root-level route for this router against `ctx` and claims a fresh
     * router number.
     * @param ctx the shared router context this router lives in — kept as a reference, must
     * outlive the router.
     * @param path the router's own path segment.
     */
    constexpr Router(RouterContext<> &ctx, std::string_view path)
        : m_ctx{ctx}, m_router_number{ctx.get_highest_router_number()},
          m_router_index{m_ctx.get().add_route(Route<>{path}.set_router_number(m_router_number))} {}

    /**
     * @brief Registers a new root-level route for this router against `ctx`, with `middlewares`
     * attached from the start.
     * @tparam Args pack of middleware-convertible callables.
     * @param ctx the shared router context this router lives in — kept as a reference, must
     * outlive the router.
     * @param path the router's own path segment.
     * @param middlewares middleware functions to attach up front, in order.
     */
    template <typename... Args>
        requires(std::is_convertible_v<Args, interfaces::MiddlewareFn> && ...)
    constexpr Router(RouterContext<> &ctx, std::string_view path, Args &&...middlewares)
        : m_ctx{ctx}, m_router_number{ctx.get_highest_router_number()},
          m_router_index{
              ctx.add_route(Route<>{path, std::forward<Args>(middlewares)...}.set_router_number(
                  m_router_number))} {}


    /**
     * @brief Builder-style: appends a middleware to this router's underlying route.
     * @param middleware the middleware function to append.
     * @note Rvalue-qualified (`&&`).
     * @return `*this`, moved out.
     */
    constexpr Router<> use(interfaces::MiddlewareFn middleware) && {
        m_ctx.get()[m_router_index].add_middleware(middleware);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return std::move(*this);
    }

    /**
     * @brief Builder-style: registers `handler` for `GET` on this router's own route.
     * @param handler the handler function to run for `GET` requests.
     * @note Rvalue-qualified (`&&`).
     * @throws std::runtime_error if a `GET` handler is already registered (via
     * `add_handler_in_place()`).
     * @return `*this`, moved out.
     */
    Router<> get(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::GET,  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                                         std::move(handler));
        return std::move(*this);
    }

    /**
     * @brief Builder-style: registers `handler` for `POST` on this router's own route.
     * @param handler the handler function to run for `POST` requests.
     * @note Rvalue-qualified (`&&`).
     * @throws std::runtime_error if a `POST` handler is already registered.
     * @return `*this`, moved out.
     */
    Router<> post(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::POST,  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                                         std::move(handler));
        return std::move(*this);
    }

    /**
     * @brief Builder-style: registers `handler` for `PUT` on this router's own route.
     * @param handler the handler function to run for `PUT` requests.
     * @note Rvalue-qualified (`&&`).
     * @throws std::runtime_error if a `PUT` handler is already registered.
     * @return `*this`, moved out.
     */
    Router<> put(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::PUT,  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                                         std::move(handler));
        return std::move(*this);
    }

    /**
     * @brief Builder-style: registers `handler` for `DELETE` on this router's own route.
     * @param handler the handler function to run for `DELETE` requests.
     * @note Rvalue-qualified (`&&`).
     * @throws std::runtime_error if a `DELETE` handler is already registered.
     * @return `*this`, moved out.
     */
    Router<> delt(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::DELETE,  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                                         std::move(handler));
        return std::move(*this);
    }

    /**
     * @brief Builder-style: registers `handler` for `PATCH` on this router's own route.
     * @param handler the handler function to run for `PATCH` requests.
     * @note Rvalue-qualified (`&&`).
     * @throws std::runtime_error if a `PATCH` handler is already registered.
     * @return `*this`, moved out.
     */
    Router<> patch(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::PATCH,  // FIXME(clang-tidy): unchecked operator[], consider .at()
                                                         std::move(handler));
        return std::move(*this);
    }

    /**
     * @brief Nests `sub` under this router: reparents it to this router's number, and syncs the
     * base-router child bookkeeping on the shared context.
     * @param sub the sub-router to nest, consumed by value.
     * @note `sub` must have been built against the same `RouterContext` as this router, or
     * `m_ctx.get()[sub.get_router_index()]` indexes into the wrong table entirely. No motion if
     * you cross contexts here.
     * @return `*this`, moved out.
     */
    constexpr Router<> add_router(Router<> sub) && {
        // reparent sub's underlying route to hang off this router instead of the root
        m_ctx.get()[sub.get_router_index()].update_base_router(m_router_number);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        // sub was previously counted as a base-router child (or assumed to be) — undo that,
        // then bump this router's own child count to reflect the new nesting
        m_ctx.get().decrement_base_router_children();
        m_ctx.get()[m_router_index].increment_child_routes();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return std::move(*this);
    }

    /**
     * @brief Adds a standalone `Route` as a child of this router.
     * @param sub the route to add, reparented to this router's number before being pushed into
     * the shared context.
     * @return `*this`, moved out.
     */
    constexpr Router<> add_route(Route<> sub) && {
        // tag the new route as belonging to this router, then push it into the shared context
        sub.update_base_router(m_router_number);
        m_ctx.get().add_route(std::move(sub));
        // and record the extra child on this router's own route entry
        m_ctx.get()[m_router_index].increment_child_routes();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return std::move(*this);
    }


    /**
     * @brief Gets this router's own router number.
     * @return the router number.
     */
    [[nodiscard]] constexpr std::size_t get_router_number() const noexcept {
        return m_router_number;
    }
    /**
     * @brief Gets the index of this router's underlying route in the shared context.
     * @return the route index.
     */
    [[nodiscard]] constexpr std::size_t get_router_index() const noexcept { return m_router_index; }

  private:
    std::reference_wrapper<RouterContext<>> m_ctx;
    std::size_t m_router_number;
    std::size_t m_router_index;
};


class RouteBuilder {
  public:
    /**
     * @brief Sets up an empty builder with a default root (`"/"`) route and a fallback (`"*"`)
     * route, both parented to the sentinel base router (`SIZE_MAX`) so they never get swept up
     * into a normal group during `build()`.
     */
    constexpr RouteBuilder()
        : m_root_route{Route<>{"/"}.set_base_router(std::numeric_limits<std::size_t>::max())},
          m_fallback_route{Route<>{"*"}.set_base_router(std::numeric_limits<std::size_t>::max())} {}

    /**
     * @brief Builder-style: sets the bind address.
     * @param address the address string to store.
     * @note Rvalue-qualified (`&&`).
     * @return `*this`, moved out.
     */
    constexpr RouteBuilder address(std::string_view address) && {
        m_address = std::string{address};
        return std::move(*this);
    }

    /**
     * @brief Builder-style: sets the bind port.
     * @param port the port number to store.
     * @warning `m_port` is a `std::uint8_t`, but a real port number needs up to 16 bits (0-65535).
     * The cast here is explicit (`static_cast<std::uint8_t>(port)`) so the compiler won't even warn
     * you — `port(8080)` silently becomes `144`. That's not lowkey, that's a straight-up L for
     * anyone passing a normal HTTP port. Document now, don't fix per instructions, but heads up:
     * this one bites hard.
     * @note Rvalue-qualified (`&&`).
     * @return `*this`, moved out.
     */
    constexpr RouteBuilder port(int port) && {
        m_port = static_cast<std::uint8_t>(port);
        return std::move(*this);
    }

    /**
     * @brief Builder-style: sets the router's name.
     * @param name the name string to store.
     * @note Rvalue-qualified (`&&`).
     * @return `*this`, moved out.
     */
    constexpr RouteBuilder name(std::string_view name) && {
        m_name = std::string{name};
        return std::move(*this);
    }


    // TODO: figue out a way to make this function consteval
    /**
     * @brief Finalizes the build: drops the root and fallback routes into slots 0 and 1 of `ctx`,
     * syncs the root's child count from the context's tracked base-router children, and delegates
     * to `RouterContext::build()`.
     * @param ctx the router context to build from, taken by value (this builder owns its own copy
     * to finalize).
     * @return the fully built `RouteHandler`, straight from `ctx.build()`.
     */
    auto build(RouterContext<> ctx) {
        // sync the root's child count from whatever the context actually tracked, then drop
        // the root and fallback routes into their reserved slots before compiling everything
        m_root_route.update_child_routes(ctx.get_base_router_children());
        ctx[0] = m_root_route;  // FIXME(clang-tidy): unchecked operator[], consider .at()
        ctx[1] = m_fallback_route;  // FIXME(clang-tidy): unchecked operator[], consider .at()
        return ctx.build();
    }

  private:
    std::string m_address;
    std::uint8_t m_port{0};
    std::string m_name;
    Route<> m_root_route;
    Route<> m_fallback_route;
};
} // namespace core::router
