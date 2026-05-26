export module model:task_def;

import std;
import :task_status;
import :policies;

export namespace model {

struct TaskDef {
    std::string                    name;
    TaskType                       type{TaskType::SIMPLE};
    std::string                    worker_type;
    std::vector<std::string>       input_keys;
    std::vector<std::string>       output_keys;
    RetryPolicy                    retry{};
    TimeoutPolicy                  timeout{};
    std::optional<RateLimitPolicy> rate_limit;
};

} // namespace model
