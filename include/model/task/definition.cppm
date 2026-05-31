export module model:task_def;

import std;
import :task_status;
import :policies;
import ser;

export namespace model {

class TaskDef {
  public:
    TaskDef() = default;

    void add_input_key(std::string key)  { m_input_keys.push_back(std::move(key)); }
    void add_output_key(std::string key) { m_output_keys.push_back(std::move(key)); }

    void set_name(std::string name)                                { m_name = std::move(name); }
    void set_type(TaskType type) noexcept                          { m_type = type; }
    void set_worker_type(std::string type)                         { m_worker_type = std::move(type); }
    void set_input_keys(std::vector<std::string> input)            { m_input_keys = std::move(input); }
    void set_output_keys(std::vector<std::string> output)          { m_output_keys = std::move(output); }
    void set_retry(RetryPolicy retry) noexcept                     { m_retry = retry; }
    void set_timeout(TimeoutPolicy timeout) noexcept               { m_timeout = timeout; }
    void set_rate_limit(std::optional<RateLimitPolicy> rate_limit) noexcept { m_rate_limit = rate_limit; }

    [[nodiscard]] const std::string& get_name() const noexcept                          { return m_name; }
    [[nodiscard]] TaskType get_type() const noexcept                                     { return m_type; }
    [[nodiscard]] const std::string& get_worker_type() const noexcept                   { return m_worker_type; }
    [[nodiscard]] const std::vector<std::string>& get_input_keys() const noexcept       { return m_input_keys; }
    [[nodiscard]] const std::vector<std::string>& get_output_keys() const noexcept      { return m_output_keys; }
    [[nodiscard]] const RetryPolicy& get_retry() const noexcept                         { return m_retry; }
    [[nodiscard]] const TimeoutPolicy& get_timeout() const noexcept                     { return m_timeout; }
    [[nodiscard]] const std::optional<RateLimitPolicy>& get_rate_limit() const noexcept { return m_rate_limit; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_name.empty())
            return std::unexpected{"TaskDef name must not be empty"};
        if (m_type == TaskType::SIMPLE && m_worker_type.empty())
            return std::unexpected{"TaskDef worker_type must not be empty for SIMPLE tasks"};
        if (auto result = m_retry.validate(); !result)   return result;
        if (auto result = m_timeout.validate(); !result) return result;
        if (m_rate_limit)
            if (auto result = m_rate_limit->validate(); !result) return result;
        return {};
    }

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

template<> struct ser::Serializable<model::TaskDef> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"name",
                &model::TaskDef::get_name,
                &model::TaskDef::set_name>(),
            ser::field<"type",
                &model::TaskDef::get_type,
                &model::TaskDef::set_type>(),
            ser::field<"worker_type",
                &model::TaskDef::get_worker_type,
                &model::TaskDef::set_worker_type>(),
            ser::field<"input_keys",
                &model::TaskDef::get_input_keys,
                &model::TaskDef::set_input_keys>(),
            ser::field<"output_keys",
                &model::TaskDef::get_output_keys,
                &model::TaskDef::set_output_keys>(),
            ser::field<"retry",
                &model::TaskDef::get_retry,
                &model::TaskDef::set_retry>(),
            ser::field<"timeout",
                &model::TaskDef::get_timeout,
                &model::TaskDef::set_timeout>(),
            ser::field<"rate_limit",
                &model::TaskDef::get_rate_limit,
                &model::TaskDef::set_rate_limit>(),
        };
    }
};
