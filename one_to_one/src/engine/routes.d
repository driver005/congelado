module engine.routes;

@nogc nothrow:

import interfaces.interfaces;
import io.shared.http.http;
import core.server.server;
import core.logger.logger;
import engine.context;
import engine.handler.task;
import engine.handler.workflow;
import engine.handler.metadata;

alias Protocol = io.shared.http.Protocol;

// Registers all engine HTTP routes into router.
// PORT-NOTE: C++ used shared_ptr captured in lambdas to keep handlers alive.
// D handlers are heap-allocated via make! and owned by the caller; raw pointers
// are captured in delegates. Caller must ensure handler lifetime >= route lifetime.
void register_routes(ref RouterContext!Protocol router, ref EngineContext ctx) {
    import util.alloc : make;

    alias Route   = core.server.Route!Protocol;
    alias Router_ = core.server.Router!Protocol;

    // PORT-NOTE: C++ std::make_shared → make! (caller owns, must call dispose)
    auto task = make!(TaskHandler!Protocol)(ctx);
    auto wf   = make!(WorkflowHandler!Protocol)(ctx);
    auto meta = make!(MetadataHandler!Protocol)(ctx);

    Router_(router, "/api")
        .add_router(
            Router_(router, "/v1")
                // ── tasks ───────────────────────────────────────────────────
                .add_router(Router_(router, "/tasks")
                                .post((ref IRequest!Protocol req,
                                       ref IResponse!Protocol res) {
                                    task.create_definition(req, res);
                                })
                                .add_route(Route("/:name")
                                               .get((ref IRequest!Protocol req,
                                                     ref IResponse!Protocol res) {
                                                   task.get_definition(req, res);
                                               })
                                               .put((ref IRequest!Protocol req,
                                                     ref IResponse!Protocol res) {
                                                   task.update_definition(req, res);
                                               })
                                               .delt((ref IRequest!Protocol req,
                                                      ref IResponse!Protocol res) {
                                                   task.remove_definition(req, res);
                                               }))
                                .add_router(Router_(router, "/queue")
                                                .add_route(Route("/:type").get(
                                                    (ref IRequest!Protocol req,
                                                     ref IResponse!Protocol res) {
                                                        task.poll(req, res);
                                                    }))))
                // ── workflows ────────────────────────────────────────────────
                .add_router(
                    Router_(router, "/workflows")
                        .post((ref IRequest!Protocol req,
                               ref IResponse!Protocol res) {
                            wf.create_definition(req, res);
                        })
                        .add_route(Route("/:name")
                                       .get((ref IRequest!Protocol req,
                                             ref IResponse!Protocol res) {
                                           wf.get_definition(req, res);
                                       })
                                       .put((ref IRequest!Protocol req,
                                             ref IResponse!Protocol res) {
                                           wf.update_definition(req, res);
                                       })
                                       .delt((ref IRequest!Protocol req,
                                              ref IResponse!Protocol res) {
                                           wf.remove_definition(req, res);
                                       }))
                        .add_router(
                            Router_(router, "/exec")
                                .add_route(Route("/:id")
                                               .get((ref IRequest!Protocol req,
                                                     ref IResponse!Protocol res) {
                                                   wf.get_execution(req, res);
                                               })
                                               .delt((ref IRequest!Protocol req,
                                                      ref IResponse!Protocol res) {
                                                   wf.terminate_execution(req, res);
                                               }))))
                // ── metadata ─────────────────────────────────────────────────
                .add_router(Router_(router, "/metadata")
                                .add_route(Route("/tasks").get(
                                    (ref IRequest!Protocol req,
                                     ref IResponse!Protocol res) {
                                        meta.list_task_definitions(req, res);
                                    }))
                                .add_route(Route("/workflows").get(
                                    (ref IRequest!Protocol req,
                                     ref IResponse!Protocol res) {
                                        meta.list_workflow_definitions(req, res);
                                    }))
                                .add_route(Route("/health").get(
                                    (ref IRequest!Protocol req,
                                     ref IResponse!Protocol res) {
                                        meta.health_check(req, res);
                                    }))));
}
