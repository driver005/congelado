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
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace engine {

// Registers all engine HTTP routes into router, and — via ApiRoute/ApiRouter — automatically
// records OpenAPI metadata for each into utils::openapi::Registry, so a Generator elsewhere
// can produce a doc without this function (or its caller) doing anything extra.
// Handlers are owned by shared_ptr captured in each lambda — alive as long as the routes are.
// OTel tracing is likewise automatic here — core::router::RouteHandler::match() wraps every
// dispatched handler in a SERVER span on its own, so nothing below needs to opt in.
//
// SECURITY: no authentication or authorization middleware is wired into any route registered
// here — full task/workflow/schedule/event-handler CRUD, workflow start/pause/resume/retry,
// bulk operations, the raw-SQL /api/v1/query endpoint, and /api/v1/admin/* are all reachable by
// any network client that can reach this port. Every other finding in this handler layer
// (SQL injection, unauthenticated Lua eval via workflow definitions, unbounded list/bulk
// endpoints) is directly amplified by this — needs an auth layer before this is safe to expose.
//
// TEST NOTE: no automated test covers the above — RouterContext<> has add_global_middleware()
// but no getter to read the registered count back out, and the built RouteHandler (only
// reachable via RouteBuilder::build(), which also needs the reserved root/fallback slots filled
// in, neither of which this function does) has the same gap. Confirming "zero auth middleware"
// structurally would need new introspection API on RouterContext/RouteHandler, which is
// production code, out of scope for a test-only pass. See admin_handler_suite in admin.cppm
// (AdminHandler::consistency()/config() driven directly with no auth setup, both reply
// normally) for the same finding pinned behaviorally instead.
void register_routes(core::router::RouterContext<>& router, EngineContext& ctx)
{
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
                        .post([task](
                                  interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                            task->create_definition(req, res, std::move(send));
                        })
                        .summary("Create task definition")
                        .body<model::TaskDef>()
                        .response<model::TaskDef>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([task](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    task->get_definition(req, res, std::move(send));
                                })
                                .summary("Get task definition")
                                .response<model::TaskDef>()
                                .put([task](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    task->update_definition(req, res, std::move(send));
                                })
                                .summary("Update task definition")
                                .body<model::TaskDef>()
                                .response<model::TaskDef>()
                                .delt([task](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    task->remove_definition(req, res, std::move(send));
                                })
                                .summary("Delete task definition")
                        )
                        .add_route(
                            Route{"/:name/enqueue"}
                                .post([task](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    task->enqueue_task(req, res, std::move(send));
                                })
                                .summary("Enqueue a task instance")
                                .response<model::TaskInstance>(201, "Created")
                        )
                        .add_route(
                            Route{"/:id/result"}
                                .post([task](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    task->submit_result(req, res, std::move(send));
                                })
                                .summary("Submit a task result")
                                .body<engine::TaskSubmitBody>()
                                .response<model::TaskInstance>()
                        )
                        .add_route(
                            Route{"/:id/heartbeat"}
                                .patch([task](
                                           interfaces::io::IRequest& req,
                                           interfaces::io::IResponse& res,
                                           std::function<void()> send
                                       ) {
                                    task->heartbeat(req, res, std::move(send));
                                })
                                .summary("Heartbeat an in-progress task instance")
                                .response<model::TaskInstance>()
                        )
                        .add_route(
                            Route{"/queue_sizes"}
                                .get([task](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    task->queue_sizes(req, res, std::move(send));
                                })
                                .summary("Scheduled task counts grouped by worker type")
                        )
                        .add_route(
                            Route{"/queue_polldata"}
                                .get([task](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    task->queue_polldata(req, res, std::move(send));
                                })
                                .summary("Last-poll heartbeat per worker type")
                                .response<std::vector<model::PollData>>()
                        )
                        .add_route(
                            Route{"/queue_requeue/:type"}
                                .post([task](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    task->queue_requeue(req, res, std::move(send));
                                })
                                .summary(
                                    "Force-requeue stuck IN_PROGRESS instances of a worker type"
                                )
                        )
                        .add_route(
                            Route{"/search"}
                                .post([search](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    search->search_tasks(req, res, std::move(send));
                                })
                                .summary("Search indexed task projections")
                                .body<engine::SearchRequestBody>()
                        )
                        .add_router(Router(router, "/queue")
                                        .add_route(
                                            Route{"/:type"}
                                                .get([task](
                                                         interfaces::io::IRequest& req,
                                                         interfaces::io::IResponse& res,
                                                         std::function<void()> send
                                                     ) {
                                                    task->poll(req, res, std::move(send));
                                                })
                                                .summary("Poll for a scheduled task instance")
                                                .response<model::TaskInstance>()
                                        )
                                        .add_route(
                                            Route{"/:type/domain/:domain"}
                                                .get([task](
                                                         interfaces::io::IRequest& req,
                                                         interfaces::io::IResponse& res,
                                                         std::function<void()> send
                                                     ) {
                                                    task->poll_domain(req, res, std::move(send));
                                                })
                                                .summary(
                                                    "Poll for a scheduled task instance, "
                                                    "scoped to an isolation domain"
                                                )
                                                .response<model::TaskInstance>()
                                        ))
                )
                // workflow routes: CRUD on definitions, plus start/get/terminate on executions
                .add_router(
                    Router(router, "/workflows")
                        .post([wf](
                                  interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                            wf->create_definition(req, res, std::move(send));
                        })
                        .summary("Create workflow definition")
                        .body<model::WorkflowDef>()
                        .response<model::WorkflowDef>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([wf](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    wf->get_definition(req, res, std::move(send));
                                })
                                .summary("Get workflow definition")
                                .response<model::WorkflowDef>()
                                .put([wf](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    wf->update_definition(req, res, std::move(send));
                                })
                                .summary("Update workflow definition")
                                .body<model::WorkflowDef>()
                                .response<model::WorkflowDef>()
                                .delt([wf](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    wf->remove_definition(req, res, std::move(send));
                                })
                                .summary("Delete workflow definition")
                        )
                        .add_route(
                            Route{"/:name/start"}
                                .post([wf](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    wf->start_execution(req, res, std::move(send));
                                })
                                .summary("Start workflow execution")
                                .response<model::WorkflowExecution>(202, "Accepted")
                        )
                        .add_router(
                            Router(router, "/exec")
                                .add_route(
                                    Route{"/:id"}
                                        .get([wf](
                                                 interfaces::io::IRequest& req,
                                                 interfaces::io::IResponse& res,
                                                 std::function<void()> send
                                             ) {
                                            wf->get_execution(req, res, std::move(send));
                                        })
                                        .summary("Get workflow execution")
                                        .response<model::WorkflowExecution>()
                                        .delt([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->terminate_execution(req, res, std::move(send));
                                        })
                                        .summary("Terminate workflow execution")
                                )
                                .add_route(
                                    Route{"/:id/pause"}
                                        .post([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->pause_execution(req, res, std::move(send));
                                        })
                                        .summary("Pause workflow execution")
                                )
                                .add_route(
                                    Route{"/:id/resume"}
                                        .post([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->resume_execution(req, res, std::move(send));
                                        })
                                        .summary("Resume workflow execution")
                                )
                                .add_route(
                                    Route{"/:id/retry"}
                                        .post([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->retry_execution(req, res, std::move(send));
                                        })
                                        .summary("Retry workflow execution")
                                )
                                .add_route(
                                    Route{"/:id/restart"}
                                        .post([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->restart_execution(req, res, std::move(send));
                                        })
                                        .summary("Restart workflow execution")
                                )
                                .add_route(
                                    Route{"/:id/rerun"}
                                        .post([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->rerun_execution(req, res, std::move(send));
                                        })
                                        .summary("Rerun workflow execution from a node")
                                        .body<engine::RerunBody>()
                                )
                                .add_route(
                                    Route{"/:id/signal"}
                                        .post([wf](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            wf->signal_execution(req, res, std::move(send));
                                        })
                                        .summary("Signal a waiting workflow instance")
                                        .body<engine::SignalBody>()
                                )
                        )
                        // bulk ops: same actions as the single-exec routes above, applied
                        // sequentially across a list of exec_ids from the request body.
                        .add_router(Router(router, "/bulk")
                                        .add_route(
                                            Route{"/pause"}
                                                .post([wf](
                                                          interfaces::io::IRequest& req,
                                                          interfaces::io::IResponse& res,
                                                          std::function<void()> send
                                                      ) {
                                                    wf->bulk_pause(req, res, std::move(send));
                                                })
                                                .summary("Bulk-pause workflow executions")
                                                .body<engine::BulkExecIdsBody>()
                                                .response<std::vector<engine::BulkResult>>()
                                        )
                                        .add_route(
                                            Route{"/resume"}
                                                .post([wf](
                                                          interfaces::io::IRequest& req,
                                                          interfaces::io::IResponse& res,
                                                          std::function<void()> send
                                                      ) {
                                                    wf->bulk_resume(req, res, std::move(send));
                                                })
                                                .summary("Bulk-resume workflow executions")
                                                .body<engine::BulkExecIdsBody>()
                                                .response<std::vector<engine::BulkResult>>()
                                        )
                                        .add_route(
                                            Route{"/retry"}
                                                .post([wf](
                                                          interfaces::io::IRequest& req,
                                                          interfaces::io::IResponse& res,
                                                          std::function<void()> send
                                                      ) {
                                                    wf->bulk_retry(req, res, std::move(send));
                                                })
                                                .summary("Bulk-retry workflow executions")
                                                .body<engine::BulkExecIdsBody>()
                                                .response<std::vector<engine::BulkResult>>()
                                        )
                                        .add_route(
                                            Route{"/restart"}
                                                .post([wf](
                                                          interfaces::io::IRequest& req,
                                                          interfaces::io::IResponse& res,
                                                          std::function<void()> send
                                                      ) {
                                                    wf->bulk_restart(req, res, std::move(send));
                                                })
                                                .summary("Bulk-restart workflow executions")
                                                .body<engine::BulkExecIdsBody>()
                                                .response<std::vector<engine::BulkResult>>()
                                        )
                                        .add_route(
                                            Route{"/terminate"}
                                                .post([wf](
                                                          interfaces::io::IRequest& req,
                                                          interfaces::io::IResponse& res,
                                                          std::function<void()> send
                                                      ) {
                                                    wf->bulk_terminate(req, res, std::move(send));
                                                })
                                                .summary("Bulk-terminate workflow executions")
                                                .body<engine::BulkExecIdsBody>()
                                                .response<std::vector<engine::BulkResult>>()
                                        )
                                        .add_route(
                                            Route{"/remove"}
                                                .post([wf](
                                                          interfaces::io::IRequest& req,
                                                          interfaces::io::IResponse& res,
                                                          std::function<void()> send
                                                      ) {
                                                    wf->bulk_remove(req, res, std::move(send));
                                                })
                                                .summary("Bulk-remove workflow executions")
                                                .body<engine::BulkExecIdsBody>()
                                                .response<std::vector<engine::BulkResult>>()
                                        ))
                )
                // metadata routes: read-only listings plus the health check — bet, no writes
                // here
                .add_router(
                    Router(router, "/metadata")
                        .add_route(
                            Route{"/tasks"}
                                .get([meta](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    meta->list_task_definitions(req, res, std::move(send));
                                })
                                .summary("List task definitions")
                                .response<std::vector<model::TaskDef>>()
                        )
                        .add_route(
                            Route{"/workflows"}
                                .get([meta](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    meta->list_workflow_definitions(req, res, std::move(send));
                                })
                                .summary("List workflow definitions")
                                .response<std::vector<model::WorkflowDef>>()
                        )
                        .add_route(
                            Route{"/health"}
                                .get([meta](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    meta->health_check(req, res, std::move(send));
                                })
                                .summary("Health check")
                        )
                )
                // query route: runs a read-only (SELECT-only, see QueryHandler::is_select())
                // SQL query against the configured database backend — result shape is whatever
                // the query returns, not a fixed model, so no .response<T>() here (same as
                // health_check above).
                .add_router(Router(router, "/query")
                                .post([query](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    query->run_query(req, res, std::move(send));
                                })
                                .summary("Run a read-only SQL query"))
                // event-handler routes: CRUD for the EventHandler subscriptions
                // Orchestrator::publish_event() fans events out to. Underscore, not hyphen —
                // the OpenAPI client-SDK generator (build.cc) turns each path segment straight
                // into a C++ namespace identifier, and a hyphen there is invalid C++.
                .add_router(
                    Router(router, "/event_handlers")
                        .get([events](
                                 interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                            events->list_handlers(req, res, std::move(send));
                        })
                        .summary("List event handlers")
                        .response<std::vector<model::EventHandler>>()
                        .post([events](
                                  interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                            events->create_handler(req, res, std::move(send));
                        })
                        .summary("Create event handler")
                        .body<model::EventHandler>()
                        .response<model::EventHandler>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([events](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    events->get_handler(req, res, std::move(send));
                                })
                                .summary("Get event handler")
                                .response<model::EventHandler>()
                                .put([events](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    events->update_handler(req, res, std::move(send));
                                })
                                .summary("Update event handler")
                                .body<model::EventHandler>()
                                .response<model::EventHandler>()
                                .delt([events](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    events->remove_handler(req, res, std::move(send));
                                })
                                .summary("Delete event handler")
                        )
                )
                // generic external-signal completion endpoint — see
                // TaskHandler::queue_update()'s own docs for why this exists alongside POST
                // /tasks/:id/result.
                .add_router(
                    Router(router, "/queue")
                        .add_route(
                            Route{"/update"}
                                .post([task](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    task->queue_update(req, res, std::move(send));
                                })
                                .summary("Complete/fail a task instance by exec_id + node_ref")
                                .body<engine::QueueUpdateBody>()
                        )
                )
                // schedule routes: CRUD for cron-driven WorkflowSchedules, plus pause/resume
                // and a next-few-runs preview.
                .add_router(
                    Router(router, "/schedules")
                        .get([schedules](
                                 interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                            schedules->list_schedules(req, res, std::move(send));
                        })
                        .summary("List workflow schedules")
                        .response<std::vector<model::WorkflowSchedule>>()
                        .post([schedules](
                                  interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                            schedules->create_schedule(req, res, std::move(send));
                        })
                        .summary("Create workflow schedule")
                        .body<model::WorkflowSchedule>()
                        .response<model::WorkflowSchedule>(201, "Created")
                        .add_route(
                            Route{"/:name"}
                                .get([schedules](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    schedules->get_schedule(req, res, std::move(send));
                                })
                                .summary("Get workflow schedule")
                                .response<model::WorkflowSchedule>()
                                .put([schedules](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    schedules->update_schedule(req, res, std::move(send));
                                })
                                .summary("Update workflow schedule")
                                .body<model::WorkflowSchedule>()
                                .response<model::WorkflowSchedule>()
                                .delt([schedules](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    schedules->remove_schedule(req, res, std::move(send));
                                })
                                .summary("Delete workflow schedule")
                        )
                        .add_route(
                            Route{"/:name/pause"}
                                .post([schedules](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    schedules->pause_schedule(req, res, std::move(send));
                                })
                                .summary("Pause workflow schedule")
                        )
                        .add_route(
                            Route{"/:name/resume"}
                                .post([schedules](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    schedules->resume_schedule(req, res, std::move(send));
                                })
                                .summary("Resume workflow schedule")
                        )
                        .add_route(
                            Route{"/:name/next_few_runs"}
                                .get([schedules](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    schedules->next_few_runs(req, res, std::move(send));
                                })
                                .summary("Preview the next few fire times")
                                .response<std::vector<engine::ScheduleNextRun>>()
                        )
                )
                // admin routes: self-healing reconcile + a read-only backend-config snapshot.
                .add_router(
                    Router(router, "/admin")
                        .add_route(
                            Route{"/consistency/:exec_id"}
                                .post([admin](
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res, std::function<void()> send
                                      ) {
                                    admin->consistency(req, res, std::move(send));
                                })
                                .summary("Re-run DAG-advance for a stuck execution")
                        )
                        .add_route(
                            Route{"/config"}
                                .get([admin](
                                         interfaces::io::IRequest& req,
                                         interfaces::io::IResponse& res, std::function<void()> send
                                     ) {
                                    admin->config(req, res, std::move(send));
                                })
                                .summary("Show wired-in backend configuration")
                                .response<engine::AdminConfig>()
                        )
                )
                // workflow (singular, matching Conductor's own asymmetric route naming) search
                // — separate top-level path from "/workflows" above, not nested under it.
                .add_router(Router(router, "/workflow")
                                .add_route(
                                    Route{"/search"}
                                        .post([search](
                                                  interfaces::io::IRequest& req,
                                                  interfaces::io::IResponse& res,
                                                  std::function<void()> send
                                              ) {
                                            search->search_workflows(req, res, std::move(send));
                                        })
                                        .summary("Search indexed workflow projections")
                                        .body<engine::SearchRequestBody>()
                                ))
        );
}

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::routes_tests {
using namespace boost::ut;

using Method = interfaces::io::types::Method;

/// @brief Scans every utils::openapi::Registry entry appended since `since` for one whose
/// `method` operation carries exactly `summary` — same helper/reasoning as
/// worker::routes_tests::has_route_with_summary() in
/// plugins/manager/worker_external/src/routes.cppm (iterates get_operations() rather than
/// calling unordered_map::find() on it — find() there segfaulted once the Registry had been
/// exercised by other suites first; iterating is the pattern doc_generator.cppm's own
/// Registry walk already uses, and it never crashes).
[[nodiscard]] bool
has_route_with_summary(std::size_t since, std::uint8_t method, std::string_view summary)
{
    auto& routes = utils::openapi::Registry::get_routes();
    for (std::size_t index = since; index < routes.size(); ++index) {
        for (const auto& [op_method, operation]: routes[index].get_operations()) {
            if (op_method == method && operation.get_summary() == summary) {
                return true;
            }
        }
    }
    return false;
}

suite<"engine register_routes"> register_routes_suite = [] {
    "populates the OpenAPI Registry with a representative route from every handler domain (relative to whatever's already registered)"_test =
        [] {
            core::router::RouterContext<> router;
            EngineContext ctx;
            auto before = utils::openapi::Registry::get_routes().size();

            register_routes(router, ctx);

            expect(utils::openapi::Registry::get_routes().size() > before) << fatal;

            // task routes
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Create task definition"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::GET), "Get task definition"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Enqueue a task instance"
            ));
            // workflow + execution routes
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Create workflow definition"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Start workflow execution"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::DELETE), "Terminate workflow execution"
            ));
            // bulk ops
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Bulk-terminate workflow executions"
            ));
            // metadata + health
            expect(has_route_with_summary(
                before, std::to_underlying(Method::GET), "List task definitions"
            ));
            expect(has_route_with_summary(before, std::to_underlying(Method::GET), "Health check"));
            // raw-SQL query endpoint
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Run a read-only SQL query"
            ));
            // event-handler CRUD
            expect(has_route_with_summary(
                before, std::to_underlying(Method::GET), "List event handlers"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::DELETE), "Delete event handler"
            ));
            // generic queue/update completion endpoint
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST),
                "Complete/fail a task instance by exec_id + node_ref"
            ));
            // schedule CRUD
            expect(has_route_with_summary(
                before, std::to_underlying(Method::GET), "List workflow schedules"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Pause workflow schedule"
            ));
            // admin
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Re-run DAG-advance for a stuck execution"
            ));
            expect(has_route_with_summary(
                before, std::to_underlying(Method::GET), "Show wired-in backend configuration"
            ));
            // workflow (singular) search
            expect(has_route_with_summary(
                before, std::to_underlying(Method::POST), "Search indexed workflow projections"
            ));
        };

    "calling it twice against fresh RouterContexts just re-adds the same route set (Registry is append-only)"_test =
        [] {
            core::router::RouterContext<> first_router;
            EngineContext first_ctx;
            auto before_first = utils::openapi::Registry::get_routes().size();
            register_routes(first_router, first_ctx);
            auto after_first = utils::openapi::Registry::get_routes().size();
            expect(after_first > before_first) << fatal;
            auto first_pass_count = after_first - before_first;

            core::router::RouterContext<> second_router;
            EngineContext second_ctx;
            register_routes(second_router, second_ctx);
            auto after_second = utils::openapi::Registry::get_routes().size();

            expect(after_second - after_first == first_pass_count);
        };
};

} // namespace engine::routes_tests
#endif
