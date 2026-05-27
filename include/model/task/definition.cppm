export module model:task_def;

import std;
import :task_status;
import :policies;

export namespace model {

class TaskDef {
  public:
    TaskDef() = default;

    void add_input_key(std::string key) { m_input_keys.push_back(std::move(key)); }
    void add_output_key(std::string key) { m_output_keys.push_back(std::move(key)); }

    void set_name(std::string name) { m_name = std::move(name); }
    void set_type(TaskType type) noexcept { m_type = type; }
    void set_worker_type(std::string type) { m_worker_type = std::move(type); }
    void set_input_keys(std::vector<std::string> input) { m_input_keys = std::move(input); }
    void set_output_keys(std::vector<std::string> output) { m_output_keys = std::move(output); }
    void set_retry(RetryPolicy retry) noexcept { m_retry = retry; }
    void set_timeout(TimeoutPolicy timout) noexcept { m_timeout = timout; }
    void set_rate_limit(std::optional<RateLimitPolicy> rate_limit) noexcept { m_rate_limit = rate_limit; }

    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    [[nodiscard]] TaskType get_type() const noexcept { return m_type; }
    [[nodiscard]] const std::string &get_worker_type() const noexcept { return m_worker_type; }
    [[nodiscard]] const std::vector<std::string> &get_input_keys() const noexcept { return m_input_keys; }
    [[nodiscard]] const std::vector<std::string> &get_output_keys() const noexcept { return m_output_keys; }
    [[nodiscard]] const RetryPolicy &get_retry() const noexcept { return m_retry; }
    [[nodiscard]] const TimeoutPolicy &get_timeout() const noexcept { return m_timeout; }
    [[nodiscard]] const std::optional<RateLimitPolicy> &get_rate_limit() const noexcept { return m_rate_limit; }


  private:
    std::string m_name;
    TaskType m_type{TaskType::SIMPLE};
    std::string m_worker_type;
    std::vector<std::string> m_input_keys;
    std::vector<std::string> m_output_keys;
    RetryPolicy m_retry;
    TimeoutPolicy m_timeout;
    std::optional<RateLimitPolicy> m_rate_limit;
};

} // namespace model
