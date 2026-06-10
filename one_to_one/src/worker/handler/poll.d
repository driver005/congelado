module worker.handler.poll;

@nogc nothrow:

import interfaces.interfaces;
import model.model;
import serde.serde;
import core.logger.logger;
import worker.context;

// Routes registered by PollHandler!Protocol:
//
//   POST   /api/v1/worker/poll/:type      → poll
//   POST   /api/v1/worker/ack/:id         → ack
//
// Usage:
//   PollHandler!Protocol.bind(worker_ctx);
//   // then register static methods as HandlerFn!Protocol in RouterContext
//
// call_engine() blocks the calling contract thread via std::future::get() while
// EngineClient's receive contract runs on a separate pool thread. Requires the
// ContractThreadPool to have at least 2 threads (default: hardware_concurrency()).
class PollHandler(Protocol) {
  public:
    // Inject identity before the first poll is issued.
    static void bind(WorkerContext ctx) { s_ctx = ctx; }

    // POST /api/v1/worker/poll/:type
    // 1. Parse :type, look up registered task worker.
    // 2. Poll engine GET /api/v1/tasks/queue/:type.
    // 3. On 204 (empty queue): respond NO_CONTENT.
    // 4. On 200: deserialise TaskInstance → TaskInput, execute task, submit result.
    static void poll(ref IRequest!Protocol req,
                     ref IResponse!Protocol res) {
        auto target = req.get_target();
        auto type = target[last_slash(target) + 1 .. $];

        // if (s_ctx.get_task_worker(type) is null) {
        //     res.set_status(Status.BAD_REQUEST);
        //     return;
        // }

        // s_ctx.call_engine("GET", "/api/v1/tasks/queue/" ~ type, "",
        //     (int status, const(char)[] body) {
        //         if (status == 204) {
        //             res.set_status(Status.NO_CONTENT);
        //             return;
        //         }
        //         if (status != 200) {
        //             error("worker/poll", "engine poll failed status={}");
        //             res.set_status(Status.INTERNAL_SERVER_ERROR);
        //             return;
        //         }
        //
        //         auto parsed = serde.Json.decode!TaskInstance(body);
        //         if (!parsed.has_value()) {
        //             error("worker/poll", "parse TaskInstance failed");
        //             res.set_status(Status.INTERNAL_SERVER_ERROR);
        //             return;
        //         }
        //
        //         auto instance = parsed.value();
        //         TaskInput input = TaskInput(instance.get_input_data());
        //         TaskOutput output;
        //         bool ran = s_ctx.run_task(instance.get_def_name(), input, output);
        //
        //         auto result = ran ? TaskResult.SUCCESS : TaskResult.FAILURE;
        //         const(char)[][const(char)[]] output_data =
        //             ran ? output.get_data() : (const(char)[][const(char)[]]).init;
        //
        //         auto task_id = instance.get_task_id();
        //         auto submit_body = build_submit_json(result, output_data);
        //
        //         s_ctx.call_engine("POST", "/api/v1/tasks/" ~ task_id ~ "/result", submit_body,
        //             (int submit_status, const(char)[]) {
        //                 if (submit_status == 200) {
        //                     res.set_status(Status.OK);
        //                 } else {
        //                     error("worker/poll", "submit result failed status={}");
        //                     res.set_status(Status.INTERNAL_SERVER_ERROR);
        //                 }
        //             });
        //     });
    }

    // POST /api/v1/worker/ack/:id
    // PATCH /api/v1/tasks/:id/heartbeat on engine — resets timeout clock.
    static void ack(ref IRequest!Protocol req,
                    ref IResponse!Protocol res) {
        auto target = req.get_target();
        auto task_id = target[last_slash(target) + 1 .. $];

        // s_ctx.call_engine("PATCH", "/api/v1/tasks/" ~ task_id ~ "/heartbeat", "",
        //     (int status, const(char)[]) {
        //         if (status == 200) {
        //             res.set_status(Status.OK);
        //         } else if (status == 404) {
        //             res.set_status(Status.NOT_FOUND);
        //         } else {
        //             res.set_status(Status.INTERNAL_SERVER_ERROR);
        //         }
        //     });
    }

  private:
    static ubyte[] build_submit_json(TaskResult result,
                                     ref const(const(char)[][const(char)[]]) data) {
        const(char)[] result_str;
        final switch (result) {
        case TaskResult.SUCCESS: result_str = "SUCCESS"; break;
        case TaskResult.FAILURE: result_str = "FAILURE"; break;
        case TaskResult.TIMEOUT: result_str = "TIMEOUT"; break;
        case TaskResult.SKIPPED: result_str = "SKIPPED"; break;
        }

        // PORT-NOTE: std::format → @nogc string build; full impl deferred to Run 2
        // Returns stub empty slice until call_engine stubs are wired.
        ubyte[] json;
        return json;
    }

    static size_t last_slash(const(char)[] s) {
        for (ptrdiff_t i = cast(ptrdiff_t) s.length - 1; i >= 0; --i) {
            if (s[i] == '/') return cast(size_t) i;
        }
        return 0;
    }

    // PORT-NOTE: C++ used static inline WorkerContext* s_ctx; D uses __gshared.
    __gshared WorkerContext s_ctx;
}
