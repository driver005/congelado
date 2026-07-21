export module engine:metadata;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import :context;

export namespace engine {

// Routes:
//   GET /api/v1/metadata/tasks       → list_task_definitions
//   GET /api/v1/metadata/workflows   → list_workflow_definitions
//   GET /api/v1/metadata/health      → health_check
class MetadataHandler {
  public:
    /**
     * @brief Builds a handler bound to the shared EngineContext — no state of its own beyond
     * that reference, every route below just leans on `m_ctx` to reach the connector.
     * @warning Same deferred-callback risk documented on TaskHandler's constructor: the local
     * `accept` captured by `[&]` into find_all()/cache/db callbacks below only survives if
     * that callback runs synchronously, which Connector::enqueue() only guarantees when no
     * database is configured. Real db wired in → deferred to a later tick → dangling capture.
     * @param ctx the engine context to bind; caller keeps it alive for this handler's whole
     * lifetime.
     */
    explicit MetadataHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Handles `GET /api/v1/metadata/tasks` — dumps every stored TaskDef, no filtering,
     * straight off Connector::find_all().
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized task-definition list into.
     */
    void list_task_definitions(interfaces::io::IRequest &req,
                               interfaces::io::IResponse &res) noexcept {
        // grab the Accept header up front so we know which format to serialize into
        auto accept = req.find_header("accept");

        // no filtering here — just dump every stored TaskDef straight to the client. Callback
        // is not noexcept: serde::Ser::serialize() may throw (alloc/format errors), and
        // Connector::find_all()'s callback parameter (std::move_only_function<void(...)>)
        // doesn't require a noexcept target.
        m_ctx.get().get_connector().find_all<model::TaskDef>(
            [&](const std::vector<model::TaskDef> &tasks) {
                reply(res, serde::Ser::serialize(accept, tasks));
            });
    }

    /**
     * @brief Handles `GET /api/v1/metadata/workflows` — dumps every stored WorkflowDef, same
     * no-filter find_all() motion as list_task_definitions().
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized workflow-definition list into.
     */
    void list_workflow_definitions(interfaces::io::IRequest &req,
                                   interfaces::io::IResponse &res) noexcept {
        // same deal as list_task_definitions — just the Accept header, nothing else needed
        auto accept = req.find_header("accept");

        // no filtering, dump every stored WorkflowDef. Not noexcept — same reasoning as
        // list_task_definitions() above.
        m_ctx.get().get_connector().find_all<model::WorkflowDef>(
            [&](const std::vector<model::WorkflowDef> &defs) {
                reply(res, serde::Ser::serialize(accept, defs));
            });
    }

    /**
     * @brief Handles `GET /api/v1/metadata/health` — cache-first health probe: serves the
     * cached `{"status":"ok"}` if one's sitting there, else pings the db (warming the cache
     * with the OK payload on the way out) if one's configured, else falls back to reporting no
     * backend wired up at all and logs a warning.
     * @warning The db ping's actual result is thrown away — the query callback's parameter is
     * deliberately left unnamed — so reaching the callback at all gets treated as "ok"
     * regardless of what the db returned. A db
     * that answers with an error body still reports healthy here. That's a real gap for a
     * health endpoint specifically, not just vibes — don't trust this one to catch a db that's
     * up but unhappy.
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the health payload into.
     */
    void health_check(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        static constexpr std::string_view CACHE_KEY = "engine:health";
        static constexpr std::string_view OK_PAYLOAD = R"({"status":"ok"})";
        static constexpr std::string_view BARE = R"({"status":"ok","db":false,"cache":false})";

        auto accept = req.find_header("accept");

        // cache-first — if a cached "ok" is already sitting there, ship it and skip the db entirely
        if (m_ctx.get().get_cache() != nullptr) {
            bool done = false;
            m_ctx.get().get_cache()->get(CACHE_KEY, [&](std::string_view cached) noexcept {
                if (!cached.empty()) {
                    reply(res, serde::Ser::serialize_raw(accept, cached));
                    done = true;
                }
            });
            if (done) {
                return;
            }
        }

        // no cache hit — fall back to pinging the db, warming the cache with the OK payload on the way out
        if (m_ctx.get().get_db() != nullptr) {
            m_ctx.get().get_db()->query(
                R"({"op":"ping"})", [&](std::string_view /*result*/) noexcept {
                    if (m_ctx.get().get_cache()) {
                        m_ctx.get().get_cache()->set(CACHE_KEY, OK_PAYLOAD,
                                                     [](std::string_view) noexcept {});
                    }
                    reply(res, serde::Ser::serialize_raw(accept, OK_PAYLOAD));
                });
            return;
        }

        // neither backend configured — log it and report the bare-bones status
        core::logger::warning("engine", "health: no db or cache");
        reply(res, serde::Ser::serialize_raw(accept, BARE));
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    /**
     * @brief Shared reply helper — writes `bytes` into the response body and sets the status,
     * defaulting to 200 OK when the caller doesn't hand over anything else. Every route above
     * funnels its response through this one spot.
     * @param res the response to fill in.
     * @param bytes the body bytes to write.
     * @param status the status code to set, defaults to OK.
     */
    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }
};

} // namespace engine
