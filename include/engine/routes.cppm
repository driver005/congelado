export module engine:routes;

import std;
import interfaces;
import io_shared;
import core_router;
import core_logger;
import :context;
import :task;
import :workflow;
import :metadata;

export namespace engine {

// Registers all engine HTTP routes into router.
// Handlers are owned by shared_ptr captured in each lambda — alive as long as the routes are.
void register_routes(core::router::RouterContext<> &router, EngineContext &ctx) {
    using Route = core::router::Route<>;
    using Router = core::router::Router<>;

    auto task = std::make_shared<TaskHandler>(ctx);
    auto wf = std::make_shared<WorkflowHandler>(ctx);
    auto meta = std::make_shared<MetadataHandler>(ctx);

    Router(router, "/api")
        .add_router(
            Router(router, "/v1")
                .add_router(Router(router, "/tasks")
                                .post([task](interfaces::io::IRequest &req,
                                             interfaces::io::IResponse &res) {
                                    task->create_definition(req, res);
                                })
                                .add_route(Route{"/:name"}
                                               .get([task](interfaces::io::IRequest &req,
                                                           interfaces::io::IResponse &res) {
                                                   task->get_definition(req, res);
                                               })
                                               .put([task](interfaces::io::IRequest &req,
                                                           interfaces::io::IResponse &res) {
                                                   task->update_definition(req, res);
                                               })
                                               .delt([task](interfaces::io::IRequest &req,
                                                            interfaces::io::IResponse &res) {
                                                   task->remove_definition(req, res);
                                               }))
                                .add_router(Router(router, "/queue")
                                                .add_route(Route{"/:type"}.get(
                                                    [task](interfaces::io::IRequest &req,
                                                           interfaces::io::IResponse &res) {
                                                        task->poll(req, res);
                                                    }))))
                .add_router(
                    Router(router, "/workflows")
                        .post([wf](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
                            wf->create_definition(req, res);
                        })
                        .add_route(Route{"/:name"}
                                       .get([wf](interfaces::io::IRequest &req,
                                                 interfaces::io::IResponse &res) {
                                           wf->get_definition(req, res);
                                       })
                                       .put([wf](interfaces::io::IRequest &req,
                                                 interfaces::io::IResponse &res) {
                                           wf->update_definition(req, res);
                                       })
                                       .delt([wf](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           wf->remove_definition(req, res);
                                       }))
                        .add_router(Router(router, "/exec")
                                        .add_route(Route{"/:id"}
                                                       .get([wf](interfaces::io::IRequest &req,
                                                                 interfaces::io::IResponse &res) {
                                                           wf->get_execution(req, res);
                                                       })
                                                       .delt([wf](interfaces::io::IRequest &req,
                                                                  interfaces::io::IResponse &res) {
                                                           wf->terminate_execution(req, res);
                                                       }))))
                .add_router(
                    Router(router, "/metadata")
                        .add_route(Route{"/tasks"}.get(
                            [meta](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
                                meta->list_task_definitions(req, res);
                            }))
                        .add_route(Route{"/workflows"}.get(
                            [meta](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
                                meta->list_workflow_definitions(req, res);
                            }))
                        .add_route(Route{"/health"}.get(
                            [meta](interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
                                meta->health_check(req, res);
                            }))));
}

} // namespace engine
