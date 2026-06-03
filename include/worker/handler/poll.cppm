export module worker:poll;

import std;
import interfaces;
import :context;

export namespace worker {

// Routes registered by PollHandler<Protocol>:
//
//   POST   /api/v1/worker/poll/:type      → poll
//   POST   /api/v1/worker/ack/:id         → ack
//
// Usage:
//   PollHandler<Protocol>::bind(worker_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
template <typename Protocol>
class PollHandler {
  public:
    // Inject identity before the first poll is issued.
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // TODO: parse :type from path, look up s_ctx->get_task_worker(:type)
    //       on nullptr: Status::BAD_REQUEST — unregistered task type
    //       outbound HTTP call to engine POST /api/v1/tasks/queue/:type
    //       on task received: deserialise → TaskInput, call worker->execute(input, output)
    //       outbound HTTP call to engine POST /api/v1/tasks/:id/result with output JSON
    //       on empty queue: Status::NO_CONTENT
    static void poll(interfaces::IRequest<Protocol> &req,
                     interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :id (TaskId)
    //       outbound HTTP call to engine PATCH /api/v1/tasks/:id/heartbeat
    //       updates last_heartbeat_at on engine side — prevents engine requeuing timed-out tasks
    //       on not found: Status::NOT_FOUND
    static void ack(interfaces::IRequest<Protocol> &req,
                    interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

  private:
    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker
