module engine.handler.metadata;

@nogc nothrow:

import interfaces.interfaces;
import model.model;
import serde.serde;
import core.logger.logger;
import engine.context;

// Routes:
//   GET /api/v1/metadata/tasks       → list_task_definitions
//   GET /api/v1/metadata/workflows   → list_workflow_definitions
//   GET /api/v1/metadata/health      → health_check
class MetadataHandler(Protocol) {
  public:
    this(ref EngineContext ctx) { m_ctx = &ctx; }

    void list_task_definitions(ref IRequest!Protocol req,
                               ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");

        m_ctx.get_connector().find_all!TaskDef(
            (TaskDef[] tasks) {
                reply(res, Ser.serialize(accept, tasks));
            });
    }

    void list_workflow_definitions(ref IRequest!Protocol req,
                                   ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");

        m_ctx.get_connector().find_all!WorkflowDef(
            (WorkflowDef[] defs) {
                reply(res, Ser.serialize(accept, defs));
            });
    }

    void health_check(ref IRequest!Protocol req,
                      ref IResponse!Protocol res) {
        static immutable const(char)[] CACHE_KEY = "engine:health";
        static immutable const(char)[] OKE = `{"status":"ok"}`;
        static immutable const(char)[] BARE = `{"status":"ok","db":false,"cache":false}`;

        auto accept = req.find_header("accept");

        if (m_ctx.get_cache() !is null) {
            bool done = false;
            m_ctx.get_cache().get(CACHE_KEY, (const(char)[] cached) {
                if (cached.length > 0) {
                    reply(res, Ser.serialize_raw(accept, cached));
                    done = true;
                }
            });
            if (done) {
                return;
            }
        }

        if (m_ctx.get_db() !is null) {
            m_ctx.get_db().query(
                `{"op":"ping"}`, (const(char)[] /*result*/) {
                    if (m_ctx.get_cache() !is null) {
                        m_ctx.get_cache().set(CACHE_KEY, OKE,
                                              (const(char)[]){});
                    }
                    reply(res, Ser.serialize_raw(accept, OKE));
                });
            return;
        }

        warning("engine", "health: no db or cache");
        reply(res, Ser.serialize_raw(accept, BARE));
    }

  private:
    // PORT-NOTE: C++ used std::reference_wrapper<EngineContext>; D uses raw pointer.
    EngineContext* m_ctx;

    static void reply(ref IResponse!Protocol res, ubyte[] bytes,
                      Status status = Status.OK) {
        res.set_body(bytes);
        res.set_status(status);
    }
}
