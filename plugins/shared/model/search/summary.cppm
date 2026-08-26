export module model:search_summary;

import std;
import :identifiers;
import :workflow_status;
import :task_status;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

/// @brief Read-model projection of a terminal WorkflowExecution, built by engine's
/// SummaryProjector and handed to whichever ISearchProvider is active — not persisted through
/// Connector itself (search backends own their own storage), just JSON-encoded at the call
/// site.
class WorkflowSummary
{
public:
    WorkflowSummary() = default;

    void set_exec_id(ExecutionId exec_id)
    {
        m_exec_id = exec_id;
    }

    void set_workflow_type(std::string workflow_type)
    {
        m_workflow_type = std::move(workflow_type);
    }

    void set_version(std::uint32_t version) noexcept
    {
        m_version = version;
    }

    void set_status(WorkflowStatus status) noexcept
    {
        m_status = status;
    }

    void set_correlation_id(std::optional<CorrelationId> correlation_id)
    {
        m_correlation_id = correlation_id;
    }

    void set_start_time(std::optional<std::chrono::system_clock::time_point> value) noexcept
    {
        m_start_time = value;
    }

    void set_end_time(std::optional<std::chrono::system_clock::time_point> value) noexcept
    {
        m_end_time = value;
    }

    void set_failed_task_names(std::vector<std::string> names)
    {
        m_failed_task_names = std::move(names);
    }

    [[nodiscard]] const ExecutionId& get_exec_id() const noexcept
    {
        return m_exec_id;
    }

    [[nodiscard]] const std::string& get_workflow_type() const noexcept
    {
        return m_workflow_type;
    }

    [[nodiscard]] std::uint32_t get_version() const noexcept
    {
        return m_version;
    }

    [[nodiscard]] WorkflowStatus get_status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::optional<CorrelationId>& get_correlation_id() const noexcept
    {
        return m_correlation_id;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>&
    get_start_time() const noexcept
    {
        return m_start_time;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>&
    get_end_time() const noexcept
    {
        return m_end_time;
    }

    [[nodiscard]] const std::vector<std::string>& get_failed_task_names() const noexcept
    {
        return m_failed_task_names;
    }

private:
    ExecutionId m_exec_id;
    std::string m_workflow_type;
    std::uint32_t m_version{1};
    WorkflowStatus m_status{WorkflowStatus::RUNNING};
    std::optional<CorrelationId> m_correlation_id;
    std::optional<std::chrono::system_clock::time_point> m_start_time;
    std::optional<std::chrono::system_clock::time_point> m_end_time;
    std::vector<std::string> m_failed_task_names;
};

/// @brief Read-model projection of a terminal TaskInstance, same "projector-built,
/// backend-JSON- encoded, never Connector-persisted" story as WorkflowSummary.
class TaskSummary
{
public:
    TaskSummary() = default;

    void set_task_id(TaskId task_id)
    {
        m_task_id = task_id;
    }

    void set_task_def_name(std::string name)
    {
        m_task_def_name = std::move(name);
    }

    void set_workflow_exec_id(ExecutionId exec_id)
    {
        m_workflow_exec_id = exec_id;
    }

    void set_status(TaskStatus status) noexcept
    {
        m_status = status;
    }

    void set_scheduled_time(std::optional<std::chrono::system_clock::time_point> value) noexcept
    {
        m_scheduled_time = value;
    }

    void set_start_time(std::optional<std::chrono::system_clock::time_point> value) noexcept
    {
        m_start_time = value;
    }

    void set_update_time(std::optional<std::chrono::system_clock::time_point> value) noexcept
    {
        m_update_time = value;
    }

    void set_queue_wait_time_ms(std::uint64_t millis) noexcept
    {
        m_queue_wait_time_ms = millis;
    }

    [[nodiscard]] const TaskId& get_task_id() const noexcept
    {
        return m_task_id;
    }

    [[nodiscard]] const std::string& get_task_def_name() const noexcept
    {
        return m_task_def_name;
    }

    [[nodiscard]] const ExecutionId& get_workflow_exec_id() const noexcept
    {
        return m_workflow_exec_id;
    }

    [[nodiscard]] TaskStatus get_status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>&
    get_scheduled_time() const noexcept
    {
        return m_scheduled_time;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>&
    get_start_time() const noexcept
    {
        return m_start_time;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>&
    get_update_time() const noexcept
    {
        return m_update_time;
    }

    [[nodiscard]] std::uint64_t get_queue_wait_time_ms() const noexcept
    {
        return m_queue_wait_time_ms;
    }

private:
    TaskId m_task_id;
    std::string m_task_def_name;
    ExecutionId m_workflow_exec_id;
    TaskStatus m_status{TaskStatus::SCHEDULED};
    std::optional<std::chrono::system_clock::time_point> m_scheduled_time;
    std::optional<std::chrono::system_clock::time_point> m_start_time;
    std::optional<std::chrono::system_clock::time_point> m_update_time;
    std::uint64_t m_queue_wait_time_ms{0};
};

} // namespace model

template<>
struct serde::Serializable<model::WorkflowSummary>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "exec_id", &model::WorkflowSummary::get_exec_id,
                &model::WorkflowSummary::set_exec_id>{},
            serde::FieldDesc<
                "workflow_type", &model::WorkflowSummary::get_workflow_type,
                &model::WorkflowSummary::set_workflow_type>{},
            serde::FieldDesc<
                "version", &model::WorkflowSummary::get_version,
                &model::WorkflowSummary::set_version>{},
            serde::FieldDesc<
                "status", &model::WorkflowSummary::get_status,
                &model::WorkflowSummary::set_status>{},
            serde::FieldDesc<
                "correlation_id", &model::WorkflowSummary::get_correlation_id,
                &model::WorkflowSummary::set_correlation_id>{},
            serde::FieldDesc<
                "start_time", &model::WorkflowSummary::get_start_time,
                &model::WorkflowSummary::set_start_time>{},
            serde::FieldDesc<
                "end_time", &model::WorkflowSummary::get_end_time,
                &model::WorkflowSummary::set_end_time>{},
            serde::FieldDesc<
                "failed_task_names", &model::WorkflowSummary::get_failed_task_names,
                &model::WorkflowSummary::set_failed_task_names>{},
        };
    }
};

template<>
struct serde::Serializable<model::TaskSummary>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "task_id", &model::TaskSummary::get_task_id, &model::TaskSummary::set_task_id>{},
            serde::FieldDesc<
                "task_def_name", &model::TaskSummary::get_task_def_name,
                &model::TaskSummary::set_task_def_name>{},
            serde::FieldDesc<
                "workflow_exec_id", &model::TaskSummary::get_workflow_exec_id,
                &model::TaskSummary::set_workflow_exec_id>{},
            serde::FieldDesc<
                "status", &model::TaskSummary::get_status, &model::TaskSummary::set_status>{},
            serde::FieldDesc<
                "scheduled_time", &model::TaskSummary::get_scheduled_time,
                &model::TaskSummary::set_scheduled_time>{},
            serde::FieldDesc<
                "start_time", &model::TaskSummary::get_start_time,
                &model::TaskSummary::set_start_time>{},
            serde::FieldDesc<
                "update_time", &model::TaskSummary::get_update_time,
                &model::TaskSummary::set_update_time>{},
            serde::FieldDesc<
                "queue_wait_time_ms", &model::TaskSummary::get_queue_wait_time_ms,
                &model::TaskSummary::set_queue_wait_time_ms>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"WorkflowSummary"> workflow_summary_suite = [] {
    "defaults to version 1 and RUNNING"_test = [] {
        WorkflowSummary summary;

        expect(summary.get_version() == 1);
        expect(summary.get_status() == WorkflowStatus::RUNNING);
        expect(summary.get_failed_task_names().empty());
    };
    "setters round-trip through their getters"_test = [] {
        WorkflowSummary summary;
        auto exec_id = generate_id();
        summary.set_exec_id(exec_id);
        summary.set_workflow_type("order_pipeline");
        summary.set_status(WorkflowStatus::FAILED);
        summary.set_failed_task_names({"charge_payment"});

        expect(summary.get_exec_id() == exec_id);
        expect(summary.get_workflow_type() == "order_pipeline");
        expect(summary.get_status() == WorkflowStatus::FAILED);
        expect(summary.get_failed_task_names().size() == 1);
    };
};

suite<"TaskSummary"> task_summary_suite = [] {
    "defaults to SCHEDULED and zero queue wait"_test = [] {
        TaskSummary summary;

        expect(summary.get_status() == TaskStatus::SCHEDULED);
        expect(summary.get_queue_wait_time_ms() == 0);
    };
    "setters round-trip through their getters"_test = [] {
        TaskSummary summary;
        summary.set_task_def_name("send_email");
        summary.set_status(TaskStatus::COMPLETED);
        summary.set_queue_wait_time_ms(1'500);

        expect(summary.get_task_def_name() == "send_email");
        expect(summary.get_status() == TaskStatus::COMPLETED);
        expect(summary.get_queue_wait_time_ms() == 1'500);
    };
};

} // namespace model::tests
#endif
