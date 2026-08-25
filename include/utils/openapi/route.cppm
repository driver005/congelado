export module utils_openapi:route;

import std;
import interfaces;
import core_router;
import :model;
import :schema;
import :registry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace utils::openapi {

// Fluent, drop-in replacement for core::router::Route<> that additionally records
// per-method OpenAPI metadata (summary/description/tags/body/response). The wrapped
// route and the accumulated metadata are handed off together once consumed by
// ApiRouter::add_route().
class ApiRoute {
  public:
    /**
     * @brief Wraps a fresh core::router::Route<> at `path` — starts with zero operations,
     * metadata only starts accumulating once get()/post()/etc. picks a method to track.
     * @param path the route's path.
     */
    explicit ApiRoute(std::string_view path) : m_route{path} {}

    /**
     * @brief Builder chain — registers `handler` as this route's GET handler and starts
     * tracking OpenAPI metadata for GET. Bread and butter method, no cap.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the metadata chain (summary()/tags()/etc.) keeps going.
     */
    ApiRoute get(interfaces::HandlerFn handler) {
        m_route = m_route.get(std::move(handler));
        return with_method(interfaces::io::types::Method::GET);
    }
    /**
     * @brief Builder chain — registers `handler` as this route's POST handler and starts
     * tracking OpenAPI metadata for POST.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute post(interfaces::HandlerFn handler) {
        m_route = m_route.post(std::move(handler));
        return with_method(interfaces::io::types::Method::POST);
    }
    /**
     * @brief Builder chain — registers `handler` as this route's PUT handler and starts
     * tracking OpenAPI metadata for PUT.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute put(interfaces::HandlerFn handler) {
        m_route = m_route.put(std::move(handler));
        return with_method(interfaces::io::types::Method::PUT);
    }
    /**
     * @brief Builder chain — registers `handler` as this route's PATCH handler and starts
     * tracking OpenAPI metadata for PATCH.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute patch(interfaces::HandlerFn handler) {
        m_route = m_route.patch(std::move(handler));
        return with_method(interfaces::io::types::Method::PATCH);
    }
    /**
     * @brief Builder chain — registers `handler` as this route's DELETE handler. Named `delt`
     * since `delete` is a reserved keyword — same naming dodge core::router::Route uses.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute delt(interfaces::HandlerFn handler) {
        m_route = m_route.delt(std::move(handler));
        return with_method(interfaces::io::types::Method::DELETE);
    }
    /**
     * @brief Builder chain — registers `handler` as this route's HEAD handler and starts
     * tracking OpenAPI metadata for HEAD.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute head(interfaces::HandlerFn handler) {
        m_route = m_route.head(std::move(handler));
        return with_method(interfaces::io::types::Method::HEAD);
    }
    /**
     * @brief Builder chain — registers `handler` as this route's OPTIONS handler and starts
     * tracking OpenAPI metadata for OPTIONS. Last method in the crew, same pattern all the way
     * down.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute options(interfaces::HandlerFn handler) {
        m_route = m_route.options(std::move(handler));
        return with_method(interfaces::io::types::Method::OPTIONS);
    }

    /**
     * @brief Builder chain — sets the summary text on whichever method's currently active (the
     * last get()/post()/etc. call).
     * @param value the summary text.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute summary(std::string_view value) {
        current_operation().set_summary(std::string{value});
        return std::move(*this);
    }
    /**
     * @brief Builder chain — sets the longer-form description on the current method's
     * operation.
     * @param value the description text.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute description(std::string_view value) {
        current_operation().set_description(std::string{value});
        return std::move(*this);
    }
    /**
     * @brief Builder chain — tags the current method's operation with every value in `values`.
     * @param values the tags to add.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRoute tags(std::initializer_list<std::string_view> values) {
        // Every value in `values` lands on the current method's operation, no dedup here.
        for (auto value : values) {
            current_operation().add_tag(std::string{value});
        }
        return std::move(*this);
    }
    /**
     * @brief Builder chain — marks the current method's request body as required and derives
     * its schema from T via build_schema<T>(). application/json only, that's the one
     * content-type this whole setup actually speaks, lowkey by design.
     * @tparam T the C++ type whose schema describes the request body.
     * @return `*this`, moved, so the chain keeps going.
     */
    template <typename T>
    ApiRoute body() {
        // Derive T's schema once, wrap it as the sole "application/json" media-type
        // entry, then attach the whole thing as the current method's request body.
        RequestBody request_body;
        request_body.set_required(true);
        MediaType media_type;
        media_type.set_schema(build_schema<T>());
        request_body.add_content("application/json", std::move(media_type));
        current_operation().set_request_body(std::move(request_body));
        return std::move(*this);
    }
    /**
     * @brief Builder chain — registers a response entry for `status` on the current method,
     * schema derived from T via build_schema<T>().
     * @tparam T the C++ type whose schema describes the response body.
     * @param status the HTTP status code this response documents, defaults to 200.
     * @param description human-readable blurb for this response, defaults to "OK".
     * @return `*this`, moved, so the chain keeps going.
     */
    template <typename T>
    ApiRoute response(int status = 200, std::string_view description = "OK") {
        // Same media-type-wrapping motion as body() above, then register the whole
        // response keyed by its status code (as a string — that's the OpenAPI shape).
        Response response_obj;
        response_obj.set_description(std::string{description});
        MediaType media_type;
        media_type.set_schema(build_schema<T>());
        response_obj.add_content("application/json", std::move(media_type));
        current_operation().add_response(std::to_string(status), std::move(response_obj));
        return std::move(*this);
    }

    /**
     * @brief Grabs the wrapped route's path, straight up.
     * @return the route's path.
     */
    [[nodiscard]] std::string_view getPath() const noexcept { return m_route.get_path(); }  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
    /**
     * @brief Terminal call — hands off ownership of the wrapped core::router::Route<>, stripped
     * of all the OpenAPI metadata tracking that lived alongside it.
     * @warning Moves out of `*this` — don't touch this ApiRoute after calling it, it's cooked.
     * @return the wrapped route, moved out.
     */
    [[nodiscard]] core::router::Route<> &&into_route() && { return std::move(m_route); }
    /**
     * @brief Terminal call — hands off the accumulated per-method operation metadata.
     * @warning Moves out of `*this`, same deal as into_route() — don't reuse this ApiRoute after.
     * @return every operation this ApiRoute built up, keyed by method's underlying value.
     */
    [[nodiscard]] std::unordered_map<std::uint8_t, Operation> &&into_operations() && {
        return std::move(m_operations);
    }
    /**
     * @brief Terminal call — hands off both the wrapped route and the accumulated per-method
     * operation metadata in a single move, so callers never need two separate rvalue-qualified
     * extractions off the same object (that pattern reads as a use-after-move to static analysis
     * even though into_route()/into_operations() touch disjoint members).
     * @warning Moves out of `*this`, same deal as into_route()/into_operations().
     * @return a pair of {wrapped route, accumulated operations}, both moved out.
     */
    [[nodiscard]] std::pair<core::router::Route<>, std::unordered_map<std::uint8_t, Operation>>
    into_parts() && {
        return {std::move(m_route), std::move(m_operations)};
    }

  private:
    /**
     * @brief Switches the "current method" pointer and seeds a blank Operation for it — every
     * summary()/description()/tags()/body()/response() call after this lands on that operation
     * until the next get()/post()/etc. call switches it again.
     * @param method the HTTP method becoming current.
     * @return `*this`, moved.
     */
    ApiRoute with_method(interfaces::io::types::Method method) {
        // Point "current" at the new method, bet, then seed a blank operation for it so
        // current_operation() has something to find right after.
        m_current_method = std::to_underlying(method);
        m_operations[m_current_method] = Operation{};
        return std::move(*this);
    }

    /**
     * @brief Grabs the operation for whichever method's currently active.
     * @warning unordered_map::at() under the hood — call a metadata method before any
     * get()/post()/etc. call and m_current_method's still its default 0 with no operation
     * seeded there, straight out_of_range L. Order matters, don't skip the method call.
     * @return mutable reference to the current method's operation.
     * @throws std::out_of_range if no operation's registered for the current method.
     */
    Operation &current_operation() { return m_operations.at(m_current_method); }

    core::router::Route<> m_route;
    std::unordered_map<std::uint8_t, Operation> m_operations;
    std::uint8_t m_current_method{0};
};

// Fluent, drop-in replacement for core::router::Router<> that additionally records
// its own node (and any directly-mounted method handlers) into the process-wide
// Registry singleton, so nested ApiRouter/ApiRoute path segments can be reconstructed
// by Generator later.
class ApiRouter {
  public:
    /**
     * @brief Wraps a fresh core::router::RouterContext<> node at `path` and immediately
     * registers itself into the process-wide Registry — that registration happens right here
     * in the ctor, not lazily, so every ApiRouter that gets constructed shows up in Generator's
     * walk even if it ends up with zero operations. No cap, no ApiRouter slips through unnoticed.
     * @param context the router context this node's built under.
     * @param path this router's own path segment.
     */
    ApiRouter(core::router::RouterContext<> &context, std::string_view path) : m_router{context, path} {
        // Build this node's RouteMeta from the (normalized) path and the freshly wrapped
        // router's own number, then register it eagerly — no lazy-init here, every
        // ApiRouter shows up in Registry the moment it's constructed.
        RouteMeta meta;
        meta.set_path(std::string{normalize_path(path)});
        meta.set_router_number(m_router.get_router_number());
        m_registry_index = Registry::add_route(std::move(meta));
    }

    /**
     * @brief Builder chain — registers `handler` as this router's GET handler and starts
     * tracking OpenAPI metadata for GET.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter get(interfaces::HandlerFn handler) && {
        m_router = std::move(m_router).get(std::move(handler));
        return with_method(interfaces::io::types::Method::GET);
    }
    /**
     * @brief Builder chain — registers `handler` as this router's POST handler and starts
     * tracking OpenAPI metadata for POST.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter post(interfaces::HandlerFn handler) && {
        m_router = std::move(m_router).post(std::move(handler));
        return with_method(interfaces::io::types::Method::POST);
    }
    /**
     * @brief Builder chain — registers `handler` as this router's PUT handler and starts
     * tracking OpenAPI metadata for PUT.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter put(interfaces::HandlerFn handler) && {
        m_router = std::move(m_router).put(std::move(handler));
        return with_method(interfaces::io::types::Method::PUT);
    }
    /**
     * @brief Builder chain — registers `handler` as this router's PATCH handler and starts
     * tracking OpenAPI metadata for PATCH.
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter patch(interfaces::HandlerFn handler) && {
        m_router = std::move(m_router).patch(std::move(handler));
        return with_method(interfaces::io::types::Method::PATCH);
    }
    /**
     * @brief Builder chain — registers `handler` as this router's DELETE handler. Named `delt`
     * for the same reserved-keyword reason as ApiRoute::delt().
     * @param handler the request handler to install.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter delt(interfaces::HandlerFn handler) && {
        m_router = std::move(m_router).delt(std::move(handler));
        return with_method(interfaces::io::types::Method::DELETE);
    }

    /**
     * @brief Builder chain — sets the summary text on whichever method's currently active.
     * @param value the summary text.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter summary(std::string_view value) && {
        current_operation().set_summary(std::string{value});
        return *this;
    }
    /**
     * @brief Builder chain — sets the longer-form description on the current method's
     * operation.
     * @param value the description text.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter description(std::string_view value) && {
        current_operation().set_description(std::string{value});
        return *this;
    }
    /**
     * @brief Builder chain — tags the current method's operation with every value in `values`.
     * @param values the tags to add.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter tags(std::initializer_list<std::string_view> values) && {
        // Every value in `values` lands on the current method's operation, no dedup here.
        for (auto value : values) {
            current_operation().add_tag(std::string{value});
        }
        return *this;
    }
    /**
     * @brief Builder chain — marks the current method's request body as required and derives
     * its schema from T via build_schema<T>().
     * @tparam T the C++ type whose schema describes the request body.
     * @return `*this`, moved, so the chain keeps going.
     */
    template <typename T>
    ApiRouter body() && {
        // Derive T's schema once, wrap it as the sole "application/json" media-type
        // entry, then attach the whole thing as the current method's request body.
        RequestBody request_body;
        request_body.set_required(true);
        MediaType media_type;
        media_type.set_schema(build_schema<T>());
        request_body.add_content("application/json", std::move(media_type));
        current_operation().set_request_body(std::move(request_body));
        return *this;
    }
    /**
     * @brief Builder chain — registers a response entry for `status` on the current method,
     * schema derived from T via build_schema<T>().
     * @tparam T the C++ type whose schema describes the response body.
     * @param status the HTTP status code this response documents, defaults to 200.
     * @param description human-readable blurb for this response, defaults to "OK".
     * @return `*this`, moved, so the chain keeps going.
     */
    template <typename T>
    ApiRouter response(int status = 200, std::string_view description = "OK") && {
        // Same media-type-wrapping motion as body() above, then register the whole
        // response keyed by its status code (as a string — that's the OpenAPI shape).
        Response response_obj;
        response_obj.set_description(std::string{description});
        MediaType media_type;
        media_type.set_schema(build_schema<T>());
        response_obj.add_content("application/json", std::move(media_type));
        current_operation().add_response(std::to_string(status), std::move(response_obj));
        return *this;
    }

    /**
     * @brief Builder chain — mounts `child_router` under this one, both in the underlying
     * core::router::Router<> tree and in the Registry (patches the child's stored base_router
     * to point at this router's number, now that nesting's actually resolved).
     * @param child_router the child router to mount, consumed.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter add_router(ApiRouter child_router) && {
        // Stash the child's registry slot before it gets consumed below, then mount it
        // into the underlying router tree — big W once that's done — and finally patch
        // that slot's base_router now that the nesting relationship is actually known.
        auto child_registry_index = child_router.m_registry_index;
        m_router = std::move(m_router).add_router(std::move(child_router).into_router());
        Registry::at(child_registry_index).set_base_router(m_router.get_router_number());
        return *this;
    }
    /**
     * @brief Builder chain — mounts `child_route` under this router, both in the underlying
     * tree and in the Registry (a fresh RouteMeta gets built from the child's path and
     * accumulated operations, base_router pointed at this router's number).
     * @param child_route the child route to mount, consumed.
     * @return `*this`, moved, so the chain keeps going.
     */
    ApiRouter add_route(ApiRoute child_route) && {
        // Grab the child's path first (no move needed), then hand off the route and its
        // accumulated operations together via into_parts() — a single move of child_route
        // rather than two separate rvalue-qualified extractions off the same object (which
        // reads as a use-after-move to static analysis even though into_route()/
        // into_operations() touch disjoint members).
        std::string path{child_route.getPath()};
        auto [route, operations] = std::move(child_route).into_parts();
        m_router = std::move(m_router).add_route(std::move(route));

        // Build a fresh RouteMeta from what got salvaged above, base_router pointed at
        // this router's own number, and register it — mirrors what the ctor does for
        // this router itself, just for the mounted child instead.
        RouteMeta meta;
        meta.set_path(std::move(path));
        meta.set_base_router(m_router.get_router_number());
        for (auto &[method, operation] : operations) {
            meta.add_operation(method, std::move(operation));
        }
        Registry::add_route(std::move(meta));
        return *this;
    }

    /**
     * @brief Terminal call — hands off ownership of the wrapped core::router::Router<>,
     * stripped of the OpenAPI tracking that rode alongside it.
     * @warning Moves out of `*this` — don't reuse this ApiRouter after, it's cooked once this
     * gets called.
     * @return the wrapped router, moved out.
     */
    [[nodiscard]] core::router::Router<> &&into_router() && { return std::move(m_router); }
    /**
     * @brief Grabs the wrapped router's own router number.
     * @return the router number.
     */
    [[nodiscard]] std::size_t getRouterNumber() const noexcept {  // NOLINT(readability-identifier-naming) — matches this project's get/set/add accessor naming convention (camelCase after prefix), not a real naming defect — the shared clang-tidy config has no accessor exception
        return m_router.get_router_number();
    }

  private:
    /**
     * @brief Strips a leading '/' off `path` (unless it's just "/" on its own) — Registry
     * stores bare segments, and this keeps the ctor from double-slashing when it builds the
     * stored RouteMeta.
     * @param path the raw path to normalize.
     * @return the normalized (leading-slash-stripped) path.
     */
    static std::string_view normalize_path(std::string_view path) noexcept {
        return path.size() > 1 && path.starts_with('/') ? path.substr(1) : path;
    }

    /**
     * @brief Switches the "current method" pointer and seeds a blank Operation in the Registry
     * entry for it.
     * @param method the HTTP method becoming current.
     * @return `*this`, moved.
     */
    ApiRouter with_method(interfaces::io::types::Method method) {
        // Point "current" at the new method, then seed a blank operation for it directly
        // in the Registry entry — unlike ApiRoute, there's no local map to seed here.
        m_current_method = std::to_underlying(method);
        Registry::at(m_registry_index).add_operation(m_current_method, Operation{});
        return *this;
    }

    /**
     * @brief Grabs the operation for whichever method's currently active, straight from the
     * Registry entry — unlike ApiRoute, this router's metadata lives in Registry the whole
     * time instead of a local member, since the ctor already registered it there.
     * @warning Same at()-throws deal as ApiRoute::current_operation() — call a metadata method
     * before get()/post()/etc. and this is an out_of_range L.
     * @return mutable reference to the current method's operation.
     * @throws std::out_of_range if no operation's registered for the current method.
     */
    [[nodiscard]] Operation &current_operation() const {
        return Registry::at(m_registry_index).get_operation(m_current_method);
    }

    core::router::Router<> m_router;
    std::size_t m_registry_index;
    std::uint8_t m_current_method{0};
};

} // namespace utils::openapi

#ifdef CONGELADO_TEST
namespace utils::openapi::route_tests {
using namespace boost::ut;

// static: internal linkage so this doesn't collide with the same-named test helpers other
// modules' test blocks define in their own tests namespaces.
static interfaces::HandlerFn noop_handler() {
    return [](interfaces::io::IRequest &, interfaces::io::IResponse &, std::function<void()>) {};
}

using Method = interfaces::io::types::Method;

suite<"ApiRoute verb builders"> api_route_verb_suite = [] {
    "get() installs a GET handler and starts tracking GET metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.get(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::GET) != nullptr);
        expect(route.get_handlers().find(Method::POST) == nullptr);
        expect(operations.contains(std::to_underlying(Method::GET)));
    };

    "post() installs a POST handler and starts tracking POST metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.post(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::POST) != nullptr);
        expect(route.get_handlers().find(Method::GET) == nullptr);
        expect(operations.contains(std::to_underlying(Method::POST)));
    };

    "put() installs a PUT handler and starts tracking PUT metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.put(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::PUT) != nullptr);
        expect(operations.contains(std::to_underlying(Method::PUT)));
    };

    "patch() installs a PATCH handler and starts tracking PATCH metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.patch(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::PATCH) != nullptr);
        expect(operations.contains(std::to_underlying(Method::PATCH)));
    };

    "delt() installs a DELETE handler and starts tracking DELETE metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.delt(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::DELETE) != nullptr);
        expect(operations.contains(std::to_underlying(Method::DELETE)));
    };

    "head() installs a HEAD handler and starts tracking HEAD metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.head(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::HEAD) != nullptr);
        expect(operations.contains(std::to_underlying(Method::HEAD)));
    };

    "options() installs an OPTIONS handler and starts tracking OPTIONS metadata"_test = [] {
        auto [route, operations] = ApiRoute{"/x"}.options(noop_handler()).into_parts();
        expect(route.get_handlers().find(Method::OPTIONS) != nullptr);
        expect(operations.contains(std::to_underlying(Method::OPTIONS)));
    };
};

suite<"ApiRoute metadata builders"> api_route_metadata_suite = [] {
    "summary/description/tags land on the current method's operation"_test = [] {
        auto operations = ApiRoute{"/x"}
                               .get(noop_handler())
                               .summary("List things")
                               .description("Returns every thing")
                               .tags({"a", "b"})
                               .into_operations();
        auto &op = operations.at(std::to_underlying(Method::GET));

        expect(op.get_summary() == "List things");
        expect(op.get_description() == "Returns every thing");
        expect(op.get_tags().size() == 2U);
        expect(op.get_tags()[0] == "a");
        expect(op.get_tags()[1] == "b");
    };

    "switching methods mid-chain starts a fresh operation per method"_test = [] {
        auto operations = ApiRoute{"/x"}
                               .get(noop_handler())
                               .summary("Get summary")
                               .post(noop_handler())
                               .summary("Post summary")
                               .into_operations();

        expect(operations.at(std::to_underlying(Method::GET)).get_summary() == "Get summary");
        expect(operations.at(std::to_underlying(Method::POST)).get_summary() == "Post summary");
    };

    "body<T>() attaches a required application/json request body derived from T"_test = [] {
        auto operations = ApiRoute{"/x"}.post(noop_handler()).body<int>().into_operations();
        auto &op = operations.at(std::to_underlying(Method::POST));

        expect(op.get_request_body() != nullptr) << fatal;
        expect(op.get_request_body()->get_required());
        expect(op.get_request_body()->get_content().contains("application/json"));
        expect(op.get_request_body()->get_content().at("application/json").get_schema().get_type() ==
               "integer");
    };

    "response<T>() defaults to status 200 / description OK"_test = [] {
        auto operations = ApiRoute{"/x"}.get(noop_handler()).response<int>().into_operations();
        auto &op = operations.at(std::to_underlying(Method::GET));

        expect(op.get_responses().contains("200"));
        expect(op.get_responses().at("200").get_description() == "OK");
        expect(op.get_responses().at("200").get_content().at("application/json").get_schema().get_type() ==
               "integer");
    };

    "response<T>() honors an explicit status and description"_test = [] {
        auto operations =
            ApiRoute{"/x"}.get(noop_handler()).response<int>(404, "Not Found").into_operations();
        auto &op = operations.at(std::to_underlying(Method::GET));

        expect(op.get_responses().contains("404"));
        expect(op.get_responses().at("404").get_description() == "Not Found");
    };
};

suite<"ApiRoute path and terminal moves"> api_route_terminal_suite = [] {
    "getPath strips exactly one leading slash, mirroring core::router::Route"_test = [] {
        ApiRoute route{"/tasks"};
        expect(route.getPath() == "tasks");
    };

    "into_route hands off the wrapped core::router::Route, stripped of metadata"_test = [] {
        core::router::Route<> plain_route = ApiRoute{"/tasks"}.get(noop_handler()).into_route();
        expect(plain_route.get_path() == "tasks");
        expect(plain_route.get_handlers().find(Method::GET) != nullptr);
    };

    "into_operations hands off the accumulated per-method operation metadata"_test = [] {
        auto operations =
            ApiRoute{"/tasks"}.get(noop_handler()).summary("List").into_operations();
        expect(operations.size() == 1U);
        expect(operations.at(std::to_underlying(Method::GET)).get_summary() == "List");
    };

    "into_parts hands off both the route and the operations in a single move"_test = [] {
        auto [plain_route, operations] =
            ApiRoute{"/tasks"}.get(noop_handler()).summary("List").into_parts();
        expect(plain_route.get_path() == "tasks");
        expect(operations.at(std::to_underlying(Method::GET)).get_summary() == "List");
    };
};

suite<"ApiRouter verb builders"> api_router_verb_suite = [] {
    "get() registers a handler for GET on the router's own route"_test = [] {
        core::router::RouterContext<> ctx;
        core::router::Router<> plain_router =
            std::move(ApiRouter{ctx, "/r-get"}.get(noop_handler())).into_router();
        expect(ctx[plain_router.get_router_index()].get_handlers().find(Method::GET) != nullptr);
    };

    "post() registers a handler for POST on the router's own route"_test = [] {
        core::router::RouterContext<> ctx;
        core::router::Router<> plain_router =
            std::move(ApiRouter{ctx, "/r-post"}.post(noop_handler())).into_router();
        expect(ctx[plain_router.get_router_index()].get_handlers().find(Method::POST) != nullptr);
    };

    "put() registers a handler for PUT on the router's own route"_test = [] {
        core::router::RouterContext<> ctx;
        core::router::Router<> plain_router =
            std::move(ApiRouter{ctx, "/r-put"}.put(noop_handler())).into_router();
        expect(ctx[plain_router.get_router_index()].get_handlers().find(Method::PUT) != nullptr);
    };

    "patch() registers a handler for PATCH on the router's own route"_test = [] {
        core::router::RouterContext<> ctx;
        core::router::Router<> plain_router =
            std::move(ApiRouter{ctx, "/r-patch"}.patch(noop_handler())).into_router();
        expect(ctx[plain_router.get_router_index()].get_handlers().find(Method::PATCH) != nullptr);
    };

    "delt() registers a handler for DELETE on the router's own route"_test = [] {
        core::router::RouterContext<> ctx;
        core::router::Router<> plain_router =
            std::move(ApiRouter{ctx, "/r-delt"}.delt(noop_handler())).into_router();
        expect(ctx[plain_router.get_router_index()].get_handlers().find(Method::DELETE) != nullptr);
    };
};

suite<"ApiRouter metadata builders"> api_router_metadata_suite = [] {
    "summary/description/tags land in the Registry entry for the current method"_test = [] {
        core::router::RouterContext<> ctx;
        auto index_before = Registry::get_routes().size();

        ApiRouter router = ApiRouter{ctx, "/r-meta"}
                               .get(noop_handler())
                               .summary("List")
                               .description("desc")
                               .tags({"a", "b"});

        auto &meta = Registry::at(index_before);
        expect(meta.get_path() == "r-meta");
        auto &op = meta.get_operation(std::to_underlying(Method::GET));
        expect(op.get_summary() == "List");
        expect(op.get_description() == "desc");
        expect(op.get_tags().size() == 2U);
        // getRouterNumber() reflects the freshly registered router's own number.
        expect(router.getRouterNumber() == meta.get_router_number());
    };

    "body<T>() attaches a required application/json request body in the Registry entry"_test = [] {
        core::router::RouterContext<> ctx;
        auto index_before = Registry::get_routes().size();

        ApiRouter router = ApiRouter{ctx, "/r-body"}.post(noop_handler()).body<int>();

        auto &meta = Registry::at(index_before);
        auto &op = meta.get_operation(std::to_underlying(Method::POST));
        expect(op.get_request_body() != nullptr) << fatal;
        expect(op.get_request_body()->get_required());
        expect(op.get_request_body()->get_content().contains("application/json"));
    };

    "response<T>() registers a response entry in the Registry entry"_test = [] {
        core::router::RouterContext<> ctx;
        auto index_before = Registry::get_routes().size();

        ApiRouter router = ApiRouter{ctx, "/r-resp"}.get(noop_handler()).response<int>(404, "Not Found");

        auto &meta = Registry::at(index_before);
        auto &op = meta.get_operation(std::to_underlying(Method::GET));
        expect(op.get_responses().contains("404"));
        expect(op.get_responses().at("404").get_description() == "Not Found");
    };
};

suite<"ApiRouter nesting and terminal moves"> api_router_nesting_suite = [] {
    "getRouterNumber returns the router's own claimed number"_test = [] {
        core::router::RouterContext<> ctx;
        ApiRouter router{ctx, "/r-num"};
        expect(router.getRouterNumber() > 0U);
    };

    "into_router hands off the wrapped core::router::Router"_test = [] {
        core::router::RouterContext<> ctx;
        ApiRouter router{ctx, "/r-into"};
        auto router_number = router.getRouterNumber();

        core::router::Router<> plain_router = std::move(router).into_router();
        expect(plain_router.get_router_number() == router_number);
    };

    "add_router nests a child router and patches its Registry base_router"_test = [] {
        core::router::RouterContext<> ctx;
        auto parent_index = Registry::get_routes().size();
        ApiRouter parent{ctx, "/r-parent"};
        auto child_index = Registry::get_routes().size();
        ApiRouter child{ctx, "/r-child"};

        ApiRouter combined = std::move(parent).add_router(std::move(child));

        expect(Registry::at(child_index).get_base_router() == combined.getRouterNumber());
        expect(Registry::at(parent_index).get_router_number() == combined.getRouterNumber());
    };

    "add_route mounts a child ApiRoute, registering its own RouteMeta parented at this router"_test =
        [] {
            core::router::RouterContext<> ctx;
            ApiRouter parent{ctx, "/r-parent2"};
            auto route_index = Registry::get_routes().size();

            ApiRoute child_route = ApiRoute{"/items"}.get(noop_handler()).summary("List items");
            ApiRouter combined = std::move(parent).add_route(std::move(child_route));

            auto &meta = Registry::at(route_index);
            expect(meta.get_path() == "items");
            expect(meta.get_base_router() == combined.getRouterNumber());
            expect(meta.get_operation(std::to_underlying(Method::GET)).get_summary() ==
                   "List items");
        };
};

} // namespace utils::openapi::route_tests
#endif
