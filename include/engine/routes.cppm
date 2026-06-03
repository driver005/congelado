export module engine:routes;

import std;
import interfaces;
import io_shared;
import core_server;
import :context;
import :task;
import :workflow;
import :metadata;

export namespace engine {

using Protocol = io::shared::http::Protocol;

// Registers all engine HTTP routes into router.
// Handlers are owned by shared_ptr captured in each lambda — alive as long as the routes are.
void register_routes(core::server::RouterContext<Protocol> &router, EngineContext &ctx) {
    auto task = std::make_shared<TaskHandler<Protocol>>(ctx);
    auto wf   = std::make_shared<WorkflowHandler<Protocol>>(ctx);
    auto meta = std::make_shared<MetadataHandler<Protocol>>(ctx);

    // ── tasks ─────────────────────────────────────────────────────────────────
    router.add_route(core::server::Route<Protocol>{"/api/v1/tasks/:name"}.get(
        [task](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            task->get_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/tasks"}.post(
        [task](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            task->create_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/tasks/:name"}.put(
        [task](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            task->update_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/tasks/:name"}.delt(
        [task](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            task->remove_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/tasks/queue/:type"}.get(
        [task](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            task->poll(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/tasks/:id/result"}.post(
        [task](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            task->submit_result(req, res);
        }));

    // ── workflows ─────────────────────────────────────────────────────────────
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows/:name"}.get(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->get_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows"}.post(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->create_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows/:name"}.put(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->update_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows/:name"}.delt(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->remove_definition(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows/:name/start"}.post(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->start_execution(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows/exec/:id"}.get(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->get_execution(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/workflows/exec/:id"}.delt(
        [wf](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            wf->terminate_execution(req, res);
        }));

    // ── metadata ──────────────────────────────────────────────────────────────
    router.add_route(core::server::Route<Protocol>{"/api/v1/metadata/tasks"}.get(
        [meta](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            meta->list_task_definitions(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/metadata/workflows"}.get(
        [meta](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            meta->list_workflow_definitions(req, res);
        }));
    router.add_route(core::server::Route<Protocol>{"/api/v1/metadata/health"}.get(
        [meta](interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) {
            meta->health_check(req, res);
        }));
}

} // namespace engine
