export module model:workflow_exec;

import std;
import :identifiers;
import :timestamps;
import :workflow_status;
import :task_instance;
import serde;

export namespace model {

class WorkflowExecution {
  public:
    /// @brief Default ctor, bet — nil exec_id, empty def_name, def_version 1, status RUNNING,
    /// no correlation id, empty variables/task_instances, default timings.
    WorkflowExecution() = default;

    /// @brief Appends a task instance to this execution's instance list — no cap, this is how
    /// the DAG's nodes turn into actual tracked work as the run progresses.
    /// @param instance the task instance to add.
    void add_task_instance(TaskInstance instance) {
        m_task_instances.push_back(std::move(instance));
    }
    /// @brief Adds a single variable key/value pair, overwriting any existing entry for that
    /// key.
    /// @param key the variable name.
    /// @param value the variable value.
    void add_variable(std::string key, std::string value) {
        m_variables.emplace(std::move(key), std::move(value));
    }

    /// @brief Sets this execution's own id.
    /// @param execution_id the new execution id.
    void set_exec_id(ExecutionId execution_id) { m_exec_id = execution_id; }
    /// @brief Sets the name of the WorkflowDef this execution was spawned from.
    /// @param name the workflow definition name.
    void set_def_name(std::string name) { m_def_name = std::move(name); }
    /// @brief Sets the version of the WorkflowDef this execution was spawned from.
    /// @param version the workflow definition version.
    void set_def_version(std::uint32_t version) noexcept { m_def_version = version; }
    /// @brief Sets the execution's current status.
    /// @param status the new status.
    void set_status(WorkflowStatus status) noexcept { m_status = status; }
    /// @brief Sets the optional correlation id used to group related executions.
    /// @param correlation_id the new correlation id, or std::nullopt to clear it.
    void set_correlation_id(std::optional<CorrelationId> correlation_id) {
        m_correlation_id = correlation_id;
    }
    /// @brief Replaces the whole variables map wholesale.
    /// @param variables the new variable key/value map.
    void set_variables(std::unordered_map<std::string, std::string> variables) {
        m_variables = std::move(variables);
    }
    /// @brief Replaces the whole task instance list wholesale.
    /// @param instances the new set of task instances.
    void set_task_instances(std::vector<TaskInstance> instances) {
        m_task_instances = std::move(instances);
    }
    /// @brief Sets the execution's scheduled/started/completed timings.
    /// @param timings the new timings.
    void set_timings(ExecutionTimings timings) { m_timings = timings; }

    /// @brief Gets the name of the WorkflowDef this execution was spawned from.
    /// @return the workflow definition name.
    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    /// @brief Gets this execution's own id.
    /// @return the execution id.
    [[nodiscard]] const ExecutionId &get_exec_id() const noexcept { return m_exec_id; }
    /// @brief Gets the version of the WorkflowDef this execution was spawned from.
    /// @return the workflow definition version.
    [[nodiscard]] std::uint32_t get_def_version() const noexcept { return m_def_version; }
    /// @brief Gets the execution's current status.
    /// @return the current status.
    [[nodiscard]] WorkflowStatus get_status() const noexcept { return m_status; }
    /// @brief Gets the optional correlation id used to group related executions — lowkey handy
    /// for tying a chain of triggered sub-workflows back to one caller.
    /// @return the correlation id, or std::nullopt if none is set.
    [[nodiscard]] const std::optional<CorrelationId> &get_correlation_id() const noexcept {
        return m_correlation_id;
    }
    /// @brief Gets the execution's variables map.
    /// @return the configured variable key/value pairs.
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_variables() const noexcept {
        return m_variables;
    }
    /// @brief Gets the execution's task instances.
    /// @return the configured task instances.
    [[nodiscard]] const std::vector<TaskInstance> &get_task_instances() const noexcept {
        return m_task_instances;
    }
    /// @brief Gets the execution's scheduled/started/completed timings.
    /// @return the configured timings.
    [[nodiscard]] const ExecutionTimings &get_timings() const noexcept { return m_timings; }

    /**
     * @brief Checks the execution is internally consistent — non-empty def_name, def_version at
     * least 1, valid timings, and every task instance validates clean.
     * @warning Unlike WorkflowEvent::validate(), this never checks exec_id for nil-ness, so a
     * default-constructed nil exec_id passes right through. Also no cross-check between
     * m_status and is_terminal()/timings — a RUNNING execution with a completed_at set, or a
     * COMPLETED one with no completed_at, both validate clean. Lowkey inconsistent with the
     * sibling types in this module, so don't lean on this alone for state-machine correctness.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Every execution has to point back at the WorkflowDef it was spawned from.
        if (m_def_name.empty()) {
            return std::unexpected{"WorkflowExecution def_name must not be empty"};
        }
        // Same versioning rule as WorkflowDef — 0 isn't a real version.
        if (m_def_version == 0) {
            return std::unexpected{"WorkflowExecution def_version must be at least 1"};
        }
        // Delegate the scheduled/started/completed ordering check to ExecutionTimings.
        if (auto result = m_timings.validate(); !result) {
            return result;
        }
        // Then sweep every spawned task instance and bail on the first one that's not clean.
        for (auto const &instance : m_task_instances) {
            if (auto result = instance.validate(); !result) {
                return result;
            }
        }
        return {};
    }

  private:
    ExecutionId m_exec_id;
    std::string m_def_name;
    std::uint32_t m_def_version{1};
    WorkflowStatus m_status{WorkflowStatus::RUNNING};
    std::optional<CorrelationId> m_correlation_id;
    std::unordered_map<std::string, std::string> m_variables;
    std::vector<TaskInstance> m_task_instances;
    ExecutionTimings m_timings;
};

} // namespace model

template <>
struct serde::Serializable<model::WorkflowExecution> {
    /// @brief The DB table this execution gets persisted to — bet, third and last table_name()
    /// in this module.
    /// @return the table name, "workflow_executions".
    static constexpr std::string_view table_name() { return "workflow_executions"; }
    /**
     * @brief Field-descriptor table wiring WorkflowExecution's columns (exec_id, def_name,
     * status, variables, task_instances, timings, etc) to their getters/setters, for serde
     * (de)serialization — exec_id is the PK.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"exec_id", &model::WorkflowExecution::get_exec_id,
                       &model::WorkflowExecution::set_exec_id,
                       serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"def_name", &model::WorkflowExecution::get_def_name,
                       &model::WorkflowExecution::set_def_name>{},
            serde::FieldDesc<"def_version", &model::WorkflowExecution::get_def_version,
                       &model::WorkflowExecution::set_def_version>{},
            serde::FieldDesc<"status", &model::WorkflowExecution::get_status,
                       &model::WorkflowExecution::set_status>{},
            serde::FieldDesc<"correlation_id", &model::WorkflowExecution::get_correlation_id,
                       &model::WorkflowExecution::set_correlation_id>{},
            serde::FieldDesc<"variables", &model::WorkflowExecution::get_variables,
                       &model::WorkflowExecution::set_variables>{},
            serde::FieldDesc<"task_instances", &model::WorkflowExecution::get_task_instances,
                       &model::WorkflowExecution::set_task_instances>{},
            serde::FieldDesc<"timings", &model::WorkflowExecution::get_timings,
                       &model::WorkflowExecution::set_timings>{},
        };
    }
};
