export module worker_runtime:context;

import std;
import interfaces;
import serde;
import core_logger;
import core_client;
#ifdef CONGELADO_TEST
import shared;
import boost.ut;
#endif

export namespace worker {

/// @brief Completion for `WorkerContext::run_task_async` — an `unexpected` `WorkerError` means the
/// task type is unregistered or `execute_async` threw (FAILURE); a present `WorkerOutput` means the
/// worker ran to completion (SUCCESS), matching `run_task`'s `WorkerResult` return contract.
using TaskCompletion = std::move_only_function<void(interfaces::WorkerResult)>;

/// @brief Worker identity + the registered `interfaces::IWorker` registry + engine HTTP comm.
/// Task workers are first-class `IWorker` plugins the host resolves and hands over via
/// `add_worker`; this context keys them by `get_task_type()` and runs them. Engine traffic goes
/// through this context's own `core::client::Register` (the host points it at the engine
/// connection via `set_runtime()`) — non-copyable/non-movable by design, so that Register member
/// stays put at one stable address for the process's whole lifetime, same as the object owning
/// it. Workers themselves are stateless — all persistence lives engine-side.
class WorkerContext {
  public:
    struct EngineResponse {
        int m_status{0};
        std::string m_body;
    };

    WorkerContext() = default;
    WorkerContext(WorkerContext const &) = delete;
    WorkerContext &operator=(WorkerContext const &) = delete;
    WorkerContext(WorkerContext &&) = delete;
    WorkerContext &operator=(WorkerContext &&) = delete;
    ~WorkerContext() = default;

    /// @brief Sets this worker's identifier. @param worker_id the id this worker identifies as.
    void set_worker_id(std::string_view worker_id) { m_worker_id = std::string{worker_id}; }

    /// @brief Points call_engine() at the transport that actually ships requests — call once
    /// before the first call_engine(), same "bind the transport once" deal every core_client
    /// runtime needs.
    /// @param client the runtime client to bind; kept by reference, must outlive this context.
    void set_runtime(interfaces::IClient &client) { m_register.set_runtime(client); }

    /// @brief Matches an incoming response back to its pending call_engine() and fires the
    /// stored callback — forwards to this context's own Register. What the transport layer calls
    /// the second a response shows up.
    /// @param request the original request, used only to read its stream id.
    /// @param response the response that arrived; handed straight to the stored callback.
    void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response) {
        m_register.dispatch(request, response);
    }

    /// @brief Registers a worker, keyed by its task type — last one for a type wins.
    /// @param worker the worker to register (non-owning; the host keeps it alive).
    void add_worker(interfaces::IWorker &worker) {
        m_workers[std::string{worker.get_task_type()}] = &worker;
    }

    /// @brief Gets this worker's id. @return the worker id.
    [[nodiscard]] std::string_view get_worker_id() const noexcept { return m_worker_id; }

    /// @brief Looks up the registered worker for a task type. @param task_type the type to look up.
    /// @return the matching IWorker, or nullptr if none is registered.
    [[nodiscard]] interfaces::IWorker *get_task_worker(std::string_view task_type) const noexcept {
        auto found = m_workers.find(std::string{task_type});
        return found == m_workers.end() ? nullptr : found->second;
    }

    /// @brief Every registered task type. @return the task type names.
    [[nodiscard]] std::vector<std::string_view> get_task_types() const noexcept {
        std::vector<std::string_view> types;
        types.reserve(m_workers.size());
        for (const auto &[type, worker] : m_workers) {
            types.push_back(type);
        }
        return types;
    }

    /// @brief Runs a registered worker by type against a dynamic input value — the failure-catching
    /// wrapper around `IWorker::execute`, firing on_error/on_released hooks.
    /// @param task_type the task type to run. @param input the task's input value.
    /// @return the worker's output on success, or a WorkerError if unregistered or execute threw.
    [[nodiscard]] interfaces::WorkerResult
    run_task(std::string_view task_type,
             const serde::Value &input) {
        auto *worker = get_task_worker(task_type);
        if (worker == nullptr) {
            return std::unexpected{interfaces::WorkerError{"unregistered worker type"}};
        }
        interfaces::WorkerResult output =
            std::unexpected{interfaces::WorkerError{"unknown error"}};
        try {
            output = worker->execute(input);
        } catch (const std::exception &error) {
            worker->on_error(error.what());
            output = std::unexpected{interfaces::WorkerError{error.what()}};
        } catch (...) {
            worker->on_error("unknown error");
            output = std::unexpected{interfaces::WorkerError{"unknown error"}};
        }
        worker->on_released();
        return output;
    }

    /// @brief Runs a registered worker by type asynchronously — the `execute_async` counterpart to
    /// `run_task`. `completion` fires exactly once with the task's WorkerResult.
    /// @param task_type the task type to run. @param input the task's input value.
    /// @param completion fired once with the task's outcome.
    /// @return true if the worker completed before this call returned (`completion` already fired),
    /// false if it parked and `completion` will fire later, from another thread.
    bool run_task_async(std::string_view task_type,
                        const serde::Value &input,
                        TaskCompletion completion) {
        auto *worker = get_task_worker(task_type);
        if (worker == nullptr) {
            completion(std::unexpected{interfaces::WorkerError{"unregistered worker type"}});
            return true;
        }
        auto shared_completion = std::make_shared<TaskCompletion>(std::move(completion));
        auto finished = std::make_shared<bool>(false);
        auto finish = [worker, shared_completion, finished](interfaces::WorkerResult output) {
            if (*finished) {
                return;
            }
            *finished = true;
            worker->on_released();
            (*shared_completion)(std::move(output));
        };
        try {
            worker->execute_async(input, finish);
        } catch (const std::exception &error) {
            worker->on_error(error.what());
            if (!*finished) {
                *finished = true;
                worker->on_released();
                (*shared_completion)(std::unexpected{interfaces::WorkerError{error.what()}});
            }
        } catch (...) {
            worker->on_error("unknown error");
            if (!*finished) {
                *finished = true;
                worker->on_released();
                (*shared_completion)(std::unexpected{interfaces::WorkerError{"unknown error"}});
            }
        }
        return *finished;
    }

    /// @brief Blocking http2 call to the engine through this context's own Register — builds a
    /// request off the bound runtime client, sends it, and blocks until the response is
    /// correlated back by stream id.
    /// @param method the HTTP method. @param path the request path. @param body optional JSON body.
    /// @return the engine's status + body.
    /// @warning Never call from the transport's own dispatch thread — instant deadlock, no timeout.
    /// @warning Aborts the process if set_runtime() was never called — same fail-fast deal as
    /// every other core_client entry point.
    [[nodiscard]] EngineResponse call_engine(std::string_view method, std::string_view path,
                                             std::string_view body = "") {
        if (!m_register.has_runtime()) {
            std::abort();
        }
        auto request = core::client::Client::custom(method, path).build(m_register.runtime());
        // The engine's content-negotiation middleware picks the response serde format off Accept —
        // without it every reply with a body is a 406. Engine traffic is JSON both ways.
        request->add_accept("application/json");
        if (!body.empty()) {
            std::move(*request).with_content_type("application/json");
            std::vector<std::byte> body_bytes;
            body_bytes.reserve(body.size());
            for (char character : body) {
                body_bytes.push_back(static_cast<std::byte>(character));
            }
            request->set_body(std::move(body_bytes));
        }
        std::promise<EngineResponse> promise;
        auto future = promise.get_future();
        m_register.send(
            std::move(request), [&promise](interfaces::io::IResponse &response) {
                auto &view = response.get_body();
                std::string resp_body;
                resp_body.reserve(view.size());
                for (auto byte : view) {
                    resp_body.push_back(static_cast<char>(byte));
                }
                promise.set_value(
                    {.m_status = static_cast<int>(
                         interfaces::io::types::status_code(response.get_status())),
                     .m_body = std::move(resp_body)});
            });
        return future.get();
    }

  private:
    std::string m_worker_id;
    std::unordered_map<std::string, interfaces::IWorker *> m_workers;
    core::client::Register m_register;
};

} // namespace worker

#ifdef CONGELADO_TEST
namespace worker::context_tests {
using namespace boost::ut;

/// @brief Minimal IWorker double — tracks call counts and lets each test dial in success,
/// std::exception, or a non-std::exception throw from execute().
class FakeWorker final : public interfaces::IWorker {
  public:
    explicit FakeWorker(std::string task_type) : m_task_type{std::move(task_type)} {}

    void set_result(interfaces::WorkerResult result) { m_result = std::move(result); }
    void set_throw_std_exception(bool value) noexcept { m_throw_std_exception = value; }
    void set_throw_unknown(bool value) noexcept { m_throw_unknown = value; }

    [[nodiscard]] std::string_view get_task_type() const noexcept override { return m_task_type; }

    [[nodiscard]] interfaces::WorkerResult execute(const interfaces::Value & /*input*/) override {
        ++m_execute_count;
        if (m_throw_std_exception) {
            throw std::runtime_error{"execute boom"};
        }
        if (m_throw_unknown) {
            throw 42; // NOLINT — deliberately not a std::exception, to hit the catch(...) branch.
        }
        return m_result;
    }
    void on_error(std::string_view message) noexcept override {
        ++m_on_error_count;
        m_last_error = std::string{message};
    }
    void on_released() noexcept override { ++m_on_released_count; }

    [[nodiscard]] int get_execute_count() const noexcept { return m_execute_count; }
    [[nodiscard]] int get_on_error_count() const noexcept { return m_on_error_count; }
    [[nodiscard]] int get_on_released_count() const noexcept { return m_on_released_count; }
    [[nodiscard]] const std::string &get_last_error() const noexcept { return m_last_error; }

  private:
    std::string m_task_type;
    interfaces::WorkerResult m_result{interfaces::WorkerOutput{}};
    bool m_throw_std_exception{false};
    bool m_throw_unknown{false};
    int m_execute_count{0};
    int m_on_error_count{0};
    int m_on_released_count{0};
    std::string m_last_error;
};

/// @brief Concrete IRequest double just complete enough for call_engine() to build a request
/// against — set_header/set_body are no-ops (store nothing meaningful), find_header hands back a
/// stub value. Mirrors core_client:registry's own RegisterFakeRequest test double.
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

suite<"WorkerContext::EngineResponse"> engine_response_suite = [] {
    "default-constructs with status 0 and an empty body"_test = [] {
        WorkerContext::EngineResponse response;
        expect(response.m_status == 0);
        expect(response.m_body.empty());
    };
};

suite<"WorkerContext identity/registry"> worker_context_registry_suite = [] {
    "set_worker_id/get_worker_id round-trip, starts empty"_test = [] {
        WorkerContext ctx;
        expect(ctx.get_worker_id().empty());
        ctx.set_worker_id("worker-1");
        expect(ctx.get_worker_id() == "worker-1");
    };

    "get_task_worker returns nullptr for an unregistered type"_test = [] {
        WorkerContext ctx;
        expect(ctx.get_task_worker("echo") == nullptr);
    };

    "add_worker registers by task_type; get_task_types lists everything registered"_test = [] {
        WorkerContext ctx;
        expect(ctx.get_task_types().empty());

        FakeWorker echo{"echo"};
        FakeWorker transform{"transform"};
        ctx.add_worker(echo);
        ctx.add_worker(transform);

        expect(ctx.get_task_worker("echo") == &echo);
        expect(ctx.get_task_worker("transform") == &transform);

        auto types = ctx.get_task_types();
        expect(types.size() == 2U);
        expect(std::ranges::find(types, "echo") != types.end());
        expect(std::ranges::find(types, "transform") != types.end());
    };

    "add_worker for a type already registered — the last registration wins"_test = [] {
        WorkerContext ctx;
        FakeWorker first{"echo"};
        FakeWorker second{"echo"};
        ctx.add_worker(first);
        ctx.add_worker(second);

        expect(ctx.get_task_worker("echo") == &second);
        expect(ctx.get_task_types().size() == 1U);
    };
};

suite<"WorkerContext::run_task"> worker_context_run_task_suite = [] {
    "returns an unregistered-worker error for an unknown type"_test = [] {
        WorkerContext ctx;
        auto result = ctx.run_task("missing", serde::Value{});
        expect(not result.has_value()) << fatal;
        expect(result.error().getMessage() == "unregistered worker type");
    };

    "returns the worker's output on success and calls on_released but not on_error"_test = [] {
        WorkerContext ctx;
        FakeWorker echo{"echo"};
        echo.set_result(interfaces::WorkerOutput{{"out", "ok"}});
        ctx.add_worker(echo);

        auto result = ctx.run_task("echo", serde::Value{});
        expect(result.has_value()) << fatal;
        expect(result->at("out") == "ok");
        expect(echo.get_execute_count() == 1);
        expect(echo.get_on_released_count() == 1);
        expect(echo.get_on_error_count() == 0);
    };

    "catches a std::exception from execute(), reports it via on_error and the returned WorkerError, still calls on_released"_test =
        [] {
            WorkerContext ctx;
            FakeWorker echo{"echo"};
            echo.set_throw_std_exception(true);
            ctx.add_worker(echo);

            auto result = ctx.run_task("echo", serde::Value{});
            expect(not result.has_value()) << fatal;
            expect(result.error().getMessage() == "execute boom");
            expect(echo.get_on_error_count() == 1);
            expect(echo.get_last_error() == "execute boom");
            expect(echo.get_on_released_count() == 1);
        };

    "catches a non-std::exception throw from execute(), reports \"unknown error\", still calls on_released"_test =
        [] {
            WorkerContext ctx;
            FakeWorker echo{"echo"};
            echo.set_throw_unknown(true);
            ctx.add_worker(echo);

            auto result = ctx.run_task("echo", serde::Value{});
            expect(not result.has_value()) << fatal;
            expect(result.error().getMessage() == "unknown error");
            expect(echo.get_on_error_count() == 1);
            expect(echo.get_last_error() == "unknown error");
            expect(echo.get_on_released_count() == 1);
        };
};

suite<"WorkerContext::run_task_async"> worker_context_run_task_async_suite = [] {
    "completes synchronously with an unregistered-worker error for an unknown type"_test = [] {
        WorkerContext ctx;
        bool completed = false;
        interfaces::WorkerResult captured{std::unexpected{interfaces::WorkerError{"unset"}}};

        auto finished = ctx.run_task_async(
            "missing", serde::Value{},
            [&completed, &captured](interfaces::WorkerResult result) noexcept {
                completed = true;
                captured = std::move(result);
            });

        expect(finished);
        expect(completed) << fatal;
        expect(not captured.has_value());
        expect(captured.error().getMessage() == "unregistered worker type");
    };

    // IWorker::execute_async() is documented to "always park the caller" — it queues the job onto
    // the worker's own TaskQueue rather than running it inline, and nothing here drains that queue
    // (that only happens once bound to a real contract via set_contract_group(), which needs a live
    // ContractGroup this unit test never stands up). So for a registered worker, run_task_async()
    // always parks: the completion never fires synchronously and the call returns false. This test
    // pins that real, documented behavior rather than forcing the queue to run.
    "parks for a registered type — completion does not fire synchronously, returns false"_test = [] {
        WorkerContext ctx;
        FakeWorker echo{"echo"};
        ctx.add_worker(echo);
        bool completed = false;

        auto finished = ctx.run_task_async("echo", serde::Value{},
                                           [&completed](interfaces::WorkerResult) noexcept {
                                               completed = true;
                                           });

        expect(not finished);
        expect(not completed);
    };
};

suite<"WorkerContext::call_engine"> worker_context_call_engine_suite = [] {
    // call_engine() blocks on a future with no timeout until the response is correlated back —
    // genuinely round-tripping it needs a live IClient whose send() actually completes, which
    // this binary never does (see core_client's own Register test suite for the same
    // "live-transport territory, not unit-testable in isolation" call). What IS safe and
    // deterministic to exercise: a client IS bound (so call_engine()'s own build() doesn't
    // std::abort() — it needs one to build a request through, same as production), but that
    // client's send() always throws synchronously, before future.get() is ever reached — no
    // blocking, no risk of hanging the shared test binary.
    static ThrowingTestClient client;

    "throws once the bound client's send() fails (GET, no body)"_test = [] {
        WorkerContext ctx;
        ctx.set_runtime(client);
        expect(throws<std::runtime_error>(
            [&] { [[maybe_unused]] auto response = ctx.call_engine("GET", "/api/v1/tasks"); }));
    };

    "throws once the bound client's send() fails (POST, with a body)"_test = [] {
        WorkerContext ctx;
        ctx.set_runtime(client);
        expect(throws<std::runtime_error>([&] {
            [[maybe_unused]] auto response =
                ctx.call_engine("POST", "/api/v1/tasks/1/result", R"({"result":"SUCCESS"})");
        }));
    };
};

} // namespace worker::context_tests
#endif
