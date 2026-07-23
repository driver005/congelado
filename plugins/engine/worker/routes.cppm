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
// wrapping needed.
void register_routes(core::router::RouterContext<> &router) {
    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    Router(router, "/api")
        .add_router(
            Router(router, "/v1")
                .add_router(
                    Router(router, "/worker")
                        .add_route(Route{"/health"}
                                       .get(&StatusHandler::health_check)
                                       .summary("Worker liveness probe"))
                        .add_route(Route{"/info"}
                                       .get(&StatusHandler::worker_info)
                                       .summary("Worker identity + registered task types"))
                        .add_route(Route{"/poll/:type"}
                                       .post(&PollHandler::poll)
                                       .summary("Poll, execute, and submit a scheduled task "
                                               "instance of the given type"))
                        .add_route(Route{"/ack/:id"}
                                       .post(&PollHandler::ack)
                                       .summary("Heartbeat a task so the engine doesn't time "
                                               "it out"))
                        .add_route(Route{"/executions"}
                                       .get(&ExecutionHandler::list_executions)
                                       .summary("List this worker's in-progress task "
                                               "executions"))
                        .add_route(Route{"/executions/:id"}
                                       .get(&ExecutionHandler::get_execution)
                                       .summary("Get a single task execution by id")
                                       .delt(&ExecutionHandler::cancel_execution)
                                       .summary("Cancel a task execution by id"))));
}

} // namespace worker
