export module worker:status;

import std;
import interfaces;
import core_events;
import core_logger;
import :context;

export namespace worker {

// Routes registered by StatusHandler:
//
//   GET /api/v1/worker/health    → health_check
//   GET /api/v1/worker/info      → worker_info
//
// Usage:
//   StatusHandler::bind(worker_ctx);
//   // then register static methods as HandlerFn in RouterContext
class StatusHandler {
  public:
    /**
     * @brief Wires this handler up to the worker context both routes below need — call once
     * before registering these as HandlerFns.
     * @param ctx the worker context to route calls through.
     */
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // Workers are stateless — no DB probe, no cache. Always returns ok.
    /**
     * @brief Route handler for `GET /api/v1/worker/health`. Liveness probe.
     * @param req unused — health check takes no input, hence the commented-out param name.
     * @param res the response to populate; always `{"status":"ok"}` with 200 OK.
     * @note Workers are stateless by design — no DB, no cache — so this never actually checks
     * anything downstream, lowkey just a heartbeat. It's an "is the process up" check, not an
     * "is everything healthy" check. Don't read more into a 200 than that.
     */
    static void health_check(interfaces::io::IRequest & /*req*/,
                             interfaces::io::IResponse &res) noexcept {
        static constexpr std::string_view OK_JSON = R"({"status":"ok"})";

        // Convert the constant literal into bytes, then ship it — no actual liveness check.
        std::vector<std::byte> bytes(OK_JSON.size());
        std::ranges::transform(OK_JSON, bytes.begin(),
                               [](char character) noexcept { return std::byte(character); });
        res.set_body(std::move(bytes));
        res.set_status(interfaces::io::types::Status::OK);
    }

    // Returns JSON with worker_id, registered task_types, and active status.
    /**
     * @brief Route handler for `GET /api/v1/worker/info`. Reports identity + capabilities: this
     * worker's id, every task type it's got registered, and a hardcoded "active" status.
     * @param req unused — no input needed, hence the commented-out param name.
     * @param res the response to populate; hand-rolled JSON body with 200 OK.
     * @note "status":"active" is a constant literal, not derived from any real liveness check —
     * same deal as health_check(), this reports presence, not health.
     */
    static void worker_info(interfaces::io::IRequest & /*req*/,
                            interfaces::io::IResponse &res) noexcept {
        try {
            // Grab identity + registered capabilities straight off the context.
            auto worker_id = std::string{s_ctx->get_worker_id()};
            auto task_types = s_ctx->get_task_types();

            // Hand-roll the JSON — open the object, id first, then start the task_types array.
            std::string json = std::format(R"({{"worker_id":"{}","task_types":[)", worker_id);
            // Comma-join every registered task type name into the array.
            bool first = true;
            for (auto type : task_types) {
                if (!first) {
                    json += ',';
                }
                json += std::format(R"("{}")", type);
                first = false;
            }
            // Close the array and tack on the (always-hardcoded) status field.
            json += R"(],"status":"active"})";

            // Convert the finished JSON string into raw bytes one char at a time — no
            // reinterpret_cast, same pattern as ExecutionHandler::bytes_from().
            std::vector<std::byte> bytes;
            bytes.reserve(json.size());
            for (char character : json) {
                bytes.push_back(static_cast<std::byte>(character));
            }
            res.set_body(std::move(bytes));
            res.set_status(interfaces::io::types::Status::OK);
        } catch (const std::exception &e) {
            core::logger::error("worker/status", "unhandled exception in worker_info: {}", e.what());
            core::events::publish("worker.status.worker_info_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        }
    }

  private:
    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker
