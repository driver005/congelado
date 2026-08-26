export module workflow_engine:orchestrator;

import std;
import model;
import connector;
import core_logger;
import :context;
import :expr;
import :system_task;
import :search_projector;
import :schema;
import core_events;
import shared;
import migration;
import serde;

export namespace engine {

// Wraps a flat string map into a dynamic Value object — the boundary where a node's input
// (gathered from predecessor outputs, which are still flat string maps) becomes the typed
// `serde::Value` a TaskInstance actually stores. Explicit per-call, not hidden behind a base
// class: the instance's stored input is genuinely a Value, typed fields survive from there on.
[[nodiscard]] serde::Value to_value_input(const std::unordered_map<std::string, std::string>& input)
{
    serde::Value::Object object;
    for (const auto& [key, value]: input) {
        object.emplace(key, serde::Value{std::string{value}});
    }
    return serde::Value{std::move(object)};
}

// The piece that was missing entirely before this module existed: something that actually walks
// a WorkflowDef's DAG as its TaskInstances complete, instead of a workflow start just inserting
// one row and never advancing again. Bound to the shared WorkflowContext the same way
// TaskHandler/ WorkflowHandler are — construct one per call site, it carries no state of its
// own besides the context reference.
//
// Phase 2 scope (see /home/default/.claude/plans/dazzling-snuggling-pine.md):
// TaskEdge::condition is now evaluated as a Lua boolean expression (edge_condition_ok(), via
// LuaEval) — an edge with no condition is still the unconditional/default case.
// JOIN/EXCLUSIVE_JOIN semantics are driven by TaskNode::join_on + join_type (ALL/ANY) rather
// than inferred from static edges. DYNAMIC TaskDefs get resolved via their dynamic_task_param
// at spawn time. InputMapping/OutputMapping resolution still only ever reads the *immediate*
// predecessor/last-completed instance's flat output_data map — no ancestor-wide
// `${ref.output.field}` resolution yet (Phase 8 backlog), and no nested JSON field paths
// (TaskInstance's data maps are flat strings, see task/instance.cppm).
//
// Phase 3 scope: TERMINATE/SET_VARIABLE/NOOP/LAMBDA/INLINE/JSON_JQ_TRANSFORM (plus JOIN/FORK as
// pure passthrough markers) now execute in-process via SystemTaskExecutor the instant they'd
// otherwise become a SCHEDULED-for-worker instance — see spawn_with_def() below.
//
// @warning DO_WHILE and FORK_JOIN_DYNAMIC are explicitly NOT wired up yet, despite TaskNode
// already carrying loop_body/loop_condition/dynamic_tasks_input_key (and WorkflowExecution
// carrying dynamic_nodes/DynamicTaskSpec) — the model groundwork is laid, but Orchestrator has
// no code path that ever makes a DO_WHILE or FORK_JOIN_DYNAMIC node eligible. A WorkflowDef
// using either today would have that node sit un-spawned forever (a visible, inert gap — not a
// silent miscalculation). Both need genuine cycle/runtime-node-materialization handling this
// pass deliberately didn't guess at without a way to execute and verify it; flagged as the next
// thing to build here, not shipped half-working.
class Orchestrator : public shared::HandlerBase
{
public:
    explicit Orchestrator(WorkflowContext& ctx) noexcept :
        m_ctx{ctx}
    {
    }

    /// @brief Contract handler identity, registered under this name via
    /// `HandlerBase::create()`.
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "engine.sweep";
    }

    /**
     * @brief The periodic background sweep, run as a `core::contract` handler instead of a raw
     * thread — catches everything the synchronous task-completion path structurally can't (a
     * worker that polled a task and never called back, an armed retry whose backoff has
     * elapsed, a node that couldn't spawn earlier because its TaskDef's RateLimitPolicy was at
     * capacity). Cron schedule firing lives in the cron plugin now (interfaces::ICron), not
     * here.
     * @note Waits on `migration::Status::is_ready()` before its first real sweep — the host's
     * one global migration pass runs after every plugin has loaded, so this contract may get
     * scheduled before its own tables are guaranteed to exist. No wall-clock delay primitive
     * exists in `core_contract` (confirmed) — pacing is the same "sleep inside the worker, then
     * self-reschedule via `shared::this_handler::shedule()`" pattern `src/worker_main.cc`'s
     * poll contracts already use, accepting that it blocks a pool thread for the sleep duration
     * each cycle.
     * @return the callable a `ContractGroup` invokes once scheduled.
     */
    shared::WorkerFunction on_execute() override
    {
        return [this]() {
            if (!migration::Status::is_ready()) {
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
                shared::this_handler::shedule();
                return;
            }
            sweep_timeouts();
            sweep_retries();
            sweep_advance();
            std::this_thread::sleep_for(std::chrono::seconds{5});
            shared::this_handler::shedule();
        };
    }

    /**
     * @brief Starts a new execution of `def_name`: 404s (via a nullopt callback) if the def
     * doesn't exist, otherwise builds a RUNNING WorkflowExecution, immediately spawns its DAG's
     * start nodes (the ones with no incoming edge) via advance(), and persists the whole thing
     * in one insert.
     * @param def_name the WorkflowDef to start.
     * @param variables seed variable bindings for the new execution.
     * @param parent_exec_id set for a SUB_WORKFLOW child (links it back to its parent
     * execution); std::nullopt for a top-level start or a START_WORKFLOW fire-and-forget child.
     * @param callback gets the created (and already-advanced) execution, or std::nullopt if
     * `def_name` doesn't exist.
     */
    void start(
        std::string def_name,
        std::unordered_map<std::string, std::string> variables,
        std::optional<model::ExecutionId> parent_exec_id,
        std::move_only_function<void(std::optional<model::WorkflowExecution>)> callback
    )
    {
        m_ctx.get().get_connector().find<model::WorkflowDef>(
            def_name,
            [this, variables = std::move(variables), parent_exec_id,
             callback = std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                if (!def) {
                    callback(std::nullopt);
                    return;
                }

                model::WorkflowExecution exec;
                exec.set_exec_id(model::generate_id());
                exec.set_def_name(def->get_name());
                exec.set_def_version(def->get_version());
                exec.set_status(model::WorkflowStatus::RUNNING);
                exec.set_variables(std::move(variables));
                exec.set_parent_exec_id(parent_exec_id);
                model::ExecutionTimings timings;
                timings.set_started_at(std::chrono::system_clock::now());
                exec.set_timings(timings);

                advance(
                    std::move(exec), *def,
                    [this,
                     callback = std::move(callback)](model::WorkflowExecution advanced) mutable {
                        auto to_return = advanced;
                        m_ctx.get().get_connector().insert<model::WorkflowExecution>(
                            std::move(advanced),
                            [callback = std::move(callback),
                             to_return = std::move(to_return)](bool oke) mutable {
                                callback(
                                    oke ? std::optional<model::WorkflowExecution>{std::move(
                                              to_return
                                          )}
                                        : std::nullopt
                                );
                            }
                        );
                    }
                );
            }
        );
    }

    /**
     * @brief Reacts to a TaskInstance reaching a terminal status — the single hook point that
     * turns "a worker/system task finished" into "the DAG actually moves." Called synchronously
     * from TaskHandler::submit_result() right after a result lands, and from the background
     * sweep's timeout handling.
     * @param instance the just-terminated instance; must already carry its terminal status and
     * output_data — this method reacts to those, it doesn't set them.
     */
    /**
     * @brief Propagates a just-terminated execution's outcome back to its parent, if it has one
     * (i.e. it was started as a SUB_WORKFLOW child) — finds the parent's waiting TaskInstance
     * (the one whose sub_workflow_exec_id matches `child`'s id) and completes/fails it to
     * match, which in turn feeds into the parent's own on_task_terminal() cascade. A no-op for
     * top-level executions and START_WORKFLOW fire-and-forget children (neither sets
     * parent_exec_id). Call this from every internal site that flips a WorkflowExecution to a
     * terminal status.
     * @param child the execution that just reached a terminal status.
     */
    void on_execution_terminal(model::WorkflowExecution child)
    {
        SummaryProjector{m_ctx.get()}.project_workflow(child);

        // WorkflowDef::workflow_status_listener_enabled — fires a "conductor:workflow_status"
        // event (Phase 5's dispatcher) so a subscribed EventHandler can react to a terminal
        // transition without polling. Reuses the internal event bus every EVENT-typed task
        // already publishes into (publish_event()'s own docs cover its "conductor:"-prefixed,
        // in-process-only scope).
        m_ctx.get().get_connector().find<model::WorkflowDef>(
            child.get_def_name(),
            [this, exec_id = std::format("{}", child.get_exec_id()),
             status = child.get_status()](std::optional<model::WorkflowDef> def) {
                if (def && def->get_workflow_status_listener_enabled()) {
                    publish_event(
                        "conductor:workflow_status",
                        {{"exec_id", exec_id}, {"status", std::string{status_name(status)}}}
                    );
                }
            }
        );

        if (!child.get_parent_exec_id()) {
            return;
        }
        m_ctx.get().get_connector().find_all<model::TaskInstance>(
            [this, child = std::move(child)](std::vector<model::TaskInstance> instances) mutable {
                for (auto& instance: instances) {
                    if (instance.get_sub_workflow_exec_id() != child.get_exec_id()) {
                        continue;
                    }
                    instance.set_status(
                        child.get_status() == model::WorkflowStatus::COMPLETED
                            ? model::TaskStatus::COMPLETED
                            : model::TaskStatus::FAILED
                    );
                    instance.set_output_data(child.get_variables());
                    auto to_terminal = instance;
                    m_ctx.get().get_connector().update<model::TaskInstance>(
                        instance, log_on_failure("on_execution_terminal parent instance update")
                    );
                    on_task_terminal(std::move(to_terminal));
                    return;
                }
            }
        );
    }

    void on_task_terminal(model::TaskInstance instance)
    {
        auto exec_key = std::format("{}", instance.get_workflow_exec_id());
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_key,
            [this,
             instance = std::move(instance)](std::optional<model::WorkflowExecution> exec) mutable {
                // no such execution, or it's already done — nothing left to advance into.
                if (!exec || model::is_terminal(exec->get_status())) {
                    return;
                }
                m_ctx.get().get_connector().find<model::WorkflowDef>(
                    exec->get_def_name(),
                    [this, instance = std::move(instance),
                     exec = std::move(*exec)](std::optional<model::WorkflowDef> def) mutable {
                        if (!def) {
                            return;
                        }
                        process_terminal(std::move(instance), std::move(exec), std::move(*def));
                    }
                );
            }
        );
    }

    /**
     * @brief Re-derives what should be scheduled next for every RUNNING execution — the
     * level-triggered counterpart to on_task_terminal()'s edge-triggered advancement. Catches
     * nodes that couldn't spawn earlier because their TaskDef's RateLimitPolicy was at
     * capacity, and is the general safety net for "nothing else was going to notice this was
     * ready." Called periodically by this class's own contract handler on_execute() (see
     * above).
     */
    void sweep_advance()
    {
        m_ctx.get().get_connector().find_all<model::WorkflowExecution>(
            [this](std::vector<model::WorkflowExecution> execs) {
                for (auto& exec: execs) {
                    if (exec.get_status() != model::WorkflowStatus::RUNNING) {
                        continue;
                    }
                    m_ctx.get().get_connector().find<model::WorkflowDef>(
                        exec.get_def_name(),
                        [this, exec](std::optional<model::WorkflowDef> def) mutable {
                            if (!def) {
                                return;
                            }
                            advance(
                                std::move(exec), std::move(*def),
                                [this](model::WorkflowExecution advanced) {
                                    m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                        std::move(advanced), log_on_failure("sweep_advance update")
                                    );
                                }
                            );
                        }
                    );
                }
            }
        );
    }

    /**
     * @brief Scans IN_PROGRESS instances whose deadline_at has passed and applies their
     * TaskDef::timeout's action: RETRY re-arms a retry per the def's RetryPolicy, FAIL_WORKFLOW
     * fails the owning execution outright, ALERT_ONLY just logs and leaves the instance running
     * (matches Conductor's own ALERT_ONLY semantics — it's a warning, not an intervention).
     */
    void sweep_timeouts()
    {
        auto now = std::chrono::system_clock::now();
        m_ctx.get().get_connector().find_all<model::TaskInstance>(
            [this, now](std::vector<model::TaskInstance> instances) {
                for (auto& instance: instances) {
                    if (instance.get_status() != model::TaskStatus::IN_PROGRESS) {
                        continue;
                    }
                    auto deadline = instance.get_deadline_at();
                    if (!deadline || *deadline > now) {
                        continue;
                    }
                    handle_timeout(std::move(instance));
                }
            }
        );
    }

    /**
     * @brief Scans FAILED/TIMED_OUT instances with an elapsed next_retry_at and flips them back
     * to SCHEDULED so poll() can pick them up again. Retry backoff can't just set SCHEDULED
     * immediately at failure time — poll() has no notion of "not yet due" — so a retry stays
     * parked in its terminal status with next_retry_at armed until this sweep releases it.
     */
    void sweep_retries()
    {
        auto now = std::chrono::system_clock::now();
        m_ctx.get().get_connector().find_all<model::TaskInstance>(
            [this, now](std::vector<model::TaskInstance> instances) {
                for (auto& instance: instances) {
                    auto next_retry = instance.get_next_retry_at();
                    if (!next_retry || *next_retry > now) {
                        continue;
                    }
                    bool retryable = instance.get_status() == model::TaskStatus::FAILED ||
                                     instance.get_status() == model::TaskStatus::TIMED_OUT;
                    if (!retryable) {
                        continue;
                    }
                    instance.set_status(model::TaskStatus::SCHEDULED);
                    instance.set_next_retry_at(std::nullopt);
                    instance.set_output_data({});
                    m_ctx.get().get_connector().update<model::TaskInstance>(
                        instance, log_on_failure("sweep_retries update")
                    );
                }
            }
        );
    }

    /**
     * @brief Pauses a RUNNING execution — advance() stops spawning new instances for it until
     * resume() flips it back. Already-IN_PROGRESS instances keep running; this only withholds
     * new work.
     * @param exec_id the execution to pause.
     * @param callback gets true if the execution was found and was RUNNING, false otherwise
     * (not found, or already in some other state).
     */
    void pause(std::string exec_id, std::move_only_function<void(bool)> callback)
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec || exec->get_status() != model::WorkflowStatus::RUNNING) {
                    callback(false);
                    return;
                }
                exec->set_status(model::WorkflowStatus::PAUSED);
                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                    std::move(*exec), [callback = std::move(callback)](bool oke) mutable {
                        callback(oke);
                    }
                );
            }
        );
    }

    /**
     * @brief Resumes a PAUSED execution back to RUNNING and immediately calls advance() to pick
     * up anything that became eligible while paused.
     * @param exec_id the execution to resume.
     * @param callback gets true if the execution was found and was PAUSED, false otherwise.
     */
    void resume(std::string exec_id, std::move_only_function<void(bool)> callback)
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec || exec->get_status() != model::WorkflowStatus::PAUSED) {
                    callback(false);
                    return;
                }
                exec->set_status(model::WorkflowStatus::RUNNING);
                m_ctx.get().get_connector().find<model::WorkflowDef>(
                    exec->get_def_name(),
                    [this, exec = std::move(*exec),
                     callback =
                         std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                        if (!def) {
                            m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                std::move(exec),
                                [callback = std::move(callback)](bool oke) mutable {
                                    callback(oke);
                                }
                            );
                            return;
                        }
                        advance(
                            std::move(exec), std::move(*def),
                            [this,
                             callback =
                                 std::move(callback)](model::WorkflowExecution advanced) mutable {
                                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                    std::move(advanced),
                                    [callback = std::move(callback)](bool oke) mutable {
                                        callback(oke);
                                    }
                                );
                            }
                        );
                    }
                );
            }
        );
    }

    /**
     * @brief Retries a FAILED execution from wherever it stopped: every FAILED/TIMED_OUT
     * instance goes back to SCHEDULED (retry_count and output cleared), the execution flips
     * back to RUNNING, then advance() picks up from there. Gated by WorkflowDef::restartable.
     * @param exec_id the execution to retry.
     * @param callback gets true if the retry was applied, false if the execution/def wasn't
     * found, wasn't FAILED, or the def forbids it (restartable == false).
     */
    void retry(std::string exec_id, std::move_only_function<void(bool)> callback)
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec || exec->get_status() != model::WorkflowStatus::FAILED) {
                    callback(false);
                    return;
                }
                m_ctx.get().get_connector().find<model::WorkflowDef>(
                    exec->get_def_name(),
                    [this, exec = std::move(*exec),
                     callback =
                         std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                        if (!def || !def->get_restartable()) {
                            callback(false);
                            return;
                        }
                        auto instances = exec.get_task_instances();
                        for (auto& instance: instances) {
                            if (instance.get_status() == model::TaskStatus::FAILED ||
                                instance.get_status() == model::TaskStatus::TIMED_OUT) {
                                instance.set_status(model::TaskStatus::SCHEDULED);
                                instance.set_retry_count(0);
                                instance.set_next_retry_at(std::nullopt);
                                instance.set_output_data({});
                                m_ctx.get().get_connector().update<model::TaskInstance>(
                                    instance, log_on_failure("retry instance update")
                                );
                            }
                        }
                        exec.set_task_instances(std::move(instances));
                        exec.set_status(model::WorkflowStatus::RUNNING);
                        advance(
                            std::move(exec), std::move(*def),
                            [this,
                             callback =
                                 std::move(callback)](model::WorkflowExecution advanced) mutable {
                                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                    std::move(advanced),
                                    [callback = std::move(callback)](bool oke) mutable {
                                        callback(oke);
                                    }
                                );
                            }
                        );
                    }
                );
            }
        );
    }

    /**
     * @brief Restarts a terminal execution from scratch: clears every task instance and
     * materialized dynamic node, flips back to RUNNING, and re-advances from the DAG's start
     * nodes — same exec_id, fresh run. Gated by WorkflowDef::restartable.
     * @param exec_id the execution to restart.
     * @param callback gets true if the restart was applied, false if the execution/def wasn't
     * found, wasn't in a terminal state, or the def forbids it.
     */
    void restart(std::string exec_id, std::move_only_function<void(bool)> callback)
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec || !model::is_terminal(exec->get_status())) {
                    callback(false);
                    return;
                }
                m_ctx.get().get_connector().find<model::WorkflowDef>(
                    exec->get_def_name(),
                    [this, exec = std::move(*exec),
                     callback =
                         std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                        if (!def || !def->get_restartable()) {
                            callback(false);
                            return;
                        }
                        exec.set_task_instances({});
                        exec.set_dynamic_nodes({});
                        exec.set_status(model::WorkflowStatus::RUNNING);
                        model::ExecutionTimings timings;
                        timings.set_started_at(std::chrono::system_clock::now());
                        exec.set_timings(timings);
                        advance(
                            std::move(exec), std::move(*def),
                            [this,
                             callback =
                                 std::move(callback)](model::WorkflowExecution advanced) mutable {
                                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                    std::move(advanced),
                                    [callback = std::move(callback)](bool oke) mutable {
                                        callback(oke);
                                    }
                                );
                            }
                        );
                    }
                );
            }
        );
    }

    /**
     * @brief Reruns a single node with fresh input: resets `node_ref`'s own instance to
     * SCHEDULED with the given input, leaving every other instance untouched, then flips the
     * execution back to RUNNING and advances.
     * @warning Simplified relative to Conductor's real rerun (which clones a whole new
     * execution and clears everything downstream of the rerun point) — this only resets the one
     * named node, not its downstream cascade. Good enough for "redo this one step with
     * different input," not a full replay-from-here. Gated by WorkflowDef::restartable.
     * @param exec_id the execution to rerun a node in.
     * @param node_ref the node to reset.
     * @param input the new input for that node's re-run — a dynamic Value (typed fields survive
     * straight into the instance's stored input_data).
     * @param callback gets true if applied, false if the execution/def/node wasn't found or the
     * def forbids it.
     */
    void rerun(
        std::string exec_id,
        std::string node_ref,
        serde::Value input,
        std::move_only_function<void(bool)> callback
    )
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this, node_ref = std::move(node_ref), input = std::move(input),
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec) {
                    callback(false);
                    return;
                }
                m_ctx.get().get_connector().find<model::WorkflowDef>(
                    exec->get_def_name(),
                    [this, exec = std::move(*exec), node_ref = std::move(node_ref),
                     input = std::move(input),
                     callback =
                         std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                        if (!def || !def->get_restartable()) {
                            callback(false);
                            return;
                        }
                        auto instances = exec.get_task_instances();
                        bool found = false;
                        for (auto& instance: instances) {
                            if (instance.get_node_ref() == node_ref) {
                                instance.set_status(model::TaskStatus::SCHEDULED);
                                instance.set_input_data(input);
                                instance.set_output_data({});
                                instance.set_retry_count(0);
                                instance.set_next_retry_at(std::nullopt);
                                m_ctx.get().get_connector().update<model::TaskInstance>(
                                    instance, log_on_failure("rerun instance update")
                                );
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            callback(false);
                            return;
                        }
                        exec.set_task_instances(std::move(instances));
                        exec.set_status(model::WorkflowStatus::RUNNING);
                        advance(
                            std::move(exec), std::move(*def),
                            [this,
                             callback =
                                 std::move(callback)](model::WorkflowExecution advanced) mutable {
                                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                    std::move(advanced),
                                    [callback = std::move(callback)](bool oke) mutable {
                                        callback(oke);
                                    }
                                );
                            }
                        );
                    }
                );
            }
        );
    }

    /**
     * @brief Applies an external SIGNAL to a specific WAIT/HUMAN instance — the counterpart to
     * an indefinite WAIT (no wait_duration_ms) or a HUMAN task, both of which otherwise only
     * ever complete via an explicit external call. Completes the named node's instance with
     * `payload` (if given) merged into its output_data, then cascades normally.
     * @param exec_id the owning execution.
     * @param node_ref which node's instance to signal.
     * @param payload optional payload merged into the instance's output_data under the key
     * `signal_payload`.
     * @param callback gets true if an IN_PROGRESS instance for that node_ref was found and
     * signaled, false otherwise.
     */
    void signal(
        std::string exec_id,
        std::string node_ref,
        std::optional<std::string> payload,
        std::move_only_function<void(bool)> callback
    )
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this, node_ref = std::move(node_ref), payload = std::move(payload),
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec) {
                    callback(false);
                    return;
                }
                const model::TaskInstance* found = nullptr;
                for (const auto& instance: exec->get_task_instances()) {
                    if (instance.get_node_ref() == node_ref &&
                        instance.get_status() == model::TaskStatus::IN_PROGRESS) {
                        found = &instance;
                        break;
                    }
                }
                if (found == nullptr) {
                    callback(false);
                    return;
                }
                auto instance = *found;
                instance.set_status(model::TaskStatus::COMPLETED);
                if (payload) {
                    instance.add_output_data("signal_payload", *payload);
                }
                m_ctx.get().get_connector().update<model::TaskInstance>(
                    instance, log_on_failure("signal instance update")
                );
                on_task_terminal(std::move(instance));
                callback(true);
            }
        );
    }

    /**
     * @brief The generic external-signal completion path for `POST /api/v1/queue/update` —
     * resolves `(exec_id, node_ref)` to the matching TaskInstance and completes/fails it,
     * feeding into the same cascade submit_result() uses. Callers that don't track internal
     * task ids (an external signal source keying off the execution + node reference instead)
     * use this rather than `POST /api/v1/tasks/:id/result`.
     * @param exec_id the owning execution.
     * @param node_ref which node's instance to complete.
     * @param status the terminal status to apply — typically COMPLETED or FAILED.
     * @param output_data output to attach to the instance.
     */
    void queue_update(
        std::string exec_id,
        std::string node_ref,
        model::TaskStatus status,
        std::unordered_map<std::string, std::string> output_data
    )
    {
        complete_instance_by_ref(
            std::move(exec_id), std::move(node_ref), status, std::move(output_data)
        );
    }

    /**
     * @brief Terminates a non-terminal execution outright — the same effect as `DELETE
     * /api/v1/workflows/exec/:id` (WorkflowHandler::terminate_execution()), factored out here
     * so `POST /api/v1/workflows/bulk/terminate` can reuse it across many executions without
     * duplicating the terminal-state guard + on_execution_terminal() propagation.
     * @param exec_id the execution to terminate.
     * @param callback gets true if it was found and wasn't already terminal, false otherwise.
     */
    void terminate(std::string exec_id, std::move_only_function<void(bool)> callback)
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec || model::is_terminal(exec->get_status())) {
                    callback(false);
                    return;
                }
                exec->set_status(model::WorkflowStatus::TERMINATED);
                auto timings = exec->get_timings();
                timings.set_completed_at(std::chrono::system_clock::now());
                exec->set_timings(timings);
                auto to_finish = *exec;
                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                    std::move(*exec), [this, to_finish = std::move(to_finish),
                                       callback = std::move(callback)](bool oke) mutable {
                        if (oke) {
                            on_execution_terminal(to_finish);
                        }
                        callback(oke);
                    }
                );
            }
        );
    }

    /**
     * @brief The self-healing admin op behind `POST /api/v1/admin/consistency/:exec_id` — just
     * re-runs advance()'s ordinary eligibility scan against whatever's currently persisted for
     * this execution. Genuinely "self-healing" rather than a no-op: advance() re-derives
     * eligibility fresh from exec.task_instances every time it's called, so anything that
     * should have been scheduled but wasn't (a rate-limit hold that never got revisited, a skip
     * cascade that stalled) gets picked back up here — the exact same mechanism sweep_advance()
     * already runs periodically, just invoked on demand for one execution.
     * @param exec_id the execution to reconcile.
     * @param callback gets true if the execution/def was found (regardless of whether anything
     * actually needed fixing), false otherwise.
     */
    void reconcile(std::string exec_id, std::move_only_function<void(bool)> callback)
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec) {
                    callback(false);
                    return;
                }
                m_ctx.get().get_connector().find<model::WorkflowDef>(
                    exec->get_def_name(),
                    [this, exec = std::move(*exec),
                     callback =
                         std::move(callback)](std::optional<model::WorkflowDef> def) mutable {
                        if (!def) {
                            callback(false);
                            return;
                        }
                        advance(
                            std::move(exec), std::move(*def),
                            [this,
                             callback =
                                 std::move(callback)](model::WorkflowExecution advanced) mutable {
                                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                    std::move(advanced),
                                    [callback = std::move(callback)](bool oke) mutable {
                                        callback(oke);
                                    }
                                );
                            }
                        );
                    }
                );
            }
        );
    }

private:
    std::reference_wrapper<WorkflowContext> m_ctx;

    static std::move_only_function<void(bool)> log_on_failure(std::string what)
    {
        return [what = std::move(what)](bool oke) {
            if (!oke) {
                core::logger::error("engine", "orchestrator: {} failed", what);
            }
        };
    }

    /// @brief Plain-text name for a WorkflowStatus — used only for event-payload/log text, not
    /// serde encoding (that already goes through reflect-cpp's own enum reflection).
    [[nodiscard]] static std::string_view status_name(model::WorkflowStatus status) noexcept
    {
        switch (status) {
            case model::WorkflowStatus::RUNNING:
                return "RUNNING";
            case model::WorkflowStatus::COMPLETED:
                return "COMPLETED";
            case model::WorkflowStatus::FAILED:
                return "FAILED";
            case model::WorkflowStatus::TIMED_OUT:
                return "TIMED_OUT";
            case model::WorkflowStatus::PAUSED:
                return "PAUSED";
            case model::WorkflowStatus::TERMINATED:
                return "TERMINATED";
        }
        return "UNKNOWN";
    }

    static const model::TaskNode*
    find_node(const model::WorkflowDef& def, std::string_view ref_name) noexcept
    {
        for (const auto& node: def.get_nodes()) {
            if (node.get_ref_name() == ref_name) {
                return &node;
            }
        }
        return nullptr;
    }

    static const model::TaskInstance* find_instance(
        const std::vector<model::TaskInstance>& instances, std::string_view node_ref
    ) noexcept
    {
        for (const auto& instance: instances) {
            if (instance.get_node_ref() == node_ref) {
                return &instance;
            }
        }
        return nullptr;
    }

    /// @brief Replaces `instance`'s entry in `exec`'s nested task_instances list (matched by
    /// task_id), or appends it if this is the first sync for that id — keeps the standalone
    /// TaskInstance row and WorkflowExecution's own nested copy in agreement, which the engine
    /// UI's execution/timeline views read directly off the latter.
    static void sync_instance(model::WorkflowExecution& exec, const model::TaskInstance& instance)
    {
        auto instances = exec.get_task_instances();
        bool replaced = false;
        for (auto& existing: instances) {
            if (existing.get_task_id() == instance.get_task_id()) {
                existing = instance;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            instances.push_back(instance);
        }
        exec.set_task_instances(std::move(instances));
    }

    /// @brief Evaluates a TaskEdge's condition (if any) as a Lua boolean expression, with
    /// exec.variables and the completed predecessor's flat output_data bound as Lua globals. An
    /// edge with no condition is always satisfied (the plain unconditional/default case).
    bool edge_condition_ok(
        const model::WorkflowExecution& exec,
        const model::TaskEdge& edge,
        const model::TaskInstance& predecessor
    ) const
    {
        if (!edge.get_condition()) {
            return true;
        }
        auto bindings = exec.get_variables();
        for (const auto& [key, value]: predecessor.get_output_data()) {
            bindings[key] = value;
        }
        return LuaEval{m_ctx.get().get_lua_bridge()}.eval_condition(
            *edge.get_condition(), to_value_input(bindings)
        );
    }

    /**
     * @brief Fans `event_name`/`payload` out to every active EventHandler subscribed to it (the
     * "react" counterpart to the EVENT task type's "publish" — see spawn_with_def()'s EVENT
     * branch) AND to the app-wide `core::events::publish()` bus (whichever `IEventSink`s are
     * configured — in-memory/RabbitMQ/Kafka/Redis), so external listeners see the same events
     * engine's own internal EventHandler subscriptions react to. Each matching EventHandler's
     * optional condition is evaluated with `payload` bound as Lua globals before its actions
     * fire.
     * @param event_name the published event's name.
     * @param payload flat key/value payload.
     */
    void publish_event(std::string event_name, std::unordered_map<std::string, std::string> payload)
    {
        core::events::publish(event_name, payload);
        m_ctx.get().get_connector().find_all<model::EventHandler>(
            [this, event_name = std::move(event_name),
             payload = std::move(payload)](std::vector<model::EventHandler> handlers) {
                for (const auto& handler: handlers) {
                    if (!handler.get_active() || handler.get_event() != event_name) {
                        continue;
                    }
                    if (handler.get_condition() &&
                        !LuaEval{m_ctx.get().get_lua_bridge()}.eval_condition(
                            *handler.get_condition(), to_value_input(payload)
                        )) {
                        continue;
                    }
                    for (const auto& action: handler.get_actions()) {
                        execute_event_action(action);
                    }
                }
            }
        );
    }

    /// @brief Routes one EventAction into already-existing Orchestrator machinery — see
    /// model::EventAction's own docs for each type's expected payload keys.
    void execute_event_action(const model::EventAction& action)
    {
        switch (action.get_type()) {
            case model::EventActionType::START_WORKFLOW:
                {
                    const auto& payload = action.get_payload();
                    auto workflow_name_it = payload.find("workflow_name");
                    if (workflow_name_it == payload.end()) {
                        core::logger::warning(
                            "engine",
                            "EventAction START_WORKFLOW missing 'workflow_name' payload key"
                        );
                        return;
                    }
                    auto variables = payload;
                    variables.erase("workflow_name");
                    start(
                        workflow_name_it->second, std::move(variables), std::nullopt,
                        [](std::optional<model::WorkflowExecution>) {}
                    );
                    return;
                }
            case model::EventActionType::COMPLETE_TASK:
            case model::EventActionType::FAIL_TASK:
                {
                    const auto& payload = action.get_payload();
                    auto exec_it = payload.find("exec_id");
                    auto ref_it = payload.find("task_ref");
                    if (exec_it == payload.end() || ref_it == payload.end()) {
                        core::logger::warning(
                            "engine",
                            "EventAction COMPLETE_TASK/FAIL_TASK missing 'exec_id'/'task_ref'"
                        );
                        return;
                    }
                    auto status = action.get_type() == model::EventActionType::COMPLETE_TASK
                                      ? model::TaskStatus::COMPLETED
                                      : model::TaskStatus::FAILED;
                    complete_instance_by_ref(exec_it->second, ref_it->second, status, payload);
                    return;
                }
            case model::EventActionType::TERMINATE_WORKFLOW:
                {
                    const auto& payload = action.get_payload();
                    auto exec_it = payload.find("exec_id");
                    if (exec_it == payload.end()) {
                        core::logger::warning(
                            "engine", "EventAction TERMINATE_WORKFLOW missing 'exec_id'"
                        );
                        return;
                    }
                    auto status = model::WorkflowStatus::TERMINATED;
                    if (auto status_it = payload.find("status"); status_it != payload.end()) {
                        if (status_it->second == "COMPLETED") {
                            status = model::WorkflowStatus::COMPLETED;
                        } else if (status_it->second == "FAILED") {
                            status = model::WorkflowStatus::FAILED;
                        } else if (status_it->second == "TIMED_OUT") {
                            status = model::WorkflowStatus::TIMED_OUT;
                        }
                    }
                    m_ctx.get().get_connector().find<model::WorkflowExecution>(
                        exec_it->second,
                        [this, status](std::optional<model::WorkflowExecution> exec) mutable {
                            if (!exec || model::is_terminal(exec->get_status())) {
                                return;
                            }
                            exec->set_status(status);
                            auto timings = exec->get_timings();
                            timings.set_completed_at(std::chrono::system_clock::now());
                            exec->set_timings(timings);
                            m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                *exec, log_on_failure("event terminate_workflow update")
                            );
                            on_execution_terminal(*exec);
                        }
                    );
                    return;
                }
            case model::EventActionType::UPDATE_WORKFLOW_VARIABLES:
                {
                    const auto& payload = action.get_payload();
                    auto exec_it = payload.find("exec_id");
                    if (exec_it == payload.end()) {
                        core::logger::warning(
                            "engine", "EventAction UPDATE_WORKFLOW_VARIABLES missing 'exec_id'"
                        );
                        return;
                    }
                    auto updates = payload;
                    updates.erase("exec_id");
                    m_ctx.get().get_connector().find<model::WorkflowExecution>(
                        exec_it->second, [this, updates = std::move(updates)](
                                             std::optional<model::WorkflowExecution> exec
                                         ) mutable {
                            if (!exec) {
                                return;
                            }
                            auto variables = exec->get_variables();
                            for (const auto& [key, value]: updates) {
                                variables[key] = value;
                            }
                            exec->set_variables(std::move(variables));
                            m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                std::move(*exec), log_on_failure("event update_variables")
                            );
                        }
                    );
                    return;
                }
        }
    }

    /// @brief Shared helper for COMPLETE_TASK/FAIL_TASK EventActions and (Phase 5's other
    /// consumer) the `POST /api/v1/queue/update` route — resolves `(exec_id, node_ref)` to the
    /// matching TaskInstance and completes it, same cascade path submit_result() uses.
    void complete_instance_by_ref(
        std::string exec_id,
        std::string node_ref,
        model::TaskStatus status,
        std::unordered_map<std::string, std::string> output
    )
    {
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id,
            [this, node_ref = std::move(node_ref), status,
             output = std::move(output)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec) {
                    return;
                }
                const model::TaskInstance* found = nullptr;
                for (const auto& instance: exec->get_task_instances()) {
                    if (instance.get_node_ref() == node_ref) {
                        found = &instance;
                        break;
                    }
                }
                if (found == nullptr) {
                    return;
                }
                auto instance = *found;
                instance.set_status(status);
                output.erase("exec_id");
                output.erase("task_ref");
                instance.set_output_data(std::move(output));
                m_ctx.get().get_connector().update<model::TaskInstance>(
                    instance, log_on_failure("complete_instance_by_ref update")
                );
                on_task_terminal(std::move(instance));
            }
        );
    }

    /**
     * @brief Figures out which of `def`'s nodes are ready to spawn this pass, which should be
     * marked SKIPPED (every predecessor terminal but no incoming edge satisfied — e.g. the
     * losing branches of a SWITCH), applies the skips synchronously, then hands eligible nodes
     * off to spawn_next() one at a time. Recurses once more if a skip cascade might have just
     * unlocked another node's eligibility, and finally checks whether the whole execution just
     * finished.
     * @param exec the execution to advance, taken by value — every nested lambda below owns its
     * own copy rather than capturing a reference across Connector's deferred-callback boundary
     * (see TaskHandler/WorkflowHandler's own constructor docs for why that reference would be
     * unsafe the moment a real database backend queues the callback for a later tick).
     * @param def the owning WorkflowDef, also taken by value for the same reason.
     * @param done gets the (possibly further-advanced) execution back once every eligible node
     * for this pass has been handled.
     */
    void advance(
        model::WorkflowExecution exec,
        model::WorkflowDef def,
        std::move_only_function<void(model::WorkflowExecution)> done
    )
    {
        if (model::is_terminal(exec.get_status()) ||
            exec.get_status() == model::WorkflowStatus::PAUSED) {
            // Terminal: nothing left to do. PAUSED: deliberately stop scheduling new instances
            // for this exec until resume() flips it back to RUNNING — already-IN_PROGRESS
            // instances keep running (pause doesn't reach into a worker and stop it), this just
            // withholds spawning anything new.
            done(std::move(exec));
            return;
        }

        std::unordered_set<std::string> spawned;
        for (const auto& instance: exec.get_task_instances()) {
            spawned.insert(instance.get_node_ref());
        }

        std::unordered_map<std::string, std::vector<model::TaskEdge>> incoming;
        for (const auto& node: def.get_nodes()) {
            for (const auto& edge: node.get_edges()) {
                incoming[edge.get_to()].push_back(edge);
            }
        }

        std::vector<std::string> eligible_refs;
        std::vector<std::string> skip_refs;
        for (const auto& node: def.get_nodes()) {
            if (spawned.contains(node.get_ref_name())) {
                continue;
            }

            // JOIN/EXCLUSIVE_JOIN — an explicit predecessor list, not inferred from edges,
            // since dynamic/fork branches vary at runtime (see class docs' Phase 2 note).
            if (!node.get_join_on().empty()) {
                bool ready =
                    node.get_join_type() == model::JoinType::ALL
                        ? std::ranges::all_of(
                              node.get_join_on(),
                              [&](const std::string& ref) {
                                  const auto* instance =
                                      find_instance(exec.get_task_instances(), ref);
                                  return instance != nullptr &&
                                         model::is_terminal(instance->get_status());
                              }
                          )
                        : std::ranges::any_of(node.get_join_on(), [&](const std::string& ref) {
                              const auto* instance = find_instance(exec.get_task_instances(), ref);
                              return instance != nullptr &&
                                     (instance->get_status() == model::TaskStatus::COMPLETED ||
                                      instance->get_status() == model::TaskStatus::SKIPPED);
                          });
                if (ready) {
                    eligible_refs.push_back(node.get_ref_name());
                }
                continue;
            }

            auto found_incoming = incoming.find(node.get_ref_name());
            if (found_incoming == incoming.end()) {
                // no predecessors at all — a start node, always eligible.
                eligible_refs.push_back(node.get_ref_name());
                continue;
            }

            // Ordinary node: eligible the moment any one incoming edge is satisfied
            // (predecessor done + condition true-or-absent) — this is what makes a SWITCH's
            // branch targets work (each has one incoming edge, only one condition is ever
            // true). A plain multi-predecessor AND-join now needs an explicit join_on, not just
            // multiple plain edges into the same node (matches Conductor's own JOIN design).
            bool any_satisfied = false;
            bool all_predecessors_terminal = true;
            for (const auto& edge: found_incoming->second) {
                const auto* predecessor = find_instance(exec.get_task_instances(), edge.get_from());
                if (predecessor == nullptr || !model::is_terminal(predecessor->get_status())) {
                    all_predecessors_terminal = false;
                    continue;
                }
                bool predecessor_ok = predecessor->get_status() == model::TaskStatus::COMPLETED ||
                                      predecessor->get_status() == model::TaskStatus::SKIPPED;
                if (predecessor_ok && edge_condition_ok(exec, edge, *predecessor)) {
                    any_satisfied = true;
                }
            }
            if (any_satisfied) {
                eligible_refs.push_back(node.get_ref_name());
            } else if (all_predecessors_terminal) {
                // every predecessor is done but nothing satisfied us — a losing SWITCH branch,
                // never going to fire. Mark it SKIPPED so maybe_complete()'s per-node coverage
                // check doesn't wait on it forever, and so it can itself cascade downstream.
                skip_refs.push_back(node.get_ref_name());
            }
        }

        if (eligible_refs.empty() && skip_refs.empty()) {
            maybe_complete(exec, def);
            done(std::move(exec));
            return;
        }

        for (const auto& ref_name: skip_refs) {
            const auto* node = find_node(def, ref_name);
            if (node == nullptr) {
                continue;
            }
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_workflow_exec_id(exec.get_exec_id());
            instance.set_def_name(node->get_def_name());
            instance.set_node_ref(ref_name);
            instance.set_status(model::TaskStatus::SKIPPED);
            exec.add_task_instance(instance);
            m_ctx.get().get_connector().insert<model::TaskInstance>(
                instance, log_on_failure(std::format("skip instance for node '{}'", ref_name))
            );
        }

        if (eligible_refs.empty()) {
            // Only skips happened this pass — recurse to catch any node a skip cascade just
            // unlocked. Guaranteed to terminate: `spawned` (which now includes this pass's
            // skips) only grows, and there are finitely many nodes.
            advance(std::move(exec), std::move(def), std::move(done));
            return;
        }

        spawn_next(std::move(exec), std::move(def), std::move(eligible_refs), 0, std::move(done));
    }

    /**
     * @brief Spawns eligible_refs[index] then recurses to index+1 — one at a time, not a plain
     * loop, since each spawn needs its own async TaskDef lookup (for rate-limit enforcement and
     * DYNAMIC resolution) before it can decide whether to actually create the instance.
     */
    void spawn_next(
        model::WorkflowExecution exec,
        model::WorkflowDef def,
        std::vector<std::string> eligible_refs,
        std::size_t index,
        std::move_only_function<void(model::WorkflowExecution)> done
    )
    {
        if (index >= eligible_refs.size()) {
            maybe_complete(exec, def);
            done(std::move(exec));
            return;
        }

        const auto* node = find_node(def, eligible_refs[index]);
        if (node == nullptr) {
            // Shouldn't happen — def doesn't change mid-pass — but don't crash if it somehow
            // does.
            spawn_next(
                std::move(exec), std::move(def), std::move(eligible_refs), index + 1,
                std::move(done)
            );
            return;
        }

        // Gather this node's input from every satisfied predecessor edge's InputMapping list —
        // flat key lookup only, straight off the predecessor's own output_data (see class docs
        // on the Phase 1 templating limitation).
        std::unordered_map<std::string, std::string> input;
        for (const auto& n: def.get_nodes()) {
            for (const auto& edge: n.get_edges()) {
                if (edge.get_to() != node->get_ref_name()) {
                    continue;
                }
                const auto* predecessor = find_instance(exec.get_task_instances(), edge.get_from());
                if (predecessor == nullptr ||
                    !(predecessor->get_status() == model::TaskStatus::COMPLETED ||
                      predecessor->get_status() == model::TaskStatus::SKIPPED) ||
                    !edge_condition_ok(exec, edge, *predecessor)) {
                    continue;
                }
                for (const auto& mapping: edge.get_mappings()) {
                    auto found = predecessor->get_output_data().find(mapping.get_source());
                    if (found != predecessor->get_output_data().end()) {
                        input[mapping.get_target()] = found->second;
                    }
                }
            }
        }

        auto def_name = node->get_def_name();
        auto ref_name = node->get_ref_name();
        m_ctx.get().get_connector().find<model::TaskDef>(
            def_name, [this, exec = std::move(exec), def = std::move(def),
                       eligible_refs = std::move(eligible_refs), index, input = std::move(input),
                       ref_name = std::move(ref_name), def_name = std::move(def_name),
                       done = std::move(done)](std::optional<model::TaskDef> task_def) mutable {
                // DYNAMIC — resolve the *actual* TaskDef to run from the named input key, then
                // spawn against that instead of the node's own nominal def_name.
                if (task_def && task_def->get_type() == model::TaskType::DYNAMIC &&
                    task_def->get_dynamic_task_param()) {
                    auto found = input.find(*task_def->get_dynamic_task_param());
                    if (found != input.end()) {
                        m_ctx.get().get_connector().find<model::TaskDef>(
                            found->second,
                            [this, exec = std::move(exec), def = std::move(def),
                             eligible_refs = std::move(eligible_refs), index,
                             input = std::move(input), ref_name = std::move(ref_name),
                             def_name = std::move(def_name),
                             done =
                                 std::move(done)](std::optional<model::TaskDef> resolved) mutable {
                                spawn_with_def(
                                    std::move(exec), std::move(def), std::move(eligible_refs),
                                    index, std::move(input), std::move(ref_name),
                                    std::move(def_name), std::move(resolved), std::move(done)
                                );
                            }
                        );
                        return;
                    }
                }
                spawn_with_def(
                    std::move(exec), std::move(def), std::move(eligible_refs), index,
                    std::move(input), std::move(ref_name), std::move(def_name), std::move(task_def),
                    std::move(done)
                );
            }
        );
    }

    /// @brief The tail half of spawn_next(): given a fully-resolved TaskDef (post-DYNAMIC, if
    /// applicable), dispatches in-process system tasks synchronously (see class docs' Phase 3
    /// note), otherwise enforces RateLimitPolicy and either creates a SCHEDULED-for-worker
    /// instance or defers it to the next sweep_advance() pass, then recurses to the next
    /// eligible ref.
    void spawn_with_def(
        model::WorkflowExecution exec,
        model::WorkflowDef def,
        std::vector<std::string> eligible_refs,
        std::size_t index,
        std::unordered_map<std::string, std::string> input,
        std::string ref_name,
        std::string original_def_name,
        std::optional<model::TaskDef> task_def,
        std::move_only_function<void(model::WorkflowExecution)> done
    )
    {
        auto resolved_def_name = task_def ? task_def->get_name() : original_def_name;

        // HUMAN — stays IN_PROGRESS forever once created, no worker poll involved (matches
        // Conductor's own no-separate-claim-endpoint design): whatever calls
        // POST /api/v1/tasks/:id/result later completes it externally, same route every other
        // instance uses. No deadline_at ever gets stamped, so sweep_timeouts() never touches
        // it.
        if (task_def && task_def->get_type() == model::TaskType::HUMAN) {
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_workflow_exec_id(exec.get_exec_id());
            instance.set_def_name(resolved_def_name);
            instance.set_node_ref(ref_name);
            instance.set_status(model::TaskStatus::IN_PROGRESS);
            instance.set_input_data(to_value_input(input));
            model::ExecutionTimings timings;
            timings.set_scheduled_at(std::chrono::system_clock::now());
            timings.set_started_at(std::chrono::system_clock::now());
            instance.set_timings(timings);
            exec.add_task_instance(instance);
            m_ctx.get().get_connector().insert<model::TaskInstance>(
                instance,
                log_on_failure(std::format("spawn HUMAN instance for node '{}'", ref_name))
            );
            spawn_next(
                std::move(exec), std::move(def), std::move(eligible_refs), index + 1,
                std::move(done)
            );
            return;
        }

        // WAIT — auto-completes via sweep_timeouts() once wait_duration_ms elapses (special-
        // cased there to mean "wait finished", not "timed out"); with no duration configured,
        // it waits indefinitely for an external SIGNAL WorkflowEvent targeting this node_ref.
        if (task_def && task_def->get_type() == model::TaskType::WAIT) {
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_workflow_exec_id(exec.get_exec_id());
            instance.set_def_name(resolved_def_name);
            instance.set_node_ref(ref_name);
            instance.set_status(model::TaskStatus::IN_PROGRESS);
            instance.set_input_data(to_value_input(input));
            auto now = std::chrono::system_clock::now();
            model::ExecutionTimings timings;
            timings.set_scheduled_at(now);
            timings.set_started_at(now);
            instance.set_timings(timings);
            if (task_def->get_wait_duration_ms()) {
                instance.set_deadline_at(
                    now + std::chrono::milliseconds{*task_def->get_wait_duration_ms()}
                );
            }
            exec.add_task_instance(instance);
            m_ctx.get().get_connector().insert<model::TaskInstance>(
                instance, log_on_failure(std::format("spawn WAIT instance for node '{}'", ref_name))
            );
            spawn_next(
                std::move(exec), std::move(def), std::move(eligible_refs), index + 1,
                std::move(done)
            );
            return;
        }

        // START_WORKFLOW — fire-and-forget: starts a new top-level execution (no parent_exec_id
        // link) and completes this instance immediately with the new exec_id, never tracking
        // the child further.
        if (task_def && task_def->get_type() == model::TaskType::START_WORKFLOW) {
            auto workflow_name_it = input.find("workflow_name");
            if (workflow_name_it == input.end()) {
                core::logger::warning(
                    "engine", "START_WORKFLOW node '{}' has no 'workflow_name' input key", ref_name
                );
                spawn_next(
                    std::move(exec), std::move(def), std::move(eligible_refs), index + 1,
                    std::move(done)
                );
                return;
            }
            auto variables = input;
            variables.erase("workflow_name");
            start(
                workflow_name_it->second, std::move(variables), std::nullopt,
                [this, exec = std::move(exec), def = std::move(def),
                 eligible_refs = std::move(eligible_refs), index, ref_name = std::move(ref_name),
                 resolved_def_name,
                 done = std::move(done)](std::optional<model::WorkflowExecution> child) mutable {
                    auto now = std::chrono::system_clock::now();
                    model::TaskInstance instance;
                    instance.set_task_id(model::generate_id());
                    instance.set_workflow_exec_id(exec.get_exec_id());
                    instance.set_def_name(resolved_def_name);
                    instance.set_node_ref(ref_name);
                    instance.set_status(
                        child ? model::TaskStatus::COMPLETED : model::TaskStatus::FAILED
                    );
                    if (child) {
                        instance.add_output_data(
                            "exec_id", std::format("{}", child->get_exec_id())
                        );
                    }
                    model::ExecutionTimings timings;
                    timings.set_scheduled_at(now);
                    timings.set_started_at(now);
                    timings.set_completed_at(now);
                    instance.set_timings(timings);
                    exec.add_task_instance(instance);
                    m_ctx.get().get_connector().insert<model::TaskInstance>(
                        instance,
                        log_on_failure(
                            std::format("spawn START_WORKFLOW instance for node '{}'", ref_name)
                        )
                    );
                    advance(std::move(exec), std::move(def), std::move(done));
                }
            );
            return;
        }

        // SUB_WORKFLOW — starts a child execution linked back via parent_exec_id, keeps this
        // instance IN_PROGRESS until the child reaches a terminal status (Orchestrator::
        // on_execution_terminal() completes it to match once that happens).
        if (task_def && task_def->get_type() == model::TaskType::SUB_WORKFLOW) {
            auto workflow_name_it = input.find("workflow_name");
            if (workflow_name_it == input.end()) {
                core::logger::warning(
                    "engine", "SUB_WORKFLOW node '{}' has no 'workflow_name' input key", ref_name
                );
                spawn_next(
                    std::move(exec), std::move(def), std::move(eligible_refs), index + 1,
                    std::move(done)
                );
                return;
            }
            auto variables = input;
            variables.erase("workflow_name");
            start(
                workflow_name_it->second, std::move(variables), exec.get_exec_id(),
                [this, exec = std::move(exec), def = std::move(def),
                 eligible_refs = std::move(eligible_refs), index, ref_name = std::move(ref_name),
                 resolved_def_name,
                 done = std::move(done)](std::optional<model::WorkflowExecution> child) mutable {
                    if (!child) {
                        // Named workflow doesn't exist — fail this instance, and (forcing
                        // immediate retry exhaustion via a 1-attempt policy) the owning
                        // execution outright, same as any other unretryable failure.
                        model::TaskInstance instance;
                        instance.set_task_id(model::generate_id());
                        instance.set_workflow_exec_id(exec.get_exec_id());
                        instance.set_def_name(resolved_def_name);
                        instance.set_node_ref(ref_name);
                        instance.set_status(model::TaskStatus::FAILED);
                        handle_failure(
                            std::move(instance), std::move(exec), std::move(def),
                            model::RetryPolicy{1, model::RetryBackoff::FIXED, 1}
                        );
                        return;
                    }
                    model::TaskInstance instance;
                    instance.set_task_id(model::generate_id());
                    instance.set_workflow_exec_id(exec.get_exec_id());
                    instance.set_def_name(resolved_def_name);
                    instance.set_node_ref(ref_name);
                    instance.set_status(model::TaskStatus::IN_PROGRESS);
                    instance.set_sub_workflow_exec_id(child->get_exec_id());
                    model::ExecutionTimings timings;
                    timings.set_scheduled_at(std::chrono::system_clock::now());
                    timings.set_started_at(std::chrono::system_clock::now());
                    instance.set_timings(timings);
                    exec.add_task_instance(instance);
                    m_ctx.get().get_connector().insert<model::TaskInstance>(
                        instance,
                        log_on_failure(
                            std::format("spawn SUB_WORKFLOW instance for node '{}'", ref_name)
                        )
                    );
                    spawn_next(
                        std::move(exec), std::move(def), std::move(eligible_refs), index + 1,
                        std::move(done)
                    );
                }
            );
            return;
        }

        // EVENT — publishes to the internal event bus (see publish_event()'s own docs on the
        // Phase 5 in-process-only sink scope), reading the event name from the `event` input
        // key and using every other input key as the published payload. Completes instantly,
        // same shape as a system task.
        if (task_def && task_def->get_type() == model::TaskType::EVENT) {
            auto event_it = input.find("event");
            if (event_it != input.end()) {
                auto payload = input;
                payload.erase("event");
                publish_event(event_it->second, payload);
            } else {
                core::logger::warning(
                    "engine", "EVENT node '{}' has no 'event' input key", ref_name
                );
            }
            auto now = std::chrono::system_clock::now();
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_workflow_exec_id(exec.get_exec_id());
            instance.set_def_name(resolved_def_name);
            instance.set_node_ref(ref_name);
            instance.set_status(model::TaskStatus::COMPLETED);
            instance.set_output_data(input);
            model::ExecutionTimings timings;
            timings.set_scheduled_at(now);
            timings.set_started_at(now);
            timings.set_completed_at(now);
            instance.set_timings(timings);
            exec.add_task_instance(instance);
            m_ctx.get().get_connector().insert<model::TaskInstance>(
                instance,
                log_on_failure(std::format("spawn EVENT instance for node '{}'", ref_name))
            );
            advance(std::move(exec), std::move(def), std::move(done));
            return;
        }

        if (task_def && SystemTaskExecutor::is_system_task(task_def->get_type())) {
            auto outcome = SystemTaskExecutor{m_ctx.get().get_lua_bridge()}.execute(
                task_def->get_type(), input, exec.get_variables()
            );

            if (task_def->get_type() == model::TaskType::SET_VARIABLE) {
                auto variables = exec.get_variables();
                for (const auto& [key, value]: outcome.output_data) {
                    variables[key] = value;
                }
                exec.set_variables(std::move(variables));
            }

            auto now = std::chrono::system_clock::now();
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_workflow_exec_id(exec.get_exec_id());
            instance.set_def_name(resolved_def_name);
            instance.set_node_ref(ref_name);
            instance.set_status(model::TaskStatus::COMPLETED);
            instance.set_output_data(outcome.output_data);
            model::ExecutionTimings timings;
            timings.set_scheduled_at(now);
            timings.set_started_at(now);
            timings.set_completed_at(now);
            instance.set_timings(timings);
            exec.add_task_instance(instance);
            m_ctx.get().get_connector().insert<model::TaskInstance>(
                instance,
                log_on_failure(std::format("spawn system-task instance for node '{}'", ref_name))
            );

            if (outcome.terminate_workflow) {
                exec.set_status(outcome.terminate_status);
                auto exec_timings = exec.get_timings();
                exec_timings.set_completed_at(now);
                exec.set_timings(exec_timings);
                on_execution_terminal(exec);
                done(std::move(exec));
                return;
            }

            // A system task resolves instantly, unlike an external-worker instance — restart
            // advance() from scratch (rather than continuing this pass's eligible_refs) so any
            // node this one just unblocked gets picked up immediately. Terminates: `spawned`
            // (recomputed fresh each call from exec.task_instances) only grows.
            advance(std::move(exec), std::move(def), std::move(done));
            return;
        }
        // Input-schema enforcement — see SchemaValidator's own docs for exactly what this
        // checks (required top-level key presence only, not full JSON-schema semantics, since
        // input_data is a flat string map with no nested shape to validate against). A failing
        // check fails this instance outright (through the same retry/failure_workflow path a
        // real worker failure takes) rather than ever reaching SCHEDULED.
        if (task_def && task_def->get_enforce_schema() && task_def->get_input_schema()) {
            if (auto check =
                    SchemaValidator::validate(*task_def->get_input_schema(), to_value_input(input));
                !check) {
                core::logger::warning(
                    "engine", "node '{}' input schema check failed: {}", ref_name, check.error()
                );
                core::events::publish(
                    "engine.task.schema_validation_failed",
                    {{"node_ref", ref_name}, {"error", check.error()}}
                );
                auto now = std::chrono::system_clock::now();
                model::TaskInstance instance;
                instance.set_task_id(model::generate_id());
                instance.set_workflow_exec_id(exec.get_exec_id());
                instance.set_def_name(resolved_def_name);
                instance.set_node_ref(ref_name);
                instance.set_status(model::TaskStatus::FAILED);
                instance.set_input_data(to_value_input(input));
                instance.add_output_data(
                    "error", std::format("input schema validation failed: {}", check.error())
                );
                model::ExecutionTimings timings;
                timings.set_scheduled_at(now);
                timings.set_started_at(now);
                instance.set_timings(timings);
                exec.add_task_instance(instance);
                m_ctx.get().get_connector().insert<model::TaskInstance>(
                    instance,
                    log_on_failure(
                        std::format("spawn schema-rejected instance for node '{}'", ref_name)
                    )
                );
                handle_failure(
                    std::move(instance), std::move(exec), std::move(def), task_def->get_retry()
                );
                return;
            }
        }

        bool has_limit = task_def.has_value() && task_def->get_rate_limit().has_value();
        std::uint32_t in_flight = 0;
        if (has_limit) {
            for (const auto& instance: exec.get_task_instances()) {
                if (instance.get_def_name() == resolved_def_name &&
                    (instance.get_status() == model::TaskStatus::SCHEDULED ||
                     instance.get_status() == model::TaskStatus::IN_PROGRESS)) {
                    ++in_flight;
                }
            }
        }
        bool within_limit =
            !has_limit || in_flight < task_def->get_rate_limit()->get_max_concurrent();

        if (within_limit) {
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_workflow_exec_id(exec.get_exec_id());
            instance.set_def_name(resolved_def_name);
            instance.set_node_ref(ref_name);
            instance.set_status(model::TaskStatus::SCHEDULED);
            instance.set_input_data(to_value_input(input));
            model::ExecutionTimings timings;
            timings.set_scheduled_at(std::chrono::system_clock::now());
            instance.set_timings(timings);
            exec.add_task_instance(instance);
            m_ctx.get().get_connector().insert<model::TaskInstance>(
                instance, log_on_failure(std::format("spawn instance for node '{}'", ref_name))
            );
        } else {
            core::logger::info(
                "engine",
                "node '{}' rate-limited (max_concurrent), deferring spawn to next "
                "sweep_advance()",
                ref_name
            );
            core::events::publish(
                "engine.task.rate_limited",
                {{"node_ref", ref_name}, {"exec_id", std::format("{}", exec.get_exec_id())}}
            );
        }

        spawn_next(
            std::move(exec), std::move(def), std::move(eligible_refs), index + 1, std::move(done)
        );
    }

    /**
     * @brief Checks whether every node in `def` now has a terminal (COMPLETED/SKIPPED) instance
     * and, if so, marks `exec` COMPLETED — applying `def`'s output_mappings off the
     * last-completed instance's output_data (a Phase 1 simplification: full ancestor-wide
     * output resolution needs more structure than a flat instance list, see class docs).
     */
    void maybe_complete(model::WorkflowExecution& exec, const model::WorkflowDef& def)
    {
        if (model::is_terminal(exec.get_status())) {
            return;
        }
        // Every node spawns exactly one instance in Phase 1 (no loops/dynamic fork yet), so
        // comparing counts is a cheap proxy for "every node has been scheduled".
        if (exec.get_task_instances().size() < def.get_nodes().size()) {
            return;
        }
        bool all_done =
            std::ranges::all_of(exec.get_task_instances(), [](const model::TaskInstance& instance) {
                return instance.get_status() == model::TaskStatus::COMPLETED ||
                       instance.get_status() == model::TaskStatus::SKIPPED;
            });
        if (!all_done) {
            return;
        }

        if (!exec.get_task_instances().empty() && !def.get_output_mappings().empty()) {
            const auto& source_instance = exec.get_task_instances().back();
            auto variables = exec.get_variables();
            for (const auto& mapping: def.get_output_mappings()) {
                auto found = source_instance.get_output_data().find(mapping.get_source());
                if (found != source_instance.get_output_data().end()) {
                    variables[mapping.get_target()] = found->second;
                }
            }
            exec.set_variables(std::move(variables));
        }

        exec.set_status(model::WorkflowStatus::COMPLETED);
        auto timings = exec.get_timings();
        timings.set_completed_at(std::chrono::system_clock::now());
        exec.set_timings(timings);
        on_execution_terminal(exec);
    }

    /**
     * @brief Handles a terminal instance already synced into `exec`'s own list by the caller:
     * retries it if TaskDef::retry attempts remain, fails the whole execution if they don't, or
     * (on a clean COMPLETED/SKIPPED) hands off to advance() to spawn whatever comes next.
     * CANCELED just persists the sync — nothing advances from a canceled node.
     */
    void process_terminal(
        model::TaskInstance instance, model::WorkflowExecution exec, model::WorkflowDef def
    )
    {
        sync_instance(exec, instance);
        SummaryProjector{m_ctx.get()}.project_task(instance);

        bool failed = instance.get_status() == model::TaskStatus::FAILED ||
                      instance.get_status() == model::TaskStatus::TIMED_OUT;
        if (failed) {
            m_ctx.get().get_connector().find<model::TaskDef>(
                instance.get_def_name(),
                [this, instance = std::move(instance), exec = std::move(exec),
                 def = std::move(def)](std::optional<model::TaskDef> task_def) mutable {
                    handle_failure(
                        std::move(instance), std::move(exec), std::move(def),
                        task_def ? task_def->get_retry() : model::RetryPolicy{}
                    );
                }
            );
            return;
        }

        bool cascades = instance.get_status() == model::TaskStatus::COMPLETED ||
                        instance.get_status() == model::TaskStatus::SKIPPED;
        if (!cascades) {
            m_ctx.get().get_connector().update<model::WorkflowExecution>(
                std::move(exec), log_on_failure("process_terminal sync (canceled)")
            );
            return;
        }

        advance(std::move(exec), std::move(def), [this](model::WorkflowExecution advanced) {
            m_ctx.get().get_connector().update<model::WorkflowExecution>(
                std::move(advanced), log_on_failure("process_terminal advance")
            );
        });
    }

    /// @brief Arms a retry (increments retry_count, stamps next_retry_at per backoff) if
    /// `retry`'s max_attempts allows another try, otherwise fails the owning execution outright
    /// and auto-starts `def`'s failure_workflow, if one's configured. Either way persists both
    /// the instance and the execution.
    void handle_failure(
        model::TaskInstance instance,
        model::WorkflowExecution exec,
        model::WorkflowDef def,
        model::RetryPolicy retry
    )
    {
        bool retries_left = instance.get_retry_count() + 1 < retry.get_max_attempts();
        if (retries_left) {
            instance.set_retry_count(instance.get_retry_count() + 1);
            instance.set_next_retry_at(
                std::chrono::system_clock::now() +
                compute_backoff(retry, instance.get_retry_count())
            );
            core::logger::info(
                "engine", "task '{}' retry {}/{} armed", instance.get_def_name(),
                instance.get_retry_count(), retry.get_max_attempts()
            );
            core::events::publish(
                "engine.task.retry_armed",
                {{"task_def_name", instance.get_def_name()},
                 {"exec_id", std::format("{}", exec.get_exec_id())},
                 {"retry_count", std::format("{}", instance.get_retry_count())}}
            );
            sync_instance(exec, instance);
            m_ctx.get().get_connector().update<model::TaskInstance>(
                instance, log_on_failure("handle_failure instance update")
            );
            m_ctx.get().get_connector().update<model::WorkflowExecution>(
                std::move(exec), log_on_failure("handle_failure exec update")
            );
            return;
        }

        core::logger::warning(
            "engine", "task '{}' exhausted retries, failing exec '{}'", instance.get_def_name(),
            exec.get_exec_id()
        );
        core::events::publish(
            "engine.task.retries_exhausted", {{"task_def_name", instance.get_def_name()},
                                              {"exec_id", std::format("{}", exec.get_exec_id())}}
        );
        exec.set_status(model::WorkflowStatus::FAILED);
        auto timings = exec.get_timings();
        timings.set_completed_at(std::chrono::system_clock::now());
        exec.set_timings(timings);
        sync_instance(exec, instance);
        m_ctx.get().get_connector().update<model::TaskInstance>(
            instance, log_on_failure("handle_failure instance update")
        );
        m_ctx.get().get_connector().update<model::WorkflowExecution>(
            exec, log_on_failure("handle_failure exec update")
        );
        on_execution_terminal(exec);
        if (def.get_failure_workflow()) {
            start(
                *def.get_failure_workflow(), exec.get_variables(), std::nullopt,
                [](std::optional<model::WorkflowExecution>) {}
            );
        }
    }

    static std::chrono::milliseconds
    compute_backoff(const model::RetryPolicy& retry, std::uint32_t attempt_number)
    {
        auto base = std::chrono::milliseconds{retry.get_interval_ms()};
        if (retry.get_backoff() == model::RetryBackoff::FIXED) {
            return base;
        }
        // EXPONENTIAL — doubles per attempt already made, capped at 2^16 so a long-lived
        // execution can't overflow this into something absurd.
        return base * (1U << std::min(attempt_number, 16U));
    }

    /// @brief Applies a timed-out instance's TaskDef::timeout action — except for a WAIT-typed
    /// instance, where an elapsed deadline_at means the wait finished successfully, not that it
    /// failed: this completes it and cascades normally, ignoring the timeout policy entirely.
    /// For every other type: ALERT_ONLY just logs and leaves the instance running, RETRY routes
    /// through the same handle_failure() path a FAILED instance uses (including the
    /// failure_workflow auto-trigger on exhaustion), FAIL_WORKFLOW skips retry entirely and
    /// fails the owning execution straight away (also triggering failure_workflow).
    void handle_timeout(model::TaskInstance instance)
    {
        m_ctx.get().get_connector().find<model::TaskDef>(
            instance.get_def_name(),
            [this, instance = std::move(instance)](std::optional<model::TaskDef> task_def) mutable {
                if (task_def && task_def->get_type() == model::TaskType::WAIT) {
                    instance.set_status(model::TaskStatus::COMPLETED);
                    on_task_terminal(instance);
                    return;
                }

                auto action = task_def ? task_def->get_timeout().get_action()
                                       : model::TimeoutAction::FAIL_WORKFLOW;
                if (action == model::TimeoutAction::ALERT_ONLY) {
                    core::logger::warning(
                        "engine", "task '{}' timed out (alert only, still running)",
                        instance.get_def_name()
                    );
                    return;
                }

                instance.set_status(model::TaskStatus::TIMED_OUT);
                auto exec_key = std::format("{}", instance.get_workflow_exec_id());
                auto retry = task_def ? task_def->get_retry() : model::RetryPolicy{};
                bool retry_action = action == model::TimeoutAction::RETRY;

                m_ctx.get().get_connector().find<model::WorkflowExecution>(
                    exec_key, [this, instance = std::move(instance), retry,
                               retry_action](std::optional<model::WorkflowExecution> exec) mutable {
                        if (!exec || model::is_terminal(exec->get_status())) {
                            m_ctx.get().get_connector().update<model::TaskInstance>(
                                instance, log_on_failure("timeout instance update")
                            );
                            return;
                        }
                        m_ctx.get().get_connector().find<model::WorkflowDef>(
                            exec->get_def_name(),
                            [this, instance = std::move(instance), exec = std::move(*exec), retry,
                             retry_action](std::optional<model::WorkflowDef> def) mutable {
                                if (retry_action) {
                                    handle_failure(
                                        std::move(instance), std::move(exec),
                                        def ? std::move(*def) : model::WorkflowDef{}, retry
                                    );
                                    return;
                                }
                                // FAIL_WORKFLOW — skip retry entirely, fail the owning
                                // execution outright and auto-trigger its failure_workflow, if
                                // any.
                                exec.set_status(model::WorkflowStatus::FAILED);
                                auto timings = exec.get_timings();
                                timings.set_completed_at(std::chrono::system_clock::now());
                                exec.set_timings(timings);
                                sync_instance(exec, instance);
                                m_ctx.get().get_connector().update<model::TaskInstance>(
                                    instance, log_on_failure("timeout instance update")
                                );
                                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                                    exec, log_on_failure("timeout exec update")
                                );
                                on_execution_terminal(exec);
                                if (def && def->get_failure_workflow()) {
                                    start(
                                        *def->get_failure_workflow(), exec.get_variables(),
                                        std::nullopt, [](std::optional<model::WorkflowExecution>) {}
                                    );
                                }
                            }
                        );
                    }
                );
            }
        );
    }
};

} // namespace engine
