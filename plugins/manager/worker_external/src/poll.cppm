export module worker_runtime:poll;

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

// Routes registered by PollHandler:
//
//   POST   /api/v1/worker/poll/:type      → poll
//   POST   /api/v1/worker/ack/:id         → ack
//
// Usage:
//   PollHandler::bind(worker_ctx, engine_client);
//   // then register static methods as HandlerFn in RouterContext
class PollHandler {
  public:
    /**
     * @brief Wires this handler up to the worker context every route below needs — call once
     * before registering these as HandlerFns.
     * @param ctx the worker context to route calls through.
     * @param engine the engine client; stashed for parity with the other handlers' bind
     * signature.
     * @note Same deal as ExecutionHandler::bind() — `s_engine` gets stored but nothing here reads
     * it back, all engine traffic goes through `s_ctx->call_engine()`.
     */
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // POST /api/v1/worker/poll/:type
    // 1. Parse :type, look up registered task worker.
    // 2. Poll engine GET /api/v1/tasks/queue/:type.
    // 3. On 204 (empty queue): respond NO_CONTENT.
    // 4. On 200: deserialise TaskInstance → TaskInput, execute task, submit result.
    /**
     * @brief Route handler for `POST /api/v1/worker/poll/:type`. The full pull-execute-submit
     * loop for this worker: checks a task worker is registered for `:type`, polls the engine's
     * queue, runs the task locally if one's waiting, then submits the result back. That's the
     * whole motion, front to back, in a single request.
     * @param req the incoming request; `:type` is pulled off the tail of the path.
     * @param res the response to populate — BAD_REQUEST if `:type` isn't registered, NO_CONTENT
     * if the queue's empty, OK once the result's been submitted, INTERNAL_SERVER_ERROR on any
     * engine/transport failure along the way.
     * @warning Task execution (`s_ctx->run_task()`) happens synchronously, inline, on whatever
     * thread handles this request — a slow or hanging task worker blocks the whole poll request
     * (and everything serialized behind it) for as long as it runs. No timeout wrapping it
     * either. If a task worker never returns, this request never completes.
     * @note On task failure, `output_data` falls back to an empty map rather than propagating
     * whatever partial output the task produced — the engine only ever sees FAILURE + nothing.
     */
    static void poll(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                     std::function<void()> send) noexcept {
        try {
            // Pull :type off the tail of the path.
            auto target = req.get_path();
            auto type = std::string{target.substr(target.rfind('/') + 1)};

            // Guard clause — no point polling the engine for a type this worker can't even run.
            if (s_ctx->get_task_worker(type) == nullptr) {
                res.set_status(interfaces::io::types::Status::BAD_REQUEST);
                send();
                return;
            }

            // Poll engine for a task of this type.
            WorkerContext::EngineResponse engine_res;
            try {
                engine_res = s_ctx->call_engine(
                    "GET", std::format("/api/v1/tasks/queue/{}", type));
            } catch (const std::exception &e) {
                core::logger::error("worker/poll", "engine poll exception: {}", e.what());
                core::events::publish("worker.poll.exception", {{"error", e.what()}});
                res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                send();
                return;
            }

            // Empty queue is a legit outcome, not an error — say so and bail.
            if (engine_res.m_status == 204) {
                res.set_status(interfaces::io::types::Status::NO_CONTENT);
                send();
                return;
            }
            // Anything besides 200/204 from the queue endpoint is a genuine failure.
            if (engine_res.m_status != 200) {
                core::logger::error("worker/poll", "engine poll failed status={}", engine_res.m_status);
                core::events::publish("worker.poll.engine_poll_failed",
                                      {{"status", std::to_string(engine_res.m_status)}});
                res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                send();
                return;
            }

            // Parse TaskInstance from engine response.
            auto parsed = serde::Ser::deserialize<model::TaskInstance>("application/json", engine_res.m_body);
            if (!parsed.has_value()) {
                core::logger::error("worker/poll", "parse TaskInstance failed: {}", parsed.error());
                core::events::publish("worker.poll.parse_task_instance_failed",
                                      {{"error", parsed.error()}});
                res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                send();
                return;
            }

            // Run the task synchronously, right here, on this request's thread — no timeout, see
            // the doxygen warning above for what that means if a task worker hangs.
            auto &instance = *parsed;
            auto output_res = s_ctx->run_task(instance.get_def_name(), instance.get_input_data());

            // On failure, output_data carries the WorkerError's message under "error" rather than
            // whatever partial result the task produced.
            auto result =
                output_res.has_value() ? model::TaskResult::SUCCESS : model::TaskResult::FAILURE;
            std::unordered_map<std::string, std::string> output_data =
                output_res.has_value()
                    ? std::move(*output_res)
                    : std::unordered_map<std::string, std::string>{
                          {"error", std::string{output_res.error().getMessage()}}};

            // Submit result back to engine.
            auto task_id = std::format("{}", instance.get_task_id());
            auto submit_path = "/api/v1/tasks/" + task_id + "/result";
            auto submit_body = build_submit_json(result, output_data);

            WorkerContext::EngineResponse submit_res;
            try {
                submit_res = s_ctx->call_engine("POST", submit_path, submit_body);
            } catch (const std::exception &e) {
                core::logger::error("worker/poll", "submit exception: {}", e.what());
                core::events::publish("worker.poll.submit_exception", {{"error", e.what()}});
                res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                send();
                return;
            }

            // Whole poll-execute-submit motion only counts as a W once the engine confirms the
            // submit landed.
            if (submit_res.m_status == 200) {
                res.set_status(interfaces::io::types::Status::OK);
            } else {
                core::logger::error("worker/poll", "submit result failed status={}",
                                    submit_res.m_status);
                core::events::publish("worker.poll.submit_result_failed",
                                      {{"status", std::to_string(submit_res.m_status)}});
                res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            }
            send();
        } catch (const std::exception &e) {
            core::logger::error("worker/poll", "unhandled exception in poll: {}", e.what());
            core::events::publish("worker.poll.unhandled_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
        }
    }

    // POST /api/v1/worker/ack/:id
    // PATCH /api/v1/tasks/:id/heartbeat on engine — resets timeout clock.
    /**
     * @brief Route handler for `POST /api/v1/worker/ack/:id`. Sends a heartbeat for the given
     * task so the engine doesn't time it out while it's still being worked.
     * @param req the incoming request; `:id` is pulled off the tail of the path.
     * @param res the response to populate — OK on a successful heartbeat, NOT_FOUND if the
     * engine no longer knows the task (already timed out/finished, probably), otherwise
     * INTERNAL_SERVER_ERROR.
     * @warning There's no retry here — if this ack gets dropped (network blip, engine hiccup) the
     * caller has to notice and re-ack on its own. Miss enough of these and the engine's timeout
     * clock catches up regardless of whether the task is actually still cooking.
     */
    static void ack(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                    std::function<void()> send) noexcept {
        // Pull :id off the tail of the path.
        auto target = req.get_path();
        auto task_id = std::string{target.substr(target.rfind('/') + 1)};

        WorkerContext::EngineResponse engine_res;
        try {
            engine_res = s_ctx->call_engine(
                "PATCH", "/api/v1/tasks/" + task_id + "/heartbeat");
        } catch (const std::exception &e) {
            core::logger::error("worker/ack", "heartbeat exception: {}", e.what());
            core::events::publish("worker.ack.heartbeat_exception", {{"error", e.what()}});
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
            return;
        }

        // Map the heartbeat outcome onto this handler's status codes — no retry on our end.
        if (engine_res.m_status == 200) {
            res.set_status(interfaces::io::types::Status::OK);
        } else if (engine_res.m_status == 404) {
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
        } else {
            res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        }
        send();
    }

  private:
    /**
     * @brief Hand-rolls the JSON body submitted back to the engine after a task runs —
     * `{"result": "...", "output_data": {...}}`.
     * @param result the task outcome to serialize as a string.
     * @param data the output key/value pairs to serialize into `output_data`.
     * @return the JSON string, ready to POST as-is.
     * @warning No escaping happens on the keys/values dropped into the JSON — a value containing
     * a `"` or backslash produces straight-up invalid JSON, no cap. Fine as long as task output
     * stays to "safe" characters, but that's an assumption baked in, not something enforced here.
     */
    static std::string
    build_submit_json(model::TaskResult result,
                      const std::unordered_map<std::string, std::string> &data) {
        // Map the enum onto its wire string first.
        std::string_view result_str;
        switch (result) {
        case model::TaskResult::SUCCESS:
            result_str = "SUCCESS";
            break;
        case model::TaskResult::FAILURE:
            result_str = "FAILURE";
            break;
        case model::TaskResult::TIMEOUT:
            result_str = "TIMEOUT";
            break;
        case model::TaskResult::SKIPPED:
            result_str = "SKIPPED";
            break;
        }

        // Hand-roll the JSON — open the object, drop in "result", start "output_data".
        std::string json = std::format(R"({{"result":"{}","output_data":{{)", result_str);
        // Join every key/value pair with commas, no escaping (see the warning above — this
        // is straight-up cooked if a value carries a quote or backslash).
        bool first = true;
        for (const auto &[key, value] : data) {
            if (!first) {
                json += ',';
            }
            json += std::format(R"("{}":"{}")", key, value);
            first = false;
        }
        // Close out both the output_data object and the outer object.
        json += "}}";
        return json;
    }

    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker

#ifdef CONGELADO_TEST
namespace worker::poll_handler_tests {
using namespace boost::ut;

/// @brief Minimal IWorker double, just enough to register a task type with the context so
/// poll()'s guard clause lets a request through to the engine-query stage.
class FakeWorker final : public interfaces::IWorker {
  public:
    explicit FakeWorker(std::string task_type) : m_task_type{std::move(task_type)} {}
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return m_task_type; }

  private:
    std::string m_task_type;
};

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
/// call_engine() below throws deterministically and synchronously instead of blocking on its
/// no-timeout future — see WorkerContext::call_engine's own test suite for why a real round-trip
/// is out of scope here.
void install_throwing_client(WorkerContext &ctx) {
    static ThrowingTestClient client;
    ctx.set_runtime(client);
}

suite<"PollHandler::poll"> poll_handler_poll_suite = [] {
    "replies BAD_REQUEST when no worker is registered for :type — never reaches the engine call"_test =
        [] {
            WorkerContext ctx;
            PollHandler::bind(ctx);
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_header(interfaces::io::types::Token::PATH, "/api/v1/worker/poll/unregistered");
            bool sent = false;

            PollHandler::poll(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::BAD_REQUEST);
        };

    "replies 500 when the registered type's engine queue query throws"_test = [] {
        WorkerContext ctx;
        install_throwing_client(ctx);
        FakeWorker echo{"echo"};
        ctx.add_worker(echo);
        PollHandler::bind(ctx);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/worker/poll/echo");
        bool sent = false;

        PollHandler::poll(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
    };
};

suite<"PollHandler::ack"> poll_handler_ack_suite = [] {
    "replies 500 when the heartbeat engine call throws"_test = [] {
        WorkerContext ctx;
        install_throwing_client(ctx);
        PollHandler::bind(ctx);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/worker/ack/task-1");
        bool sent = false;

        PollHandler::ack(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
    };
};

} // namespace worker::poll_handler_tests
#endif
