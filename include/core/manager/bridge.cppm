// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
module;
#include "abi.h"

#include <Python.h>
#include <lua.hpp>
export module core_plugin:bridge;
import std;
import :types;
import :value;
import interfaces;

using InvokeFn = std::function<core::plugin::Value(std::span<const core::plugin::Value>)>;

export namespace core::plugin::bridge {

struct PyFnBridge {
    FnContext m_fn;
    class PythonBridge *m_bridge;
};

class PythonBridge : public interfaces::IBridge {
  public:
    /**
     * @brief Wraps a borrowed Python module object as a bridge, taking a new reference on it.
     * @note Takes ownership via `Py_INCREF` — the module ref gets released through `PyDeleter`
     * when this bridge is destroyed. Prefer setup() over calling this directly; it handles
     * Python interpreter init and module lookup for you.
     * @param table the shared HandleTable used to resolve map/array handles crossing the FFI.
     * @param module the Python module object to install registered methods onto.
     */
    PythonBridge(HandleTable &table, PyObject *module) : m_handles{table}, m_module{module} {
        if (m_module) {
            Py_INCREF(m_module.get());
        }
    }

    /**
     * @brief Initializes the Python interpreter if needed and builds a PythonBridge bound to
     * a named module.
     * @param table the shared HandleTable to hand off to the constructed bridge.
     * @param module_name name of the Python module to add/resolve methods onto.
     * @return a live PythonBridge, or null if the module couldn't be resolved — no throw here,
     * a null return is the whole error signal.
     */
    [[nodiscard]] static std::unique_ptr<PythonBridge> setup(HandleTable &table,
                                                             const std::string &module_name) {
        // Bring the interpreter up if this is the first bridge touching it —
        // safe to call repeatedly since it's a no-op once initialized.
        if (Py_IsInitialized() == 0) {
            Py_Initialize();
        }

        // Resolve (or create) the target module — no module means no bridge, bet.
        auto *module = PyImport_AddModule(module_name.c_str());
        if (module == nullptr) {
            return nullptr;
        }

        return std::make_unique<PythonBridge>(table, module);
    }

    /**
     * @brief Converts a native Python object pointer into the cross-ABI CongeladoAny form.
     * @param native_obj a `PyObject *` handed across the C ABI boundary as `void *`.
     * @return the equivalent CongeladoAny — see from_py() for the actual conversion rules.
     */
    [[nodiscard]] CongeladoAny from_native(void *native_obj) override {
        return from_py(static_cast<PyObject *>(native_obj));
    }

    /**
     * @brief Converts a CongeladoAny into a native `PyObject *`, exposed as `void *` for the
     * generic IBridge interface.
     * @param any the value to convert.
     * @return a new-reference `PyObject *` (as `void *`) — see to_py() for the actual mapping.
     */
    void *to_native(const CongeladoAny &any) override { return to_py(any); }

    /**
     * @brief Installs a registered function as a callable method on the bound Python module.
     * @warning The installed `ml_meth` trampoline `PyCapsule_GetPointer`s a `PyFnBridge *` and
     * dereferences it on every call — that capsule owns the `PyFnBridge`, and its destructor
     * `delete`s it when the module (or capsule) goes away. Outlive the module with a stray
     * reference to the callable and this is a straight use-after-free, no safety net, no cap.
     * @param fn_context the function context (invoke callable + registered key) to expose.
     * @param lang_name the attribute name the function is installed under in the Python module.
     */
    void install_method(std::unique_ptr<FnContext> fn_context,
                        const std::string &lang_name) override {
        // Build the PyMethodDef this method will be installed under — kept alive in
        // m_method_defs since CPython just borrows the pointer, doesn't own it.
        auto method_name = std::string{lang_name};
        auto method_def_ptr = std::make_unique<PyMethodDef>();

        auto *method_def = method_def_ptr.get();

        method_def->ml_name = method_name.data();

        // The actual trampoline CPython calls into — unpacks the capsule back to
        // the FnContext, marshals args in, invokes, marshals the result back out.
        method_def->ml_meth = [](PyObject *self_capsule, PyObject *py_args) -> PyObject * {
            try {
                // Recover the FnContext + owning bridge from the capsule that was
                // stashed alongside this trampoline at registration time.
                auto *fn_bridge =
                    static_cast<PyFnBridge *>(PyCapsule_GetPointer(self_capsule, "cg.fn"));
                auto py_size = PyTuple_GET_SIZE(py_args);

                // Convert every positional Python arg into the cross-ABI Value form.
                auto args = std::views::iota(Py_ssize_t{0}, py_size) |
                            std::views::transform([&](Py_ssize_t idx) {
                                return AnyConverter::from_any(
                                    fn_bridge->m_bridge->from_py(PyTuple_GET_ITEM(py_args, idx)));  // FIXME(clang-tidy): cppcoreguidelines-pro-type-cstyle-cast — CPython's own PyTuple_GET_ITEM macro expands to a C-style cast; switching to the checked PyTuple_GetItem() function would change error behavior (it validates the index and sets a Python exception, GET_ITEM doesn't), not a drop-in swap
                            }) |
                            std::ranges::to<std::vector>();

                // Dispatch to the real registered callable and convert the result back.
                auto result = std::any_cast<const InvokeFn &>(fn_bridge->m_fn.m_invoke)(args);

                return fn_bridge->m_bridge->to_py(AnyConverter::to_any(result));
            } catch (const std::exception &e) {
                // Never let a C++ exception cross back into the Python interpreter raw —
                // translate it into a proper Python exception instead.
                PyErr_SetString(PyExc_RuntimeError, e.what());

                return nullptr;
            }
        };

        method_def->ml_flags = METH_VARARGS;
        method_def->ml_doc = nullptr;

        PythonBridge *bridge_ptr = this;

        // Wrap the FnContext + bridge pointer in a capsule so the trampoline above
        // can reach them — the capsule's destructor is what frees this on module unload.
        auto py_fn_bridge = std::make_unique<PyFnBridge>(
            PyFnBridge{.m_fn = std::move(*fn_context), .m_bridge = bridge_ptr});

        auto *capsule = PyCapsule_New(py_fn_bridge.release(), "cg.fn", [](PyObject *cap) {
            // FIXME(clang-tidy): cppcoreguidelines-owning-memory — would need
            // gsl::owner<PyFnBridge *>, but this codebase has no GSL dependency; this is a
            // CPython capsule destructor callback (fixed C function-pointer signature), can't
            // change its parameter type to a smart pointer.
            delete static_cast<PyFnBridge *>(PyCapsule_GetPointer(cap, "cg.fn"));  // NOLINT(cppcoreguidelines-owning-memory)
        });

        // Install the bound method onto the module and drop our own capsule ref —
        // the module now owns the only reference that matters.
        PyModule_AddObject(m_module.get(), method_def->ml_name,
                           PyCFunction_NewEx(method_def, capsule, nullptr));
        Py_DECREF(capsule);

        m_method_defs.push_back(std::move(method_def_ptr));
    }

    /**
     * @brief Converts a Python object into a CongeladoAny, recursively for dict values.
     * @note None → CG_NONE, bool → CG_BOOL, int → CG_INT (via `PyLong_AsLongLong`, so out-of-
     * range Python ints truncate silently — no overflow check here), float → CG_FLOAT, str →
     * CG_STR (borrowed UTF-8 pointer, only valid as long as `obj` lives), dict → a fresh map
     * handle populated recursively via the shared HandleTable. Everything else falls through
     * to a raw CG_PTR wrapping the PyObject pointer itself.
     * @param obj the Python object to convert; must not be null.
     * @return the equivalent CongeladoAny.
     */
    [[nodiscard]] CongeladoAny from_py(PyObject *obj) {
        CongeladoAny any{};
        // Walk the Python type checks from most to least specific — None, bool, int,
        // float, str each map straight to a scalar CongeladoAny variant.
        if (obj == Py_None) {
            any.type_index = CG_NONE;
            return any;
        }
        if (PyBool_Check(obj)) {
            any.type_index = CG_BOOL;
            any.v_int64 = (obj == Py_True) ? 1 : 0;  // FIXME(clang-tidy): cppcoreguidelines-pro-type-cstyle-cast — CPython's Py_True macro expands to a C-style cast on this Python version; not something this codebase can change
            return any;
        }
        if (PyLong_Check(obj)) {
            any.type_index = CG_INT;
            any.v_int64 = PyLong_AsLongLong(obj);
            return any;
        }
        if (PyFloat_Check(obj)) {
            any.type_index = CG_FLOAT;
            any.v_float64 = PyFloat_AsDouble(obj);
            return any;
        }
        if (PyUnicode_Check(obj)) {
            any.type_index = CG_STR;
            any.v_cstr = PyUnicode_AsUTF8(obj);
            return any;
        }
        // Dicts get their own fresh map handle, populated recursively — no cap,
        // nested dicts convert all the way down before this call returns.
        if (PyDict_Check(obj)) {
            PyObject *py_key{};
            PyObject *py_value{};
            Py_ssize_t pos{};

            auto handle = m_handles.get().map_create();

            while (PyDict_Next(obj, &pos, &py_key, &py_value) != 0) {
                const auto *dict_key = PyUnicode_AsUTF8(py_key);
                auto dict_value = from_py(py_value);

                m_handles.get().map_set(handle.v_int64, dict_key, dict_value);
            }

            any = handle;

            return any;
        }
        // Everything else that's not recognized falls back to a raw opaque pointer.
        any.type_index = CG_PTR;
        any.v_ptr = obj;
        return any;
    }

    /**
     * @brief Converts a CongeladoAny into a new-reference Python object.
     * @warning Returns a **new reference** — callers own it and must `Py_DECREF` (or hand it
     * off to something that will, like `PyDict_SetItemString` followed by a decref, which is
     * exactly what the CG_MAP_HANDLE branch does internally). Leak this and it's a real
     * refcount leak, not just vibes.
     * @param any the value to convert; CG_MAP_HANDLE recurses through the shared HandleTable
     * to rebuild a Python dict, anything unrecognized falls back to a plain int from the
     * pointer bits.
     * @return a new-reference PyObject representing `any`.
     */
    [[nodiscard]] PyObject *to_py(const CongeladoAny &any) {
        // Scalar cases just wrap straight into the matching Python builtin.
        switch (any.type_index) {
        case CG_NONE: {
            Py_RETURN_NONE;
            break;
        }
        case CG_BOOL: {
            return PyBool_FromLong(any.v_int64);
        }
        case CG_INT: {
            return PyLong_FromLongLong(any.v_int64);
        }
        case CG_FLOAT: {
            return PyFloat_FromDouble(any.v_float64);
        }
        case CG_STR: {
            return PyUnicode_FromString((any.v_cstr != nullptr) ? any.v_cstr : "");
        }
        // Map handles get rebuilt into a fresh dict, key by key, via the shared
        // HandleTable — the temporary keys array is freed once it's been walked.
        case CG_MAP_HANDLE: {
            auto *dict = PyDict_New();

            int64_t map_size = m_handles.get().get_map_size(any.v_int64).v_int64;
            int64_t keys_handle = m_handles.get().get_map_keys(any.v_int64).v_int64;

            for (int64_t idx = 0; idx < map_size; ++idx) {
                CongeladoAny key = m_handles.get().array_get(keys_handle, idx);
                CongeladoAny map_value = m_handles.get().map_get(any.v_int64, key.v_cstr);

                auto *py_value = to_py(map_value);

                PyDict_SetItemString(dict, key.v_cstr, py_value);

                Py_DECREF(py_value);
            }

            m_handles.get().handle_free(keys_handle);
            return dict;
        }
        // Unrecognized kinds fall back to exposing the raw pointer bits as an int.
        default: {
            return PyLong_FromVoidPtr(any.v_ptr);
        }
        }
    }

  private:
    std::reference_wrapper<HandleTable> m_handles;
    struct PyDeleter {
        /**
         * @brief Releases a reference on a PyObject — the deleter for `unique_ptr<PyObject, PyDeleter>`.
         * @param obj the object to decref; no-op if null.
         */
        void operator()(PyObject *obj) const {
            if (obj != nullptr) {
                Py_DECREF(obj);
            }
        }
    };
    std::unique_ptr<PyObject, PyDeleter> m_module;
    std::vector<std::unique_ptr<PyMethodDef>> m_method_defs;
};

class LuaBridge : public interfaces::IBridge {
  public:
    /**
     * @brief Wraps an already-positioned Lua table as a bridge.
     * @note Prefer setup() over calling this directly — it handles creating/finding the global
     * table and leaves it sitting on the stack at `table_index` for this bridge to use.
     * @param table the shared HandleTable used to resolve map/array handles crossing the FFI.
     * @param state the owning Lua state (shared so multiple bridges/runtimes can share one VM).
     * @param table_index stack index where the target Lua table already sits.
     */
    LuaBridge(HandleTable &table, std::shared_ptr<lua_State> state, int table_index)
        : m_handles{table}, m_state{std::move(state)}, m_table_index{table_index} {}

    /**
     * @brief Creates a fresh Lua state and finds-or-creates a named global table to bind methods
     * onto.
     * @param table the shared HandleTable to hand off to the constructed bridge.
     * @param table_name name of the Lua global table to install registered methods into.
     * @return a live LuaBridge with the target table left on the Lua stack, or null if Lua
     * state creation failed.
     */
    [[nodiscard]] static std::unique_ptr<LuaBridge> setup(HandleTable &table,
                                                          const std::string &table_name) {
        // Stand up a fresh Lua state — no state, no bridge.
        auto state = types::make_lua_state();
        if (!state) {
            return nullptr;
        }

        // Find the named global table, or create it fresh if it's not already a table.
        lua_getglobal(state.get(), table_name.c_str());
        if (!lua_istable(state.get(), -1)) {
            lua_pop(state.get(), 1);
            lua_newtable(state.get());
            lua_setglobal(state.get(), table_name.c_str());
            lua_getglobal(state.get(), table_name.c_str());
        }

        // Leave the resolved table sitting on the stack, ready for the bridge to use.
        int table_index = lua_gettop(state.get());
        return std::make_unique<LuaBridge>(table, std::move(state), table_index);
    }

    /**
     * @brief Pops the bridge's bound table off the Lua stack.
     * @warning After this call `m_table_index` no longer points at a valid stack slot — this
     * bridge shouldn't be used for install_method()/from_lua()/to_lua() again afterward. It's
     * a one-way door, not something you undo.
     */
    void pop_table() {
        if (m_state) {
            lua_pop(m_state.get(), 1);
        }
    }

    /**
     * @brief Converts a native Lua stack index into the cross-ABI CongeladoAny form.
     * @param native_obj a Lua stack index, smuggled across the C ABI boundary as `void *` via
     * `reinterpret_cast<std::intptr_t>` — this is the Lua-side quirk of the generic IBridge
     * interface, since Lua values live on a VM stack, not as free-standing objects.
     * @return the equivalent CongeladoAny — see from_lua() for the actual conversion rules.
     */
    [[nodiscard]] CongeladoAny from_native(void *native_obj) override {
        auto idx = static_cast<int>(reinterpret_cast<std::intptr_t>(native_obj));  // FIXME(clang-tidy): reinterpret_cast usage — smuggling a Lua stack index through void* across the generic IBridge interface, see the doc comment above
        return from_lua(idx);
    }

    /**
     * @brief Pushes a CongeladoAny onto the Lua stack as its native representation.
     * @param any the value to convert and push — see to_lua() for the actual mapping.
     * @return always null; unlike PythonBridge, Lua values live on the VM stack, so there's no
     * pointer to hand back through the generic IBridge interface.
     */
    void *to_native(const CongeladoAny &any) override {
        to_lua(any);
        return nullptr;
    }

    /**
     * @brief Installs a registered function as a callable field on the bound Lua table.
     * @warning The pushed closure captures raw `fn_context`/`bridge` pointers as light userdata
     * upvalues — no ownership transfer at the Lua-C boundary itself. Ownership instead comes
     * from `m_fn_contexts.push_back(std::move(fn_context))` right before the closure is
     * created, which is what actually keeps the FnContext alive for as long as this LuaBridge
     * does. Move that push_back or drop it and the closure's upvalue dangles — straight cooked.
     * @param fn_context the function context (invoke callable + registered key) to expose.
     * @param lang_name the field name the function is installed under in the Lua table.
     */
    void install_method(std::unique_ptr<FnContext> fn_context,
                        const std::string &lang_name) override {
        auto *fn_context_ptr = fn_context.get();
        auto *bridge_ptr = this;

        // Ownership of the FnContext moves into m_fn_contexts right here — the raw
        // pointer captured as a closure upvalue below stays valid only because of this.
        m_fn_contexts.push_back(std::move(fn_context));

        // Stash the FnContext + bridge pointers as light userdata upvalues for the
        // closure to recover on every call.
        lua_pushlightuserdata(m_state.get(), fn_context_ptr);
        lua_pushlightuserdata(m_state.get(), bridge_ptr);

        lua_pushcclosure(
            m_state.get(),
            [](lua_State *state) -> int {
                auto *fn_context =
                    static_cast<FnContext *>(lua_touserdata(state, lua_upvalueindex(1)));
                auto *bridge =
                    static_cast<LuaBridge *>(lua_touserdata(state, lua_upvalueindex(2)));

                try {
                    // Convert every arg currently on the stack into the cross-ABI Value form.
                    int arg_count = lua_gettop(state);

                    auto args =
                        std::views::iota(0, arg_count) | std::views::transform([&](int idx) {
                            return AnyConverter::from_any(bridge->from_lua(idx + 1));
                        }) |
                        std::ranges::to<std::vector>();

                    // Dispatch to the real registered callable and push the result back.
                    auto result = std::any_cast<const InvokeFn &>(fn_context->m_invoke)(args);

                    return bridge->to_lua(AnyConverter::to_any(result));
                } catch (const std::exception &e) {
                    // Translate a C++ exception into a proper Lua error, lowkey a
                    // safety net, instead of letting it unwind straight through the VM.
                    return luaL_error(state, "%s", e.what());  // FIXME(clang-tidy): cppcoreguidelines-pro-type-vararg — luaL_error is Lua's own printf-style C API function, inherently vararg; no non-vararg equivalent exists
                }
            },
            2);

        lua_setfield(m_state.get(), m_table_index, lang_name.c_str());
    }

    /**
     * @brief Converts the Lua value at a stack index into a CongeladoAny, recursively for table
     * values.
     * @note nil → CG_NONE, boolean → CG_BOOL, number → CG_INT or CG_FLOAT depending on
     * `lua_isinteger`, string → CG_STR (borrowed pointer, only valid while the Lua value stays
     * on the stack), table → a fresh map handle populated recursively for string-keyed entries
     * only (non-string keys are silently skipped). Everything else falls through to a raw
     * CG_PTR via `lua_touserdata`.
     * @param stack_index the Lua stack index to read from.
     * @return the equivalent CongeladoAny.
     */
    [[nodiscard]] CongeladoAny from_lua(int stack_index) {
        CongeladoAny any{};

        // Dispatch on the Lua value's runtime type at this stack slot.
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
            // Lua 5.3+ distinguishes integer vs float subtypes at the number level.
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
        // Tables get a fresh map handle, walked entry by entry — non-string keys
        // are silently skipped since map handles only support string keys.
        case LUA_TTABLE: {
            auto handle = m_handles.get().map_create();

            lua_pushnil(m_state.get());

            // Negative indices shift by one once the nil sentinel is pushed above,
            // since that adds a new slot on top of the stack.
            int adjusted_index = stack_index < 0 ? stack_index - 1 : stack_index;

            while (lua_next(m_state.get(), adjusted_index) != 0) {
                if (lua_type(m_state.get(), -2) == LUA_TSTRING) {
                    const auto *lua_key = lua_tostring(m_state.get(), -2);

                    auto dict_value = from_lua(-1);

                    m_handles.get().map_set(handle.v_int64, lua_key, dict_value);
                }
                lua_pop(m_state.get(), 1);
            }

            any = handle;
            return any;
        }
        // Anything else falls back to a raw opaque pointer.
        default: {
            any.type_index = CG_PTR;
            any.v_ptr = lua_touserdata(m_state.get(), stack_index);
            return any;
        }
        }
    }

    /**
     * @brief Pushes a CongeladoAny onto the Lua stack as its native representation.
     * @note CG_MAP_HANDLE recurses through the shared HandleTable to rebuild a Lua table field
     * by field. Every branch pushes exactly one value, hence the always-1 return — matches the
     * usual Lua C-function convention for "how many results did you push."
     * @param any the value to convert and push.
     * @return the number of values pushed onto the Lua stack — always 1.
     */
    int to_lua(const CongeladoAny &any) {
        // Scalar cases push straight onto the Lua stack via the matching push* call.
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
        // Map handles get rebuilt into a fresh table, field by field, via the
        // shared HandleTable — the temporary keys array is freed once walked.
        case CG_MAP_HANDLE: {
            lua_newtable(m_state.get());

            int64_t map_size = m_handles.get().get_map_size(any.v_int64).v_int64;
            int64_t keys_handle = m_handles.get().get_map_keys(any.v_int64).v_int64;

            for (int64_t idx = 0; idx < map_size; ++idx) {
                CongeladoAny key = m_handles.get().array_get(keys_handle, idx);
                CongeladoAny map_value = m_handles.get().map_get(any.v_int64, key.v_cstr);

                to_lua(map_value);

                lua_setfield(m_state.get(), -2, key.v_cstr);
            }

            m_handles.get().handle_free(keys_handle);
            return 1;
        }
        // Unrecognized kinds fall back to pushing the raw pointer as light userdata.
        default: {
            lua_pushlightuserdata(m_state.get(), any.v_ptr);
            return 1;
        }
        }
    }

  private:
    std::reference_wrapper<HandleTable> m_handles;
    std::shared_ptr<lua_State> m_state;
    int m_table_index;
    std::vector<std::unique_ptr<FnContext>> m_fn_contexts;
};

} // namespace core::plugin::bridge
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
