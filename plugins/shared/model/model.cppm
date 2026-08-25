export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
export import :workflow_dag;
export import :workflow_def;
export import :workflow_exec;
export import :workflow_event;
export import :event_handler;
export import :schedule_def;
export import :poll_data;
export import :search_summary;
export import :auth_user;
export import serde;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

/// @brief Every persisted model type, in baseline/migration creation order — single source of
/// truth for both the engine's baseline migration and the schema-diff migration generator, so
/// they can never drift apart from each other.
using AllModels = std::tuple<WorkflowDef, TaskDef, WorkflowExecution, TaskInstance, WorkflowEvent,
                             EventHandler, WorkflowSchedule, PollData>;

} // namespace model

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"AllModels"> model_all_models_suite = [] {
    "lists every persisted model type exactly once"_test = [] {
        expect(std::tuple_size_v<model::AllModels> == 8);
    };
};

} // namespace model::tests
#endif
