export module worker_runtime:execution;

import std;
import interfaces;
import model;
import serde;
import core_events;
import core_logger;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import shared;
import boost.ut;
#endif

export namespace worker {

// Routes registered by ExecutionHandler:
//
//   GET    /api/v1/worker/executions           → list_executions
//   GET    /api/v1/worker/executions/:id       → get_execution
//   DELETE /api/v1/worker/executions/:id       → cancel_execution
//
// Usage:
//   ExecutionHandler::bind(worker_ctx, engine_client);
//   // then register static methods as HandlerFn in RouterContext
class ExecutionHandler {
  public:
    /**
     * @brief Wires this handler up to the worker context every route below needs — call once
     * before registering these as HandlerFns.
     * @param ctx the worker context to route calls through.
     * @param engine the engine client; stashed for parity with the other handlers' bind
     * signature.
     * @note `s_engine` gets set here but nothing in this class actually reads it back — every
     * outbound call below goes through `s_ctx->call_engine()` instead. Kinda sus, harmless
     * though.
     */
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // GET /api/v1/worker/executions
    // Queries engine for IN_PROGRESS tasks owned by this worker, forwards JSON.
    /**
     * @brief Route handler for `GET /api/v1/worker/executions`. Fetches every IN_PROGRESS task
     * owned by this worker from the engine and forwards the JSON straight through, no reshaping.
     * @param req the incoming request (unused — no path params, no body).
     * @param res the response to populate; body + status get set based on the engine's reply.
     * @note Any exception from call_engine() (e.g. no engine client configured) gets swallowed
     * into a 500 here — logged first, response never propagates the actual cause past the log
     * line.
     */
    static void list_executions(interfaces::io::IRequest &req,
                                interfaces::io::IResponse &res,
                                std::function<void()> send) noexcept {
        // Build the engine query — this worker's own in-progress tasks, no more no less.
        auto worker_id = std::string{s_ctx->get_worker_id()};
        auto path = "/api/v1/tasks?worker_id=" + worker_id + "&status=IN_PROGRESS";

        // Any transport/config exception (e.g. no engine set) becomes a flat 500 — actual
        // cause only survives in the log line.
        WorkerContext::EngineResponse engine_res;
        try {
            engine_res = s_ctx->call_engine("GET", path);
        } catch (const std::exception &e) {
            core::logger::error("worker/executions", "list exception: {}", e.what());
            core::events::publish("worker.executions.list_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
            return;
        }

        // Forward the engine's JSON straight through on success, else surface a 500.
        if (engine_res.m_status == 200) {
            reply(res, bytes_from(engine_res.m_body));
        } else {
            core::logger::error("worker/executions", "list failed status={}", engine_res.m_status);
            core::events::publish("worker.executions.list_failed",
                                  {{"status", std::to_string(engine_res.m_status)}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        }
        send();
    }

    // GET /api/v1/worker/executions/:id
    // Queries engine for task :id, forwards result.
    /**
     * @brief Route handler for `GET /api/v1/worker/executions/:id`. Looks up a single task by id
     * on the engine and forwards the result.
     * @param req the incoming request; `:id` is pulled off the tail of the path.
     * @param res the response to populate — 404 if the engine doesn't know the task, 500 on
     * transport/engine failure, otherwise the forwarded JSON body with 200.
     * @note Path parsing is a plain `rfind('/')` — works fine for the documented route shape, but
     * don't expect it to handle trailing slashes or query strings gracefully.
     */
    static void get_execution(interfaces::io::IRequest &req,
                              interfaces::io::IResponse &res,
                              std::function<void()> send) noexcept {
        // Pull :id off the tail of the path — plain rfind, no query-string handling.
        auto target = req.get_path();
        auto task_id = std::string{target.substr(target.rfind('/') + 1)};

        WorkerContext::EngineResponse engine_res;
        try {
            engine_res = s_ctx->call_engine("GET", "/api/v1/tasks/" + task_id);
        } catch (const std::exception &e) {
            core::logger::error("worker/executions", "get exception: {}", e.what());
            core::events::publish("worker.executions.get_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
            return;
        }

        // Engine doesn't know this task — pass the 404 straight through.
        if (engine_res.m_status == 404) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            send();
            return;
        }
        // Anything else that's not a clean 200 is treated as a server-side L.
        if (engine_res.m_status != 200) {
            core::logger::error("worker/executions", "get {} failed status={}", task_id,
                                engine_res.m_status);
            core::events::publish("worker.executions.get_failed",
                                  {{"task_id", task_id}, {"status", std::to_string(engine_res.m_status)}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
            return;
        }
        // Happy path — forward the task JSON as-is.
        reply(res, bytes_from(engine_res.m_body));
        send();
    }

    // DELETE /api/v1/worker/executions/:id
    // Forwards cancel to engine DELETE /api/v1/tasks/:id.
    /**
     * @brief Route handler for `DELETE /api/v1/worker/executions/:id`. Forwards the cancel
     * straight through to `DELETE /api/v1/tasks/:id` on the engine.
     * @param req the incoming request; `:id` is pulled off the tail of the path.
     * @param res the response to populate — NO_CONTENT on engine 200/204, NOT_FOUND on engine
     * 404, INTERNAL_SERVER_ERROR on anything else (including transport exceptions).
     */
    static void cancel_execution(interfaces::io::IRequest &req,
                                 interfaces::io::IResponse &res,
                                 std::function<void()> send) noexcept {
        // Pull :id off the tail of the path, same deal as get_execution().
        auto target = req.get_path();
        auto task_id = std::string{target.substr(target.rfind('/') + 1)};

        WorkerContext::EngineResponse engine_res;
        try {
            engine_res = s_ctx->call_engine("DELETE", "/api/v1/tasks/" + task_id);
        } catch (const std::exception &e) {
            core::logger::error("worker/executions", "cancel exception: {}", e.what());
            core::events::publish("worker.executions.cancel_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
            return;
        }

        // Map the engine's cancel outcome onto this handler's own status codes.
        if (engine_res.m_status == 200 || engine_res.m_status == 204) {
            res.set_status(interfaces::io::types::Status::NO_CONTENT);
        } else if (engine_res.m_status == 404) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
        } else {
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        }
        send();
    }

  private:
    /**
     * @brief Sets a response's body and status in one motion — small helper so the route
     * handlers above don't repeat the same two-liner every time.
     * @param res the response to populate.
     * @param bytes the body bytes to set, moved in.
     * @param status the status to set, defaults to OK.
     */
    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    /**
     * @brief Converts a std::string into a std::vector<std::byte>, one char at a time.
     * @param text the text to convert.
     * @return the same bytes, repackaged as `std::byte`s for IResponse::set_body().
     */
    static std::vector<std::byte> bytes_from(const std::string &text) noexcept {
        // Reserve up front, then convert one char at a time — no bulk reinterpret here.
        std::vector<std::byte> result;
        result.reserve(text.size());
        for (char character : text) {
            result.push_back(static_cast<std::byte>(character));
        }
        return result;
    }

    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker

#ifdef CONGELADO_TEST
namespace worker::execution_handler_tests {
using namespace boost::ut;

/// @brief Concrete IRequest double just complete enough for call_engine() to build a request
/// against — same shape as core_client:registry's own RegisterFakeRequest test double.
class TestClientRequest final : public interfaces::io::IRequest {
  public:
    explicit TestClientRequest(std::uint32_t stream_id) : interfaces::io::IRequest{stream_id} {}

    void set_header(std::variant<std::string_view, interfaces::io::types::Token>,
                    std::string_view) & override {}
    [[nodiscard]] std::string_view find_header(
        std::variant<std::string_view, interfaces::io::types::Token>) const noexcept override {
        return "x";
    }
    void set_body(std::vector<std::byte> &&body) & override { m_body = std::move(body); }

  private:
    std::vector<std::byte> m_body;
};

/// @brief Concrete IClient double whose create_request() hands back a working TestClientRequest
/// (so call_engine()'s own build() doesn't abort) but whose send() always throws — used to
/// deterministically exercise call_engine()'s failure path without a live connection and without
/// blocking on its no-timeout future.
class ThrowingTestClient final : public interfaces::IClient {
  public:
    shared::ReadCallback on_connect(shared::SendCallback, shared::CloseCallback) override {
        return {};
    }
    std::uint32_t send(interfaces::io::IRequest &) override {
        throw std::runtime_error("ThrowingTestClient: send always fails");
    }
    [[nodiscard]] std::unique_ptr<interfaces::io::IRequest>
    create_request(std::uint32_t stream_id) override {
        return std::make_unique<TestClientRequest>(stream_id);
    }
};

/// @brief Binds `ctx`'s own runtime to a client whose send() always throws, so every
/// ExecutionHandler route below hits its own `call_engine()` try/catch and replies 500 —
/// deterministic, and never risks blocking on call_engine()'s no-timeout future (see
/// WorkerContext::call_engine's own test suite for why round-tripping it for real is out of scope
/// here).
void install_throwing_client(WorkerContext &ctx) {
    static ThrowingTestClient client;
    ctx.set_runtime(client);
}

suite<"ExecutionHandler"> execution_handler_suite = [] {
    "list_executions replies 500 when the engine call throws"_test = [] {
        WorkerContext ctx;
        install_throwing_client(ctx);
        ctx.set_worker_id("worker-1");
        ExecutionHandler::bind(ctx);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        ExecutionHandler::list_executions(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
    };

    "get_execution replies 500 when the engine call throws"_test = [] {
        WorkerContext ctx;
        install_throwing_client(ctx);
        ExecutionHandler::bind(ctx);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/worker/executions/task-1");
        bool sent = false;

        ExecutionHandler::get_execution(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
    };

    "cancel_execution replies 500 when the engine call throws"_test = [] {
        WorkerContext ctx;
        install_throwing_client(ctx);
        ExecutionHandler::bind(ctx);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/worker/executions/task-1");
        bool sent = false;

        ExecutionHandler::cancel_execution(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
    };
};

} // namespace worker::execution_handler_tests
#endif
