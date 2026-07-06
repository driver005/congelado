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

class PythonBridge : public interfaces::IBridge {
  public:
    explicit PythonBridge(PyObject *mod) : m_mod{mod} {
        if (m_mod)
            Py_INCREF(m_mod.get());
    }

    [[nodiscard]] static std::unique_ptr<PythonBridge> setup(const std::string &module_name) {
        if (!Py_IsInitialized())
            Py_Initialize();
        auto *mod = PyImport_AddModule(module_name.c_str());
        if (!mod)
            return nullptr;
        return std::make_unique<PythonBridge>(mod);
    }

    [[nodiscard]] CongeladoAny from_native(void *native_obj) override {
        return from_py(static_cast<PyObject *>(native_obj));
    }

    void *to_native(const CongeladoAny &a) override { return to_py(a); }

    void install_method(std::unique_ptr<FnContext> ctx, const std::string &lang_name) override {
        auto name_buf = std::string{lang_name};
        auto def = std::make_unique<PyMethodDef>();
        auto *raw_def = def.get();
        raw_def->ml_name = name_buf.data();
        raw_def->ml_meth = [](PyObject *self_cap, PyObject *py_args) -> PyObject * {
            try {
                auto *ctx = static_cast<FnContext *>(PyCapsule_GetPointer(self_cap, "cg.fn"));
                auto n = PyTuple_GET_SIZE(py_args);
                auto args = std::views::iota(Py_ssize_t{0}, n) |
                            std::views::transform([&](Py_ssize_t i) {
                                return AnyConverter::from_any(
                                    PythonBridge::from_py(PyTuple_GET_ITEM(py_args, i)));
                            }) |
                            std::ranges::to<std::vector>();
                auto result = std::any_cast<const InvokeFn &>(ctx->invoke)(args);
                return PythonBridge::to_py(AnyConverter::to_any(result));
            } catch (const std::exception &e) {
                PyErr_SetString(PyExc_RuntimeError, e.what());
                return nullptr;
            }
        };
        raw_def->ml_flags = METH_VARARGS;
        raw_def->ml_doc = nullptr;
        auto *cap = PyCapsule_New(ctx.release(), "cg.fn", [](PyObject *c) {
            delete static_cast<FnContext *>(PyCapsule_GetPointer(c, "cg.fn"));
        });
        PyModule_AddObject(m_mod.get(), raw_def->ml_name, PyCFunction_NewEx(raw_def, cap, nullptr));
        Py_DECREF(cap);
        m_method_defs.push_back(std::move(def));
    }

    [[nodiscard]] static CongeladoAny from_py(PyObject *obj) {
        CongeladoAny a{};
        if (obj == Py_None) {
            a.type_index = CG_NONE;
            return a;
        }
        if (PyBool_Check(obj)) {
            a.type_index = CG_BOOL;
            a.v_int64 = (obj == Py_True) ? 1 : 0;
            return a;
        }
        if (PyLong_Check(obj)) {
            a.type_index = CG_INT;
            a.v_int64 = PyLong_AsLongLong(obj);
            return a;
        }
        if (PyFloat_Check(obj)) {
            a.type_index = CG_FLOAT;
            a.v_float64 = PyFloat_AsDouble(obj);
            return a;
        }
        if (PyUnicode_Check(obj)) {
            a.type_index = CG_STR;
            a.v_cstr = PyUnicode_AsUTF8(obj);
            return a;
        }
        if (PyDict_Check(obj)) {
            auto h = HandleTable::map_create();
            PyObject *k{};
            PyObject *v{};
            Py_ssize_t pos{};
            while (PyDict_Next(obj, &pos, &k, &v)) {
                auto *ck = PyUnicode_AsUTF8(k);
                CongeladoAny ck_any{.type_index = CG_STR, .v_cstr = ck};
                auto cv = from_py(v);
                HandleTable::map_set(&h, &ck_any, &cv);
            }
            a = h;
            return a;
        }
        a.type_index = CG_PTR;
        a.v_ptr = obj;
        return a;
    }

    [[nodiscard]] static PyObject *to_py(const CongeladoAny &a) {
        switch (a.type_index) {
        case CG_NONE:
            Py_RETURN_NONE;
        case CG_BOOL:
            return PyBool_FromLong(a.v_int64);
        case CG_INT:
            return PyLong_FromLongLong(a.v_int64);
        case CG_FLOAT:
            return PyFloat_FromDouble(a.v_float64);
        case CG_STR:
            return PyUnicode_FromString(a.v_cstr ? a.v_cstr : "");
        case CG_MAP_HANDLE: {
            auto *dict = PyDict_New();
            CongeladoAny h_size = HandleTable::get_map_size(&a);
            CongeladoAny h_keys = HandleTable::get_map_keys(&a);
            for (int64_t i = 0; i < h_size.v_int64; ++i) {
                CongeladoAny idx{.type_index = CG_INT, .v_int64 = i};
                CongeladoAny k = HandleTable::array_get(&h_keys, &idx);
                CongeladoAny slot = HandleTable::map_get(&a, &k);
                auto *pv = to_py(slot);
                PyDict_SetItemString(dict, k.v_cstr, pv);
                Py_DECREF(pv);
            }
            HandleTable::handle_free(&h_keys);
            return dict;
        }
        default:
            return PyLong_FromVoidPtr(a.v_ptr);
        }
    }

  private:
    struct PyDeleter {
        void operator()(PyObject *p) const {
            if (p)
                Py_DECREF(p);
        }
    };
    std::unique_ptr<PyObject, PyDeleter> m_mod;
    std::vector<std::unique_ptr<PyMethodDef>> m_method_defs;
};

} // namespace core::plugin::bridge

export namespace core::plugin::bridge {

class LuaBridge : public interfaces::IBridge {
  public:
    LuaBridge(std::shared_ptr<lua_State> L, int table_idx)
        : m_state{std::move(L)}, m_table_idx{table_idx} {}

    [[nodiscard]] static std::unique_ptr<LuaBridge> setup(const std::string &table_name) {
        auto L = types::make_lua_state();
        if (!L)
            return nullptr;
        lua_getglobal(L.get(), table_name.c_str());
        if (!lua_istable(L.get(), -1)) {
            lua_pop(L.get(), 1);
            lua_newtable(L.get());
            lua_setglobal(L.get(), table_name.c_str());
            lua_getglobal(L.get(), table_name.c_str());
        }
        int tbl = lua_gettop(L.get());
        return std::make_unique<LuaBridge>(std::move(L), tbl);
    }

    void pop_table() {
        if (m_state)
            lua_pop(m_state.get(), 1);
    }

    [[nodiscard]] CongeladoAny from_native(void *native_obj) override {
        auto idx = static_cast<int>(reinterpret_cast<std::intptr_t>(native_obj));
        return from_lua(m_state, idx);
    }

    void *to_native(const CongeladoAny &a) override {
        to_lua(m_state, a);
        return nullptr;
    }

    void install_method(std::unique_ptr<FnContext> ctx, const std::string &lang_name) override {
        auto *raw = ctx.get();
        auto *self = this;
        m_fn_contexts.push_back(std::move(ctx));
        lua_pushlightuserdata(m_state.get(), raw);
        lua_pushlightuserdata(m_state.get(), self);
        lua_pushcclosure(
            m_state.get(),
            [](lua_State *LS) -> int {
                auto *ctx = static_cast<FnContext *>(lua_touserdata(LS, lua_upvalueindex(1)));
                auto *self = static_cast<LuaBridge *>(lua_touserdata(LS, lua_upvalueindex(2)));
                try {
                    int n = lua_gettop(LS);
                    auto args =
                        std::views::iota(0, n) | std::views::transform([&](int i) {
                            return AnyConverter::from_any(self->from_lua(self->m_state, i + 1));
                        }) |
                        std::ranges::to<std::vector>();
                    auto result = std::any_cast<const InvokeFn &>(ctx->invoke)(args);
                    return self->to_lua(self->m_state, AnyConverter::to_any(result));
                } catch (const std::exception &e) {
                    return luaL_error(LS, "%s", e.what());
                }
            },
            2);
        lua_setfield(m_state.get(), m_table_idx, lang_name.c_str());
    }

    [[nodiscard]] CongeladoAny from_lua(const std::shared_ptr<lua_State> &L, int idx) {
        CongeladoAny a{};
        switch (lua_type(L.get(), idx)) {
        case LUA_TNIL:
            a.type_index = CG_NONE;
            return a;
        case LUA_TBOOLEAN:
            a.type_index = CG_BOOL;
            a.v_int64 = lua_toboolean(L.get(), idx);
            return a;
        case LUA_TNUMBER:
            if (lua_isinteger(L.get(), idx)) {
                a.type_index = CG_INT;
                a.v_int64 = lua_tointeger(L.get(), idx);
            } else {
                a.type_index = CG_FLOAT;
                a.v_float64 = lua_tonumber(L.get(), idx);
            }
            return a;
        case LUA_TSTRING:
            a.type_index = CG_STR;
            a.v_cstr = lua_tostring(L.get(), idx);
            return a;
        case LUA_TTABLE: {
            auto h = HandleTable::map_create();
            lua_pushnil(L.get());
            int ti = idx < 0 ? idx - 1 : idx;
            while (lua_next(L.get(), ti)) {
                if (lua_type(L.get(), -2) == LUA_TSTRING) {
                    auto *k = lua_tostring(L.get(), -2);
                    CongeladoAny ck{.type_index = CG_STR, .v_cstr = k};
                    auto v = from_lua(L, -1);
                    HandleTable::map_set(&h, &ck, &v);
                }
                lua_pop(L.get(), 1);
            }
            a = h;
            return a;
        }
        default:
            a.type_index = CG_PTR;
            a.v_ptr = lua_touserdata(L.get(), idx);
            return a;
        }
    }

    int to_lua(const std::shared_ptr<lua_State> &L, const CongeladoAny &a) {
        switch (a.type_index) {
        case CG_NONE:
            lua_pushnil(L.get());
            return 1;
        case CG_BOOL:
            lua_pushboolean(L.get(), static_cast<int>(a.v_int64));
            return 1;
        case CG_INT:
            lua_pushinteger(L.get(), static_cast<lua_Integer>(a.v_int64));
            return 1;
        case CG_FLOAT:
            lua_pushnumber(L.get(), static_cast<lua_Number>(a.v_float64));
            return 1;
        case CG_STR:
            lua_pushstring(L.get(), a.v_cstr ? a.v_cstr : "");
            return 1;
        case CG_MAP_HANDLE: {
            lua_newtable(L.get());
            CongeladoAny h_size = HandleTable::get_map_size(&a);
            CongeladoAny h_keys = HandleTable::get_map_keys(&a);
            for (int64_t i = 0; i < h_size.v_int64; ++i) {
                CongeladoAny idx{.type_index = CG_INT, .v_int64 = i};
                CongeladoAny k = HandleTable::array_get(&h_keys, &idx);
                CongeladoAny slot = HandleTable::map_get(&a, &k);
                to_lua(L, slot);
                lua_setfield(L.get(), -2, k.v_cstr);
            }
            HandleTable::handle_free(&h_keys);
            return 1;
        }
        default:
            lua_pushlightuserdata(L.get(), a.v_ptr);
            return 1;
        }
    }

  private:
    std::shared_ptr<lua_State> m_state;
    int m_table_idx;
    std::vector<std::unique_ptr<FnContext>> m_fn_contexts;
};

} // namespace core::plugin::bridge
