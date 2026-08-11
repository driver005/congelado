export module model:task_instance;

import std;
import :identifiers;
import :timestamps;
import :task_status;
import serde;

export namespace model {

class TaskInstance {
  public:
    /// @brief Default ctor, bet — nil task_id/workflow_exec_id, SCHEDULED status,
    /// seq/retry_count at 0, everything else empty.
    TaskInstance() = default;

    /// @brief Adds a single input key/value pair, overwriting any existing entry for that key.
    /// @param key the input key.
    /// @param value the input value.
    void add_input_data(std::string key, std::string value) {
        m_input_data.emplace(std::move(key), std::move(value));
    }
    /// @brief Adds a single output key/value pair, overwriting any existing entry for that key.
    /// @param key the output key.
    /// @param value the output value.
    void add_output_data(std::string key, std::string value) {
        m_output_data.emplace(std::move(key), std::move(value));
    }

    /// @brief Sets which workflow execution this task instance belongs to.
    /// @param execution_id the owning workflow execution's id.
    void set_workflow_exec_id(ExecutionId execution_id) { m_workflow_exec_id = execution_id; }
    /// @brief Sets the name of the TaskDef this instance was spawned from.
    /// @param def_name the task definition name.
    void set_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    /// @brief Sets which WorkflowDef node (by TaskNode::ref_name) this instance was spawned
    /// from — the Orchestrator walks this instance's owning node's outgoing edges once the
    /// instance reaches a terminal status, so it needs to know which node that was.
    /// @param node_ref the spawning node's ref_name.
    void set_node_ref(std::string node_ref) { m_node_ref = std::move(node_ref); }
    /// @brief Sets this instance's own task id.
    /// @param task_id the new task id.
    void set_task_id(TaskId task_id) { m_task_id = task_id; }
    /// @brief Sets the instance's current status — lowkey the field that drives most of the
    /// engine's polling logic.
    /// @param status the new status.
    void set_status(TaskStatus status) noexcept { m_status = status; }
    /// @brief Sets the instance's sequence number within its workflow execution.
    /// @param seq the new sequence number.
    void set_seq(std::uint32_t seq) noexcept { m_seq = seq; }
    /// @brief Replaces the whole input data map wholesale.
    /// @param data the new input key/value map.
    void set_input_data(std::unordered_map<std::string, std::string> data) {
        m_input_data = std::move(data);
    }
    /// @brief Replaces the whole output data map wholesale.
    /// @param data the new output key/value map.
    void set_output_data(std::unordered_map<std::string, std::string> data) {
        m_output_data = std::move(data);
    }
    /// @brief Sets the instance's scheduled/started/completed timings.
    /// @param timing the new timings.
    void set_timings(ExecutionTimings timing) { m_timings = timing; }
    /// @brief Sets how many retries this instance has gone through so far.
    /// @param count the new retry count.
    void set_retry_count(std::uint32_t count) noexcept { m_retry_count = count; }
    /// @brief Sets when this instance's TaskDef::timeout should fire, if it hasn't reached a
    /// terminal status by then — computed once at claim time (started_at + timeout_ms), so the
    /// background sweep can do a cheap deadline check instead of re-joining against TaskDef
    /// every tick.
    /// @param deadline the timeout deadline, or std::nullopt to clear it.
    void set_deadline_at(std::optional<std::chrono::system_clock::time_point> deadline) noexcept {
        m_deadline_at = deadline;
    }
    /// @brief Sets when this instance should next be retried, for a FAILED/TIMED_OUT instance
    /// that still has retry attempts left — the background sweep flips it back to SCHEDULED once
    /// this elapses.
    /// @param next_retry the next retry time, or std::nullopt to clear it (no retry pending).
    void set_next_retry_at(std::optional<std::chrono::system_clock::time_point> next_retry) noexcept {
        m_next_retry_at = next_retry;
    }
    /// @brief Sets which child WorkflowExecution this SUB_WORKFLOW instance is waiting on —
    /// stays IN_PROGRESS until that child reaches a terminal status, at which point
    /// Orchestrator::on_execution_terminal() completes this instance to match.
    /// @param exec_id the child execution's id, or std::nullopt for a non-SUB_WORKFLOW instance.
    void set_sub_workflow_exec_id(std::optional<ExecutionId> exec_id) noexcept {
        m_sub_workflow_exec_id = exec_id;
    }

    /// @brief Gets this instance's own task id.
    /// @return the task id.
    [[nodiscard]] const TaskId &get_task_id() const noexcept { return m_task_id; }
    /// @brief Gets the name of the TaskDef this instance was spawned from.
    /// @return the task definition name.
    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    /// @brief Gets which WorkflowDef node (by TaskNode::ref_name) this instance was spawned
    /// from.
    /// @return the spawning node's ref_name, or empty for instances not spawned by the
    /// orchestrator (e.g. a bare POST /tasks/:name/enqueue).
    [[nodiscard]] const std::string &get_node_ref() const noexcept { return m_node_ref; }
    /// @brief Gets which workflow execution this task instance belongs to.
    /// @return the owning workflow execution's id.
    [[nodiscard]] const ExecutionId &get_workflow_exec_id() const noexcept {
        return m_workflow_exec_id;
    }
    /// @brief Gets the instance's current status.
    /// @return the current status.
    [[nodiscard]] TaskStatus get_status() const noexcept { return m_status; }
    /// @brief Gets the instance's sequence number within its workflow execution.
    /// @return the sequence number.
    [[nodiscard]] std::uint32_t get_seq() const noexcept { return m_seq; }
    /// @brief Gets the input data map.
    /// @return the configured input key/value pairs.
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_input_data() const noexcept {
        return m_input_data;
    }
    /// @brief Gets the output data map.
    /// @return the configured output key/value pairs.
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_output_data() const noexcept {
        return m_output_data;
    }
    /// @brief Gets the instance's scheduled/started/completed timings.
    /// @return the configured timings.
    [[nodiscard]] const ExecutionTimings &get_timings() const noexcept { return m_timings; }
    /// @brief Gets how many retries this instance has gone through so far — no cap, worth
    /// checking against the TaskDef's RetryPolicy max_attempts before retrying again.
    /// @return the retry count.
    [[nodiscard]] std::uint32_t get_retry_count() const noexcept { return m_retry_count; }
    /// @brief Gets when this instance's timeout should fire, if any.
    /// @return the deadline, or std::nullopt if unset.
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_deadline_at() const noexcept {
        return m_deadline_at;
    }
    /// @brief Gets when this instance should next be retried, if a retry is pending.
    /// @return the next retry time, or std::nullopt if none is pending.
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_next_retry_at() const noexcept {
        return m_next_retry_at;
    }
    /// @brief Gets the child WorkflowExecution this SUB_WORKFLOW instance is waiting on.
    /// @return the child execution's id, or std::nullopt if this isn't a SUB_WORKFLOW instance.
    [[nodiscard]] const std::optional<ExecutionId> &get_sub_workflow_exec_id() const noexcept {
        return m_sub_workflow_exec_id;
    }

    /**
     * @brief Checks the instance is internally consistent — non-empty def_name and timings that
     * respect scheduled → started → completed order.
     * @warning No check on m_status vs m_timings agreement — a TERMINAL status like COMPLETED
     * with a nullopt completed_at will still validate clean here. If you need that invariant,
     * enforce it yourself, this W doesn't cover it.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Gotta know which TaskDef spawned this instance — empty def_name means nothing to
        // trace it back to.
        if (m_def_name.empty()) {
            return std::unexpected{"TaskInstance def_name must not be empty"};
        }
        // Delegate the scheduled/started/completed ordering check to ExecutionTimings itself.
        if (auto result = m_timings.validate(); !result) {
            return result;
        }
        return {};
    }

  private:
    TaskId m_task_id;
    std::string m_def_name;
    std::string m_node_ref;
    ExecutionId m_workflow_exec_id;
    TaskStatus m_status{TaskStatus::SCHEDULED};
    std::uint32_t m_seq{0};
    std::unordered_map<std::string, std::string> m_input_data;
    std::unordered_map<std::string, std::string> m_output_data;
    ExecutionTimings m_timings;
    std::uint32_t m_retry_count{0};
    std::optional<std::chrono::system_clock::time_point> m_deadline_at;
    std::optional<std::chrono::system_clock::time_point> m_next_retry_at;
    std::optional<ExecutionId> m_sub_workflow_exec_id;
};

} // namespace model

template <>
struct serde::Serializable<model::TaskInstance> {
    /// @brief The DB table this instance gets persisted to — motion, one table per row.
    /// @return the table name, "task_instances".
    static constexpr std::string_view table_name() { return "task_instances"; }
    /**
     * @brief Field-descriptor table wiring TaskInstance's columns (task_id, status, data maps,
     * timings, etc) to their getters/setters, for serde (de)serialization — task_id is the PK.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"task_id", &model::TaskInstance::get_task_id,
                       &model::TaskInstance::set_task_id,
                       serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"workflow_exec_id", &model::TaskInstance::get_workflow_exec_id,
                       &model::TaskInstance::set_workflow_exec_id>{},
            serde::FieldDesc<"def_name", &model::TaskInstance::get_def_name,
                       &model::TaskInstance::set_def_name>{},
            serde::FieldDesc<"node_ref", &model::TaskInstance::get_node_ref,
                       &model::TaskInstance::set_node_ref>{},
            serde::FieldDesc<"status", &model::TaskInstance::get_status,
                       &model::TaskInstance::set_status>{},
            serde::FieldDesc<"seq", &model::TaskInstance::get_seq, &model::TaskInstance::set_seq>{},
            serde::FieldDesc<"retry_count", &model::TaskInstance::get_retry_count,
                       &model::TaskInstance::set_retry_count>{},
            serde::FieldDesc<"input_data", &model::TaskInstance::get_input_data,
                       &model::TaskInstance::set_input_data>{},
            serde::FieldDesc<"output_data", &model::TaskInstance::get_output_data,
                       &model::TaskInstance::set_output_data>{},
            serde::FieldDesc<"timings", &model::TaskInstance::get_timings,
                       &model::TaskInstance::set_timings>{},
            serde::FieldDesc<"deadline_at", &model::TaskInstance::get_deadline_at,
                       &model::TaskInstance::set_deadline_at>{},
            serde::FieldDesc<"next_retry_at", &model::TaskInstance::get_next_retry_at,
                       &model::TaskInstance::set_next_retry_at>{},
            serde::FieldDesc<"sub_workflow_exec_id", &model::TaskInstance::get_sub_workflow_exec_id,
                       &model::TaskInstance::set_sub_workflow_exec_id>{},
        };
    }
};
