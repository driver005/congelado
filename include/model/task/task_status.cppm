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
};

enum class TaskResult : std::uint8_t {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    SKIPPED,
};

[[nodiscard]] constexpr bool is_terminal(TaskStatus s) noexcept {
    using enum TaskStatus;
    return s == COMPLETED || s == FAILED || s == TIMED_OUT
        || s == SKIPPED    || s == CANCELED;
}

} // namespace model
