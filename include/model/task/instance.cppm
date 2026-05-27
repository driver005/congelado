export module model:task_instance;

import std;
import :identifiers;
import :timestamps;
import :task_status;

export namespace model {

class TaskInstance {
  public:
    TaskInstance() = default;

    void add_input_data(std::string key, std::string value) { m_input_data.emplace(std::move(key), std::move(value)); }
    void add_output_data(std::string key, std::string value) { m_output_data.emplace(std::move(key), std::move(value)); }

    void set_workflow_exec_id(ExecutionId execution_id) { m_workflow_exec_id = execution_id; }
    void set_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    void set_task_id(TaskId task_id) { m_task_id = task_id; }
    void set_status(TaskStatus status) noexcept { m_status = status; }
    void set_seq(std::uint32_t seq) noexcept { m_seq = seq; }
    void set_input_data(std::unordered_map<std::string, std::string> data) { m_input_data = std::move(data); }
    void set_output_data(std::unordered_map<std::string, std::string> data) { m_output_data = std::move(data); }
    void set_timings(ExecutionTimings timing) { m_timings = timing; }
    void set_retry_count(std::uint32_t count) noexcept { m_retry_count = count; }

    [[nodiscard]] const TaskId &get_task_id() const noexcept { return m_task_id; }
    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    [[nodiscard]] const ExecutionId &get_workflow_exec_id() const noexcept { return m_workflow_exec_id; }
    [[nodiscard]] TaskStatus get_status() const noexcept { return m_status; }
    [[nodiscard]] std::uint32_t get_seq() const noexcept { return m_seq; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_input_data() const noexcept {
        return m_input_data;
    }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_output_data() const noexcept {
        return m_output_data;
    }
    [[nodiscard]] const ExecutionTimings &get_timings() const noexcept { return m_timings; }
    [[nodiscard]] std::uint32_t get_retry_count() const noexcept { return m_retry_count; }

  private:
    TaskId m_task_id;
    std::string m_def_name;
    ExecutionId m_workflow_exec_id;
    TaskStatus m_status{TaskStatus::SCHEDULED};
    std::uint32_t m_seq{0};
    std::unordered_map<std::string, std::string> m_input_data;
    std::unordered_map<std::string, std::string> m_output_data;
    ExecutionTimings m_timings;
    std::uint32_t m_retry_count{0};
};

} // namespace model
