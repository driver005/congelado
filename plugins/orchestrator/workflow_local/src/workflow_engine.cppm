export module workflow_engine;

export import :context;
export import :expr;
export import :system_task;
export import :schema;
export import :search_projector;
export import :orchestrator;

#ifdef CONGELADO_TEST
import std;
import model;
import connector;
import serde;
import boost.ut;

// Pure barrel module (no logic of its own — see the export-import list above), so there's
// nothing new to unit-test here. This smoke-tests the aggregation itself: every partition's
// exported type stays reachable through a plain `import workflow_engine;` rather than needing
// callers to import individual partitions by name. Each type's own real behavior is already
// covered by that partition's own test file (lua_eval.cppm, schema_validator.cppm,
// system_task.cppm, projector.cppm, orchestrator.cppm, workflow_context.cppm).
namespace engine::workflow_engine_barrel_tests {
using namespace boost::ut;

suite<"workflow_engine barrel re-exports"> workflow_engine_barrel_suite = [] {
    "every partition's public type is reachable through the aggregate module import"_test = [] {
        WorkflowContext ctx;
        connector::Connector local_connector;
        ctx.set_connector(&local_connector);

        LuaEval eval{nullptr};
        expect(!eval.eval_condition("true", serde::Value{serde::Value::Object{}}));

        SystemTaskExecutor executor{nullptr};
        auto outcome = executor.execute(model::TaskType::NOOP, {}, {});
        expect(outcome.output_data.empty());

        auto schema_result = SchemaValidator::validate("{}", serde::Value{serde::Value::Object{}});
        expect(bool(schema_result));

        SummaryProjector projector{ctx};
        model::TaskInstance instance;
        instance.set_task_id(model::generate_id());
        expect(nothrow([&] { projector.project_task(instance); }));

        Orchestrator orchestrator{ctx};
        expect(orchestrator.get_name() == "engine.sweep");
    };
};

} // namespace engine::workflow_engine_barrel_tests
#endif
