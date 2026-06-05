export module engine:routes;

import std;
import interfaces;
import io_shared;
import core_server;
import core_logger;
import :context;
import :task;
import :workflow;
import :metadata;

export namespace engine {

using Protocol = io::shared::http::Protocol;

// Registers all engine HTTP routes into router.
// Handlers are owned by shared_ptr captured in each lambda — alive as long as the routes are.
void register_routes(core::server::RouterContext<Protocol> &router, EngineContext &ctx) {
    using Route = core::server::Route<Protocol>;
    using Router = core::server::Router<Protocol>;

    auto task = std::make_shared<TaskHandler<Protocol>>(ctx);
    auto wf = std::make_shared<WorkflowHandler<Protocol>>(ctx);
    auto meta = std::make_shared<MetadataHandler<Protocol>>(ctx);

    Router(router, "/api")
        .add_router(
            Router(router, "/v1")
                // ── tasks ───────────────────────────────────────────────────
                .add_router(Router(router, "/tasks")
                                .post([task](interfaces::IRequest<Protocol> &req,
                                             interfaces::IResponse<Protocol> &res) {
                                    task->create_definition(req, res);
                                })
                                .add_route(Route{"/:name"}
                                               .get([task](interfaces::IRequest<Protocol> &req,
                                                           interfaces::IResponse<Protocol> &res) {
                                                   task->get_definition(req, res);
                                               })
                                               .put([task](interfaces::IRequest<Protocol> &req,
                                                           interfaces::IResponse<Protocol> &res) {
                                                   task->update_definition(req, res);
                                               })
                                               .delt([task](interfaces::IRequest<Protocol> &req,
                                                            interfaces::IResponse<Protocol> &res) {
                                                   task->remove_definition(req, res);
                                               }))
                                .add_router(Router(router, "/queue")
                                                .add_route(Route{"/:type"}.get(
                                                    [task](interfaces::IRequest<Protocol> &req,
                                                           interfaces::IResponse<Protocol> &res) {
                                                        task->poll(req, res);
                                                    }))))
                // ── workflows ────────────────────────────────────────────────
                .add_router(
                    Router(router, "/workflows")
                        .post([wf](interfaces::IRequest<Protocol> &req,
                                   interfaces::IResponse<Protocol> &res) {
                            wf->create_definition(req, res);
                        })
                        .add_route(Route{"/:name"}
                                       .get([wf](interfaces::IRequest<Protocol> &req,
                                                 interfaces::IResponse<Protocol> &res) {
                                           wf->get_definition(req, res);
                                       })
                                       .put([wf](interfaces::IRequest<Protocol> &req,
                                                 interfaces::IResponse<Protocol> &res) {
                                           wf->update_definition(req, res);
                                       })
                                       .delt([wf](interfaces::IRequest<Protocol> &req,
                                                  interfaces::IResponse<Protocol> &res) {
                                           wf->remove_definition(req, res);
                                       }))
                        .add_router(
                            Router(router, "/exec")
                                .add_route(Route{"/:id"}
                                               .get([wf](interfaces::IRequest<Protocol> &req,
                                                         interfaces::IResponse<Protocol> &res) {
                                                   wf->get_execution(req, res);
                                               })
                                               .delt([wf](interfaces::IRequest<Protocol> &req,
                                                          interfaces::IResponse<Protocol> &res) {
                                                   wf->terminate_execution(req, res);
                                               }))))
                // ── metadata ─────────────────────────────────────────────────
                .add_router(Router(router, "/metadata")
                                .add_route(Route{"/tasks"}.get(
                                    [meta](interfaces::IRequest<Protocol> &req,
                                           interfaces::IResponse<Protocol> &res) {
                                        meta->list_task_definitions(req, res);
                                    }))
                                .add_route(Route{"/workflows"}.get(
                                    [meta](interfaces::IRequest<Protocol> &req,
                                           interfaces::IResponse<Protocol> &res) {
                                        meta->list_workflow_definitions(req, res);
                                    }))
                                .add_route(Route{"/health"}.get(
                                    [meta](interfaces::IRequest<Protocol> &req,
                                           interfaces::IResponse<Protocol> &res) {
                                        meta->health_check(req, res);
                                    }))));
}

} // namespace engine
