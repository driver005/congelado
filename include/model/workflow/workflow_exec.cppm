export module model:workflow_exec;

import std;
import :identifiers;
import :timestamps;
import :workflow_status;
import :task_instance;

export namespace model {

struct WorkflowExecution {
    ExecutionId                                   exec_id;
    std::string                                   def_name;
    std::uint32_t                                 def_version{1};
    WorkflowStatus                                status{WorkflowStatus::RUNNING};
    std::optional<CorrelationId>                  correlation_id;
    std::unordered_map<std::string, std::string>  variables;    // shared execution context; both explicit mappings and task outputs land here
    std::vector<TaskInstance>                     task_instances;
    ExecutionTimings                              timings{};
};

} // namespace model
