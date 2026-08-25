export module interfaces:worker_orchestrator;

import std;
import :serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace interfaces {

/// @brief The orchestrator's last-known health snapshot — the engine-side counterpart to
/// `WorkerInfo`. `queued_total`/`in_flight_total` are the dispatch backlog view; `alive` flips false
/// if the backend loses its store. Fed to the health callback whenever the snapshot changes.
struct OrchestratorInfo {
    std::string backend;
    bool alive{true};
    std::size_t queued_total{0};
    std::size_t in_flight_total{0};
};

/// @brief Pluggable worker-orchestrator backend (the "worker_orchestrator" capability) — the
/// engine-side counterpart to `IWorkerManager`. Single-active-backend, same shape as
/// `IWorkerManager`/`ICron`/`ICache`: a deployment picks exactly one at a time and the engine host
/// resolves it before build() (see `worker_orchestrator_ctx`). Where `IWorkerManager` owns worker
/// LIFECYCLE on the worker side, this owns task DISPATCH + workflow ORCHESTRATION on the engine side
/// — the queue workers claim from, result submission, and workflow start.
///
/// Every dispatch/orchestration op is **asynchronous**: it returns void and delivers its result
/// through a trailing callback, mirroring `connector::Connector`'s callback ops and the route
/// handlers' `send` callback — so the engine's already-async Orchestrator implements it with no
/// blocking. Kept dependency-free (flat key/value in/out + serialized JSON strings, like `IWorker`/
/// `IAppDefs`) so `interfaces` stays free of any model/serde coupling; the config for a concrete
/// backend arrives through the plugin's own `on_load(host, cfg)`, exactly as the external worker
/// manager reads its `config_path`.
class IWorkerOrchestrator {
  public:
    /**
     * @brief Virtual dtor, default's good — polymorphic orchestrator backends clean up fine through
     * the base pointer.
     */
    virtual ~IWorkerOrchestrator() = default;
    IWorkerOrchestrator() = default;
    IWorkerOrchestrator(const IWorkerOrchestrator &) = delete;
    IWorkerOrchestrator &operator=(const IWorkerOrchestrator &) = delete;
    IWorkerOrchestrator(IWorkerOrchestrator &&) = delete;
    IWorkerOrchestrator &operator=(IWorkerOrchestrator &&) = delete;

    /**
     * @brief Tells you which orchestrator backend is actually running the show (the built-in
     * in-process dispatcher, a distributed queue, whatever got plugged in).
     * @return the backend's name.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    /**
     * @brief Says whether this backend is load-bearing. Defaults to false — with no orchestrator
     * plugin the engine falls back to its built-in Orchestrator, so its absence is a soft degrade.
     * @return false by default.
     */
    [[nodiscard]] virtual bool required() const noexcept { return false; }

    /**
     * @brief Enqueues a task for a worker type, delivering the new task's id — the dispatch entry
     * point. Workers later claim() this and submit_result() against the returned id.
     * @param task_type the worker/queue type to enqueue under.
     * @param input the task's input as a dynamic value (object of string/int/bool/nested fields),
     * not a flat string map.
     * @param callback gets the new task id, or std::nullopt if the enqueue failed.
     */
    virtual void enqueue(std::string_view task_type, const interfaces::Value &input,
                         std::move_only_function<void(std::optional<std::string>)> callback) = 0;
    /**
     * @brief Claims the next ready task for `worker_type` (optionally scoped to `domain`), delivering
     * it as a serialized TaskInstance (JSON) — the same shape a worker polls today. std::nullopt
     * means nothing is queued.
     * @param worker_type the worker type claiming work.
     * @param domain optional isolation domain filter.
     * @param callback gets the serialized claimed TaskInstance, or std::nullopt if the queue is
     * empty.
     */
    virtual void claim(std::string_view worker_type, std::optional<std::string_view> domain,
                       std::move_only_function<void(std::optional<std::string>)> callback) = 0;
    /**
     * @brief Submits a worker's result for a claimed task, advancing the owning workflow.
     * @param task_id the claimed task's id.
     * @param success whether the task succeeded.
     * @param output the task's flat key/value output.
     * @param callback gets true if the result was recorded.
     */
    virtual void submit_result(std::string_view task_id, bool success,
                               const std::unordered_map<std::string, std::string> &output,
                               std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Requeues every stuck/in-flight task for a worker type back to ready — the recovery path
     * for a worker that claimed work and never reported back.
     * @param worker_type the worker type to requeue.
     * @param callback gets how many tasks were requeued.
     */
    virtual void requeue(std::string_view worker_type,
                         std::move_only_function<void(std::size_t)> callback) = 0;
    /**
     * @brief Current ready-queue depth for a worker type.
     * @param worker_type the worker type to measure.
     * @param callback gets the number of ready (claimable) tasks.
     */
    virtual void queue_size(std::string_view worker_type,
                            std::move_only_function<void(std::size_t)> callback) = 0;

    /**
     * @brief Starts a new execution of a WorkflowDef, delivering its execution id — the
     * orchestration entry point (spawns start nodes, which enqueue() their first tasks).
     * @param def_name the WorkflowDef to start.
     * @param variables seed variable bindings for the new execution.
     * @param callback gets the new execution id, or std::nullopt if the def doesn't exist.
     */
    virtual void start_workflow(std::string_view def_name,
                                const std::unordered_map<std::string, std::string> &variables,
                                std::move_only_function<void(std::optional<std::string>)> callback) = 0;

    /**
     * @brief Stands up the backend's own inbound surface, if any (a distributed backend may expose a
     * control API). Called once by the engine host after the backend is resolved. A backend with no
     * inbound surface just no-ops.
     */
    virtual void start_server() = 0;
    /**
     * @brief Tears the backend down — called at engine shutdown before the plugin unloads.
     */
    virtual void shutdown_all() = 0;

    /**
     * @brief Installs the callback the backend invokes whenever its health snapshot changes (queue
     * depth, liveness). Called once, before any dispatch.
     * @param callback the health callback; takes the updated snapshot.
     */
    virtual void
    set_health_callback(std::move_only_function<void(const OrchestratorInfo &)> callback) = 0;
};

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::worker_orchestrator_tests {

// Minimal IWorkerOrchestrator fixture — every pure virtual gets a trivial body so required()'s
// default implementation can be exercised in isolation.
class MockWorkerOrchestrator final : public IWorkerOrchestrator {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "mock"; }
    void enqueue(std::string_view, const interfaces::Value &,
                std::move_only_function<void(std::optional<std::string>)> callback) override {
        callback(std::nullopt);
    }
    void claim(std::string_view, std::optional<std::string_view>,
              std::move_only_function<void(std::optional<std::string>)> callback) override {
        callback(std::nullopt);
    }
    void submit_result(std::string_view, bool, const std::unordered_map<std::string, std::string> &,
                       std::move_only_function<void(bool)> callback) override {
        callback(false);
    }
    void requeue(std::string_view, std::move_only_function<void(std::size_t)> callback) override {
        callback(0);
    }
    void queue_size(std::string_view, std::move_only_function<void(std::size_t)> callback) override {
        callback(0);
    }
    void start_workflow(std::string_view, const std::unordered_map<std::string, std::string> &,
                        std::move_only_function<void(std::optional<std::string>)> callback) override {
        callback(std::nullopt);
    }
    void start_server() override {}
    void shutdown_all() override {}
    void set_health_callback(std::move_only_function<void(const OrchestratorInfo &)>) override {}
};

using namespace boost::ut;

suite<"IWorkerOrchestrator"> worker_orchestrator_suite = [] {
    "required() defaults to false when not overridden"_test = [] {
        MockWorkerOrchestrator orchestrator;
        expect(!orchestrator.required());
    };
};

} // namespace interfaces::worker_orchestrator_tests
#endif
