module;
#include <lua.hpp>
#ifdef CONGELADO_TEST
#    include "core/manager/abi.h"
#endif

export module workflow_engine:expr;

import std;
import serde;
import interfaces;
import core_events;
import core_logger;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace engine {

// Thin wrapper over a resolved Lua IBridge's native_handle() — the engine's condition evaluator
// for SWITCH edges, DO_WHILE loop conditions, and (Phase 5) EventHandler conditions, plus a
// value-returning evaluator backing LAMBDA/INLINE/JSON_JQ_TRANSFORM system tasks (Phase 3).
// Reuses lua_bridge's already-running lua_State rather than embedding a JS engine or a real jq
// parser: see the Conductor-parity plan's cross-cutting decision (b) for why Lua, not JS, is
// the right fit here — most of Conductor's own real-world condition usage is a plain field read
// anyway, so this only needs to cover the minority of genuinely-scripted cases.
class LuaEval
{
public:
    /**
     * @brief Binds this evaluator to a resolved Lua bridge.
     * @param bridge the resolved "lua" IBridge*, or nullptr if none was found — every eval
     * method degrades to a safe failure in that case rather than crashing.
     */
    explicit LuaEval(interfaces::IBridge* bridge) noexcept :
        m_bridge{bridge}
    {
    }

    /**
     * @brief Evaluates `expr` as a Lua boolean expression, with `bindings` exposed as global
     * Lua variables — keys of the object value, each bound as its native Lua type
     * (boolean/integer/ number/string/table/nil).
     * @warning Fails closed (returns false) on every error path: no bridge resolved, the bridge
     * has no native handle, a parse error, or a runtime error — a condition that can't be
     * evaluated is treated the same as one that evaluated to false, never silently treated as
     * true. Every failure path logs a warning so a broken expression doesn't fail silently.
     * @param expr the Lua expression source, e.g. `"variables.retries > 3"`.
     * @param bindings object value whose entries are exposed as Lua globals before evaluating.
     * @return the expression's truthiness per Lua's own rules (only nil/false are falsy), or
     * false if it couldn't be evaluated at all.
     */
    [[nodiscard]] bool eval_condition(std::string_view expr, const serde::Value& bindings) const
    {
        auto* state = load_and_run(expr, bindings);
        if (state == nullptr) {
            return false;
        }
        bool result = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        return result;
    }

    /**
     * @brief Evaluates `expr` and stringifies whatever it returns — backs LAMBDA/INLINE (a
     * script computing a task's output) and JSON_JQ_TRANSFORM (approximated via Lua
     * string/table manipulation, not a real jq grammar — see the system_task.cppm docs for
     * why).
     * @warning Same fail-closed contract as eval_condition(): returns std::nullopt on any
     * error, never a guessed/default value, and every failure path logs a warning.
     * @param expr the Lua expression source.
     * @param bindings object value whose entries are exposed as Lua globals before evaluating.
     * @return the result stringified via Lua's own tostring semantics, or std::nullopt if it
     * couldn't be evaluated (including evaluating to nil).
     */
    [[nodiscard]] std::optional<std::string>
    eval_value(std::string_view expr, const serde::Value& bindings) const
    {
        auto* state = load_and_run(expr, bindings);
        if (state == nullptr) {
            return std::nullopt;
        }
        if (lua_isnil(state, -1)) {
            lua_pop(state, 1);
            return std::nullopt;
        }
        std::size_t length = 0;
        const char* text = luaL_tolstring(state, -1, &length);
        std::string result{text, length};
        lua_pop(state, 2); // luaL_tolstring pushes its own string on top of the original value
        return result;
    }

private:
    interfaces::IBridge* m_bridge;

    /// @brief Shared load/bind/call machinery for both eval methods above. On success, leaves
    /// exactly one result value on top of the Lua stack and returns the (non-null) lua_State
    /// for the caller to read it off of. On any failure, logs a warning, cleans up after
    /// itself, and returns nullptr.
    lua_State* load_and_run(std::string_view expr, const serde::Value& bindings) const
    {
        if (expr.empty()) {
            return nullptr;
        }
        if (m_bridge == nullptr) {
            core::logger::warning(
                "engine", "lua_eval: no lua bridge resolved, expr '{}' skipped", expr
            );
            core::events::publish("engine.lua_eval.no_bridge", {{"expr", std::string{expr}}});
            return nullptr;
        }
        auto* state = static_cast<lua_State*>(m_bridge->native_handle());
        if (state == nullptr) {
            core::logger::warning("engine", "lua_eval: lua bridge has no native handle");
            core::events::publish("engine.lua_eval.no_native_handle");
            return nullptr;
        }

        // Push every entry of the object value as a Lua global before evaluating — each mapped
        // to its native Lua type via push_value, so nested structures work instead of the old
        // flat-string-only surface. A non-object (or absent) input simply binds no globals.
        if (auto object = bindings.to_object()) {
            for (const auto& [key, value]: *object) {
                push_value(state, value);
                lua_setglobal(state, key.c_str());
            }
        }

        // SECURITY: unsandboxed eval of attacker-supplied text — no expression allow-list, no
        // timeout/instruction-count limit. `expr` comes from WorkflowDef SWITCH/DO_WHILE edge
        // conditions and LAMBDA/INLINE/JSON_JQ_TRANSFORM task scripts, settable unauthenticated
        // via POST/PUT /api/v1/workflows (see routes.cppm's SECURITY note) and triggered via
        // POST /api/v1/workflows/:name/start. `state` is shared with lua_bridge's
        // native_handle() — if lua_bridge_plugin.cc's run_script() (which calls luaL_openlibs)
        // ever ran first on this same interpreter, the full Lua stdlib (os.execute, io.open,
        // os.remove) becomes reachable from here too, turning this into unauthenticated OS
        // command execution.
        auto script = std::format("return ({})", expr);
        if (luaL_loadstring(state, script.c_str()) != 0) {
            core::logger::warning(
                "engine", "lua_eval: parse error in '{}': {}", expr, lua_tostring(state, -1)
            );
            core::events::publish(
                "engine.lua_eval.parse_error",
                {{"expr", std::string{expr}}, {"error", lua_tostring(state, -1)}}
            );
            lua_pop(state, 1);
            return nullptr;
        }
        if (lua_pcall(state, 0, 1, 0) != 0) {
            core::logger::warning(
                "engine", "lua_eval: runtime error in '{}': {}", expr, lua_tostring(state, -1)
            );
            core::events::publish(
                "engine.lua_eval.runtime_error",
                {{"expr", std::string{expr}}, {"error", lua_tostring(state, -1)}}
            );
            lua_pop(state, 1);
            return nullptr;
        }
        return state;
    }

    /// @brief Pushes a dynamic `serde::Value` onto the Lua stack, mapping each category to its
    /// native Lua type: null → nil, bool → boolean, integer → integer, double → number, string
    /// → string, object/array → table (built recursively so nested values come along).
    static void push_value(lua_State* state, const serde::Value& value)
    {
        if (value.is_null()) {
            lua_pushnil(state);
        } else if (auto as_bool = value.to_bool()) {
            lua_pushboolean(state, *as_bool);
        } else if (auto as_int = value.to_int64()) {
            lua_pushinteger(state, *as_int);
        } else if (auto as_double = value.to_double()) {
            lua_pushnumber(state, *as_double);
        } else if (auto as_string = value.to_string()) {
            lua_pushlstring(state, as_string->data(), as_string->size());
        } else if (auto as_object = value.to_object()) {
            lua_newtable(state);
            for (const auto& [key, child]: *as_object) {
                push_value(state, child);
                lua_setfield(state, -2, key.c_str());
            }
        } else if (auto as_array = value.to_array()) {
            lua_createtable(state, static_cast<int>(as_array->size()), 0);
            int index = 1;
            for (const auto& child: *as_array) {
                push_value(state, child);
                lua_rawseti(state, -2, index);
                ++index;
            }
        } else {
            lua_pushnil(state);
        }
    }
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::lua_eval_tests {
using namespace boost::ut;

/// @brief Minimal `IBridge` test double modeling a resolved "lua" bridge whose
/// `native_handle()` hands back a real, bare `lua_State*` the test owns directly (via
/// `luaL_newstate()`, same bare-interpreter construction pattern lua_bridge_plugin.cc's own
/// tests use) — lets `LuaEval::load_and_run()` see a genuine native handle without standing up
/// a whole `LuaBridgePlugin`/`congelado::Plugin` instance.
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

/// @brief `IBridge` test double whose `native_handle()` stays null — models a resolved bridge
/// whose runtime never stood up a state (mirrors a fresh `LuaBridgePlugin` before `on_load()`).
class MockNoHandleBridge final : public interfaces::IBridge
{
public:
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
};

suite<"LuaEval fail-closed paths"> lua_eval_failclosed_suite = [] {
    "eval_condition returns false when no bridge was resolved (nullptr)"_test = [] {
        LuaEval eval{nullptr};
        expect(!eval.eval_condition("1 == 1", serde::Value{serde::Value::Object{}}));
    };

    "eval_value returns nullopt when no bridge was resolved (nullptr)"_test = [] {
        LuaEval eval{nullptr};
        expect(!eval.eval_value("1 + 1", serde::Value{serde::Value::Object{}}).has_value());
    };

    "eval_condition returns false when the bridge has no native handle"_test = [] {
        MockNoHandleBridge bridge;
        LuaEval eval{&bridge};
        expect(!eval.eval_condition("1 == 1", serde::Value{serde::Value::Object{}}));
    };

    "eval_condition returns false on an empty expression"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        LuaEval eval{&bridge};
        expect(!eval.eval_condition("", serde::Value{serde::Value::Object{}}));
    };

    "eval_condition returns false on a parse error, without crashing"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        LuaEval eval{&bridge};
        expect(!eval.eval_condition("this is not lua (((", serde::Value{serde::Value::Object{}}));
    };

    "eval_condition returns false on a runtime error, without crashing"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        LuaEval eval{&bridge};
        expect(!eval.eval_condition("nil + 1", serde::Value{serde::Value::Object{}}));
    };

    "eval_value returns nullopt when the expression evaluates to nil"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        LuaEval eval{&bridge};
        expect(!eval.eval_value("nil", serde::Value{serde::Value::Object{}}).has_value());
    };
};

suite<"LuaEval binds bindings as Lua globals"> lua_eval_bindings_suite = [] {
    "eval_condition exposes an object binding entry as a readable Lua global"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        LuaEval eval{&bridge};

        serde::Value::Object bindings{};
        bindings["retries"] = std::int64_t{4};
        bool result = eval.eval_condition("retries > 3", serde::Value{std::move(bindings)});
        expect(result);
    };

    "eval_value stringifies an arithmetic expression over a bound global"_test = [] {
        std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                             lua_close(raw);
                                         }};
        MockLuaBridge bridge{state.get()};
        LuaEval eval{&bridge};

        serde::Value::Object bindings{};
        bindings["base"] = std::int64_t{10};
        auto result = eval.eval_value("base + 5", serde::Value{std::move(bindings)});
        expect(result.has_value()) << fatal;
        expect(*result == "15");
    };
};

/// @brief Proves finding #4's SECURITY note: `load_and_run()` runs arbitrary Lua with no
/// allow-list, sandbox, or instruction/time limit. The only global installed here besides the
/// (empty) bindings is one hand-injected C function — no `luaL_openlibs()` call anywhere in
/// this test, so `os`/`io`/every stdlib table is unreachable — and `eval_condition` still
/// evaluates a full expression that (a) *calls* that function as a side effect and (b) combines
/// the call with ordinary arithmetic/comparison, then the mutation the C function made to an
/// unrelated Lua global (never passed in as a binding, invisible to the expression's own return
/// value) is read back through the raw `lua_State` afterward. A sandboxed/allow-listed
/// evaluator would either reject the function call outright or keep the expression from having
/// any observable side effect beyond its own result — neither restriction exists here, which is
/// exactly the "unauthenticated OS command execution" reachability the SECURITY comment above
/// load_and_run() describes (this test just proves the *mechanism*, without ever touching a
/// real destructive call like os.execute/io.open).
suite<"LuaEval::eval_condition is unsandboxed (finding #4)"> lua_eval_unsandboxed_suite = [] {
    "an injected C function's side-effect global mutation survives full expression evaluation, unrestricted by any sandbox"_test =
        [] {
            std::shared_ptr<lua_State> state{luaL_newstate(), [](lua_State* raw) {
                                                 lua_close(raw);
                                             }};
            // Deliberately bare: no luaL_openlibs() call, so os/io/every stdlib table stays
            // unreachable — the ONLY callable in this interpreter is the one function below.
            lua_pushcfunction(state.get(), [](lua_State* inner) -> int {
                lua_pushboolean(inner, 1);
                lua_setglobal(inner, "side_effect_flag");
                lua_pushboolean(inner, 1);
                return 1;
            });
            lua_setglobal(state.get(), "mark");

            MockLuaBridge bridge{state.get()};
            LuaEval eval{&bridge};

            bool result = eval.eval_condition(
                "mark() and (2 + 2 == 4)", serde::Value{serde::Value::Object{}}
            );
            expect(result);

            lua_getglobal(state.get(), "side_effect_flag");
            expect(lua_toboolean(state.get(), -1) != 0);
            lua_pop(state.get(), 1);
        };
};

} // namespace engine::lua_eval_tests
#endif
