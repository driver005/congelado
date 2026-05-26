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

[[nodiscard]] constexpr bool is_terminal(WorkflowStatus s) noexcept {
    using enum WorkflowStatus;
    return s == COMPLETED || s == FAILED || s == TIMED_OUT || s == TERMINATED;
}

} // namespace model
