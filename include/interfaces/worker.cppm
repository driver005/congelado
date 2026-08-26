module;

export module interfaces:worker;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif
import shared;
import :serde;
import :io;
import :client;

export namespace interfaces {

/// @brief A dynamic JSON-like value (string/bool/int/double/array/object), back of every task's
/// input and `IWorker`'s `execute`. Canonically defined in `interfaces:serde` (as
/// `rfl::Generic`) and re-exported by `serde` as `serde::Value` — `serde` imports `interfaces`,
/// so the alias flows that direction, never the reverse (which would cycle).

/// @brief A task's flat key/value output map.
using WorkerOutput = std::unordered_map<std::string, std::string>;

/// @brief A worker-reported failure — carried through `WorkerResult`'s error channel instead of
/// a magic `"..._status": "error"` key in the output map. Room to grow (a code/category)
/// without touching every worker's call sites again.
class WorkerError
{
public:
    explicit WorkerError(std::string message) :
        m_message{std::move(message)}
    {
    }

    [[nodiscard]] std::string_view getMessage() const noexcept
    {
        return m_message;
    }

private:
    std::string m_message;
};

/// @brief A task's outcome — either its output map, or the `WorkerError` explaining why it
/// didn't produce one. `unexpected` always means the task failed end to end (reported to the
/// engine as `TaskResult::FAILURE`); there's no other "error, but still counts as success"
/// channel.
using WorkerResult = std::expected<WorkerOutput, WorkerError>;

/// @brief Completion callback for `IWorker::execute_async` — fired once with the task's
/// outcome.
using WorkerCompletion = std::move_only_function<void(WorkerResult)>;

/// @brief A unit of task-execution logic — the first-class "worker" abstraction. A worker
/// declares which task type it handles and runs one task against a dynamic `Value` input,
/// returning a flat key/value output. Every worker dispatches through its own dedicated
/// `core::contract::Contract` (a `shared::TaskQueue`, bound via `set_contract_group`) instead
/// of running inline on the caller's — the SDK worker host claims tasks and calls
/// `execute_async`, which always parks the caller and completes later from this worker's own
/// contract.
class IWorker
{
public:
    /**
     * @brief Virtual dtor, default's good — polymorphic workers clean up fine through the base
     * pointer.
     */
    virtual ~IWorker() = default;
    IWorker() = default;
    IWorker(const IWorker&) = delete;
    IWorker& operator=(const IWorker&) = delete;
    IWorker(IWorker&&) = delete;
    IWorker& operator=(IWorker&&) = delete;

    /**
     * @brief Tells the runner which task type this worker handles — used to route claimed tasks
     * to the right worker.
     * @return the task type string.
     */
    [[nodiscard]] virtual std::string_view get_task_type() const noexcept = 0;

    /**
     * @brief Binds this worker's `TaskQueue` onto the host's shared contract group — call once
     * from `on_load` with `*core::contract::ContractGroup<>` and
     * `core::contract::ContractState::IDLE` (caller checks for null first). Templated so
     * `interfaces` never has to name a `core_contract` type — `core_contract` transitively
     * imports `core_events`, which imports `interfaces`, a real circular module dependency, not
     * a style choice. Mirrors `shared::HandlerBase::create` itself, which is generic for the
     * exact same reason.
     * @param controller the contract group to bind against.
     * @param args forwarded straight through to `TaskQueue::create` (e.g. the initial
     * `ContractState`).
     */
    template<typename TController, typename... Args>
    void set_contract_group(TController& controller, Args&&... args)
    {
        m_contract_group = static_cast<void*>(&controller);
        auto contract = m_queue.create(controller, std::forward<Args>(args)...);
        m_queue.set_wake([contract]() mutable {
            contract.schedule();
        });
    }

    /**
     * @brief Runs the task against `input` and returns its output synchronously — the actual
     * work for a worker with nothing to await. Override this (the default `run()` bridges into
     * it) unless the work is genuinely async-shaped, in which case override `run()` instead.
     * @param input the task's input.
     * @return the task's flat key/value output.
     */
    [[nodiscard]] virtual WorkerResult execute(const Value& input)
    {
        throw std::logic_error("IWorker: override execute() or run()");
    }

    /**
     * @brief Runs the task against `input` and reports its output via `on_complete` —
     * dispatched onto this worker's own `TaskQueue` contract, never the caller's thread. Always
     * parks the caller.
     * @param input the task's input.
     * @param on_complete fired once with the task's output.
     */
    void execute_async(const Value& input, WorkerCompletion on_complete)
    {
        m_queue.push([this, input = input, on_complete = std::move(on_complete)]() mutable {
            run(input, std::move(on_complete));
        });
    }

    /**
     * @brief Optional hook fired when `execute` throws — default no-op.
     * @param message the error text.
     */
    virtual void on_error(std::string_view /*message*/) noexcept {}

    /**
     * @brief Optional cleanup hook fired after `execute` completes (success or failure) —
     * default no-op.
     */
    virtual void on_released() noexcept {}

protected:
    /**
     * @brief The extension point for genuinely-async workers (issue, return, complete later
     * from whatever thread the response lands on). Default bridges into synchronous `execute()`
     * — a worker with nothing to await never needs to touch this. Runs on this worker's own
     * dedicated contract, never the caller's; may block that one contract's thread if it must
     * (e.g. a synchronous network call) without affecting anything else in the shared contract
     * pool. Call `on_complete` exactly once.
     * @param input the task's input.
     * @param on_complete fired once with the task's output.
     */
    virtual void run(const Value& input, WorkerCompletion on_complete)
    {
        on_complete(execute(input));
    }

private:
    void* m_contract_group{nullptr};
    shared::TaskQueue m_queue{"worker-queue"};
};

/// @brief What to bring up when spawning a worker instance. `worker_type` is the task-type
/// queue the worker serves (the engine keys dispatch off this), `concurrency` is how many poll
/// cycles it runs, and `config_path` is an opaque config file handed to the worker process —
/// the manager never reads it, it just forwards it to whatever it spawns.
struct WorkerSpec
{
    std::string worker_type;
    std::size_t concurrency{1};
    std::string config_path;
};

/// @brief A live worker instance's identity plus its last-known health snapshot. `endpoint` is
/// the transport address the manager reaches the worker on (an HTTP URL for the external
/// supervisor, a shared-memory key for a future in-process pool).
/// `alive`/`last_heartbeat_ms`/`in_flight` are the manager's health view — task dispatch still
/// lives engine-side, so these are supervision signals, not scheduling state.
struct WorkerInfo
{
    std::string worker_id;
    std::string worker_type;
    std::string endpoint;
    bool alive{false};
    std::uint64_t last_heartbeat_ms{0};
    std::size_t in_flight{0};
};

/// @brief Pluggable worker-manager backend (the "worker_manager" capability).
/// Single-active-backend, same shape as `ICron`/`ICache` — a deployment picks exactly one
/// worker manager at a time. It is transport-agnostic on purpose: the first backend supervises
/// the `engine_worker` process over HTTP, a later one can drive an in-process shared-memory
/// pool behind this same interface. It owns worker LIFECYCLE (spawn/start/stop/restart) and
/// HEALTH reporting only — the manager knows nothing about what a task does and never routes
/// work; workers still pull tasks from the engine queue, so task dispatch stays engine-side.
class IWorkerManager
{
public:
    /**
     * @brief Virtual dtor, default's good — polymorphic worker-manager backends clean up fine
     * through the base pointer.
     */
    virtual ~IWorkerManager() = default;
    IWorkerManager() = default;
    IWorkerManager(const IWorkerManager&) = delete;
    IWorkerManager& operator=(const IWorkerManager&) = delete;
    IWorkerManager(IWorkerManager&&) = delete;
    IWorkerManager& operator=(IWorkerManager&&) = delete;

    /**
     * @brief Tells you which worker-manager backend is actually running the show (the built-in
     * external process supervisor, a shared-memory pool, whatever got plugged in).
     * @return the backend's name.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;

    /**
     * @brief Says whether this backend is load-bearing. Defaults to false — with no worker
     * manager, workers just aren't auto-supervised (they can still be launched externally), so
     * its absence is a soft degrade, not a misconfiguration to abort on.
     * @return false by default.
     */
    [[nodiscard]] virtual bool required() const noexcept
    {
        return false;
    }

    /**
     * @brief Registers a worker the manager will poll for and dispatch to, keyed by its
     * `get_task_type()`. The host resolves each worker plugin's `IWorker` and hands it here
     * before the poll loop starts; the manager stores a non-owning reference, so the worker
     * must outlive the manager.
     * @param worker the worker to register.
     */
    virtual void add_worker(IWorker& worker) = 0;
    /**
     * @brief Runs one poll-execute-submit cycle across every registered worker's task type —
     * the host schedules this on its contract thread pool, one call per concurrency slot per
     * tick. The manager issues the http2 requests to the engine itself (it owns the poll
     * transport), so this is where the actual engine polling lives. Blocking: never call from
     * the transport's own dispatch thread.
     */
    virtual void poll_once() = 0;

    /**
     * @brief Registers the wake callback for one concurrency slot — a backend that supports
     * async parking (see `poll_slot`) calls this to resume the slot's contract once a parked
     * task's response lands, from whatever thread that happens on. Default no-op for backends
     * that don't park (they only ever run inline through `poll_once`/`poll_slot`'s default).
     * @param slot the concurrency slot index. @param wake the resume callback.
     */
    virtual void register_poll_slot(std::size_t /*slot*/, std::move_only_function<void()> /*wake*/)
    {
    }

    /**
     * @brief Runs one poll-execute-submit cycle for a single concurrency slot — the async-aware
     * counterpart to `poll_once()`. A backend may park (return without finishing a claimed
     * task) and resume later via the wake callback registered through `register_poll_slot`,
     * instead of blocking the calling thread. Default bridges straight to `poll_once()`.
     * @param slot the concurrency slot index to run this cycle for.
     */
    virtual void poll_slot(std::size_t /*slot*/)
    {
        poll_once();
    }

    /**
     * @brief Points this manager's own engine HTTP comm (poll/ack/submit) at the transport that
     * actually ships requests — called once by the host after the engine connection is up,
     * before the first poll_once()/poll_slot(). Default no-op for backends with no engine comm
     * of their own (e.g. a shared-memory pool talking to the host process directly).
     * @param client the runtime client to bind; kept by reference, must outlive this manager.
     */
    virtual void set_runtime(IClient& /*client*/) {}

    /**
     * @brief Matches an incoming response back to whatever this manager's own engine HTTP comm
     * has pending, keyed by stream id — the transport layer calls this for every response that
     * arrives on the connection set_runtime() bound. Default no-op for backends with no engine
     * comm of their own (mirrors set_runtime()'s default).
     * @param request the original request, used only to read its stream id.
     * @param response the response that arrived; handed to whichever pending call it matches.
     */
    virtual void dispatch(io::IRequest& /*request*/, io::IResponse& /*response*/) {}

    /**
     * @brief Stands up the manager's own inbound HTTP API server
     * (health/info/poll/ack/executions) using the host-supplied contract group + leverager.
     * Called once by the host after the engine connection is up and workers are registered —
     * separate from construction because it needs the serde registry populated (config parsing)
     * and the workers registered (for /info). A backend with no inbound server just no-ops.
     */
    virtual void start_server() = 0;

    /**
     * @brief Brings up a new worker instance for the given spec.
     * @param spec the worker type/concurrency/config to launch.
     * @return the newly assigned worker_id, or std::nullopt if the spawn failed.
     */
    [[nodiscard]] virtual std::optional<std::string> spawn(const WorkerSpec& spec) = 0;
    /**
     * @brief Starts a spawned-but-stopped worker so it begins polling the engine queue.
     * @param worker_id the worker to start.
     * @return true if the worker transitioned to running, false if unknown or already running.
     */
    virtual bool start(std::string_view worker_id) = 0;
    /**
     * @brief Stops a running worker, leaving its identity registered so it can be restarted.
     * @param worker_id the worker to stop.
     * @return true if the worker was stopped, false if unknown or already stopped.
     */
    virtual bool stop(std::string_view worker_id) = 0;
    /**
     * @brief Stops then starts a worker, keeping the same worker_id.
     * @param worker_id the worker to restart.
     * @return true if the worker came back up, false if unknown.
     */
    virtual bool restart(std::string_view worker_id) = 0;
    /**
     * @brief Tears down every managed worker — called at host shutdown before the plugin
     * unloads.
     */
    virtual void shutdown_all() = 0;

    /**
     * @brief Lists every worker the manager currently tracks, with its latest health snapshot.
     * @return the tracked workers.
     */
    [[nodiscard]] virtual std::vector<WorkerInfo> list() const = 0;
    /**
     * @brief Fetches one worker's identity plus latest health snapshot.
     * @param worker_id the worker to look up.
     * @return the snapshot, or std::nullopt if the worker_id isn't tracked.
     */
    [[nodiscard]] virtual std::optional<WorkerInfo> status(std::string_view worker_id) const = 0;
    /**
     * @brief Installs the callback the backend invokes whenever a worker's health snapshot
     * changes (heartbeat, liveness flip, in-flight count). Called once, before any worker is
     * spawned.
     * @param callback the health callback; takes the updated snapshot.
     */
    virtual void set_health_callback(std::move_only_function<void(const WorkerInfo&)> callback) = 0;

    /**
     * @brief Registers app-supplied TaskDefs with the engine on load. Each string is a
     * serialized TaskDef (JSON) — the manager POSTs it to the engine's task endpoint over its
     * existing engine connection. Kept as raw strings so this interface stays free of any
     * model/serde dependency. Must be idempotent: a worker restart re-registering the same defs
     * is a no-op. Default no-op for backends with no engine connection.
     * @param defs_json the serialized TaskDefs to register.
     */
    virtual void register_task_defs(const std::vector<std::string>& /*defs_json*/) {}

    /**
     * @brief Registers app-supplied WorkflowDefs with the engine on load — same contract as
     * register_task_defs, POSTed to the engine's workflow endpoint. Idempotent. Default no-op.
     * @param defs_json the serialized WorkflowDefs to register.
     */
    virtual void register_workflow_defs(const std::vector<std::string>& /*defs_json*/) {}
};

/// @brief The C++-builder counterpart to app def files: an app plugin exporting this hands the
/// worker host serialized TaskDef/WorkflowDef JSON it built in code, which the host registers
/// with the engine on load exactly like the file-based defs. Returned as strings so this
/// interface stays free of any model/serde dependency. Many app plugins may export it; the host
/// collects every one.
class IAppDefs
{
public:
    /**
     * @brief Virtual dtor, default's good — polymorphic app-def providers clean up fine through
     * the base pointer.
     */
    virtual ~IAppDefs() = default;
    IAppDefs() = default;
    IAppDefs(const IAppDefs&) = delete;
    IAppDefs& operator=(const IAppDefs&) = delete;
    IAppDefs(IAppDefs&&) = delete;
    IAppDefs& operator=(IAppDefs&&) = delete;

    /**
     * @brief The app's code-built TaskDefs, each serialized (JSON).
     * @return the serialized TaskDefs, empty if none.
     */
    [[nodiscard]] virtual std::vector<std::string> get_task_defs() const = 0;
    /**
     * @brief The app's code-built WorkflowDefs, each serialized (JSON).
     * @return the serialized WorkflowDefs, empty if none.
     */
    [[nodiscard]] virtual std::vector<std::string> get_workflow_defs() const = 0;
};

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::tests {
using namespace boost::ut;

suite<"WorkerError"> worker_error_suite = [] {
    "getMessage returns exactly what the ctor stored"_test = [] {
        WorkerError error{"task timed out"};

        expect(error.getMessage() == "task timed out");
    };

    "empty message round-trips as empty"_test = [] {
        WorkerError error{""};

        expect(error.getMessage().empty());
    };
};

} // namespace interfaces::tests
#endif
