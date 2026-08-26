export module utils_openapi:registry;

import std;
import :model;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::openapi {

// One entry per route/router node registered via ApiRoute/ApiRouter. Mirrors the identity
// fields core::router::Route already carries (path segment, own router_number, parent's
// base_router) so Generator can reconstruct full paths the same way RouterContext::build()
// reconstructs its trie — by walking the base_router/router_number parent chain.
class RouteMeta
{
public:
    /**
     * @brief Default ctor — blank route entry, filled in via the setters below.
     */
    RouteMeta() = default;

    /**
     * @brief Sets this route's own path segment (not the full resolved path — Generator walks
     * the base_router chain to stitch the full thing together later).
     * @param value the path segment to store.
     */
    void set_path(std::string value)
    {
        m_path = std::move(value);
    }

    /**
     * @brief Sets this route's own router number, mirroring core::router::Route's identity
     * field.
     * @param value the router number to store.
     */
    void set_router_number(std::size_t value) noexcept
    {
        m_router_number = value;
    }

    /**
     * @brief Sets the parent router number this route's nested under.
     * @param value the parent's router number.
     */
    void set_base_router(std::size_t value) noexcept
    {
        m_base_router = value;
    }

    /**
     * @brief Registers an operation for a given HTTP method on this route.
     * @param method the HTTP method, as its underlying std::uint8_t value.
     * @param operation the operation spec, moved in.
     */
    void add_operation(std::uint8_t method, Operation operation)
    {
        m_operations[method] = std::move(operation);
    }

    /**
     * @brief Grabs this route's own path segment.
     * @return the path segment.
     */
    [[nodiscard]] const std::string& get_path() const noexcept
    {
        return m_path;
    }

    /**
     * @brief Grabs this route's own router number.
     * @return the router number.
     */
    [[nodiscard]] std::size_t get_router_number() const noexcept
    {
        return m_router_number;
    }

    /**
     * @brief Grabs the parent router number.
     * @return the parent's router number, 0 if this route isn't nested under anything.
     */
    [[nodiscard]] std::size_t get_base_router() const noexcept
    {
        return m_base_router;
    }

    /**
     * @brief Grabs every operation registered on this route.
     * @return all operations, keyed by method's underlying std::uint8_t value.
     */
    [[nodiscard]] const std::unordered_map<std::uint8_t, Operation>& get_operations() const noexcept
    {
        return m_operations;
    }

    /**
     * @brief Grabs a mutable reference to the operation registered for a given method.
     * @warning Uses unordered_map::at() under the hood — call this with a method that was never
     * registered via add_operation() and it's an out_of_range L waiting to happen, no silent
     * fallback here.
     * @param method the HTTP method to look up.
     * @return mutable reference to that method's operation.
     * @throws std::out_of_range if no operation's registered for `method`.
     */
    [[nodiscard]] Operation& get_operation(std::uint8_t method)
    {
        return m_operations.at(method);
    }

private:
    std::string m_path;
    std::size_t m_router_number{0};
    std::size_t m_base_router{0};
    std::unordered_map<std::uint8_t, Operation> m_operations;
};

// Append-only, process-wide collector populated automatically by ApiRoute/ApiRouter as
// routes are registered — every plugin dlopen'd into the same process links against this
// same congelado_lib symbol, so no host-context plumbing is needed to reach it (mirrors
// core::logger::LoggerRegistry's "static inline" singleton pattern). Unrelated to
// core::router::RouterContext's compiled trie — Generator walks this separately to build
// the OpenAPI document.
class Registry
{
public:
    /**
     * @brief Appends a route entry to the process-wide registry and hands back its index — this
     * index is what ApiRouter holds onto so it can go patch set_base_router() on its own entry
     * later once nesting's resolved. Straight append-only motion, nothing ever gets removed.
     * @param route the route entry to store, moved in.
     * @return the index this entry landed at, for later at() lookups.
     */
    static std::size_t add_route(RouteMeta route)
    {
        m_routes.push_back(std::move(route));
        return m_routes.size() - 1;
    }

    /**
     * @brief Grabs a mutable reference to a registered route entry by index.
     * @warning vector::at() under the hood — an out-of-range index throws, no silent bypass
     * here, that's on purpose.
     * @param index the index returned by add_route().
     * @return mutable reference to that route entry.
     * @throws std::out_of_range if `index` is out of bounds.
     */
    [[nodiscard]] static RouteMeta& at(std::size_t index)
    {
        return m_routes.at(index);
    }

    /**
     * @brief Grabs every registered route entry, no cap.
     * @return all route entries, in registration order.
     */
    [[nodiscard]] static const std::vector<RouteMeta>& get_routes() noexcept
    {
        return m_routes;
    }

private:
    static inline std::vector<RouteMeta> m_routes;
};

} // namespace utils::openapi

#ifdef CONGELADO_TEST
namespace utils::openapi::tests {
using namespace boost::ut;

suite<"RouteMeta"> route_meta_suite = [] {
    "setters round-trip"_test = [] {
        RouteMeta meta;
        meta.set_path("tasks");
        meta.set_router_number(3);
        meta.set_base_router(1);

        expect(meta.get_path() == "tasks");
        expect(meta.get_router_number() == 3);
        expect(meta.get_base_router() == 1);
    };
    "add_operation registers by method, get_operation throws for an unregistered one"_test = [] {
        RouteMeta meta;
        meta.add_operation(0, Operation{});

        expect(meta.get_operations().contains(0));
        expect(throws<std::out_of_range>([&] {
            [[maybe_unused]] auto& op = meta.get_operation(99);
        }));
    };
};

suite<"Registry"> registry_suite = [] {
    // Registry is a process-wide, append-only singleton shared across every test in this
    // binary — assertions here are relative (before/after), never assuming it starts empty.
    "add_route appends and returns an index usable with at()"_test = [] {
        auto before = Registry::get_routes().size();

        RouteMeta meta;
        meta.set_path("widgets");
        auto index = Registry::add_route(meta);

        expect(Registry::get_routes().size() == before + 1);
        expect(Registry::at(index).get_path() == "widgets");
    };
    "at() throws for an out-of-range index"_test = [] {
        auto way_out_of_range = Registry::get_routes().size() + 1'000;
        expect(throws<std::out_of_range>([&] {
            [[maybe_unused]] auto& route = Registry::at(way_out_of_range);
        }));
    };
};

} // namespace utils::openapi::tests
#endif
