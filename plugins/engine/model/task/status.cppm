export module model:task_status;

import std;

export namespace model {

enum class TaskStatus : std::uint8_t {
    SCHEDULED,
    IN_PROGRESS,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    SKIPPED,
    CANCELED,
};

enum class TaskType : std::uint8_t {
    SIMPLE,
    FORK,
    JOIN,
    SWITCH,
    SUB_WORKFLOW,
    DYNAMIC,
    TERMINATE,
    SET_VARIABLE,
    NOOP,
    LAMBDA,
    INLINE,
    JSON_JQ_TRANSFORM,
    DO_WHILE,
    FORK_JOIN_DYNAMIC,
    START_WORKFLOW,
    WAIT,
    HUMAN,
    EVENT,
};

enum class TaskResult : std::uint8_t {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    SKIPPED,
};

[[nodiscard]] constexpr bool is_terminal(TaskStatus status) noexcept {
    return status == TaskStatus::COMPLETED || status == TaskStatus::FAILED || status == TaskStatus::TIMED_OUT ||
           status == TaskStatus::SKIPPED || status == TaskStatus::CANCELED;
}

} // namespace model
