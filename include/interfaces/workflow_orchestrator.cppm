export module interfaces:workflow_orchestrator;

import std;
import :serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace interfaces {

/// @brief Pluggable workflow-orchestrator backend (the "workflow_orchestrator" capability) —
/// owns the workflow EXECUTION LIFECYCLE (the DAG side), the counterpart to
/// `IWorkerOrchestrator` (which owns task DISPATCH). Single-active-backend, resolved by the
/// engine host before build() (see `workflow_orchestrator_ctx`). Every op is asynchronous
/// (callback-per-op), mirroring `connector::Connector` and the route handlers' `send` callback,
/// so the engine's already-async orchestration wires in with no blocking. Dependency-free at
/// the boundary (ids/refs as strings, variables/payload as flat key/value maps) so `interfaces`
/// stays free of any model/serde coupling; a concrete backend's config arrives through the
/// plugin's own `on_load(host, cfg)`.
class IWorkflowOrchestrator
{
public:
    /**
     * @brief Virtual dtor, default's good — polymorphic workflow-orchestrator backends clean up
     * fine through the base pointer.
     */
    virtual ~IWorkflowOrchestrator() = default;
    IWorkflowOrchestrator() = default;
    IWorkflowOrchestrator(const IWorkflowOrchestrator&) = delete;
    IWorkflowOrchestrator& operator=(const IWorkflowOrchestrator&) = delete;
    IWorkflowOrchestrator(IWorkflowOrchestrator&&) = delete;
    IWorkflowOrchestrator& operator=(IWorkflowOrchestrator&&) = delete;

    /**
     * @brief Tells you which workflow-orchestrator backend is running (the built-in in-process
     * DAG walker, a distributed one, whatever got plugged in).
     * @return the backend's name.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;

    /**
     * @brief Says whether this backend is load-bearing. Defaults to false — a soft degrade.
     * @return false by default.
     */
    [[nodiscard]] virtual bool required() const noexcept
    {
        return false;
    }

    /**
     * @brief Starts a new execution of a WorkflowDef, delivering its execution id — spawns the
     * DAG's start nodes.
     * @param def_name the WorkflowDef to start.
     * @param variables seed variable bindings for the new execution.
     * @param callback gets the new execution id, or std::nullopt if the def doesn't exist.
     */
    virtual void start_workflow(
        std::string_view def_name,
        const std::unordered_map<std::string, std::string>& variables,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) = 0;
    /**
     * @brief Reacts to a task reaching a terminal status — the hook that advances the owning
     * execution's DAG. The engine fires this when a task result lands.
     * @param task_id the just-terminated task instance's id.
     * @param callback gets true if the advance was applied.
     */
    virtual void
    on_task_terminal(std::string_view task_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Propagates a just-terminated execution's outcome to its parent (for a SUB_WORKFLOW
     * child) — the engine calls this after an external terminate persists the terminal status.
     * @param exec_id the execution that just reached a terminal status.
     * @param callback gets true once applied.
     */
    virtual void on_execution_terminal(
        std::string_view exec_id, std::move_only_function<void(bool)> callback
    ) = 0;

    /**
     * @brief Pauses a RUNNING execution — stops spawning new instances until resumed.
     * @param exec_id the execution to pause.
     * @param callback gets true if it was found and RUNNING.
     */
    virtual void pause(std::string_view exec_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Resumes a PAUSED execution back to RUNNING and re-advances.
     * @param exec_id the execution to resume.
     * @param callback gets true if it was found and PAUSED.
     */
    virtual void resume(std::string_view exec_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Retries a FAILED execution from where it stopped.
     * @param exec_id the execution to retry.
     * @param callback gets true if the retry was applied.
     */
    virtual void retry(std::string_view exec_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Restarts a terminal execution from scratch (same exec_id, fresh run).
     * @param exec_id the execution to restart.
     * @param callback gets true if the restart was applied.
     */
    virtual void
    restart(std::string_view exec_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Terminates a non-terminal execution outright.
     * @param exec_id the execution to terminate.
     * @param callback gets true if it was found and wasn't already terminal.
     */
    virtual void
    terminate(std::string_view exec_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Re-derives what should be scheduled next for an execution — the self-healing
     * reconcile.
     * @param exec_id the execution to reconcile.
     * @param callback gets true if the execution/def was found.
     */
    virtual void
    reconcile(std::string_view exec_id, std::move_only_function<void(bool)> callback) = 0;
    /**
     * @brief Reruns a single node with fresh input, leaving every other instance untouched.
     * @param exec_id the execution to rerun a node in.
     * @param node_ref the node to reset.
     * @param input the new input for that node's re-run.
     * @param callback gets true if applied.
     */
    virtual void rerun(
        std::string_view exec_id,
        std::string_view node_ref,
        const interfaces::Value& input,
        std::move_only_function<void(bool)> callback
    ) = 0;
    /**
     * @brief Applies an external signal to a specific WAIT/HUMAN node instance.
     * @param exec_id the owning execution.
     * @param node_ref which node's instance to signal.
     * @param payload optional payload merged into the instance's output.
     * @param callback gets true if an eligible instance was found and signaled.
     */
    virtual void signal(
        std::string_view exec_id,
        std::string_view node_ref,
        std::optional<std::string_view> payload,
        std::move_only_function<void(bool)> callback
    ) = 0;

    /**
     * @brief Completes a node's instance by `(exec_id, node_ref)` with a success/failure
     * outcome + output, then cascades the DAG — the generic external-signal completion path
     * (behind `POST /api/v1/queue/update`) for callers that key off the execution + node
     * reference rather than an internal task id.
     * @param exec_id the owning execution.
     * @param node_ref which node's instance to complete.
     * @param success whether the node succeeded.
     * @param output output to attach to the instance.
     * @param callback gets true once applied.
     */
    virtual void complete_task(
        std::string_view exec_id,
        std::string_view node_ref,
        bool success,
        const std::unordered_map<std::string, std::string>& output,
        std::move_only_function<void(bool)> callback
    ) = 0;

    /**
     * @brief Stands up the backend's own inbound surface, if any. Called once by the engine
     * host.
     */
    virtual void start_server() = 0;
    /**
     * @brief Tears the backend down — called at engine shutdown before the plugin unloads.
     */
    virtual void shutdown_all() = 0;
};

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::workflow_orchestrator_tests {

// Minimal IWorkflowOrchestrator fixture — every pure virtual gets a trivial body so
// required()'s default implementation can be exercised in isolation.
class MockWorkflowOrchestrator final : public IWorkflowOrchestrator
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "mock";
    }

    void start_workflow(
        std::string_view,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(std::nullopt);
    }

    void on_task_terminal(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void on_execution_terminal(
        std::string_view, std::move_only_function<void(bool)> callback
    ) override
    {
        callback(false);
    }

    void pause(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void resume(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void retry(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void restart(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void terminate(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void reconcile(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(false);
    }

    void rerun(
        std::string_view,
        std::string_view,
        const interfaces::Value&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(false);
    }

    void signal(
        std::string_view,
        std::string_view,
        std::optional<std::string_view>,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(false);
    }

    void complete_task(
        std::string_view,
        std::string_view,
        bool,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(false);
    }

    void start_server() override {}

    void shutdown_all() override {}
};

using namespace boost::ut;

suite<"IWorkflowOrchestrator"> workflow_orchestrator_suite = [] {
    "required() defaults to false when not overridden"_test = [] {
        MockWorkflowOrchestrator orchestrator;
        expect(!orchestrator.required());
    };
};

} // namespace interfaces::workflow_orchestrator_tests
#endif
