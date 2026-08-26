export module model:workflow_status;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

enum class WorkflowStatus : std::uint8_t
{
    RUNNING,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    PAUSED,
    TERMINATED,
};

[[nodiscard]] constexpr bool is_terminal(WorkflowStatus status) noexcept
{
    return status == WorkflowStatus::COMPLETED || status == WorkflowStatus::FAILED ||
           status == WorkflowStatus::TIMED_OUT || status == WorkflowStatus::TERMINATED;
}

} // namespace model

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"is_terminal(WorkflowStatus)"> is_terminal_workflow_status_suite = [] {
    "COMPLETED, FAILED, TIMED_OUT, TERMINATED are terminal"_test = [] {
        expect(is_terminal(WorkflowStatus::COMPLETED));
        expect(is_terminal(WorkflowStatus::FAILED));
        expect(is_terminal(WorkflowStatus::TIMED_OUT));
        expect(is_terminal(WorkflowStatus::TERMINATED));
    };
    "RUNNING and PAUSED are not terminal"_test = [] {
        expect(not is_terminal(WorkflowStatus::RUNNING));
        expect(not is_terminal(WorkflowStatus::PAUSED));
    };
};

} // namespace model::tests
#endif
