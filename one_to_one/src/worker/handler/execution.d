module worker.handler.execution;

@nogc nothrow:

import interfaces.interfaces;
import model.model;
import serde.serde;
import core.logger.logger;
import worker.context;

// Routes registered by ExecutionHandler!Protocol:
//
//   GET    /api/v1/worker/executions           → list_executions
//   GET    /api/v1/worker/executions/:id       → get_execution
//   DELETE /api/v1/worker/executions/:id       → cancel_execution
//
// Usage:
//   ExecutionHandler!Protocol.bind(worker_ctx);
//   // then register static methods as HandlerFn!Protocol in RouterContext
class ExecutionHandler(Protocol) {
  public:
    // Inject identity before the first request arrives.
    static void bind(WorkerContext ctx) { s_ctx = ctx; }

    // GET /api/v1/worker/executions
    // Queries engine for IN_PROGRESS tasks owned by this worker, forwards JSON.
    static void list_executions(ref IRequest!Protocol req,
                                ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto worker_id = s_ctx.get_worker_id();
        // auto path = "/api/v1/tasks?worker_id=" ~ worker_id ~ "&status=IN_PROGRESS";

        // s_ctx.call_engine("GET", path, "",
        //     (int status, const(char)[] body) {
        //         if (status == 200) {
        //             reply(res, bytes_from(body));
        //         } else {
        //             error("worker/executions", "list failed status={}");
        //             res.set_status(Status.INTERNAL_SERVER_ERROR);
        //         }
        //     });
    }

    // GET /api/v1/worker/executions/:id
    // Queries engine for task :id, forwards result.
    // Note: TaskInstance has no worker_id field yet — ownership check skipped.
    static void get_execution(ref IRequest!Protocol req,
                              ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto task_id = target[last_slash(target) + 1 .. $];

        // s_ctx.call_engine("GET", "/api/v1/tasks/" ~ task_id, "",
        //     (int status, const(char)[] body) {
        //         if (status == 404) {
        //             res.set_status(Status.NOT_FOUND);
        //             return;
        //         }
        //         if (status != 200) {
        //             error("worker/executions", "get failed status={}");
        //             res.set_status(Status.INTERNAL_SERVER_ERROR);
        //             return;
        //         }
        //         reply(res, bytes_from(body));
        //     });
    }

    // DELETE /api/v1/worker/executions/:id
    // Forwards cancel to engine DELETE /api/v1/tasks/:id.
    static void cancel_execution(ref IRequest!Protocol req,
                                 ref IResponse!Protocol res) {
        auto target = req.get_target();
        auto task_id = target[last_slash(target) + 1 .. $];

        // s_ctx.call_engine("DELETE", "/api/v1/tasks/" ~ task_id, "",
        //     (int status, const(char)[]) {
        //         if (status == 200 || status == 204) {
        //             res.set_status(Status.NO_CONTENT);
        //         } else if (status == 404) {
        //             res.set_status(Status.NOT_FOUND);
        //         } else {
        //             res.set_status(Status.INTERNAL_SERVER_ERROR);
        //         }
        //     });
    }

  private:
    static void reply(ref IResponse!Protocol res, ubyte[] bytes,
                      Status status = Status.OK) {
        res.set_body(bytes);
        res.set_status(status);
    }

    static ubyte[] bytes_from(const(char)[] text) {
        // PORT-NOTE: @nogc alloc deferred — wire util.alloc in Run 2
        ubyte[] result;
        foreach (ch; text) {
            result ~= cast(ubyte) ch;
        }
        return result;
    }

    static size_t last_slash(const(char)[] s) {
        for (ptrdiff_t i = cast(ptrdiff_t) s.length - 1; i >= 0; --i) {
            if (s[i] == '/') return cast(size_t) i;
        }
        return 0;
    }

    // PORT-NOTE: C++ used static inline WorkerContext* s_ctx; D uses __gshared for TLS-free static.
    __gshared WorkerContext s_ctx;
}
