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
import :query;
import :event_handler;
import :schedule_handler;
import :admin_handler;
import :search_handler;

export namespace engine {

// Registers all engine HTTP routes into router, and — via ApiRoute/ApiRouter — automatically
// records OpenAPI metadata for each into utils::openapi::Registry, so a Generator elsewhere
// can produce a doc without this function (or its caller) doing anything extra.
// Handlers are owned by shared_ptr captured in each lambda — alive as long as the routes are.
// OTel tracing is likewise automatic here — core::router::RouteHandler::match() wraps every
// dispatched handler in a SERVER span on its own, so nothing below needs to opt in.
void register_routes(core::router::RouterContext<> &router, EngineContext &ctx) {
    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    // one handler instance per domain, shared so every route lambda below can hold its own copy
    auto task = std::make_shared<TaskHandler>(ctx);
    auto wf = std::make_shared<WorkflowHandler>(ctx);
    auto meta = std::make_shared<MetadataHandler>(ctx);
    auto query = std::make_shared<QueryHandler>(ctx);
    auto events = std::make_shared<EventHandlerHandler>(ctx);
    auto schedules = std::make_shared<ScheduleHandler>(ctx);
    auto admin = std::make_shared<AdminHandler>(ctx);
    auto search = std::make_shared<SearchHandler>(ctx);

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
                        .add_route(Route{"/:id/result"}
                                       .post([task](interfaces::io::IRequest &req,
                                                    interfaces::io::IResponse &res) {
                                           task->submit_result(req, res);
                                       })
                                       .summary("Submit a task result")
                                       .body<engine::TaskSubmitBody>()
                                       .response<model::TaskInstance>())
                        .add_route(Route{"/:id/heartbeat"}
                                       .patch([task](interfaces::io::IRequest &req,
                                                     interfaces::io::IResponse &res) {
                                           task->heartbeat(req, res);
                                       })
                                       .summary("Heartbeat an in-progress task instance")
                                       .response<model::TaskInstance>())
                        .add_route(Route{"/queue_sizes"}
                                       .get([task](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           task->queue_sizes(req, res);
                                       })
                                       .summary("Scheduled task counts grouped by worker type"))
                        .add_route(Route{"/queue_polldata"}
                                       .get([task](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           task->queue_polldata(req, res);
                                       })
                                       .summary("Last-poll heartbeat per worker type")
                                       .response<std::vector<model::PollData>>())
                        .add_route(Route{"/queue_requeue/:type"}
                                       .post([task](interfaces::io::IRequest &req,
                                                    interfaces::io::IResponse &res) {
                                           task->queue_requeue(req, res);
                                       })
                                       .summary("Force-requeue stuck IN_PROGRESS instances of a worker type"))
                        .add_route(Route{"/search"}
                                       .post([search](interfaces::io::IRequest &req,
                                                      interfaces::io::IResponse &res) {
                                           search->search_tasks(req, res);
                                       })
                                       .summary("Search indexed task projections")
                                       .body<engine::SearchRequestBody>())
                        .add_router(Router(router, "/queue")
                                        .add_route(Route{"/:type"}
                                                       .get([task](interfaces::io::IRequest &req,
                                                                  interfaces::io::IResponse &res) {
                                                           task->poll(req, res);
                                                       })
                                                       .summary("Poll for a scheduled task instance")
                                                       .response<model::TaskInstance>())
                                        .add_route(Route{"/:type/domain/:domain"}
                                                       .get([task](interfaces::io::IRequest &req,
                                                                  interfaces::io::IResponse &res) {
                                                           task->poll_domain(req, res);
                                                       })
                                                       .summary("Poll for a scheduled task instance, scoped to an isolation domain")
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
                        .add_route(Route{"/:name/start"}
                                       .post([wf](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                           wf->start_execution(req, res);
                                       })
                                       .summary("Start workflow execution")
                                       .response<model::WorkflowExecution>(202, "Accepted"))
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
                                        .summary("Terminate workflow execution"))
                                .add_route(Route{"/:id/pause"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->pause_execution(req, res);
                                               })
                                               .summary("Pause workflow execution"))
                                .add_route(Route{"/:id/resume"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->resume_execution(req, res);
                                               })
                                               .summary("Resume workflow execution"))
                                .add_route(Route{"/:id/retry"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->retry_execution(req, res);
                                               })
                                               .summary("Retry workflow execution"))
                                .add_route(Route{"/:id/restart"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->restart_execution(req, res);
                                               })
                                               .summary("Restart workflow execution"))
                                .add_route(Route{"/:id/rerun"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->rerun_execution(req, res);
                                               })
                                               .summary("Rerun workflow execution from a node")
                                               .body<engine::RerunBody>())
                                .add_route(Route{"/:id/signal"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->signal_execution(req, res);
                                               })
                                               .summary("Signal a waiting workflow instance")
                                               .body<engine::SignalBody>()))
                        // bulk ops: same actions as the single-exec routes above, applied
                        // sequentially across a list of exec_ids from the request body.
                        .add_router(
                            Router(router, "/bulk")
                                .add_route(Route{"/pause"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->bulk_pause(req, res);
                                               })
                                               .summary("Bulk-pause workflow executions")
                                               .body<engine::BulkExecIdsBody>()
                                               .response<std::vector<engine::BulkResult>>())
                                .add_route(Route{"/resume"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->bulk_resume(req, res);
                                               })
                                               .summary("Bulk-resume workflow executions")
                                               .body<engine::BulkExecIdsBody>()
                                               .response<std::vector<engine::BulkResult>>())
                                .add_route(Route{"/retry"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->bulk_retry(req, res);
                                               })
                                               .summary("Bulk-retry workflow executions")
                                               .body<engine::BulkExecIdsBody>()
                                               .response<std::vector<engine::BulkResult>>())
                                .add_route(Route{"/restart"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->bulk_restart(req, res);
                                               })
                                               .summary("Bulk-restart workflow executions")
                                               .body<engine::BulkExecIdsBody>()
                                               .response<std::vector<engine::BulkResult>>())
                                .add_route(Route{"/terminate"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->bulk_terminate(req, res);
                                               })
                                               .summary("Bulk-terminate workflow executions")
                                               .body<engine::BulkExecIdsBody>()
                                               .response<std::vector<engine::BulkResult>>())
                                .add_route(Route{"/remove"}
                                               .post([wf](interfaces::io::IRequest &req,
                                                          interfaces::io::IResponse &res) {
                                                   wf->bulk_remove(req, res);
                                               })
                                               .summary("Bulk-remove workflow executions")
                                               .body<engine::BulkExecIdsBody>()
                                               .response<std::vector<engine::BulkResult>>())))
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
                                       .summary("Health check")))
                // query route: runs a read-only (SELECT-only, see QueryHandler::is_select())
                // SQL query against the configured database backend — result shape is whatever
                // the query returns, not a fixed model, so no .response<T>() here (same as
                // health_check above).
                .add_router(
                    Router(router, "/query")
                        .post([query](interfaces::io::IRequest &req,
                                     interfaces::io::IResponse &res) {
                            query->run_query(req, res);
                        })
                        .summary("Run a read-only SQL query"))
                // event-handler routes: CRUD for the EventHandler subscriptions
                // Orchestrator::publish_event() fans events out to. Underscore, not hyphen — the
                // OpenAPI client-SDK generator (build.cc) turns each path segment straight into a
                // C++ namespace identifier, and a hyphen there is invalid C++.
                .add_router(
                    Router(router, "/event_handlers")
                        .get([events](interfaces::io::IRequest &req,
                                     interfaces::io::IResponse &res) {
                            events->list_handlers(req, res);
                        })
                        .summary("List event handlers")
                        .response<std::vector<model::EventHandler>>()
                        .post([events](interfaces::io::IRequest &req,
                                      interfaces::io::IResponse &res) {
                            events->create_handler(req, res);
                        })
                        .summary("Create event handler")
                        .body<model::EventHandler>()
                        .response<model::EventHandler>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([events](interfaces::io::IRequest &req,
                                             interfaces::io::IResponse &res) {
                                    events->get_handler(req, res);
                                })
                                .summary("Get event handler")
                                .response<model::EventHandler>()
                                .put([events](interfaces::io::IRequest &req,
                                             interfaces::io::IResponse &res) {
                                    events->update_handler(req, res);
                                })
                                .summary("Update event handler")
                                .body<model::EventHandler>()
                                .response<model::EventHandler>()
                                .delt([events](interfaces::io::IRequest &req,
                                              interfaces::io::IResponse &res) {
                                    events->remove_handler(req, res);
                                })
                                .summary("Delete event handler")))
                // generic external-signal completion endpoint — see TaskHandler::queue_update()'s
                // own docs for why this exists alongside POST /tasks/:id/result.
                .add_router(
                    Router(router, "/queue")
                        .add_route(Route{"/update"}
                                       .post([task](interfaces::io::IRequest &req,
                                                    interfaces::io::IResponse &res) {
                                           task->queue_update(req, res);
                                       })
                                       .summary("Complete/fail a task instance by exec_id + node_ref")
                                       .body<engine::QueueUpdateBody>()))
                // schedule routes: CRUD for cron-driven WorkflowSchedules, plus pause/resume and
                // a next-few-runs preview.
                .add_router(
                    Router(router, "/schedules")
                        .get([schedules](interfaces::io::IRequest &req,
                                        interfaces::io::IResponse &res) {
                            schedules->list_schedules(req, res);
                        })
                        .summary("List workflow schedules")
                        .response<std::vector<model::WorkflowSchedule>>()
                        .post([schedules](interfaces::io::IRequest &req,
                                         interfaces::io::IResponse &res) {
                            schedules->create_schedule(req, res);
                        })
                        .summary("Create workflow schedule")
                        .body<model::WorkflowSchedule>()
                        .response<model::WorkflowSchedule>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([schedules](interfaces::io::IRequest &req,
                                                 interfaces::io::IResponse &res) {
                                    schedules->get_schedule(req, res);
                                })
                                .summary("Get workflow schedule")
                                .response<model::WorkflowSchedule>()
                                .put([schedules](interfaces::io::IRequest &req,
                                                 interfaces::io::IResponse &res) {
                                    schedules->update_schedule(req, res);
                                })
                                .summary("Update workflow schedule")
                                .body<model::WorkflowSchedule>()
                                .response<model::WorkflowSchedule>()
                                .delt([schedules](interfaces::io::IRequest &req,
                                                  interfaces::io::IResponse &res) {
                                    schedules->remove_schedule(req, res);
                                })
                                .summary("Delete workflow schedule"))
                        .add_route(Route{"/:name/pause"}
                                       .post([schedules](interfaces::io::IRequest &req,
                                                         interfaces::io::IResponse &res) {
                                           schedules->pause_schedule(req, res);
                                       })
                                       .summary("Pause workflow schedule"))
                        .add_route(Route{"/:name/resume"}
                                       .post([schedules](interfaces::io::IRequest &req,
                                                         interfaces::io::IResponse &res) {
                                           schedules->resume_schedule(req, res);
                                       })
                                       .summary("Resume workflow schedule"))
                        .add_route(Route{"/:name/next_few_runs"}
                                       .get([schedules](interfaces::io::IRequest &req,
                                                        interfaces::io::IResponse &res) {
                                           schedules->next_few_runs(req, res);
                                       })
                                       .summary("Preview the next few fire times")
                                       .response<std::vector<engine::ScheduleNextRun>>()))
                // admin routes: self-healing reconcile + a read-only backend-config snapshot.
                .add_router(
                    Router(router, "/admin")
                        .add_route(Route{"/consistency/:exec_id"}
                                       .post([admin](interfaces::io::IRequest &req,
                                                     interfaces::io::IResponse &res) {
                                           admin->consistency(req, res);
                                       })
                                       .summary("Re-run DAG-advance for a stuck execution"))
                        .add_route(Route{"/config"}
                                       .get([admin](interfaces::io::IRequest &req,
                                                    interfaces::io::IResponse &res) {
                                           admin->config(req, res);
                                       })
                                       .summary("Show wired-in backend configuration")
                                       .response<engine::AdminConfig>()))
                // workflow (singular, matching Conductor's own asymmetric route naming) search —
                // separate top-level path from "/workflows" above, not nested under it.
                .add_router(
                    Router(router, "/workflow")
                        .add_route(Route{"/search"}
                                       .post([search](interfaces::io::IRequest &req,
                                                      interfaces::io::IResponse &res) {
                                           search->search_workflows(req, res);
                                       })
                                       .summary("Search indexed workflow projections")
                                       .body<engine::SearchRequestBody>())));
}

} // namespace engine
