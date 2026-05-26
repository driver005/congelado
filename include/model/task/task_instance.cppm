export module model:task_instance;

import std;
import :identifiers;
import :timestamps;
import :task_status;

export namespace model {

struct TaskInstance {
    TaskId                                        task_id;
    std::string                                   def_name;
    ExecutionId                                   workflow_exec_id;
    TaskStatus                                    status{TaskStatus::SCHEDULED};
    std::uint32_t                                 seq{0};
    std::unordered_map<std::string, std::string>  input_data;
    std::unordered_map<std::string, std::string>  output_data;
    ExecutionTimings                              timings{};
    std::uint32_t                                 retry_count{0};
};

} // namespace model
