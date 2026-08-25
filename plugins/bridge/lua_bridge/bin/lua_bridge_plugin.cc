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
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
            // lua_next() pushes a key+value pair each iteration on top of our own pushnil()
            // below, and none of that is covered by Lua's automatic per-call stack reservation
            // once we're several levels into this function's own C++-side recursion (each
            // recursive from_lua() call reuses the same underlying lua_State stack without
            // requesting more room) — nested tables silently overflowed the VM's internal value
            // stack and corrupted it, confirmed via a real crash inside lua_next()/luaH_next
            // around 5-6 levels of nesting. Request enough headroom for our own pushnil() plus
            // lua_next()'s key+value pair before touching the stack, and fail cleanly instead of
            // corrupting memory if a table is nested deep enough to exhaust it.
            if (lua_checkstack(m_state.get(), 3) == 0) {
                throw std::runtime_error(
                    "from_lua: table nested too deeply, out of Lua stack space");
            }

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
            // Same missing-headroom class of bug as from_lua()'s LUA_TTABLE case: lua_newtable()
            // plus a recursive to_lua() call per entry (which itself may push a nested table)
            // pushes onto the shared Lua stack with no lua_checkstack() to ensure room —
            // deeply-nested CongeladoAny structures could overflow/corrupt the VM stack the same
            // way. Request headroom before pushing.
            if (lua_checkstack(m_state.get(), 2) == 0) {
                throw std::runtime_error(
                    "to_lua: table nested too deeply, out of Lua stack space");
            }

            lua_newtable(m_state.get());

            int64_t map_size = m_handles.get_map_size(any.v_int64).v_int64;
            int64_t keys_handle = m_handles.get_map_keys(any.v_int64).v_int64;

            for (int64_t idx = 0; idx < map_size; ++idx) {
                CongeladoAny key = m_handles.array_get(keys_handle, idx);
                CongeladoAny map_value = m_handles.map_get(any.v_int64, key.v_cstr);

                to_lua(map_value);
                lua_setfield(m_state.get(), -2, key.v_cstr);

                // map_get() hands back a fresh aliasing handle for nested Map/Array values
                // (see value.cppm) — free it now that to_lua() is done reading through it,
                // same as keys_handle below.
                if (map_value.type_index == CG_MAP_HANDLE || map_value.type_index == CG_ARRAY_HANDLE) {
                    m_handles.handle_free(map_value.v_int64);
                }
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

#ifdef CONGELADO_TEST
namespace lua_bridge_plugin_tests {
using namespace boost::ut;

/// @brief Small test-only helper class — keeps the "class-only, no free functions" convention
/// even for test scaffolding. `LuaBridgePlugin`'s `congelado::Plugin` base has its copy/move
/// ctors deleted, so a helper can't construct-and-return a loaded instance by value; instead
/// this loads an already-constructed instance in place via an out-param.
class LuaBridgeTestHelper {
  public:
    LuaBridgeTestHelper() = delete;

    /// @brief Runs `on_load()` against `plugin` with an empty host/config view — exercises the
    /// default `table_name` ("congelado") path, a fresh self-contained Lua state, no external
    /// script files or host wiring needed.
    /// @param plugin the plugin to load in place.
    static void load(LuaBridgePlugin &plugin) {
        plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{});
    }
};

// NOTE on coverage gaps, both deliberate:
// - run_script()'s success path (a real luaL_dofile() that actually parses/runs) is skipped —
//   it needs a real .lua file on disk, which would make this suite depend on external script
//   fixtures rather than staying self-contained. Its two failure branches ("no state up" and
//   "file not found") are both covered below without touching the filesystem for anything but a
//   guaranteed-nonexistent path.
suite<"LuaBridgePlugin"> lua_bridge_plugin_suite = [] {
    "get_name reports 'lua_bridge'"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.get_name() == "lua_bridge");
    };

    "get_version reports a non-empty version string"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_BRIDGE"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_BRIDGE);
    };

    "bridge_get returns this instance upcast to IBridge*"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.bridge_get() == static_cast<interfaces::IBridge *>(&plugin));
    };

    "runtime_name reports 'lua'"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.runtime_name() == "lua");
    };

    "script_extension reports '.lua'"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.script_extension() == ".lua");
    };

    "bridge_native_handle is null before on_load ever runs"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.bridge_native_handle() == nullptr);
    };

    "native_handle is null before on_load ever runs"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.native_handle() == nullptr);
    };

    "run_script with no state up reports failure (1)"_test = [] {
        LuaBridgePlugin plugin;
        expect(plugin.run_script("whatever.lua") == 1);
    };

    "on_load stands up a live Lua state, native_handle/bridge_native_handle agree"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        expect(plugin.bridge_native_handle() != nullptr);
        expect(plugin.native_handle() == plugin.bridge_native_handle());
    };

    "run_script against a missing file reports failure (1)"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        expect(plugin.run_script("/definitely/does/not/exist/congelado_test_fixture.lua") == 1);
    };

    "from_lua converts LUA_TNIL to CG_NONE"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        lua_pushnil(state);
        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_NONE);
    };

    "from_lua converts LUA_TBOOLEAN to CG_BOOL"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        lua_pushboolean(state, 1);
        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_BOOL);
        expect(any.v_int64 == 1);
    };

    "from_lua converts an integer LUA_TNUMBER to CG_INT"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        lua_pushinteger(state, 42);
        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_INT);
        expect(any.v_int64 == 42);
    };

    "from_lua converts a fractional LUA_TNUMBER to CG_FLOAT"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        lua_pushnumber(state, 3.5);
        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_FLOAT);
        expect(any.v_float64 == 3.5);
    };

    "from_lua converts LUA_TSTRING to CG_STR"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        lua_pushstring(state, "hello");
        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_STR);
        expect(std::string_view{any.v_cstr} == "hello");
    };

    "from_lua falls back to CG_PTR for an unhandled type (light userdata)"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        int marker{};
        lua_pushlightuserdata(state, &marker);
        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_PTR);
        expect(any.v_ptr == &marker);
    };

    "from_lua/to_lua round-trip a table through a CG_MAP_HANDLE"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());

        lua_newtable(state);
        lua_pushinteger(state, 7);
        lua_setfield(state, -2, "count");
        lua_pushstring(state, "bar");
        lua_setfield(state, -2, "foo");

        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_MAP_HANDLE);

        int pushed = plugin.to_lua(any);
        expect(pushed == 1);

        lua_getfield(state, -1, "count");
        expect(lua_tointeger(state, -1) == 7);
        lua_pop(state, 1);

        lua_getfield(state, -1, "foo");
        const char *foo_value = lua_tostring(state, -1);
        expect(foo_value != nullptr) << fatal;
        expect(std::string_view{foo_value} == "bar");
        lua_pop(state, 1);
    };

    // SECURITY: this finding is WORSE than originally assumed — from_lua()'s LUA_TTABLE branch
    // (case LUA_TTABLE, ~line 200) recurses per nested table via lua_next()/lua_pushnil() with
    // NO call to lua_checkstack() anywhere, a classic Lua C-API pitfall: each recursive C call
    // consumed Lua-VM stack slots without requesting more, so sufficiently nested input silently
    // overflowed the interpreter's internal value-stack array — confirmed via a real crash
    // (segfault inside luaH_next/lua_next, corrupted hash table internals) around 5-6 levels of
    // recursion before the fix. Both from_lua()'s LUA_TTABLE case and to_lua()'s CG_MAP_HANDLE
    // case now call lua_checkstack() before pushing and throw a clean std::runtime_error instead
    // of silently corrupting memory if a table is nested deep enough to exhaust the VM stack.
    // This test proves the fixed round-trip handles 50 levels of nesting correctly.
    "from_lua/to_lua round-trip a table nested 50 levels deep with no crash or corruption "
    "(regression test for a missing lua_checkstack() found while writing this test — see the "
    "fix in from_lua()'s LUA_TTABLE case and to_lua()'s CG_MAP_HANDLE case)"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());

        constexpr int depth = 50;

        // Build depth+1 nested tables bottom-up: innermost table holds "leaf" = 42, then
        // each subsequent iteration wraps the previous table under an "inner" field.
        lua_newtable(state);
        lua_pushinteger(state, 42);
        lua_setfield(state, -2, "leaf");

        for (int level = 0; level < depth; ++level) {
            // stack: [inner]
            lua_newtable(state);
            // stack: [inner, outer]
            lua_pushvalue(state, -2);
            // stack: [inner, outer, inner_copy]
            lua_setfield(state, -2, "inner");
            // outer["inner"] = inner_copy (popped); stack: [inner, outer]
            lua_remove(state, -2);
            // drop the original inner reference; stack: [outer]
        }

        auto any = plugin.from_lua(lua_gettop(state));
        expect(any.type_index == CG_MAP_HANDLE) << fatal;

        int pushed = plugin.to_lua(any);
        expect(pushed == 1) << fatal;

        // Walk back down the `depth` "inner" hops and confirm the leaf survived the round trip.
        for (int level = 0; level < depth; ++level) {
            lua_getfield(state, -1, "inner");
            expect(lua_istable(state, -1) != 0) << fatal;
            lua_remove(state, -2);
        }
        lua_getfield(state, -1, "leaf");
        expect(lua_tointeger(state, -1) == 42);
    };

    "to_lua pushes CG_NONE as a Lua nil"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        int pushed = plugin.to_lua(CongeladoAny{.type_index = CG_NONE});
        expect(pushed == 1);
        expect(lua_isnil(state, -1) != 0);
    };

    "to_lua pushes CG_BOOL as a Lua boolean"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        plugin.to_lua(CongeladoAny{.type_index = CG_BOOL, .v_int64 = 1});
        expect(lua_toboolean(state, -1) != 0);
    };

    "to_lua pushes CG_INT as a Lua integer"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        plugin.to_lua(CongeladoAny{.type_index = CG_INT, .v_int64 = 99});
        expect(lua_tointeger(state, -1) == 99);
    };

    "to_lua pushes CG_FLOAT as a Lua number"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        plugin.to_lua(CongeladoAny{.type_index = CG_FLOAT, .v_float64 = 2.5});
        expect(lua_tonumber(state, -1) == 2.5);
    };

    "to_lua pushes CG_STR as a Lua string"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        plugin.to_lua(CongeladoAny{.type_index = CG_STR, .v_cstr = "cheese"});
        expect(std::string_view{lua_tostring(state, -1)} == "cheese");
    };

    "to_lua pushes a null CG_STR as an empty Lua string"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        plugin.to_lua(CongeladoAny{.type_index = CG_STR, .v_cstr = nullptr});
        expect(std::string_view{lua_tostring(state, -1)}.empty());
    };

    "to_lua falls back to light userdata for an unhandled type tag"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        int marker{};
        plugin.to_lua(CongeladoAny{.type_index = CG_PTR, .v_ptr = &marker});
        expect(lua_touserdata(state, -1) == &marker);
    };

    "from_native treats the void* as a stack index and delegates to from_lua"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        lua_pushinteger(state, 13);
        auto index = lua_gettop(state);
        auto any = plugin.from_native(reinterpret_cast<void *>(static_cast<std::intptr_t>(index)));  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
        expect(any.type_index == CG_INT);
        expect(any.v_int64 == 13);
    };

    "to_native delegates to to_lua and always returns nullptr"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());
        auto *result = plugin.to_native(CongeladoAny{.type_index = CG_INT, .v_int64 = 5});
        expect(result == nullptr);
        expect(lua_tointeger(state, -1) == 5);
    };

    "install_method exposes an InvokeFn as a callable Lua function"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());

        auto fn_context = std::make_unique<FnContext>();
        fn_context->m_key = "sum";
        fn_context->m_invoke =
            InvokeFn{[](std::span<const core::plugin::Value> args) -> core::plugin::Value {
                std::int64_t total = 0;
                for (const auto &arg : args) {
                    if (const auto *value = std::get_if<core::plugin::Int>(&arg)) {
                        total += value->m_value;
                    }
                }
                return core::plugin::Int{total};
            }};

        plugin.install_method(std::move(fn_context), "sum");

        lua_getglobal(state, "congelado");
        lua_getfield(state, -1, "sum");
        lua_pushinteger(state, 3);
        lua_pushinteger(state, 4);
        int rc = lua_pcall(state, 2, 1, 0);
        expect(rc == 0) << (rc != 0 ? lua_tostring(state, -1) : "");
        expect(lua_tointeger(state, -1) == 7);
    };

    "install_method's installed closure surfaces a C++ exception as a Lua error"_test = [] {
        LuaBridgePlugin plugin;
        LuaBridgeTestHelper::load(plugin);
        auto *state = static_cast<lua_State *>(plugin.native_handle());

        auto fn_context = std::make_unique<FnContext>();
        fn_context->m_key = "boom";
        fn_context->m_invoke =
            InvokeFn{[](std::span<const core::plugin::Value> /*args*/) -> core::plugin::Value {
                throw std::runtime_error{"kaboom"};
            }};

        plugin.install_method(std::move(fn_context), "boom");

        lua_getglobal(state, "congelado");
        lua_getfield(state, -1, "boom");
        int rc = lua_pcall(state, 0, 1, 0);
        expect(rc != 0);
        expect(std::string_view{lua_tostring(state, -1)}.contains("kaboom"));
    };
};

} // namespace lua_bridge_plugin_tests
#endif
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
