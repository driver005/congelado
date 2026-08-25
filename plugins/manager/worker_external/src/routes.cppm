export module worker_runtime:routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import :poll;
import :execution;
import :status;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker {

// Registers every worker HTTP route into router, and — via ApiRoute/ApiRouter — automatically
// records OpenAPI metadata for each into utils::openapi::Registry, mirroring exactly how
// engine::register_routes() wires the engine's own routes. PollHandler/ExecutionHandler/
// StatusHandler's methods are static (bound once via their own ::bind() calls, done by the
// caller before this runs), so — unlike the engine's per-instance handler classes — they can
// be handed straight to ApiRoute::get()/post()/delt() with no per-route lambda/shared_ptr
// wrapping needed. OTel tracing is likewise automatic — core::router::RouteHandler::match()
// wraps every dispatched handler in a SERVER span on its own, same as engine::register_routes().
void register_routes(core::router::RouterContext<> &router) {
    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    // Static handlers wrapped in lambdas rather than handed over as bare function pointers —
    // passing `&StatusHandler::health_check` etc. directly into ApiRoute::get()/post() (which
    // used to work here) started crashing clang's module compiler outright (an ICE while
    // building std::function from a noexcept function pointer) once this file started pulling
    // in core_otel transitively via core_router. Lambdas sidestep the exact instantiation path
    // that trips it; behavior is identical either way.
    Router(router, "/api")
        .add_router(
            Router(router, "/v1")
                .add_router(
                    Router(router, "/worker")
                        .add_route(Route{"/health"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res,
                                              std::function<void()> send) {
                                           StatusHandler::health_check(req, res, std::move(send));
                                       })
                                       .summary("Worker liveness probe"))
                        .add_route(Route{"/info"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res,
                                              std::function<void()> send) {
                                           StatusHandler::worker_info(req, res, std::move(send));
                                       })
                                       .summary("Worker identity + registered task types"))
                        .add_route(Route{"/poll/:type"}
                                       .post([](interfaces::io::IRequest &req,
                                               interfaces::io::IResponse &res,
                                               std::function<void()> send) {
                                           PollHandler::poll(req, res, std::move(send));
                                       })
                                       .summary("Poll, execute, and submit a scheduled task "
                                               "instance of the given type"))
                        .add_route(Route{"/ack/:id"}
                                       .post([](interfaces::io::IRequest &req,
                                               interfaces::io::IResponse &res,
                                               std::function<void()> send) {
                                           PollHandler::ack(req, res, std::move(send));
                                       })
                                       .summary("Heartbeat a task so the engine doesn't time "
                                               "it out"))
                        .add_route(Route{"/executions"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res,
                                              std::function<void()> send) {
                                           ExecutionHandler::list_executions(req, res,
                                                                               std::move(send));
                                       })
                                       .summary("List this worker's in-progress task "
                                               "executions"))
                        .add_route(Route{"/executions/:id"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res,
                                              std::function<void()> send) {
                                           ExecutionHandler::get_execution(req, res,
                                                                           std::move(send));
                                       })
                                       .summary("Get a single task execution by id")
                                       .delt([](interfaces::io::IRequest &req,
                                               interfaces::io::IResponse &res,
                                               std::function<void()> send) {
                                           ExecutionHandler::cancel_execution(req, res,
                                                                              std::move(send));
                                       })
                                       .summary("Cancel a task execution by id"))));
}

} // namespace worker

#ifdef CONGELADO_TEST
namespace worker::routes_tests {
using namespace boost::ut;

using Method = interfaces::io::types::Method;

/// @brief Scans every utils::openapi::Registry entry appended since `since` for one whose
/// `method` operation carries exactly `summary` — Registry only stores each route's own path
/// segment (not the full resolved path), and several segments here (e.g. "/:type") aren't
/// unique on their own, so matching on the human-authored summary text is the reliable way to
/// confirm one specific route landed with the right method/metadata.
[[nodiscard]] bool has_route_with_summary(std::size_t since, std::uint8_t method,
                                          std::string_view summary) {
    auto &routes = utils::openapi::Registry::get_routes();
    for (std::size_t index = since; index < routes.size(); ++index) {
        // Iterate instead of unordered_map::find() — find() here segfaulted (found comparing
        // unequal to end() yet dereferencing garbage) once this ran after other suites had
        // exercised the process's allocator; iterating is exactly the pattern the only other
        // Registry::get_operations() walker (openapi_generator's doc_generator.cppm::generate())
        // already uses, and it never crashes. Router nodes (e.g. "/api", "/v1") register with
        // zero operations, so this loop is simply a no-op for them.
        for (const auto &[op_method, operation] : routes[index].get_operations()) {
            if (op_method == method && operation.get_summary() == summary) {
                return true;
            }
        }
    }
    return false;
}

suite<"register_routes"> register_routes_suite = [] {
    "populates the OpenAPI Registry with every worker route (relative to whatever's already registered)"_test =
        [] {
            core::router::RouterContext<> router;
            auto before = utils::openapi::Registry::get_routes().size();

            register_routes(router);

            expect(utils::openapi::Registry::get_routes().size() > before) << fatal;

            expect(has_route_with_summary(before, std::to_underlying(Method::GET),
                                          "Worker liveness probe"));
            expect(has_route_with_summary(before, std::to_underlying(Method::GET),
                                          "Worker identity + registered task types"));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST),
                "Poll, execute, and submit a scheduled task instance of the given type"));
            expect(has_route_with_summary(before, std::to_underlying(Method::POST),
                                          "Heartbeat a task so the engine doesn't time it out"));
            expect(has_route_with_summary(before, std::to_underlying(Method::GET),
                                          "List this worker's in-progress task executions"));
            expect(has_route_with_summary(before, std::to_underlying(Method::GET),
                                          "Get a single task execution by id"));
            expect(has_route_with_summary(before, std::to_underlying(Method::DELETE),
                                          "Cancel a task execution by id"));
        };

    "calling it twice against fresh RouterContexts just re-adds the same route set (Registry is append-only)"_test =
        [] {
            core::router::RouterContext<> first_router;
            auto before_first = utils::openapi::Registry::get_routes().size();
            register_routes(first_router);
            auto after_first = utils::openapi::Registry::get_routes().size();
            expect(after_first > before_first) << fatal;
            auto first_pass_count = after_first - before_first;

            core::router::RouterContext<> second_router;
            register_routes(second_router);
            auto after_second = utils::openapi::Registry::get_routes().size();

            expect(after_second - after_first == first_pass_count);
        };
};

} // namespace worker::routes_tests
#endif
