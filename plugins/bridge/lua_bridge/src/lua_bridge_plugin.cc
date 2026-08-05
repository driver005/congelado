// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <lua.hpp>

export module lua_bridge_plugin;

import congelado_plugin;
import core_plugin;
import interfaces;
import core_events;
import core_logger;
import std;

using InvokeFn = std::function<core::plugin::Value(std::span<const core::plugin::Value>)>;

namespace {

// The Lua FFI bridge as a genuine plugin — relocated verbatim from the old always-linked
// include/core/manager/bridge.cppm (see plugins/python_bridge for the matching PythonBridge
// move and why). Owns its own HandleTable, same reasoning as PythonBridgePlugin.
class LuaBridgePlugin : public congelado::Plugin, public interfaces::IBridge {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "lua_bridge"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_BRIDGE;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IBridge` surface.
     * @return this instance, upcast to `interfaces::IBridge*`.
     */
    void *bridge_get() noexcept { return static_cast<interfaces::IBridge *>(this); }

    /// @brief The runtime this bridge implements. @return `"lua"`.
    [[nodiscard]] std::string_view runtime_name() const noexcept override { return "lua"; }

    /// @brief The script file extension this bridge runs. @return `".lua"`.
    [[nodiscard]] std::string_view script_extension() const noexcept override { return ".lua"; }

    /**
     * @brief Runs a Lua script file against this bridge's own state — the one `on_load()`
     * bound methods into, not a fresh one, so the script actually sees them.
     * @param path path to the `.lua` file to run.
     * @return `0` on success, `1` if no state is up or the script errored (message printed to
     * stderr in the latter case).
     */
    [[nodiscard]] int run_script(std::string_view path) override {
        if (!m_state) {
            core::logger::warning("lua_bridge", "run_script: no Lua state up (on_load failed?)");
            core::events::publish("lua_bridge.no_state");
            return 1;
        }
        std::string owned_path{path};
        luaL_openlibs(m_state.get());
        if (luaL_dofile(m_state.get(), owned_path.c_str()) != 0) {
            core::logger::warning("lua_bridge", "run_script '{}' failed: {}", owned_path,
                                  lua_tostring(m_state.get(), -1));
            core::events::publish("lua_bridge.script_failed",
                                  {{"path", owned_path}, {"error", lua_tostring(m_state.get(), -1)}});
            std::println(stderr, "lua error: {}", lua_tostring(m_state.get(), -1));
            return 1;
        }
        core::logger::debug("lua_bridge", "ran '{}' successfully", owned_path);
        return 0;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's raw `lua_State*` across the
     * ABI (`congelado_call(BRIDGE, GET_NATIVE_HANDLE, ...)`) — a script runner needs this exact
     * state, not a fresh one, to see the bound table.
     * @return the Lua state as `void*`, or `nullptr` if `on_load` never got a state up.
     */
    void *bridge_native_handle() noexcept { return m_state.get(); }

    /**
     * @brief `interfaces::IBridge::native_handle()` override — the direct C++ virtual path
     * `FfiRuntime::get_bridge_native_handle()` actually calls once it already holds a live
     * `IBridge*` (resolved via `congelado_call(BRIDGE, GET, ...)`, not `GET_NATIVE_HANDLE`).
     * Without this override, `IBridge`'s `nullptr` default would win and every caller through
     * `FfiRuntime` would see no Lua state at all, even though one exists.
     * @return the Lua state as `void*`, same as `bridge_native_handle()`.
     */
    [[nodiscard]] void *native_handle() noexcept override { return m_state.get(); }

    /**
     * @brief Creates a fresh Lua state and finds-or-creates a named global table to bind
     * methods onto — `table_name` config key, default `"congelado"`.
     * @param cfg this plugin's config view.
     */
    void on_load(CongeladoHostCallbacks const & /*host*/, CongeladoConfigView const &cfg) override {
        auto table_name =
            std::string{congelado::config_get(cfg, "table_name").value_or("congelado")};

        auto state = make_lua_state();
        if (!state) {
            core::logger::warning("lua_bridge", "failed to create Lua state");
            core::events::publish("lua_bridge.state_create_failed");
            return;
        }

        lua_getglobal(state.get(), table_name.c_str());
        if (!lua_istable(state.get(), -1)) {
            lua_pop(state.get(), 1);
            lua_newtable(state.get());
            lua_setglobal(state.get(), table_name.c_str());
            lua_getglobal(state.get(), table_name.c_str());
        }

        m_table_index = lua_gettop(state.get());
        m_state = std::move(state);
        core::logger::debug("lua_bridge", "interpreter ready, table '{}'", table_name);
    }

    [[nodiscard]] CongeladoAny from_native(void *native_obj) override {
        auto idx = static_cast<int>(reinterpret_cast<std::intptr_t>(native_obj));  // FIXME(clang-tidy): reinterpret_cast usage — smuggling a Lua stack index through void* across the generic IBridge interface
        return from_lua(idx);
    }

    void *to_native(const CongeladoAny &any) override {
        to_lua(any);
        return nullptr;
    }

    void install_method(std::unique_ptr<FnContext> fn_context,
                        const std::string &lang_name) override {
        core::logger::debug("lua_bridge", "installing method '{}'", lang_name);
        auto *fn_context_ptr = fn_context.get();
        auto *bridge_ptr = this;

        m_fn_contexts.push_back(std::move(fn_context));

        lua_pushlightuserdata(m_state.get(), fn_context_ptr);
        lua_pushlightuserdata(m_state.get(), bridge_ptr);

        lua_pushcclosure(
            m_state.get(),
            [](lua_State *state) -> int {
                auto *fn_context =
                    static_cast<FnContext *>(lua_touserdata(state, lua_upvalueindex(1)));
                auto *bridge =
                    static_cast<LuaBridgePlugin *>(lua_touserdata(state, lua_upvalueindex(2)));

                try {
                    int arg_count = lua_gettop(state);

                    auto args =
                        std::views::iota(0, arg_count) | std::views::transform([&](int idx) {
                            return core::plugin::AnyConverter::from_any(bridge->from_lua(idx + 1));
                        }) |
                        std::ranges::to<std::vector>();

                    auto result = std::any_cast<const InvokeFn &>(fn_context->m_invoke)(args);

                    return bridge->to_lua(core::plugin::AnyConverter::to_any(result));
                } catch (const std::exception &e) {
                    core::events::publish("lua_bridge.invoke_exception", {{"error", e.what()}});
                    return luaL_error(state, "%s", e.what());  // FIXME(clang-tidy): cppcoreguidelines-pro-type-vararg — luaL_error is Lua's own printf-style C API function, inherently vararg
                }
            },
            2);

        lua_setfield(m_state.get(), m_table_index, lang_name.c_str());
    }

    [[nodiscard]] CongeladoAny from_lua(int stack_index) {
        CongeladoAny any{};

        switch (lua_type(m_state.get(), stack_index)) {
        case LUA_TNIL: {
            any.type_index = CG_NONE;
            return any;
        }
        case LUA_TBOOLEAN: {
            any.type_index = CG_BOOL;
            any.v_int64 = lua_toboolean(m_state.get(), stack_index);
            return any;
        }
        case LUA_TNUMBER: {
            if (lua_isinteger(m_state.get(), stack_index) != 0) {
                any.type_index = CG_INT;
                any.v_int64 = lua_tointeger(m_state.get(), stack_index);
            } else {
                any.type_index = CG_FLOAT;
                any.v_float64 = lua_tonumber(m_state.get(), stack_index);
            }
            return any;
        }
        case LUA_TSTRING: {
            any.type_index = CG_STR;
            any.v_cstr = lua_tostring(m_state.get(), stack_index);
            return any;
        }
        case LUA_TTABLE: {
            auto handle = m_handles.map_create();

            lua_pushnil(m_state.get());
            int adjusted_index = stack_index < 0 ? stack_index - 1 : stack_index;

            while (lua_next(m_state.get(), adjusted_index) != 0) {
                if (lua_type(m_state.get(), -2) == LUA_TSTRING) {
                    const auto *lua_key = lua_tostring(m_state.get(), -2);
                    auto dict_value = from_lua(-1);
                    m_handles.map_set(handle.v_int64, lua_key, dict_value);
                }
                lua_pop(m_state.get(), 1);
            }

            any = handle;
            return any;
        }
        default: {
            any.type_index = CG_PTR;
            any.v_ptr = lua_touserdata(m_state.get(), stack_index);
            return any;
        }
        }
    }

    int to_lua(const CongeladoAny &any) {
        switch (any.type_index) {
        case CG_NONE: {
            lua_pushnil(m_state.get());
            return 1;
        }
        case CG_BOOL: {
            lua_pushboolean(m_state.get(), static_cast<int>(any.v_int64));
            return 1;
        }
        case CG_INT: {
            lua_pushinteger(m_state.get(), static_cast<lua_Integer>(any.v_int64));
            return 1;
        }
        case CG_FLOAT: {
            lua_pushnumber(m_state.get(), static_cast<lua_Number>(any.v_float64));
            return 1;
        }
        case CG_STR: {
            lua_pushstring(m_state.get(), (any.v_cstr != nullptr) ? any.v_cstr : "");
            return 1;
        }
        case CG_MAP_HANDLE: {
            lua_newtable(m_state.get());

            int64_t map_size = m_handles.get_map_size(any.v_int64).v_int64;
            int64_t keys_handle = m_handles.get_map_keys(any.v_int64).v_int64;

            for (int64_t idx = 0; idx < map_size; ++idx) {
                CongeladoAny key = m_handles.array_get(keys_handle, idx);
                CongeladoAny map_value = m_handles.map_get(any.v_int64, key.v_cstr);

                to_lua(map_value);
                lua_setfield(m_state.get(), -2, key.v_cstr);
            }

            m_handles.handle_free(keys_handle);
            return 1;
        }
        default: {
            lua_pushlightuserdata(m_state.get(), any.v_ptr);
            return 1;
        }
        }
    }

  private:
    /**
     * @brief Creates a fresh, owning Lua state — relocated from the old always-linked
     * `core::plugin::types::make_lua_state()` (core_plugin doesn't include `<lua.hpp>` any
     * more, only this plugin does now).
     * @return a new Lua state, closed automatically via `lua_close` when the last reference
     * drops.
     */
    [[nodiscard]] static std::shared_ptr<lua_State> make_lua_state() {
        return std::shared_ptr<lua_State>{luaL_newstate(), [](lua_State *state) { lua_close(state); }};
    }

    core::plugin::HandleTable m_handles;
    std::shared_ptr<lua_State> m_state;
    int m_table_index{0};
    std::vector<std::unique_ptr<FnContext>> m_fn_contexts;
};

} // namespace

CONGELADO_PLUGIN(LuaBridgePlugin);
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
