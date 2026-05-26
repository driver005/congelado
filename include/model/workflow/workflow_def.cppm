export module model:workflow_def;

import std;
import :workflow_dag;
import :policies;

export namespace model {

struct WorkflowDef {
    std::string                  name;
    std::uint32_t                version{1};
    std::vector<TaskNode>        nodes;
    std::vector<std::string>     input_params;
    std::vector<OutputMapping>   output_mappings;
    std::optional<std::string>   failure_workflow; // name of fallback WorkflowDef; spawned on terminal FAILED
    std::optional<TimeoutPolicy> timeout;
};

} // namespace model
