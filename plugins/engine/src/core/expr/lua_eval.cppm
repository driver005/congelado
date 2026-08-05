module;
#include <lua.hpp>

export module engine:expr;

import std;
import interfaces;
import core_events;
import core_logger;

export namespace engine {

// Thin wrapper over a resolved Lua IBridge's native_handle() — the engine's condition evaluator
// for SWITCH edges, DO_WHILE loop conditions, and (Phase 5) EventHandler conditions, plus a
// value-returning evaluator backing LAMBDA/INLINE/JSON_JQ_TRANSFORM system tasks (Phase 3).
// Reuses lua_bridge's already-running lua_State rather than embedding a JS engine or a real jq
// parser: see the Conductor-parity plan's cross-cutting decision (b) for why Lua, not JS, is the
// right fit here — most of Conductor's own real-world condition usage is a plain field read
// anyway, so this only needs to cover the minority of genuinely-scripted cases.
class LuaEval {
  public:
    /**
     * @brief Binds this evaluator to a resolved Lua bridge.
     * @param bridge the resolved "lua" IBridge*, or nullptr if none was found — every eval
     * method degrades to a safe failure in that case rather than crashing.
     */
    explicit LuaEval(interfaces::IBridge *bridge) noexcept : m_bridge{bridge} {}

    /**
     * @brief Evaluates `expr` as a Lua boolean expression, with `bindings` exposed as global Lua
     * variables — flat string keys/values only, matching this model's own flat-map constraint
     * (see model::TaskInstance's input_data/output_data).
     * @warning Fails closed (returns false) on every error path: no bridge resolved, the bridge
     * has no native handle, a parse error, or a runtime error — a condition that can't be
     * evaluated is treated the same as one that evaluated to false, never silently treated as
     * true. Every failure path logs a warning so a broken expression doesn't fail silently.
     * @param expr the Lua expression source, e.g. `"variables.retries > 3"`.
     * @param bindings flat key/value pairs exposed as Lua globals before evaluating.
     * @return the expression's truthiness per Lua's own rules (only nil/false are falsy), or
     * false if it couldn't be evaluated at all.
     */
    [[nodiscard]] bool
    eval_condition(std::string_view expr,
                  const std::unordered_map<std::string, std::string> &bindings) const {
        auto *state = load_and_run(expr, bindings);
        if (state == nullptr) {
            return false;
        }
        bool result = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        return result;
    }

    /**
     * @brief Evaluates `expr` and stringifies whatever it returns — backs LAMBDA/INLINE (a
     * script computing a task's output) and JSON_JQ_TRANSFORM (approximated via Lua string/table
     * manipulation, not a real jq grammar — see the system_task.cppm docs for why).
     * @warning Same fail-closed contract as eval_condition(): returns std::nullopt on any error,
     * never a guessed/default value, and every failure path logs a warning.
     * @param expr the Lua expression source.
     * @param bindings flat key/value pairs exposed as Lua globals before evaluating.
     * @return the result stringified via Lua's own tostring semantics, or std::nullopt if it
     * couldn't be evaluated (including evaluating to nil).
     */
    [[nodiscard]] std::optional<std::string>
    eval_value(std::string_view expr,
              const std::unordered_map<std::string, std::string> &bindings) const {
        auto *state = load_and_run(expr, bindings);
        if (state == nullptr) {
            return std::nullopt;
        }
        if (lua_isnil(state, -1)) {
            lua_pop(state, 1);
            return std::nullopt;
        }
        std::size_t length = 0;
        const char *text = luaL_tolstring(state, -1, &length);
        std::string result{text, length};
        lua_pop(state, 2); // luaL_tolstring pushes its own string on top of the original value
        return result;
    }

  private:
    interfaces::IBridge *m_bridge;

    /// @brief Shared load/bind/call machinery for both eval methods above. On success, leaves
    /// exactly one result value on top of the Lua stack and returns the (non-null) lua_State for
    /// the caller to read it off of. On any failure, logs a warning, cleans up after itself, and
    /// returns nullptr.
    lua_State *load_and_run(std::string_view expr,
                            const std::unordered_map<std::string, std::string> &bindings) const {
        if (expr.empty()) {
            return nullptr;
        }
        if (m_bridge == nullptr) {
            core::logger::warning("engine", "lua_eval: no lua bridge resolved, expr '{}' skipped",
                                  expr);
            core::events::publish("engine.lua_eval.no_bridge", {{"expr", std::string{expr}}});
            return nullptr;
        }
        auto *state = static_cast<lua_State *>(m_bridge->native_handle());
        if (state == nullptr) {
            core::logger::warning("engine", "lua_eval: lua bridge has no native handle");
            core::events::publish("engine.lua_eval.no_native_handle");
            return nullptr;
        }

        // Bind every flat key/value pair as a Lua global before evaluating — simplest possible
        // binding surface, matches the flat-string constraint everywhere else in this model.
        for (auto const &[key, value] : bindings) {
            lua_pushlstring(state, value.data(), value.size());
            lua_setglobal(state, key.c_str());
        }

        auto script = std::format("return ({})", expr);
        if (luaL_loadstring(state, script.c_str()) != 0) {
            core::logger::warning("engine", "lua_eval: parse error in '{}': {}", expr,
                                  lua_tostring(state, -1));
            core::events::publish("engine.lua_eval.parse_error",
                                  {{"expr", std::string{expr}}, {"error", lua_tostring(state, -1)}});
            lua_pop(state, 1);
            return nullptr;
        }
        if (lua_pcall(state, 0, 1, 0) != 0) {
            core::logger::warning("engine", "lua_eval: runtime error in '{}': {}", expr,
                                  lua_tostring(state, -1));
            core::events::publish("engine.lua_eval.runtime_error",
                                  {{"expr", std::string{expr}}, {"error", lua_tostring(state, -1)}});
            lua_pop(state, 1);
            return nullptr;
        }
        return state;
    }
};

} // namespace engine
