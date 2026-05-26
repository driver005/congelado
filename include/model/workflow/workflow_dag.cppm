export module model:workflow_dag;

import std;

export namespace model {

struct InputMapping {
    std::string source; // "$.task_name.output.key" or "$.workflow.input.key"
    std::string target; // input_key name on the receiving task
};

struct OutputMapping {
    std::string source; // "$.task_name.output.key"
    std::string target; // workflow output key name
};

struct TaskEdge {
    std::string                  from;
    std::string                  to;
    std::optional<std::string>   condition; // expression string; null on non-SWITCH edges
    std::vector<InputMapping>    mappings;
};

struct TaskNode {
    std::string           task_def_name;
    std::vector<TaskEdge> edges;
};

} // namespace model
