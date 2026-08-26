module;

#include <congelado/abi.h>

export module workflow_local;

import std;
import interfaces;
import model;
import connector;
import core_contract;
import core_logger;
import workflow_engine;
#ifdef CONGELADO_TEST
import shared;
import serde;
import boost.ut;
#endif

export namespace workflow_orchestrator {

/// @brief The local (in-process) workflow-orchestrator backend — now the sole home of the DAG
/// brain (moved wholesale out of the engine). Wraps the moved `engine::Orchestrator` (the DAG
/// walker) + `engine::WorkflowContext` (connector/lua/search), translating the async string/map
/// `IWorkflowOrchestrator` interface into the orchestrator's model-based methods. Also owns the
/// background sweep contract that used to run engine-side.
class WorkflowLocal final : public interfaces::IWorkflowOrchestrator
{
public:
    /**
     * @brief Wires the backend from the host callbacks and registers the sweep contract. Called
     * once by the plugin's `on_load`. Takes opaque pointers so the plugin's entry `.cc` never
     * imports connector/contract types directly.
     * @param connector_ctx the shared `connector::Connector*`.
     * @param lua_ctx the resolved "lua" `interfaces::IBridge*` (or nullptr).
     * @param search_ctx the resolved `interfaces::ISearchProvider*` (or nullptr).
     * @param controller_ctx the host `core::contract::ContractGroup<>*` (for the sweep).
     * @param registry_ctx the host `core::contract::ContractRegistry*` (auto-release at
     * shutdown).
     */
    void on_load(
        void* connector_ctx,
        void* lua_ctx,
        void* search_ctx,
        void* controller_ctx,
        void* registry_ctx
    )
    {
        m_context.set_connector(static_cast<connector::Connector*>(connector_ctx));
        m_context.set_lua_bridge(static_cast<interfaces::IBridge*>(lua_ctx));
        m_context.set_search(static_cast<interfaces::ISearchProvider*>(search_ctx));

        auto* group = static_cast<core::contract::ContractGroup<>*>(controller_ctx);
        auto* registry = static_cast<core::contract::ContractRegistry*>(registry_ctx);
        if (group == nullptr || registry == nullptr) {
            core::logger::error(
                "workflow_orchestrator.local", "no contract group/registry — sweep not started"
            );
            return;
        }
        registry->add(m_orchestrator.create(*group, core::contract::ContractState::SCHEDULED));
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "local";
    }

    void start_workflow(
        std::string_view def_name,
        const std::unordered_map<std::string, std::string>& variables,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        m_orchestrator.start(
            std::string{def_name}, variables, std::nullopt,
            [callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                callback(
                    exec ? std::optional<std::string>{std::format("{}", exec->get_exec_id())}
                         : std::nullopt
                );
            }
        );
    }

    void on_task_terminal(
        std::string_view task_id, std::move_only_function<void(bool)> callback
    ) override
    {
        m_context.get_connector().find<model::TaskInstance>(
            std::string{task_id},
            [this,
             callback = std::move(callback)](std::optional<model::TaskInstance> found) mutable {
                if (!found) {
                    callback(false);
                    return;
                }
                m_orchestrator.on_task_terminal(std::move(*found));
                callback(true);
            }
        );
    }

    void on_execution_terminal(
        std::string_view exec_id, std::move_only_function<void(bool)> callback
    ) override
    {
        m_context.get_connector().find<model::WorkflowExecution>(
            std::string{exec_id},
            [this,
             callback = std::move(callback)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec) {
                    callback(false);
                    return;
                }
                m_orchestrator.on_execution_terminal(std::move(*exec));
                callback(true);
            }
        );
    }

    void pause(std::string_view exec_id, std::move_only_function<void(bool)> callback) override
    {
        m_orchestrator.pause(std::string{exec_id}, std::move(callback));
    }

    void resume(std::string_view exec_id, std::move_only_function<void(bool)> callback) override
    {
        m_orchestrator.resume(std::string{exec_id}, std::move(callback));
    }

    void retry(std::string_view exec_id, std::move_only_function<void(bool)> callback) override
    {
        m_orchestrator.retry(std::string{exec_id}, std::move(callback));
    }

    void restart(std::string_view exec_id, std::move_only_function<void(bool)> callback) override
    {
        m_orchestrator.restart(std::string{exec_id}, std::move(callback));
    }

    void terminate(std::string_view exec_id, std::move_only_function<void(bool)> callback) override
    {
        m_orchestrator.terminate(std::string{exec_id}, std::move(callback));
    }

    void reconcile(std::string_view exec_id, std::move_only_function<void(bool)> callback) override
    {
        m_orchestrator.reconcile(std::string{exec_id}, std::move(callback));
    }

    void rerun(
        std::string_view exec_id,
        std::string_view node_ref,
        const interfaces::Value& input,
        std::move_only_function<void(bool)> callback
    ) override
    {
        m_orchestrator.rerun(
            std::string{exec_id}, std::string{node_ref}, input, std::move(callback)
        );
    }

    void signal(
        std::string_view exec_id,
        std::string_view node_ref,
        std::optional<std::string_view> payload,
        std::move_only_function<void(bool)> callback
    ) override
    {
        auto payload_owned =
            payload ? std::optional<std::string>{std::string{*payload}} : std::nullopt;
        m_orchestrator.signal(
            std::string{exec_id}, std::string{node_ref}, std::move(payload_owned),
            std::move(callback)
        );
    }

    void complete_task(
        std::string_view exec_id,
        std::string_view node_ref,
        bool success,
        const std::unordered_map<std::string, std::string>& output,
        std::move_only_function<void(bool)> callback
    ) override
    {
        // BUG: Orchestrator::queue_update() is fire-and-forget (returns void, no completion
        // callback at all — see its own signature) and internally no-ops silently if `exec_id`
        // doesn't exist or `node_ref` doesn't match any instance. callback(true) fires
        // unconditionally right after, regardless of whether anything was actually
        // found/updated — so a caller of this IWorkflowOrchestrator::complete_task() (e.g.
        // `POST /api/v1/queue/update` via TaskHandler::queue_update()) gets a false-positive
        // success for a bogus exec_id/node_ref pair.
        m_orchestrator.queue_update(
            std::string{exec_id}, std::string{node_ref},
            success ? model::TaskStatus::COMPLETED : model::TaskStatus::FAILED, output
        );
        callback(true);
    }

    void start_server() override {}

    void shutdown_all() override {}

private:
    engine::WorkflowContext m_context;
    engine::Orchestrator m_orchestrator{m_context};
};

} // namespace workflow_orchestrator

#ifdef CONGELADO_TEST
namespace workflow_orchestrator::workflow_local_tests {
using namespace boost::ut;

/// @brief Trivial synchronous in-memory ICache — Connector aborts via active_cache() if none is
/// wired in, so every WorkflowLocal test needs one of these behind its connector_ctx.
class FakeCache final : public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cache";
    }

    void get(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }

    void set(
        std::string_view key, std::string_view value, shared::QueryReadFn&& result
    ) noexcept override
    {
        m_store[std::string{key}] = std::string{value};
        result("ok");
    }

    void remove(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        m_store.erase(std::string{key});
        result("ok");
    }

private:
    std::unordered_map<std::string, std::string> m_store;
};

/// @brief Builds a connector+cache pair and inserts one minimal single-node RUNNING-capable
/// WorkflowDef, ready to start() against.
class WorkflowLocalFixture
{
public:
    WorkflowLocalFixture()
    {
        m_connector.set_cache(&m_cache);
    }

    [[nodiscard]] connector::Connector& get_connector() noexcept
    {
        return m_connector;
    }

    void seed_def(std::string name)
    {
        model::TaskNode node;
        node.set_task_def_name("noop_task");
        node.set_ref_name("noop_task_1");
        model::WorkflowDef def;
        def.set_name(name);
        def.add_node(node);
        m_connector.insert<model::WorkflowDef>(def, [](bool) {});

        model::TaskDef task_def;
        task_def.set_name("noop_task");
        task_def.set_type(model::TaskType::NOOP);
        m_connector.insert<model::TaskDef>(task_def, [](bool) {});
    }

private:
    FakeCache m_cache;
    connector::Connector m_connector;
};

suite<"WorkflowLocal::on_load"> workflow_local_on_load_suite = [] {
    "wires the connector before checking group/registry — start_workflow works even with a "
    "null contract group/registry"_test = [] {
        WorkflowLocalFixture fixture;
        fixture.seed_def("order_pipeline");

        WorkflowLocal local;
        local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

        std::optional<std::string> result;
        local.start_workflow("order_pipeline", {}, [&result](std::optional<std::string> id) {
            result = std::move(id);
        });

        expect(result.has_value()) << fatal;
        expect(!result->empty());
    };
};

suite<"WorkflowLocal basic identity"> workflow_local_identity_suite = [] {
    "backend_name reports 'local'"_test = [] {
        WorkflowLocal local;
        expect(local.backend_name() == "local");
    };

    "start_server/shutdown_all are harmless no-ops"_test = [] {
        WorkflowLocal local;
        expect(nothrow([&] {
            local.start_server();
            local.shutdown_all();
        }));
    };
};

suite<"WorkflowLocal::start_workflow"> workflow_local_start_suite = [] {
    "a nonexistent def name reports std::nullopt"_test = [] {
        WorkflowLocalFixture fixture;
        WorkflowLocal local;
        local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

        std::optional<std::string> result{"unset"};
        local.start_workflow("missing_def", {}, [&result](std::optional<std::string> id) {
            result = std::move(id);
        });

        expect(!result.has_value());
    };
};

suite<"WorkflowLocal::on_task_terminal / on_execution_terminal"> workflow_local_terminal_suite =
    [] {
        "on_task_terminal reports false for an unknown task id"_test = [] {
            WorkflowLocalFixture fixture;
            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

            bool result = true;
            local.on_task_terminal(std::format("{}", model::generate_id()), [&result](bool ok) {
                result = ok;
            });
            expect(!result);
        };

        "on_task_terminal reports true and re-finds a real instance by id"_test = [] {
            WorkflowLocalFixture fixture;
            model::TaskInstance instance;
            instance.set_task_id(model::generate_id());
            instance.set_def_name("noop_task");
            instance.set_status(model::TaskStatus::COMPLETED);
            fixture.get_connector().insert<model::TaskInstance>(instance, [](bool) {});

            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

            bool result = false;
            local.on_task_terminal(std::format("{}", instance.get_task_id()), [&result](bool ok) {
                result = ok;
            });
            expect(result);
        };

        "on_execution_terminal reports false for an unknown exec id"_test = [] {
            WorkflowLocalFixture fixture;
            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

            bool result = true;
            local.on_execution_terminal(
                std::format("{}", model::generate_id()), [&result](bool ok) {
                    result = ok;
                }
            );
            expect(!result);
        };

        "on_execution_terminal reports true for a real execution"_test = [] {
            WorkflowLocalFixture fixture;
            model::WorkflowExecution exec;
            exec.set_exec_id(model::generate_id());
            exec.set_def_name("order_pipeline");
            exec.set_status(model::WorkflowStatus::COMPLETED);
            fixture.get_connector().insert<model::WorkflowExecution>(exec, [](bool) {});

            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

            bool result = false;
            local.on_execution_terminal(std::format("{}", exec.get_exec_id()), [&result](bool ok) {
                result = ok;
            });
            expect(result);
        };
    };

suite<"WorkflowLocal lifecycle wrappers forward to Orchestrator"> workflow_local_lifecycle_suite =
    [] {
        "pause/resume/retry/restart/reconcile all report false for an unknown exec id"_test = [] {
            WorkflowLocalFixture fixture;
            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);
            auto missing = std::format("{}", model::generate_id());

            bool pause_result = true, resume_result = true, retry_result = true,
                 restart_result = true, reconcile_result = true;
            local.pause(missing, [&](bool ok) {
                pause_result = ok;
            });
            local.resume(missing, [&](bool ok) {
                resume_result = ok;
            });
            local.retry(missing, [&](bool ok) {
                retry_result = ok;
            });
            local.restart(missing, [&](bool ok) {
                restart_result = ok;
            });
            local.reconcile(missing, [&](bool ok) {
                reconcile_result = ok;
            });

            expect(!pause_result);
            expect(!resume_result);
            expect(!retry_result);
            expect(!restart_result);
            expect(!reconcile_result);
        };

        "rerun reports false for an unknown exec id"_test = [] {
            WorkflowLocalFixture fixture;
            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

            bool result = true;
            local.rerun(
                std::format("{}", model::generate_id()), "some_ref",
                interfaces::Value{std::string{"x"}}, [&result](bool ok) {
                    result = ok;
                }
            );
            expect(!result);
        };

        "signal reports false when there's no IN_PROGRESS instance for that node_ref"_test = [] {
            WorkflowLocalFixture fixture;
            WorkflowLocal local;
            local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

            bool result = true;
            local.signal(
                std::format("{}", model::generate_id()), "some_ref", std::nullopt,
                [&result](bool ok) {
                    result = ok;
                }
            );
            expect(!result);
        };
    };

suite<"WorkflowLocal::complete_task"> workflow_local_complete_task_suite = [] {
    // BUG pin (see the `// BUG:` comment above complete_task()'s callback(true) call): the
    // callback fires true even for a wholly-nonexistent exec_id/node_ref pair, since
    // Orchestrator::queue_update() is fire-and-forget and never reports back whether anything
    // was actually found and updated.
    "BUG: reports true even for a bogus exec_id/node_ref that updates nothing"_test = [] {
        WorkflowLocalFixture fixture;
        WorkflowLocal local;
        local.on_load(&fixture.get_connector(), nullptr, nullptr, nullptr, nullptr);

        bool result = false;
        local.complete_task(
            std::format("{}", model::generate_id()), "no_such_node", true, {}, [&result](bool ok) {
                result = ok;
            }
        );

        expect(result);

        std::optional<model::WorkflowExecution> found{model::WorkflowExecution{}};
        fixture.get_connector().find<model::WorkflowExecution>(
            "does-not-exist", [&found](std::optional<model::WorkflowExecution> value) {
                found = std::move(value);
            }
        );
        expect(!found.has_value());
    };
};

} // namespace workflow_orchestrator::workflow_local_tests
#endif
