export module worker_runtime:status;

import std;
import interfaces;
import core_events;
import core_logger;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import boost.ut;
#endif

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
                             interfaces::io::IResponse &res,
                             std::function<void()> send) noexcept {
        static constexpr std::string_view OK_JSON = R"({"status":"ok"})";

        // Convert the constant literal into bytes, then ship it — no actual liveness check.
        std::vector<std::byte> bytes(OK_JSON.size());
        std::ranges::transform(OK_JSON, bytes.begin(),
                               [](char character) noexcept { return std::byte(character); });
        res.set_body(std::move(bytes));
        res.set_status(interfaces::io::types::Status::OK);
        send();
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
                            interfaces::io::IResponse &res,
                            std::function<void()> send) noexcept {
        try {
            // Grab identity + registered capabilities straight off the context.
            auto worker_id = std::string{s_ctx->get_worker_id()};
            auto task_types = s_ctx->get_task_types();

            // SECURITY: worker_id (sourced from the worker's own TOML config, see
            // WorkerConfig::from_file() in external_worker_manager.cc) and every registered
            // task_type are dropped straight into this hand-rolled JSON string with zero
            // escaping — a worker_id or task_type containing a `"` or `\` produces invalid JSON,
            // or worse, lets a crafted value inject extra JSON keys/fields into this response.
            // Same class of bug PollHandler::build_submit_json already documents for
            // output_data below.
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
            send();
        } catch (const std::exception &e) {
            core::logger::error("worker/status", "unhandled exception in worker_info: {}", e.what());
            core::events::publish("worker.status.worker_info_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
        }
    }

  private:
    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker

#ifdef CONGELADO_TEST
namespace worker::status_handler_tests {
using namespace boost::ut;

/// @brief Minimal IWorker double, just enough to populate get_task_types() for worker_info().
class FakeWorker final : public interfaces::IWorker {
  public:
    explicit FakeWorker(std::string task_type) : m_task_type{std::move(task_type)} {}
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return m_task_type; }

  private:
    std::string m_task_type;
};

/// @brief Flattens an IResponse's body bytes back into a std::string for assertions, mirroring
/// ExecutionHandler::bytes_from()'s own (private) byte-to-string motion.
[[nodiscard]] std::string body_to_string(interfaces::io::IResponse &res) {
    std::string out;
    auto &view = res.get_body();
    out.reserve(view.size());
    for (auto byte : view) {
        out += static_cast<char>(byte);
    }
    return out;
}

suite<"StatusHandler::health_check"> status_handler_health_suite = [] {
    "replies 200 with the fixed {\"status\":\"ok\"} body"_test = [] {
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        StatusHandler::health_check(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
        expect(body_to_string(res) == R"({"status":"ok"})");
    };
};

suite<"StatusHandler::worker_info"> status_handler_info_suite = [] {
    "replies 200 with worker_id, empty task_types, and status active when nothing's registered"_test =
        [] {
            WorkerContext ctx;
            ctx.set_worker_id("worker-1");
            StatusHandler::bind(ctx);
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            StatusHandler::worker_info(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) ==
                   R"({"worker_id":"worker-1","task_types":[],"status":"active"})");
        };

    "lists a single registered task type"_test = [] {
        WorkerContext ctx;
        ctx.set_worker_id("worker-1");
        FakeWorker echo{"echo"};
        ctx.add_worker(echo);
        StatusHandler::bind(ctx);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        StatusHandler::worker_info(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(body_to_string(res) ==
               R"({"worker_id":"worker-1","task_types":["echo"],"status":"active"})");
    };

    // Pins the SECURITY finding flagged above worker_info()'s JSON-building block: an unescaped
    // `"` inside worker_id lands raw in the response body, corrupting the JSON's own structure
    // (a real JSON parser would fail on this, not just see a surprising string value). Current
    // (buggy) behavior is documented here, not fixed.
    "a `\"` inside worker_id breaks the hand-rolled JSON's structure instead of being escaped"_test =
        [] {
            WorkerContext ctx;
            ctx.set_worker_id(R"(bad"id)");
            StatusHandler::bind(ctx);
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            StatusHandler::worker_info(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            auto body = body_to_string(res);
            // The raw quote from worker_id lands unescaped, splitting what should be one JSON
            // string value into `"worker_id":"bad"` followed by a stray `id"` — not valid JSON.
            expect(body == R"({"worker_id":"bad"id","task_types":[],"status":"active"})");
        };
};

} // namespace worker::status_handler_tests
#endif
