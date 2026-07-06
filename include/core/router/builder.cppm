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
    constexpr RouterContext()
        : m_routes{}, m_router_size{2}, m_highest_router_number{1}, m_base_router_children{0} {};

    constexpr std::size_t add_route(Route<MaxHandlerSize, MaxMiddlewareSize> route) {
        const auto idx = m_router_size++;
        m_routes[idx] = std::move(route);
        if (route.get_base_router() == 0) {
            ++m_base_router_children;
        }
        return idx;
    }

    constexpr Route<MaxHandlerSize, MaxMiddlewareSize> &operator[](std::size_t index) {
        if (index >= m_router_size) {
            throw std::out_of_range("RouterContext index out of bounds");
        }
        return m_routes[index];
    }

    // TODO: figue out a way to make this function consteval
    RouteHandler<> build() {
        std::vector<std::vector<RouterBuildingHelper>> table;
        std::size_t idx{0};
        const auto sentinel = std::numeric_limits<std::size_t>::max();

        auto active_routes = std::span(m_routes.data(), m_router_size);
        auto group_start = active_routes.begin();

        while (group_start != active_routes.end()) {
            auto group_end =
                std::ranges::find_if(group_start, active_routes.end(), [&](const auto &r) {
                    return r.get_base_router() != group_start->get_base_router();
                });

            std::size_t group_size = std::distance(group_start, group_end);
            std::ranges::sort(group_start, group_end, [&](const auto &a, const auto &b) {
                if (a.get_path() == "*")
                    return false;
                if (b.get_path() == "*")
                    return true;
                if (a.get_base_router() == sentinel && b.get_base_router() != sentinel)
                    return true;
                if (b.get_base_router() == sentinel && a.get_base_router() != sentinel)
                    return false;
                return fnv1a(a.get_path()) % group_size < fnv1a(b.get_path()) % group_size;
            });


            group_start = group_end;
        }

        RouteHandler<> handler{};

        while (idx < m_router_size) {
            insert_route(m_routes[idx], table);
            ++idx;
        }

        for (const auto &row : table) {
            std::size_t cumulative_children = 0;
            for (const auto &[index, node] : row | std::views::enumerate) {
                std::size_t offset{0};

                if (node.children > 0) {
                    offset = row.size() - index + cumulative_children;
                }

                cumulative_children += node.children;

                handler.add_route(node.kind, node.literal, offset, node.handlers, node.middlewares,
                                  node.children);
            }
        }

        std::println("Finished building");

        return std::move(handler);
    }

    constexpr std::size_t get_highest_router_number() noexcept { return m_highest_router_number++; }
    constexpr std::size_t get_router_size() const noexcept { return m_router_size; }
    constexpr std::size_t get_base_router_children() const noexcept {
        return m_base_router_children;
    }

    constexpr void decrement_base_router_children() noexcept {
        if (m_base_router_children > 0) {
            --m_base_router_children;
        }
    }


  private:
    class RouterBuildingHelper {
      public:
        EdgeKind kind{EdgeKind::Path};
        std::string literal{};
        Handler<MaxHandlerSize> handlers{};
        Middleware<MaxMiddlewareSize> middlewares{};
        std::size_t children{0};
        std::size_t router_number{0};
    };

    static constexpr std::pair<EdgeKind, std::string_view>
    determine_node_kind(std::string_view path) {
        if (path.empty()) {
            throw std::runtime_error("Path cannot be empty");
        }

        if (path == "*") {
            return std::make_pair(EdgeKind::Wild, std::string_view{});
        }

        if (path.front() == ':') {
            return std::make_pair(EdgeKind::Param, std::string_view{});
        }

        return std::make_pair(EdgeKind::Path, path);
    }

    constexpr void insert_route(const Route<MaxHandlerSize, MaxMiddlewareSize> &route,
                                std::vector<std::vector<RouterBuildingHelper>> &table) {
        if (route.get_child_routes() != 0) {
            if (route.is_build()) {
                return;
            }

            auto [kind, literal] = determine_node_kind(route.get_path());

            switch (kind) {
            case EdgeKind::Path:
                break;
            case EdgeKind::Param:
                throw std::runtime_error("Router path cannot be parameterized");
            case EdgeKind::Wild:
                throw std::runtime_error("Router path cannot be wildcard");
            }

            std::size_t new_idx = 0;

            if (route.get_base_router() != std::numeric_limits<std::size_t>::max()) {
                auto it = std::ranges::find_if(table, [&](const auto &node) {
                    for (const auto &sub : node) {
                        if (sub.router_number == route.get_base_router()) {
                            return true;
                        }
                    }
                    return false;
                });

                if (it == table.end()) {
                    throw std::runtime_error(
                        std::format("Parent router with number {} not found for route {}",
                                    route.get_base_router(), route.get_path()));
                }

                new_idx = std::distance(table.begin(), it) + 1;
            }

            if (new_idx >= table.size()) {
                table.resize(new_idx + 1);
            }

            table[new_idx].push_back(RouterBuildingHelper{
                kind,
                std::string{literal},
                route.get_handlers(),
                route.get_middlewares(),
                route.get_child_routes(),
                route.get_router_number(),
            });

            route.is_build();
        } else {
            if (route.is_build()) {
                return;
            }

            auto [kind, literal] = determine_node_kind(route.get_path());

            std::size_t new_idx = 0;

            if (route.get_base_router() != std::numeric_limits<std::size_t>::max()) {
                auto it = std::ranges::find_if(table, [&](const auto &node) {
                    for (const auto &sub : node) {
                        if (sub.router_number == route.get_base_router()) {
                            return true;
                        }
                    }
                    return false;
                });

                if (it == table.end()) {
                    throw std::runtime_error(
                        std::format("Parent router with number {} not found for route {}",
                                    route.get_base_router(), route.get_path()));
                }

                new_idx = std::distance(table.begin(), it) + 1;
            }

            if (new_idx >= table.size()) {
                table.resize(new_idx + 1);
            }

            table[new_idx].push_back(RouterBuildingHelper{
                kind,
                std::string{literal},
                route.get_handlers(),
                route.get_middlewares(),
                route.get_child_routes(),
                0,
            });

            route.is_build();
        }
    }

    // Middleware<MaxMiddlewareSize> m_middlewares;
    std::array<Route<MaxHandlerSize, MaxMiddlewareSize>, RouterSize> m_routes;
    std::size_t m_router_size;
    std::size_t m_highest_router_number;
    std::size_t m_base_router_children;
};

template <std::size_t MaxHandlerSize = 8, std::size_t MaxMiddlewareSize = 10>
class Route {
  public:
    constexpr Route()
        : m_path{}, m_is_build{false}, m_router_number{0}, m_base_router{0}, m_child_routes{0},
          m_handler{}, m_local_middleware{} {}

    constexpr Route<>(std::string_view path)
        : m_path{path.size() > 1 && path.starts_with('/') ? path.substr(1) : path},
          m_is_build{false}, m_router_number{0}, m_base_router{0}, m_child_routes{0}, m_handler{},
          m_local_middleware{} {
        if (path != "*") {
            if (!path.contains('/')) {
                throw std::runtime_error("Route path must start '/'");
            }
        }
    }

    template <typename... Args>
        requires(std::is_convertible_v<Args, interfaces::MiddlewareFn> && ...)
    constexpr Route(std::string_view path, Args &&...middlewares)
        : m_path{path.size() > 1 && path.starts_with('/') ? path.substr(1) : path},
          m_base_router{0}, m_handler{} {
        if (path != "*") {
            if (!path.contains('/')) {
                throw std::runtime_error("Route path must start '/'");
            }
        }
        (m_local_middleware.add_middleware(std::forward<Args>(middlewares)), ...);
    }

    constexpr Route<> use(interfaces::MiddlewareFn mw) && {
        m_local_middleware.add_middleware(std::move(mw));
        return std::move(*this);
    }

    Route<> get(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::GET, std::move(handler));
    }


    Route<> post(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::POST, std::move(handler));
    }

    Route<> put(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::PUT, std::move(handler));
    }

    constexpr Route<> patch(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::PATCH, std::move(handler));
    }

    constexpr Route<> delt(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::DELETE, std::move(handler));
    }

    constexpr Route<> head(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::HEAD, std::move(handler));
    }

    constexpr Route<> options(interfaces::HandlerFn handler) {
        return add_handler(interfaces::io::types::Method::OPTIONS, std::move(handler));
    }

    constexpr Route<> add_handler(interfaces::io::types::Method method,
                                  interfaces::HandlerFn handler) {
        auto handler_fnc = m_handler.find(method);
        if (!handler_fnc) {
            m_handler.add_handler(method, std::move(handler));
            return std::move(*this);
        }


        if consteval {
            // static_assert(false, "Duplicate handler found for method please check your routes");
            return std::move(*this);
        } else {
            throw std::runtime_error(std::format("Handler for method {} already exists on path {}",
                                                 std::to_underlying(method), m_path));
        }
    }

    void add_handler_in_place(interfaces::io::types::Method method, interfaces::HandlerFn handler) {
        if (m_handler.find(method)) {
            throw std::runtime_error(std::format("Handler for method {} already exists on path {}",
                                                 std::to_underlying(method), m_path));
        }
        m_handler.add_handler(method, std::move(handler));
    }

    constexpr Route<> set_base_router(std::size_t router_number) && {
        m_base_router = router_number;
        return std::move(*this);
    }

    constexpr Route<> set_router_number(std::size_t router_number) && {
        m_router_number = router_number;
        return std::move(*this);
    }

    constexpr void add_middleware(interfaces::MiddlewareFn middleware) {
        m_local_middleware.add_middleware(std::move(middleware));
    }

    constexpr std::string_view get_path() const noexcept { return m_path; }
    constexpr bool is_build() const noexcept { return m_is_build; }
    constexpr std::size_t get_child_routes() const noexcept { return m_child_routes; }
    constexpr std::size_t get_base_router() const noexcept { return m_base_router; }
    constexpr std::size_t get_router_number() const noexcept { return m_router_number; }
    constexpr const Handler<MaxHandlerSize> &get_handlers() const noexcept { return m_handler; }
    constexpr const Middleware<MaxMiddlewareSize> &get_middlewares() const noexcept {
        return m_local_middleware;
    }

    constexpr void update_base_router(const std::size_t &number) noexcept {
        m_base_router = number;
    }
    constexpr void update_child_routes(const std::size_t &number) noexcept {
        m_child_routes = number;
    }
    constexpr void update_router_number(const std::size_t &number) noexcept {
        m_router_number = number;
    }
    constexpr void set_build() noexcept { m_is_build = true; }
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

    constexpr Router(RouterContext<> &ctx, std::string_view path)
        : m_ctx{ctx}, m_router_number{ctx.get_highest_router_number()},
          m_router_index{m_ctx.get().add_route(Route<>{path}.set_router_number(m_router_number))} {}

    template <typename... Args>
        requires(std::is_convertible_v<Args, interfaces::MiddlewareFn> && ...)
    constexpr Router(RouterContext<> &ctx, std::string_view path, Args &&...middlewares)
        : m_ctx{ctx}, m_router_number{ctx.get_highest_router_number()},
          m_router_index{
              ctx.add_route(Route<>{path, std::forward<Args>(middlewares)...}.set_router_number(
                  m_router_number))} {}


    constexpr Router<> use(interfaces::MiddlewareFn middleware) && {
        m_ctx.get()[m_router_index].add_middleware(std::move(middleware));
        return std::move(*this);
    }

    Router<> get(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::GET,
                                                         std::move(handler));
        return std::move(*this);
    }

    Router<> post(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::POST,
                                                         std::move(handler));
        return std::move(*this);
    }

    Router<> put(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::PUT,
                                                         std::move(handler));
        return std::move(*this);
    }

    Router<> delt(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::DELETE,
                                                         std::move(handler));
        return std::move(*this);
    }

    Router<> patch(interfaces::HandlerFn handler) && {
        m_ctx.get()[m_router_index].add_handler_in_place(interfaces::io::types::Method::PATCH,
                                                         std::move(handler));
        return std::move(*this);
    }

    constexpr Router<> add_router(Router<> sub) && {
        m_ctx.get()[sub.get_router_index()].update_base_router(m_router_number);
        m_ctx.get().decrement_base_router_children();
        m_ctx.get()[m_router_index].increment_child_routes();
        return std::move(*this);
    }

    constexpr Router<> add_route(Route<> sub) && {
        sub.update_base_router(m_router_number);
        m_ctx.get().add_route(std::move(sub));
        m_ctx.get()[m_router_index].increment_child_routes();
        return std::move(*this);
    }


    [[nodiscard]] constexpr std::size_t get_router_number() const noexcept {
        return m_router_number;
    }
    [[nodiscard]] constexpr std::size_t get_router_index() const noexcept { return m_router_index; }

  private:
    std::reference_wrapper<RouterContext<>> m_ctx;
    std::size_t m_router_number;
    std::size_t m_router_index;
};


class RouteBuilder {
  public:
    constexpr RouteBuilder()
        : m_address{}, m_port{0}, m_name{},
          m_root_route{Route<>{"/"}.set_base_router(std::numeric_limits<std::size_t>::max())},
          m_fallback_route{Route<>{"*"}.set_base_router(std::numeric_limits<std::size_t>::max())} {}

    constexpr RouteBuilder address(std::string_view address) && {
        m_address = std::string{address};
        return std::move(*this);
    }

    constexpr RouteBuilder port(int port) && {
        m_port = std::uint8_t(port);
        return std::move(*this);
    }

    constexpr RouteBuilder name(std::string_view name) && {
        m_name = std::string{name};
        return std::move(*this);
    }


    // TODO: figue out a way to make this function consteval
    auto build(RouterContext<> ctx) {
        m_root_route.update_child_routes(ctx.get_base_router_children());
        ctx[0] = m_root_route;
        ctx[1] = m_fallback_route;
        return ctx.build();
    }

  private:
    std::string m_address;
    std::uint8_t m_port;
    std::string m_name;
    Route<> m_root_route;
    Route<> m_fallback_route;
};
} // namespace core::router
