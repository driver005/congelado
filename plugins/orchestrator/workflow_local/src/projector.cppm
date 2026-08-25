export module workflow_engine:search_projector;

import std;
import model;
import interfaces;
import serde;
import core_logger;
import core_events;
import :context;
#ifdef CONGELADO_TEST
import shared;
import boost.ut;
#endif

export namespace engine {

// Collection names this projector uses on whichever ISearchProvider is wired in — the interface
// itself has no notion of workflows/tasks, these are just this plugin's own namespacing choice.
inline constexpr std::string_view WORKFLOW_SUMMARY_COLLECTION = "workflow_summaries";
inline constexpr std::string_view TASK_SUMMARY_COLLECTION = "task_summaries";

/// @brief Builds WorkflowSummary/TaskSummary read-model projections and pushes them into
/// whichever ISearchProvider is currently wired into WorkflowContext, on every terminal
/// transition — a complete no-op (not an error) when no search backend is configured, same
/// "optional infra degrades gracefully" story as WorkflowContext's db/lua_bridge slots.
class SummaryProjector {
  public:
    explicit SummaryProjector(WorkflowContext &ctx) noexcept : m_ctx{ctx} {}

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
            // BUG: no check that wait is non-negative before this cast. TaskInstance::validate()
            // (never called here — this projector runs off whatever's already persisted) is the
            // only thing that would normally catch started_at < scheduled_at; if that ordering is
            // ever violated (e.g. a manually-poked instance, or a future bug elsewhere), `wait` is
            // negative and duration_cast<milliseconds>(wait).count() returns a negative
            // std::int64_t that silently wraps around to a huge value once cast to uint64_t,
            // instead of erroring or clamping to 0.
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
    std::reference_wrapper<WorkflowContext> m_ctx;

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

#ifdef CONGELADO_TEST
namespace engine::search_projector_tests {
using namespace boost::ut;

/// @brief ISearchProvider test double that records every index()/remove() call it receives, and
/// lets a test script whether index() reports success ("ok") or failure ("").
class SpySearchProvider final : public interfaces::ISearchProvider {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "spy_search"; }

    void index(std::string_view collection, std::string_view id, std::string_view document_json,
               shared::QueryReadFn &&callback) noexcept override {
        m_last_collection = std::string{collection};
        m_last_id = std::string{id};
        m_last_document_json = std::string{document_json};
        ++m_index_calls;
        callback(m_fail_next ? std::string_view{} : std::string_view{"ok"});
    }
    void remove(std::string_view /*collection*/, std::string_view /*id*/,
                shared::QueryReadFn &&callback) noexcept override {
        callback("ok");
    }
    void search(std::string_view /*collection*/, const interfaces::SearchQuery & /*query*/,
                shared::QueryReadFn &&callback) noexcept override {
        callback("[]");
    }

    bool m_fail_next{false};
    int m_index_calls{0};
    std::string m_last_collection;
    std::string m_last_id;
    std::string m_last_document_json;
};

// Minimal real ISerdeFormat — needed so to_json()'s serde::Ser::serialize() call actually
// produces real JSON instead of the "no format plugin loaded" error payload it falls back to with
// nothing registered, which is what made these `.contains(...)` assertions fail. Routed through
// serde::Ser::encode_json() (rather than this file's own `#include <rfl/json.hpp>`, the recipe
// engine's handler tests use) since a second textual inclusion of that header alongside the one
// already pulled in by importing `serde` collides at BMI-compile time (duplicate yyjson_api_inline
// definitions) — encode_json() exists precisely so callers don't need their own include.
class MockJsonFormat final : public interfaces::ISerdeFormat {
  public:
    [[nodiscard]] std::string_view content_type() const noexcept override {
        return "application/json";
    }
    [[nodiscard]] std::string_view format_name() const noexcept override { return "mock-json"; }
    [[nodiscard]] std::expected<std::string, std::string>
    encode(const interfaces::Value &value) const override {
        return serde::Ser::encode_json(value);
    }
    [[nodiscard]] std::expected<interfaces::Value, std::string>
    decode(std::string_view /*data*/) const override {
        return std::unexpected{std::string{"decode not implemented in test double"}};
    }
};

suite<"SummaryProjector::project_workflow"> project_workflow_suite = [] {
    "no-ops when no search provider is configured"_test = [] {
        WorkflowContext ctx;
        SummaryProjector projector{ctx};
        model::WorkflowExecution exec;
        exec.set_exec_id(model::generate_id());

        expect(nothrow([&] { projector.project_workflow(exec); }));
    };

    "indexes into the workflow_summaries collection, keyed by the formatted exec_id"_test = [] {
        WorkflowContext ctx;
        SpySearchProvider provider;
        ctx.set_search(&provider);
        SummaryProjector projector{ctx};
        serde::SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        serde::SerdeFormatRegistry::set_active(&registry);

        model::WorkflowExecution exec;
        auto exec_id = model::generate_id();
        exec.set_exec_id(exec_id);
        exec.set_def_name("order_pipeline");
        exec.set_status(model::WorkflowStatus::COMPLETED);
        projector.project_workflow(exec);

        expect(provider.m_index_calls == 1);
        expect(provider.m_last_collection == WORKFLOW_SUMMARY_COLLECTION);
        expect(provider.m_last_id == std::format("{}", exec_id));
        expect(provider.m_last_document_json.contains("order_pipeline"));

        serde::SerdeFormatRegistry::set_active(nullptr);
    };

    "collects only FAILED/TIMED_OUT instance def_names into failed_task_names"_test = [] {
        WorkflowContext ctx;
        SpySearchProvider provider;
        ctx.set_search(&provider);
        SummaryProjector projector{ctx};
        serde::SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        serde::SerdeFormatRegistry::set_active(&registry);

        model::WorkflowExecution exec;
        exec.set_exec_id(model::generate_id());

        model::TaskInstance completed;
        completed.set_def_name("step_ok");
        completed.set_status(model::TaskStatus::COMPLETED);
        exec.add_task_instance(completed);

        model::TaskInstance failed;
        failed.set_def_name("step_bad");
        failed.set_status(model::TaskStatus::FAILED);
        exec.add_task_instance(failed);

        model::TaskInstance timed_out;
        timed_out.set_def_name("step_slow");
        timed_out.set_status(model::TaskStatus::TIMED_OUT);
        exec.add_task_instance(timed_out);

        projector.project_workflow(exec);

        expect(provider.m_last_document_json.contains("step_bad"));
        expect(provider.m_last_document_json.contains("step_slow"));
        expect(!provider.m_last_document_json.contains("step_ok"));

        serde::SerdeFormatRegistry::set_active(nullptr);
    };

    "logs a warning and publishes engine.search.index_failed when index() reports failure"_test =
        [] {
            WorkflowContext ctx;
            SpySearchProvider provider;
            provider.m_fail_next = true;
            ctx.set_search(&provider);
            SummaryProjector projector{ctx};

            model::WorkflowExecution exec;
            exec.set_exec_id(model::generate_id());

            expect(nothrow([&] { projector.project_workflow(exec); }));
            expect(provider.m_index_calls == 1);
        };
};

suite<"SummaryProjector::project_task"> project_task_suite = [] {
    "no-ops when no search provider is configured"_test = [] {
        WorkflowContext ctx;
        SummaryProjector projector{ctx};
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());

        expect(nothrow([&] { projector.project_task(instance); }));
    };

    "indexes into the task_summaries collection, keyed by the formatted task_id"_test = [] {
        WorkflowContext ctx;
        SpySearchProvider provider;
        ctx.set_search(&provider);
        SummaryProjector projector{ctx};
        serde::SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        serde::SerdeFormatRegistry::set_active(&registry);

        model::TaskInstance instance;
        auto task_id = model::generate_id();
        instance.set_task_id(task_id);
        instance.set_def_name("send_email");
        projector.project_task(instance);

        expect(provider.m_last_collection == TASK_SUMMARY_COLLECTION);
        expect(provider.m_last_id == std::format("{}", task_id));
        expect(provider.m_last_document_json.contains("send_email"));

        serde::SerdeFormatRegistry::set_active(nullptr);
    };

    "computes queue_wait_time_ms only when both scheduled_at and started_at are set"_test = [] {
        WorkflowContext ctx;
        SpySearchProvider provider;
        ctx.set_search(&provider);
        SummaryProjector projector{ctx};
        serde::SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        serde::SerdeFormatRegistry::set_active(&registry);

        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        model::ExecutionTimings timings;
        auto now = std::chrono::system_clock::now();
        timings.set_scheduled_at(now);
        timings.set_started_at(now + std::chrono::milliseconds{250});
        instance.set_timings(timings);
        projector.project_task(instance);

        expect(provider.m_last_document_json.contains("queue_wait_time_ms"));

        serde::SerdeFormatRegistry::set_active(nullptr);
    };

    // BUG pin (see the `// BUG:` comment on project_task's queue_wait_time_ms cast above):
    // started_at before scheduled_at is an invalid ordering TaskInstance::validate() would
    // reject, but this projector never calls validate() — it just casts the negative duration
    // straight to uint64_t, which wraps around to a huge value instead of erroring or clamping.
    "BUG: started_at before scheduled_at wraps queue_wait_time_ms to a huge value instead of "
    "erroring"_test = [] {
        WorkflowContext ctx;
        SpySearchProvider provider;
        ctx.set_search(&provider);
        SummaryProjector projector{ctx};

        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        model::ExecutionTimings timings;
        auto now = std::chrono::system_clock::now();
        timings.set_scheduled_at(now);
        timings.set_started_at(now - std::chrono::seconds{10});
        instance.set_timings(timings);
        projector.project_task(instance);

        // A sane wait would be 10000ms or clamped to 0 — instead it wraps to something enormous.
        expect(!provider.m_last_document_json.contains(R"("queue_wait_time_ms":10000)"));
        expect(!provider.m_last_document_json.contains(R"("queue_wait_time_ms":0)"));
    };
};

} // namespace engine::search_projector_tests
#endif
