export module model:workflow_status;

import std;

export namespace model {

enum class WorkflowStatus : std::uint8_t {
    RUNNING,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    PAUSED,
    TERMINATED,
};

[[nodiscard]] constexpr bool is_terminal(WorkflowStatus status) noexcept {
    return status == WorkflowStatus::COMPLETED || status == WorkflowStatus::FAILED ||
           status == WorkflowStatus::TIMED_OUT || status == WorkflowStatus::TERMINATED;
}

} // namespace model
