export module worker:routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import :poll;
import :execution;
import :status;

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
                                              interfaces::io::IResponse &res) {
                                           StatusHandler::health_check(req, res);
                                       })
                                       .summary("Worker liveness probe"))
                        .add_route(Route{"/info"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res) {
                                           StatusHandler::worker_info(req, res);
                                       })
                                       .summary("Worker identity + registered task types"))
                        .add_route(Route{"/poll/:type"}
                                       .post([](interfaces::io::IRequest &req,
                                               interfaces::io::IResponse &res) {
                                           PollHandler::poll(req, res);
                                       })
                                       .summary("Poll, execute, and submit a scheduled task "
                                               "instance of the given type"))
                        .add_route(Route{"/ack/:id"}
                                       .post([](interfaces::io::IRequest &req,
                                               interfaces::io::IResponse &res) {
                                           PollHandler::ack(req, res);
                                       })
                                       .summary("Heartbeat a task so the engine doesn't time "
                                               "it out"))
                        .add_route(Route{"/executions"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res) {
                                           ExecutionHandler::list_executions(req, res);
                                       })
                                       .summary("List this worker's in-progress task "
                                               "executions"))
                        .add_route(Route{"/executions/:id"}
                                       .get([](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res) {
                                           ExecutionHandler::get_execution(req, res);
                                       })
                                       .summary("Get a single task execution by id")
                                       .delt([](interfaces::io::IRequest &req,
                                               interfaces::io::IResponse &res) {
                                           ExecutionHandler::cancel_execution(req, res);
                                       })
                                       .summary("Cancel a task execution by id"))));
}

} // namespace worker
