export module engine:system_task;

import std;
import model;
import interfaces;
import core_events;
import core_logger;
import :expr;

export namespace engine {

/// @brief What a system task produced: output data for its own TaskInstance, plus (TERMINATE
/// only) a request to end the owning WorkflowExecution outright.
struct SystemTaskOutcome {
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
class SystemTaskExecutor {
  public:
    /// @param lua_bridge the resolved "lua" IBridge*, or nullptr — forwarded straight to LuaEval,
    /// which already degrades safely when there's no bridge.
    explicit SystemTaskExecutor(interfaces::IBridge *lua_bridge) noexcept : m_lua{lua_bridge} {}

    /// @brief Whether `type` is one this class handles in-process — the check Orchestrator makes
    /// before ever creating a SCHEDULED instance meant for external worker pickup.
    [[nodiscard]] static bool is_system_task(model::TaskType type) noexcept {
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
    [[nodiscard]] SystemTaskOutcome
    execute(model::TaskType type, const std::unordered_map<std::string, std::string> &input,
           const std::unordered_map<std::string, std::string> &variables) const {
        switch (type) {
        case model::TaskType::TERMINATE:
            return execute_terminate(input);
        case model::TaskType::SET_VARIABLE:
        case model::TaskType::NOOP:
        case model::TaskType::JOIN:
        case model::TaskType::FORK:
            // SET_VARIABLE's actual variable-merge happens in Orchestrator (it mutates the
            // owning WorkflowExecution, not just this instance's own output) — here it's just a
            // passthrough, same as NOOP/JOIN/FORK.
            return {.output_data = input};
        case model::TaskType::LAMBDA:
        case model::TaskType::INLINE:
        case model::TaskType::JSON_JQ_TRANSFORM:
            return execute_script(input, variables);
        default:
            core::logger::error("engine", "system_task: unhandled type dispatched to execute()");
            core::events::publish("engine.system_task.unhandled_type");
            return {};
        }
    }

  private:
    interfaces::IBridge *m_lua;

    /// @brief TERMINATE reads `status` (COMPLETED/FAILED/TIMED_OUT/TERMINATED, defaults to
    /// COMPLETED if absent or unrecognized) out of its input and requests the owning execution
    /// end with that status — every other input key becomes part of the instance's own
    /// output_data untouched.
    static SystemTaskOutcome
    execute_terminate(const std::unordered_map<std::string, std::string> &input) {
        SystemTaskOutcome outcome;
        outcome.terminate_workflow = true;
        outcome.terminate_status = model::WorkflowStatus::COMPLETED;
        for (auto const &[key, value] : input) {
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
    SystemTaskOutcome
    execute_script(const std::unordered_map<std::string, std::string> &input,
                  const std::unordered_map<std::string, std::string> &variables) const {
        SystemTaskOutcome outcome;
        auto script_it = input.find("script");
        if (script_it == input.end()) {
            core::logger::warning(
                "engine",
                "system_task: LAMBDA/INLINE/JSON_JQ_TRANSFORM instance has no 'script' input key");
            core::events::publish("engine.system_task.missing_script_input");
            return outcome;
        }
        auto bindings = variables;
        for (auto const &[key, value] : input) {
            if (key != "script") {
                bindings[key] = value;
            }
        }
        auto result = LuaEval{m_lua}.eval_value(script_it->second, bindings);
        if (result) {
            outcome.output_data["result"] = *result;
        }
        return outcome;
    }
};

} // namespace engine
