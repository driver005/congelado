// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <Python.h>
#include <cstdio>

export module python_bridge_plugin;

import congelado_plugin;
import core_plugin;
import interfaces;
import core_events;
import core_logger;
import std;

using InvokeFn = std::function<core::plugin::Value(std::span<const core::plugin::Value>)>;

namespace {

struct PyFnBridge {
    FnContext m_fn;
    class PythonBridgePlugin *m_bridge;
};

// The Python FFI bridge as a genuine plugin — formats/bridges are just plugins, no special
// category. Relocated verbatim from the old always-linked include/core/manager/bridge.cppm
// (core_plugin used to hard #include <Python.h>/<lua.hpp> for every consumer, whether or not
// they ever touched FFI); now Python is only linked into this one .so. Owns its own
// HandleTable (was previously shared with FfiRuntime via reference — HandleTable has no
// other consumers, safe to make self-contained per bridge instance).
class PythonBridgePlugin : public congelado::Plugin, public interfaces::IBridge {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "python_bridge"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_BRIDGE;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IBridge` surface.
     * @return this instance, upcast to `interfaces::IBridge*`.
     */
    void *bridge_get() noexcept { return static_cast<interfaces::IBridge *>(this); }

    /// @brief The runtime this bridge implements. @return `"python"`.
    [[nodiscard]] std::string_view runtime_name() const noexcept override { return "python"; }

    /// @brief The script file extension this bridge runs. @return `".py"`.
    [[nodiscard]] std::string_view script_extension() const noexcept override { return ".py"; }

    /**
     * @brief Runs a Python script file via `PyRun_SimpleFile` — the interpreter is already up
     * from `on_load()`, so this just opens and executes.
     * @param path path to the `.py` file to run.
     * @return the script's exit code, or `1` if `path` couldn't be opened.
     */
    [[nodiscard]] int run_script(std::string_view path) override {
        std::string owned_path{path};
        FILE *file = std::fopen(owned_path.c_str(), "r");
        if (file == nullptr) {
            core::logger::warning("python_bridge", "run_script: couldn't open '{}'", owned_path);
            core::events::publish("python_bridge.script_not_found", {{"path", owned_path}});
            return 1;
        }
        int rc = PyRun_SimpleFile(file, owned_path.c_str());
        std::fclose(file);
        core::logger::debug("python_bridge", "ran '{}', exit code {}", owned_path, rc);
        return rc;
    }

    /**
     * @brief Initializes the Python interpreter (safe to call repeatedly, no-op once already
     * initialized) and resolves/creates the target module — `module_name` config key, default
     * `"congelado"`.
     * @param cfg this plugin's config view.
     */
    void on_load(CongeladoHostCallbacks const & /*host*/, CongeladoConfigView const &cfg) override {
        auto module_name =
            std::string{congelado::config_get(cfg, "module_name").value_or("congelado")};

        if (Py_IsInitialized() == 0) {
            Py_Initialize();
        }

        auto *module = PyImport_AddModule(module_name.c_str());
        if (module != nullptr) {
            Py_INCREF(module);
            m_module.reset(module);
            core::logger::debug("python_bridge", "interpreter ready, module '{}'", module_name);
        } else {
            core::logger::warning("python_bridge", "failed to resolve module '{}'", module_name);
            core::events::publish("python_bridge.module_resolve_failed", {{"module", module_name}});
        }
    }

    /**
     * @brief Releases the resolved module reference, then finalizes the interpreter — the mirror
     * image of on_load()'s Py_Initialize(), so the interpreter's entire internal allocator pool
     * (obmalloc arenas, every object it ever created) doesn't just get leaked on every shutdown.
     * @note m_module — the only PyObject reference this plugin holds outside short-lived locals —
     * is released first; Py_FinalizeEx() itself runs every registered atexit hook and tears down
     * whatever the interpreter spun up internally.
     */
    void on_unload() noexcept override {
        m_module.reset();
        if (Py_IsInitialized() != 0) {
            Py_FinalizeEx();
        }
    }

    [[nodiscard]] CongeladoAny from_native(void *native_obj) override {
        return from_py(static_cast<PyObject *>(native_obj));
    }

    void *to_native(const CongeladoAny &any) override { return to_py(any); }

    void install_method(std::unique_ptr<FnContext> fn_context,
                        const std::string &lang_name) override {
        core::logger::debug("python_bridge", "installing method '{}'", lang_name);
        auto method_name = std::string{lang_name};
        auto method_def_ptr = std::make_unique<PyMethodDef>();
        auto *method_def = method_def_ptr.get();

        method_def->ml_name = method_name.data();
        method_def->ml_meth = [](PyObject *self_capsule, PyObject *py_args) -> PyObject * {
            try {
                auto *fn_bridge =
                    static_cast<PyFnBridge *>(PyCapsule_GetPointer(self_capsule, "cg.fn"));
                auto py_size = PyTuple_GET_SIZE(py_args);

                auto args = std::views::iota(Py_ssize_t{0}, py_size) |
                            std::views::transform([&](Py_ssize_t idx) {
                                return core::plugin::AnyConverter::from_any(
                                    fn_bridge->m_bridge->from_py(PyTuple_GET_ITEM(py_args, idx)));  // FIXME(clang-tidy): cppcoreguidelines-pro-type-cstyle-cast — CPython's own PyTuple_GET_ITEM macro expands to a C-style cast; switching to the checked PyTuple_GetItem() function would change error behavior (it validates the index and sets a Python exception, GET_ITEM doesn't), not a drop-in swap
                            }) |
                            std::ranges::to<std::vector>();

                auto result = std::any_cast<const InvokeFn &>(fn_bridge->m_fn.m_invoke)(args);

                return fn_bridge->m_bridge->to_py(core::plugin::AnyConverter::to_any(result));
            } catch (const std::exception &e) {
                core::events::publish("python_bridge.invoke_exception", {{"error", e.what()}});
                PyErr_SetString(PyExc_RuntimeError, e.what());
                return nullptr;
            }
        };
        method_def->ml_flags = METH_VARARGS;
        method_def->ml_doc = nullptr;

        auto py_fn_bridge = std::make_unique<PyFnBridge>(
            PyFnBridge{.m_fn = std::move(*fn_context), .m_bridge = this});

        auto *capsule = PyCapsule_New(py_fn_bridge.release(), "cg.fn", [](PyObject *cap) {
            delete static_cast<PyFnBridge *>(PyCapsule_GetPointer(cap, "cg.fn"));  // NOLINT(cppcoreguidelines-owning-memory)
        });

        PyModule_AddObject(m_module.get(), method_def->ml_name,
                           PyCFunction_NewEx(method_def, capsule, nullptr));
        Py_DECREF(capsule);

        m_method_defs.push_back(std::move(method_def_ptr));
    }

    [[nodiscard]] CongeladoAny from_py(PyObject *obj) {
        CongeladoAny any{};
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
        if (PyDict_Check(obj)) {
            PyObject *py_key{};
            PyObject *py_value{};
            Py_ssize_t pos{};

            auto handle = m_handles.map_create();

            while (PyDict_Next(obj, &pos, &py_key, &py_value) != 0) {
                const auto *dict_key = PyUnicode_AsUTF8(py_key);
                auto dict_value = from_py(py_value);
                m_handles.map_set(handle.v_int64, dict_key, dict_value);
            }

            any = handle;
            return any;
        }
        any.type_index = CG_PTR;
        any.v_ptr = obj;
        return any;
    }

    [[nodiscard]] PyObject *to_py(const CongeladoAny &any) {
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
        case CG_MAP_HANDLE: {
            auto *dict = PyDict_New();

            int64_t map_size = m_handles.get_map_size(any.v_int64).v_int64;
            int64_t keys_handle = m_handles.get_map_keys(any.v_int64).v_int64;

            for (int64_t idx = 0; idx < map_size; ++idx) {
                CongeladoAny key = m_handles.array_get(keys_handle, idx);
                CongeladoAny map_value = m_handles.map_get(any.v_int64, key.v_cstr);

                auto *py_value = to_py(map_value);
                PyDict_SetItemString(dict, key.v_cstr, py_value);
                Py_DECREF(py_value);
            }

            m_handles.handle_free(keys_handle);
            return dict;
        }
        default: {
            return PyLong_FromVoidPtr(any.v_ptr);
        }
        }
    }

  private:
    core::plugin::HandleTable m_handles;
    struct PyDeleter {
        void operator()(PyObject *obj) const {
            if (obj != nullptr) {
                Py_DECREF(obj);
            }
        }
    };
    std::unique_ptr<PyObject, PyDeleter> m_module;
    std::vector<std::unique_ptr<PyMethodDef>> m_method_defs;
};

} // namespace

CONGELADO_PLUGIN(PythonBridgePlugin);
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
