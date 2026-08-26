module;
#ifdef CONGELADO_TEST
#    include "core/manager/abi.h"

#    include <lua.hpp>
#endif

export module workflow_engine:system_task;

import std;
import serde;
import model;
import interfaces;
import core_events;
import core_logger;
import :expr;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace engine {

/// @brief What a system task produced: output data for its own TaskInstance, plus (TERMINATE
/// only) a request to end the owning WorkflowExecution outright.
struct SystemTaskOutcome
{
    std::unordered_map<std::string, std::string> output_data;
    bool terminate_workflow{false};
    model::WorkflowStatus terminate_status{model::WorkflowStatus::COMPLETED};
};

/// @brief Dispatch table for TaskTypes that execute in-process, synchronously, the instant an
/// instance becomes SCHEDULED — no external worker poll involved (only SIMPLE/HTTP-equivalent
/// types are ever meant to be polled by a real worker). Covers Conductor's TERMINATE/
/// SET_VARIABLE/NOOP/LAMBDA/INLINE/JSON_JQ_TRANSFORM system tasks; JOIN/FORK are also routed
/// here as pure passthrough markers — their real control-flow semantics live in Orchestrator's
/// join_on handling and static-edge fan-out, not in any per-instance work.
class SystemTaskExecutor
{
public:
    /// @param lua_bridge the resolved "lua" IBridge*, or nullptr — forwarded straight to
    /// LuaEval, which already degrades safely when there's no bridge.
    explicit SystemTaskExecutor(interfaces::IBridge* lua_bridge) noexcept :
        m_lua{lua_bridge}
    {
    }

    /// @brief Whether `type` is one this class handles in-process — the check Orchestrator
    /// makes before ever creating a SCHEDULED instance meant for external worker pickup.
    [[nodiscard]] static bool is_system_task(model::TaskType type) noexcept
    {
        switch (type) {
            case model::TaskType::TERMINATE:
            case model::TaskType::SET_VARIABLE:
            case model::TaskType::NOOP:
            case model::TaskType::JOIN:
            case model::TaskType::FORK:
            case model::TaskType::LAMBDA:
            case model::TaskType::INLINE:
            case model::TaskType::JSON_JQ_TRANSFORM:
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Executes a system-task instance synchronously.
     * @param type the resolved TaskDef's type — caller must have already checked
     * is_system_task(type), every other type falls through to a logged no-op.
     * @param input the instance's resolved input_data.
     * @param variables the owning execution's variables — bound alongside `input` for LAMBDA/
     * INLINE/JSON_JQ_TRANSFORM's Lua script.
     * @return the outcome: output_data for the instance, plus (TERMINATE only) a workflow-level
     * termination request the caller applies to the owning WorkflowExecution.
     */
    [[nodiscard]] SystemTaskOutcome execute(
        model::TaskType type,
        const std::unordered_map<std::string, std::string>& input,
        const std::unordered_map<std::string, std::string>& variables
    ) const
    {
        switch (type) {
            case model::TaskType::TERMINATE:
                return execute_terminate(input);
            case model::TaskType::SET_VARIABLE:
            case model::TaskType::NOOP:
            case model::TaskType::JOIN:
            case model::TaskType::FORK:
                // SET_VARIABLE's actual variable-merge happens in Orchestrator (it mutates the
                // owning WorkflowExecution, not just this instance's own output) — here it's
                // just a passthrough, same as NOOP/JOIN/FORK.
                return {.output_data = input};
            case model::TaskType::LAMBDA:
            case model::TaskType::INLINE:
            case model::TaskType::JSON_JQ_TRANSFORM:
                return execute_script(input, variables);
            default:
                core::logger::error(
                    "engine", "system_task: unhandled type dispatched to execute()"
                );
                core::events::publish("engine.system_task.unhandled_type");
                return {};
        }
    }

private:
    interfaces::IBridge* m_lua;

    /// @brief TERMINATE reads `status` (COMPLETED/FAILED/TIMED_OUT/TERMINATED, defaults to
    /// COMPLETED if absent or unrecognized) out of its input and requests the owning execution
    /// end with that status — every other input key becomes part of the instance's own
    /// output_data untouched.
    static SystemTaskOutcome
    execute_terminate(const std::unordered_map<std::string, std::string>& input)
    {
        SystemTaskOutcome outcome;
        outcome.terminate_workflow = true;
        outcome.terminate_status = model::WorkflowStatus::COMPLETED;
        for (const auto& [key, value]: input) {
            if (key == "status") {
                if (value == "FAILED") {
                    outcome.terminate_status = model::WorkflowStatus::FAILED;
                } else if (value == "TIMED_OUT") {
                    outcome.terminate_status = model::WorkflowStatus::TIMED_OUT;
                } else if (value == "TERMINATED") {
                    outcome.terminate_status = model::WorkflowStatus::TERMINATED;
                }
                continue;
            }
            outcome.output_data[key] = value;
        }
        return outcome;
    }

    /// @brief LAMBDA/INLINE/JSON_JQ_TRANSFORM all share one shape: run a Lua expression (read
    /// from the `script` input key) with every other input key plus every workflow variable
    /// bound as Lua globals, and store its stringified result under the output key `result`.
    /// @warning JSON_JQ_TRANSFORM is explicitly NOT real jq — it's approximated by letting the
    /// script do its own string/table manipulation in Lua. A genuine jq-grammar implementation
    /// is optional backlog, consistent with how the Conductor-parity plan itself deprioritizes
    /// full JQ compliance against this feature's actual cost/benefit.
    SystemTaskOutcome execute_script(
        const std::unordered_map<std::string, std::string>& input,
        const std::unordered_map<std::string, std::string>& variables
    ) const
    {
        SystemTaskOutcome outcome;
        auto script_it = input.find("script");
        if (script_it == input.end()) {
            core::logger::warning(
                "engine", "system_task: LAMBDA/INLINE/JSON_JQ_TRANSFORM instance has no "
                          "'script' input key"
            );
            core::events::publish("engine.system_task.missing_script_input");
            return outcome;
        }
        auto bindings = serde::Value::Object{};
        for (const auto& [key, value]: variables) {
            bindings[key] = value;
        }
        for (const auto& [key, value]: input) {
            if (key != "script") {
                bindings[key] = value;
            }
        }
        auto result =
            LuaEval{m_lua}.eval_value(script_it->second, serde::Value{std::move(bindings)});
        if (result) {
            outcome.output_data["result"] = *result;
        }
        return outcome;
    }
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::system_task_tests {
using namespace boost::ut;

/// @brief Minimal IBridge test double whose native_handle() hands back a real bare lua_State*
/// the test owns — same construction shape as lua_eval.cppm's own MockLuaBridge, duplicated
/// here (rather than shared) since test scaffolding stays private per-file per this codebase's
/// per-partition test-namespace convention.
class MockLuaBridge final : public interfaces::IBridge
{
public:
    explicit MockLuaBridge(lua_State* state) noexcept :
        m_state{state}
    {
    }

    [[nodiscard]] CongeladoAny from_native(void* /*native_obj*/) override
    {
        return CongeladoAny{};
    }

    void* to_native(const CongeladoAny& /*value*/) override
    {
        return nullptr;
    }

    void install_method(
        std::unique_ptr<FnContext> /*ctx*/, const std::string& /*lang_name*/
    ) override
    {
    }

    [[nodiscard]] std::string_view runtime_name() const noexcept override
    {
        return "mock_lua";
    }

    [[nodiscard]] std::string_view script_extension() const noexcept override
    {
        return ".lua";
    }

    [[nodiscard]] int run_script(std::string_view /*path*/) override
    {
        return 0;
    }

    [[nodiscard]] void* native_handle() noexcept override
    {
        return m_state;
    }

private:
    lua_State* m_state;
};

suite<"SystemTaskExecutor::is_system_task"> is_system_task_suite = [] {
    "TERMINATE/SET_VARIABLE/NOOP/JOIN/FORK/LAMBDA/INLINE/JSON_JQ_TRANSFORM are system tasks"_test =
        [] {
            expect(SystemTaskExecutor::is_system_task(model::TaskType::TERMINATE));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::SET_VARIABLE));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::NOOP));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::JOIN));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::FORK));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::LAMBDA));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::INLINE));
            expect(SystemTaskExecutor::is_system_task(model::TaskType::JSON_JQ_TRANSFORM));
        };

    "every worker-dispatched or control-flow-only type is NOT a system task"_test = [] {
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::SIMPLE));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::SWITCH));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::SUB_WORKFLOW));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::DYNAMIC));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::DO_WHILE));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::FORK_JOIN_DYNAMIC));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::START_WORKFLOW));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::WAIT));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::HUMAN));
        expect(!SystemTaskExecutor::is_system_task(model::TaskType::EVENT));
    };
};

suite<"SystemTaskExecutor::execute TERMINATE"> execute_terminate_suite = [] {
    "no 'status' key defaults to terminate_status COMPLETED"_test = [] {
        SystemTaskExecutor executor{nullptr};
        auto outcome = executor.execute(model::TaskType::TERMINATE, {{"reason", "done"}}, {});

        expect(outcome.terminate_workflow);
        expect(outcome.terminate_status == model::WorkflowStatus::COMPLETED);
        expect(outcome.output_data.at("reason") == "done");
    };

    "status=FAILED/TIMED_OUT/TERMINATED map onto their matching WorkflowStatus"_test = [] {
        SystemTaskExecutor executor{nullptr};

        auto failed = executor.execute(model::TaskType::TERMINATE, {{"status", "FAILED"}}, {});
        expect(failed.terminate_status == model::WorkflowStatus::FAILED);

        auto timed_out =
            executor.execute(model::TaskType::TERMINATE, {{"status", "TIMED_OUT"}}, {});
        expect(timed_out.terminate_status == model::WorkflowStatus::TIMED_OUT);

        auto terminated =
            executor.execute(model::TaskType::TERMINATE, {{"status", "TERMINATED"}}, {});
        expect(terminated.terminate_status == model::WorkflowStatus::TERMINATED);
    };

    "an unrecognized status value falls back to COMPLETED rather than erroring"_test = [] {
        SystemTaskExecutor executor{nullptr};
        auto outcome =
            executor.execute(model::TaskType::TERMINATE, {{"status", "NOT_A_REAL_STATUS"}}, {});

        expect(outcome.terminate_status == model::WorkflowStatus::COMPLETED);
    };

    "the 'status' key itself never leaks into output_data"_test = [] {
        SystemTaskExecutor executor{nullptr};
        auto outcome = executor.execute(model::TaskType::TERMINATE, {{"status", "FAILED"}}, {});

        expect(outcome.output_data.find("status") == outcome.output_data.end());
    };
};

suite<"SystemTaskExecutor::execute passthrough types"> execute_passthrough_suite = [] {
    "SET_VARIABLE/NOOP/JOIN/FORK all echo input straight into output_data"_test = [] {
        SystemTaskExecutor executor{nullptr};
        std::unordered_map<std::string, std::string> input{{"a", "1"}, {"b", "2"}};

        for (auto type:
             {model::TaskType::SET_VARIABLE, model::TaskType::NOOP, model::TaskType::JOIN,
              model::TaskType::FORK}) {
            auto outcome = executor.execute(type, input, {});
            // boost::ut's printer can't format std::unordered_map for a failure message —
            // compare via bool{} to sidestep the printer entirely (same class of issue as the
            // std::byte-container comparison gotcha elsewhere in this codebase).
            expect(bool{outcome.output_data == input});
            expect(!outcome.terminate_workflow);
        }
    };
};

suite<"SystemTaskExecutor::execute LAMBDA/INLINE/JSON_JQ_TRANSFORM"> execute_script_suite = [] {
    "no 'script' input key produces empty output_data, logged not crashed"_test = [] {
        SystemTaskExecutor executor{nullptr};
        auto outcome = executor.execute(model::TaskType::LAMBDA, {{"other", "x"}}, {});

        expect(outcome.output_data.empty());
        expect(!outcome.terminate_workflow);
    };

    "a nullptr lua bridge fails closed even when a 'script' key is present"_test = [] {
        SystemTaskExecutor executor{nullptr};
        auto outcome = executor.execute(model::TaskType::LAMBDA, {{"script", "1 + 1"}}, {});

        expect(outcome.output_data.empty());
    };

    "with a real bridge, the script's result is stringified under the 'result' key, with input "
    "and variables both bound as globals"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        SystemTaskExecutor executor{&bridge};

        // LuaEval::load_and_run wraps expr as `return (expr)` itself — a leading "return " here
        // would double up into invalid Lua and fail closed (see lua_eval.cppm). Concatenation,
        // not arithmetic: execute_script's input/variables are always string-typed, and Lua 5.4
        // only coerces strings for `+` via the string library's metatable (luaL_openlibs, never
        // called on this bare state) — `..` concatenates the already-string operands directly,
        // no stdlib needed.
        auto outcome = executor.execute(
            model::TaskType::LAMBDA, {{"script", "base .. offset"}, {"offset", "5"}},
            {{"base", "10"}}
        );

        expect(outcome.output_data.at("result") == "105");
    };

    "the 'script' input key itself is excluded from the bindings — referencing it in the "
    "script "
    "sees nil, not the script source"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        SystemTaskExecutor executor{&bridge};

        auto outcome = executor.execute(model::TaskType::LAMBDA, {{"script", "script"}}, {});

        // `script` reads back nil (unbound), so eval_value() returns nullopt, so no "result"
        // key.
        expect(outcome.output_data.find("result") == outcome.output_data.end());
    };

    "JSON_JQ_TRANSFORM and INLINE share the exact same script-execution path as LAMBDA"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        SystemTaskExecutor executor{&bridge};

        auto inline_outcome = executor.execute(model::TaskType::INLINE, {{"script", "'hi'"}}, {});
        auto jq_outcome =
            executor.execute(model::TaskType::JSON_JQ_TRANSFORM, {{"script", "'hi'"}}, {});

        expect(inline_outcome.output_data.at("result") == "hi");
        expect(jq_outcome.output_data.at("result") == "hi");
    };
};

suite<"SystemTaskExecutor::execute unhandled type"> execute_default_suite = [] {
    "a type outside is_system_task()'s own set falls through to a logged empty outcome"_test = [] {
        SystemTaskExecutor executor{nullptr};
        auto outcome = executor.execute(model::TaskType::SIMPLE, {{"x", "y"}}, {});

        expect(outcome.output_data.empty());
        expect(!outcome.terminate_workflow);
        expect(outcome.terminate_status == model::WorkflowStatus::COMPLETED);
    };
};

} // namespace engine::system_task_tests
#endif
