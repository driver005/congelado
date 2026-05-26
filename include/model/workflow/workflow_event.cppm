export module model:workflow_event;

import std;
import :identifiers;

export namespace model {

enum class WorkflowEventType : std::uint8_t {
    PAUSE,
    RESUME,
    TERMINATE,
    RESTART,
    SIGNAL,
};

struct WorkflowEvent {
    ExecutionId                          exec_id;
    WorkflowEventType                    type;
    std::optional<std::string>           payload;
    std::chrono::system_clock::time_point issued_at;
};

} // namespace model
