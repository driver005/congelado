export module engine:workflow_handler;

import std;
import interfaces;
import model;
import shared;
import :context;

export namespace engine {

// Routes registered by WorkflowHandler<Protocol>:
//
//   GET    /api/v1/workflows/:name          → get_definition
//   POST   /api/v1/workflows                → create_definition
//   PUT    /api/v1/workflows/:name          → update_definition
//   DELETE /api/v1/workflows/:name          → remove_definition
//   POST   /api/v1/workflows/:name/start    → start_execution
//   GET    /api/v1/workflows/exec/:id       → get_execution
//   DELETE /api/v1/workflows/exec/:id       → terminate_execution
//
// Usage:
//   WorkflowHandler<Protocol>::bind(engine_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
template <typename Protocol>
class WorkflowHandler {
  public:
    // Inject dependencies before the first request arrives.
    static void bind(EngineContext& ctx) noexcept { s_ctx = &ctx; }

    // TODO: parse :name from path
    //       cache-first lookup key "wf:def:{name}:{version}"
    //       on cache miss: IDatabase::query → decode WorkflowDef → re-populate cache
    //       on not found: Status::NOT_FOUND
    static void get_definition(interfaces::IRequest<Protocol>& req,
                               interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: deserialise body → WorkflowDef, validate DAG (no cycles, all task refs exist)
    //       IDatabase::insert → on success Status::CREATED
    //       version starts at 1; bump if name already exists
    static void create_definition(interfaces::IRequest<Protocol>& req,
                                  interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :name, deserialise body → updated WorkflowDef
    //       IDatabase::update, bump AuditRecord::version
    //       ICache::remove "wf:def:{name}:*"
    static void update_definition(interfaces::IRequest<Protocol>& req,
                                  interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :name, guard no RUNNING executions reference this definition
    //       IDatabase::remove → Status::NO_CONTENT
    //       ICache::remove "wf:def:{name}:*"
    static void remove_definition(interfaces::IRequest<Protocol>& req,
                                  interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :name, deserialise input_params from body
    //       generate ExecutionId via model::generate_id()
    //       IDatabase::insert WorkflowExec (status=RUNNING, timings.start=now)
    //       enqueue first ready TaskInstances → Status::ACCEPTED, body = {execution_id}
    static void start_execution(interfaces::IRequest<Protocol>& req,
                                interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :id (ExecutionId) from path
    //       cache-first lookup key "wf:exec:{id}"
    //       on cache miss: IDatabase::query → decode WorkflowExec → cache if !is_terminal
    //       on not found: Status::NOT_FOUND
    static void get_execution(interfaces::IRequest<Protocol>& req,
                              interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :id, guard !model::is_terminal(exec.status)
    //       IDatabase::update status → TERMINATED, timings.end = now
    //       ICache::remove "wf:exec:{id}"
    //       cancel all non-terminal TaskInstances under this execution
    static void terminate_execution(interfaces::IRequest<Protocol>& req,
                                    interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

  private:
    static inline EngineContext* s_ctx{nullptr};
};

} // namespace engine
