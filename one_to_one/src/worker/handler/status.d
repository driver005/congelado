module worker.handler.status;

@nogc nothrow:

import interfaces.interfaces;
import worker.context;

// Routes registered by StatusHandler!Protocol:
//
//   GET /api/v1/worker/health    → health_check   ← fully implemented
//   GET /api/v1/worker/info      → worker_info
//
// Usage:
//   StatusHandler!Protocol.bind(worker_ctx);
//   // then register static methods as HandlerFn!Protocol in RouterContext
class StatusHandler(Protocol) {
  public:
    // Inject identity before the first request arrives.
    // static void bind(WorkerContext ctx) { s_ctx = ctx; }

    // Workers are stateless — no DB probe, no cache. Always returns ok.
    static void health_check(ref IRequest!Protocol /*req*/,
                             ref IResponse!Protocol res) {
        static immutable const(char)[] k_ok = `{"status":"ok"}`;

        ubyte[] bytes;
        // PORT-NOTE: @nogc alloc deferred to Run 2; GC allocation used here as placeholder
        bytes.length = k_ok.length;
        foreach (i, ch; k_ok) {
            bytes[i] = cast(ubyte) ch;
        }
        res.set_body(bytes);
        res.set_status(Status.OK);
    }

    // TODO: build JSON from WorkerContext fields
    //       {"worker_id":"...","task_types":[...],"status":"active"}
    static void worker_info(ref IRequest!Protocol req,
                            ref IResponse!Protocol res) {
        res.set_status(Status.NOT_IMPLEMENTED);
    }

    // private:
    //   __gshared WorkerContext s_ctx;
}
