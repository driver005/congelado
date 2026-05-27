export module model:workflow_exec;

import std;
import :identifiers;
import :timestamps;
import :workflow_status;
import :task_instance;

export namespace model {

class WorkflowExecution {
  public:
    WorkflowExecution() = default;

    void add_task_instance(TaskInstance instance) { m_task_instances.push_back(std::move(instance)); }
    void add_variable(std::string key, std::string value) { m_variables.emplace(std::move(key), std::move(value)); }

    void set_exec_id(ExecutionId execution_id) { m_exec_id = execution_id; }
    void set_def_name(std::string name) { m_def_name = std::move(name); }
    void set_def_version(std::uint32_t version) noexcept { m_def_version = version; }
    void set_status(WorkflowStatus status) noexcept { m_status = status; }
    void set_correlation_id(std::optional<CorrelationId> correlation_id) { m_correlation_id = correlation_id; }
    void set_variables(std::unordered_map<std::string, std::string> variables) { m_variables = std::move(variables); }
    void set_task_instances(std::vector<TaskInstance> instances) { m_task_instances = std::move(instances); }
    void set_timings(ExecutionTimings timiming) { m_timings = timiming; }

    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    [[nodiscard]] const ExecutionId &get_exec_id() const noexcept { return m_exec_id; }
    [[nodiscard]] std::uint32_t get_def_version() const noexcept { return m_def_version; }
    [[nodiscard]] WorkflowStatus get_status() const noexcept { return m_status; }
    [[nodiscard]] const std::optional<CorrelationId> &get_correlation_id() const noexcept { return m_correlation_id; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_variables() const noexcept {
        return m_variables;
    }
    [[nodiscard]] const std::vector<TaskInstance> &get_task_instances() const noexcept { return m_task_instances; }
    [[nodiscard]] const ExecutionTimings &get_timings() const noexcept { return m_timings; }

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
