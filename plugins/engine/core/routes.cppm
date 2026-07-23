export module engine:routes;

import std;
import interfaces;
import io_shared;
import core_router;
import core_logger;
import model;
import utils_openapi;
import :context;
import :task;
import :workflow;
import :metadata;

export namespace engine {

// Registers all engine HTTP routes into router, and — via ApiRoute/ApiRouter — automatically
// records OpenAPI metadata for each into utils::openapi::Registry, so a Generator elsewhere
// can produce a doc without this function (or its caller) doing anything extra.
// Handlers are owned by shared_ptr captured in each lambda — alive as long as the routes are.
void register_routes(core::router::RouterContext<> &router, EngineContext &ctx) {
    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    // one handler instance per domain, shared so every route lambda below can hold its own copy
    auto task = std::make_shared<TaskHandler>(ctx);
    auto wf = std::make_shared<WorkflowHandler>(ctx);
    auto meta = std::make_shared<MetadataHandler>(ctx);

    // task routes: CRUD on definitions, plus the queue/enqueue/result endpoints workers hit
    Router(router, "/api")
        .add_router(
            Router(router, "/v1")
                .add_router(
                    Router(router, "/tasks")
                        .post([task](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
                            task->create_definition(req, res);
                        })
                        .summary("Create task definition")
                        .body<model::TaskDef>()
                        .response<model::TaskDef>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([task](interfaces::io::IRequest &req,
                                           interfaces::io::IResponse &res) {
                                    task->get_definition(req, res);
                                })
                                .summary("Get task definition")
                                .response<model::TaskDef>()
                                .put([task](interfaces::io::IRequest &req,
                                           interfaces::io::IResponse &res) {
                                    task->update_definition(req, res);
                                })
                                .summary("Update task definition")
                                .body<model::TaskDef>()
                                .response<model::TaskDef>()
                                .delt([task](interfaces::io::IRequest &req,
                                            interfaces::io::IResponse &res) {
                                    task->remove_definition(req, res);
                                })
                                .summary("Delete task definition"))
                        .add_route(Route{"/:name/enqueue"}
                                       .post([task](interfaces::io::IRequest &req,
                                                    interfaces::io::IResponse &res) {
                                           task->enqueue_task(req, res);
                                       })
                                       .summary("Enqueue a task instance")
                                       .response<model::TaskInstance>(201, "Created"))
                        .add_router(Router(router, "/queue")
                                        .add_route(Route{"/:type"}
                                                       .get([task](interfaces::io::IRequest &req,
                                                                  interfaces::io::IResponse &res) {
                                                           task->poll(req, res);
                                                       })
                                                       .summary("Poll for a scheduled task instance")
                                                       .response<model::TaskInstance>())))
                // workflow routes: CRUD on definitions, plus start/get/terminate on executions
                .add_router(
                    Router(router, "/workflows")
                        .post([wf](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
                            wf->create_definition(req, res);
                        })
                        .summary("Create workflow definition")
                        .body<model::WorkflowDef>()
                        .response<model::WorkflowDef>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([wf](interfaces::io::IRequest &req,
                                         interfaces::io::IResponse &res) {
                                    wf->get_definition(req, res);
                                })
                                .summary("Get workflow definition")
                                .response<model::WorkflowDef>()
                                .put([wf](interfaces::io::IRequest &req,
                                         interfaces::io::IResponse &res) {
                                    wf->update_definition(req, res);
                                })
                                .summary("Update workflow definition")
                                .body<model::WorkflowDef>()
                                .response<model::WorkflowDef>()
                                .delt([wf](interfaces::io::IRequest &req,
                                          interfaces::io::IResponse &res) {
                                    wf->remove_definition(req, res);
                                })
                                .summary("Delete workflow definition"))
                        .add_router(
                            Router(router, "/exec")
                                .add_route(
                                    Route{"/:id"}
                                        .get([wf](interfaces::io::IRequest &req,
                                                 interfaces::io::IResponse &res) {
                                            wf->get_execution(req, res);
                                        })
                                        .summary("Get workflow execution")
                                        .response<model::WorkflowExecution>()
                                        .delt([wf](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                            wf->terminate_execution(req, res);
                                        })
                                        .summary("Terminate workflow execution"))))
                // metadata routes: read-only listings plus the health check — bet, no writes here
                .add_router(
                    Router(router, "/metadata")
                        .add_route(Route{"/tasks"}
                                       .get([meta](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           meta->list_task_definitions(req, res);
                                       })
                                       .summary("List task definitions")
                                       .response<std::vector<model::TaskDef>>())
                        .add_route(Route{"/workflows"}
                                       .get([meta](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           meta->list_workflow_definitions(req, res);
                                       })
                                       .summary("List workflow definitions")
                                       .response<std::vector<model::WorkflowDef>>())
                        .add_route(Route{"/health"}
                                       .get([meta](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           meta->health_check(req, res);
                                       })
                                       .summary("Health check"))));
}

} // namespace engine
