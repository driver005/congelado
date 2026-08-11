export module engine:search_projector;

import std;
import model;
import interfaces;
import serde;
import core_logger;
import core_events;
import :context;

export namespace engine {

// Collection names this projector uses on whichever ISearchProvider is wired in — the interface
// itself has no notion of workflows/tasks, these are just this plugin's own namespacing choice.
inline constexpr std::string_view WORKFLOW_SUMMARY_COLLECTION = "workflow_summaries";
inline constexpr std::string_view TASK_SUMMARY_COLLECTION = "task_summaries";

/// @brief Builds WorkflowSummary/TaskSummary read-model projections and pushes them into
/// whichever ISearchProvider is currently wired into EngineContext, on every terminal
/// transition — a complete no-op (not an error) when no search backend is configured, same
/// "optional infra degrades gracefully" story as EngineContext's db/lua_bridge slots.
class SummaryProjector {
  public:
    explicit SummaryProjector(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Projects a just-terminated WorkflowExecution into the active search backend.
     * @param exec the execution to project — read-only, this doesn't mutate or persist it.
     */
    void project_workflow(const model::WorkflowExecution &exec) noexcept {
        auto *provider = m_ctx.get().get_search();
        if (provider == nullptr) {
            return;
        }
        model::WorkflowSummary summary;
        summary.set_exec_id(exec.get_exec_id());
        summary.set_workflow_type(exec.get_def_name());
        summary.set_version(exec.get_def_version());
        summary.set_status(exec.get_status());
        summary.set_correlation_id(exec.get_correlation_id());
        summary.set_start_time(exec.get_timings().get_started_at());
        summary.set_end_time(exec.get_timings().get_completed_at());
        std::vector<std::string> failed_names;
        for (auto const &instance : exec.get_task_instances()) {
            if (instance.get_status() == model::TaskStatus::FAILED ||
                instance.get_status() == model::TaskStatus::TIMED_OUT) {
                failed_names.push_back(instance.get_def_name());
            }
        }
        summary.set_failed_task_names(std::move(failed_names));

        auto exec_id = std::format("{}", exec.get_exec_id());
        auto json = to_json(summary);
        provider->index(
            WORKFLOW_SUMMARY_COLLECTION, exec_id, json, [exec_id](std::string_view result) {
                if (result.empty()) {
                    core::logger::warning("engine", "search index failed for workflow '{}'",
                                          exec_id);
                    core::events::publish("engine.search.index_failed",
                                          {{"collection", std::string{WORKFLOW_SUMMARY_COLLECTION}},
                                           {"id", exec_id}});
                }
            });
    }

    /**
     * @brief Projects a just-terminated TaskInstance into the active search backend.
     * @param instance the task instance to project — read-only, this doesn't mutate or persist
     * it.
     */
    void project_task(const model::TaskInstance &instance) noexcept {
        auto *provider = m_ctx.get().get_search();
        if (provider == nullptr) {
            return;
        }
        model::TaskSummary summary;
        summary.set_task_id(instance.get_task_id());
        summary.set_task_def_name(instance.get_def_name());
        summary.set_workflow_exec_id(instance.get_workflow_exec_id());
        summary.set_status(instance.get_status());
        auto const &timings = instance.get_timings();
        summary.set_scheduled_time(timings.get_scheduled_at());
        summary.set_start_time(timings.get_started_at());
        summary.set_update_time(timings.get_completed_at());
        if (timings.get_scheduled_at() && timings.get_started_at()) {
            auto wait = *timings.get_started_at() - *timings.get_scheduled_at();
            summary.set_queue_wait_time_ms(static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(wait).count()));
        }

        auto task_id = std::format("{}", instance.get_task_id());
        auto json = to_json(summary);
        provider->index(TASK_SUMMARY_COLLECTION, task_id, json, [task_id](std::string_view result) {
            if (result.empty()) {
                core::logger::warning("engine", "search index failed for task '{}'", task_id);
                core::events::publish(
                    "engine.search.index_failed",
                    {{"collection", std::string{TASK_SUMMARY_COLLECTION}}, {"id", task_id}});
            }
        });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    template <typename T>
    [[nodiscard]] static std::string to_json(const T &value) noexcept {
        auto bytes = serde::Ser::serialize("application/json", value);
        std::string out;
        out.reserve(bytes.size());
        for (std::byte byte : bytes) {
            out += static_cast<char>(byte);
        }
        return out;
    }
};

} // namespace engine
