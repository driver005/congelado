export module model:task_status;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"is_terminal(TaskStatus)"> is_terminal_task_status_suite = [] {
    "COMPLETED, FAILED, TIMED_OUT, SKIPPED, CANCELED are terminal"_test = [] {
        expect(is_terminal(TaskStatus::COMPLETED));
        expect(is_terminal(TaskStatus::FAILED));
        expect(is_terminal(TaskStatus::TIMED_OUT));
        expect(is_terminal(TaskStatus::SKIPPED));
        expect(is_terminal(TaskStatus::CANCELED));
    };
    "SCHEDULED and IN_PROGRESS are not terminal"_test = [] {
        expect(not is_terminal(TaskStatus::SCHEDULED));
        expect(not is_terminal(TaskStatus::IN_PROGRESS));
    };
};

} // namespace model::tests
#endif
