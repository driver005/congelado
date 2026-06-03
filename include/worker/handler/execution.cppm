export module worker:execution;

import std;
import interfaces;
import model;
import :context;

export namespace worker {

// Routes registered by ExecutionHandler<Protocol>:
//
//   GET    /api/v1/worker/executions           → list_executions
//   GET    /api/v1/worker/executions/:id       → get_execution
//   DELETE /api/v1/worker/executions/:id       → cancel_execution
//
// Usage:
//   ExecutionHandler<Protocol>::bind(worker_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
template <typename Protocol>
class ExecutionHandler {
  public:
    // Inject identity before the first request arrives.
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // TODO: query engine GET /api/v1/tasks?worker_id={s_ctx->get_worker_id()}&status=IN_PROGRESS
    //       forward result JSON to caller
    static void list_executions(interfaces::IRequest<Protocol> &req,
                                interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :id (TaskId)
    //       outbound HTTP call to engine GET /api/v1/tasks/:id
    //       verify instance worker_id matches s_ctx->get_worker_id() before forwarding
    //       on mismatch: Status::NOT_FOUND
    static void get_execution(interfaces::IRequest<Protocol> &req,
                              interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :id, outbound HTTP call to engine DELETE /api/v1/tasks/:id
    //       signal local executor thread to abort if task is actively running
    static void cancel_execution(interfaces::IRequest<Protocol> &req,
                                 interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

  private:
    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker
