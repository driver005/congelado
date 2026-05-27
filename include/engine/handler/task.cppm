export module engine:task_handler;

import std;
import interfaces;
import model;
import shared;
import :context;

export namespace engine {

// Routes registered by TaskHandler<Protocol>:
//
//   GET    /api/v1/tasks/:name            → get_definition
//   POST   /api/v1/tasks                  → create_definition
//   PUT    /api/v1/tasks/:name            → update_definition
//   DELETE /api/v1/tasks/:name            → remove_definition
//   GET    /api/v1/tasks/queue/:type      → poll
//   POST   /api/v1/tasks/:id/result       → submit_result
//
// Usage:
//   TaskHandler<Protocol>::bind(engine_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
template <typename Protocol>
class TaskHandler {
  public:
    // Inject dependencies before the first request arrives.
    static void bind(EngineContext& ctx) noexcept { s_ctx = &ctx; }

    // TODO: parse :name from req.get_target() suffix
    //       cache-first lookup with key "task:def:{name}"
    //       on cache miss: IDatabase::query → decode TaskDef → re-populate cache
    //       on not found: Status::NOT_FOUND
    static void get_definition(interfaces::IRequest<Protocol>& req,
                               interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: deserialise body → TaskDef, validate (name non-empty, worker_type set)
    //       IDatabase::insert → on success Status::CREATED, body = created entity
    //       invalidate cache key "task:def:{name}"
    static void create_definition(interfaces::IRequest<Protocol>& req,
                                  interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :name, deserialise body → updated TaskDef
    //       IDatabase::update → on success Status::OK
    //       ICache::remove "task:def:{name}"
    static void update_definition(interfaces::IRequest<Protocol>& req,
                                  interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :name, IDatabase::remove → Status::NO_CONTENT
    //       ICache::remove "task:def:{name}"
    //       guard: reject if active TaskInstances reference this definition
    static void remove_definition(interfaces::IRequest<Protocol>& req,
                                  interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :type (worker_type), IDatabase::query for next SCHEDULED TaskInstance
    //       transition status → IN_PROGRESS, return instance JSON
    //       long-poll: if queue empty, hold connection until timeout or item arrives
    static void poll(interfaces::IRequest<Protocol>& req,
                     interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: parse :id (TaskId), deserialise body → TaskResult + output_data
    //       IDatabase::update TaskInstance (status, output_data, timings.end)
    //       trigger workflow engine step evaluation via scheduler
    static void submit_result(interfaces::IRequest<Protocol>& req,
                              interfaces::IResponse<Protocol>& res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

  private:
    static inline EngineContext* s_ctx{nullptr};
};

} // namespace engine
