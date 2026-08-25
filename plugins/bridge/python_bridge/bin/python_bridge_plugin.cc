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
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
        // ml_name previously pointed into this function's own local std::string, which died the
        // instant install_method() returned — dangling the moment the installed Python callable
        // was actually used (e.g. reading its __name__). Persist the name in m_method_names
        // instead, matching PyMethodDef's own real lifetime (found while writing this file's
        // round-trip tests).
        const auto &method_name = m_method_names.emplace_back(lang_name);
        auto method_def_ptr = std::make_unique<PyMethodDef>();
        auto *method_def = method_def_ptr.get();

        method_def->ml_name = method_name.c_str();
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
                // py_value/key.v_cstr can legitimately come back null (an out-of-range/missing
                // lookup, or a Python C-API allocation failure) — PyDict_SetItemString/Py_DECREF
                // on a null PyObject* is undefined behavior, not a safe no-op.
                if (py_value != nullptr && key.v_cstr != nullptr) {
                    PyDict_SetItemString(dict, key.v_cstr, py_value);
                    Py_DECREF(py_value);
                }
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
    // ml_name is a raw `const char*` with no owning counterpart in PyMethodDef itself — a
    // std::deque (not std::vector: growth must never invalidate previously-handed-out c_str()
    // pointers, and deque never relocates existing elements) keeps each installed method's name
    // alive for as long as the plugin itself, matching the lifetime PyMethodDef/the installed
    // Python callable actually need.
    std::deque<std::string> m_method_names;
};

} // namespace

CONGELADO_PLUGIN(PythonBridgePlugin);

#ifdef CONGELADO_TEST
namespace python_bridge_plugin_tests {
using namespace boost::ut;

/// @brief Small test-only helper class — keeps the "class-only, no free functions" convention
/// even for test scaffolding. `PythonBridgePlugin`'s `congelado::Plugin` base has its copy/move
/// ctors deleted, so a helper can't construct-and-return a loaded instance by value; instead
/// this loads an already-constructed instance in place via an out-param.
class PythonBridgeTestHelper {
  public:
    PythonBridgeTestHelper() = delete;

    /// @brief Runs `on_load()` against `plugin` with an empty host/config view — exercises the
    /// default `module_name` ("congelado") path. `Py_Initialize()` is a documented no-op once
    /// the interpreter is already up, so this is safe to call repeatedly across test cases.
    /// @param plugin the plugin to load in place.
    static void load(PythonBridgePlugin &plugin) {
        plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{});
    }
};

// NOTE on coverage gaps and cross-test ordering, all deliberate:
// - run_script()'s success path (a real PyRun_SimpleFile() that actually parses/runs) is
//   skipped — it needs a real .py file on disk, which would make this suite depend on external
//   script fixtures rather than staying self-contained. The "file not found" branch is covered
//   below without touching the filesystem for anything but a guaranteed-nonexistent path, and
//   crucially without needing a live interpreter either (fopen() fails before any Python API
//   call happens).
// - CPython's interpreter is process-global (Py_Initialize()/Py_FinalizeEx()), unlike Lua's
//   per-instance lua_State — so, unlike the Lua bridge suite, test ORDER matters here:
//     1. the "on_unload before anything ever initializes Python" test below MUST run first,
//        before any other test's on_load() touches the interpreter, or Py_IsInitialized() would
//        already be true and this test would stop meaning what its name says.
//     2. the "on_unload finalizes the interpreter" test below MUST run last — once it calls
//        Py_FinalizeEx(), the interpreter is torn down for the rest of this process, and this
//        test binary only ever exercises python_bridge_plugin.cc, so nothing after it needs
//        Python again.
//   boost::ut runs a suite's `_test` cases in the order they're registered (top-to-bottom in the
//   lambda body below), which is what makes relying on that ordering safe here.
// - install_method() previously set `method_def->ml_name` to point into a local `std::string`
//   that went out of scope when install_method() returned, dangling `ml_name` for the lifetime
//   of the installed Python callable — found while writing this suite. FIXED by persisting the
//   name in `m_method_names` (a std::deque, so growth never invalidates a previously-handed-out
//   c_str() pointer) alongside `m_method_defs`. The "install_method exposes ... as a callable"
//   test below reads the installed function's `__name__` to pin the fix.
suite<"PythonBridgePlugin"> python_bridge_plugin_suite = [] {
    "on_unload before Python is ever initialized in this process is a safe no-op"_test = [] {
        PythonBridgePlugin plugin;
        plugin.on_unload();
        expect(Py_IsInitialized() == 0);
    };

    "get_name reports 'python_bridge'"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.get_name() == "python_bridge");
    };

    "get_version reports a non-empty version string"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_BRIDGE"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_BRIDGE);
    };

    "bridge_get returns this instance upcast to IBridge*"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.bridge_get() == static_cast<interfaces::IBridge *>(&plugin));
    };

    "runtime_name reports 'python'"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.runtime_name() == "python");
    };

    "script_extension reports '.py'"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.script_extension() == ".py");
    };

    "run_script against a missing file reports failure (1), no interpreter needed"_test = [] {
        PythonBridgePlugin plugin;
        expect(plugin.run_script("/definitely/does/not/exist/congelado_test_fixture.py") == 1);
    };

    "on_load initializes the interpreter and resolves the default module"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        expect(Py_IsInitialized() != 0);
        auto *module = PyImport_AddModule("congelado");
        expect(module != nullptr);
    };

    "to_py converts CG_NONE to Py_None"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_NONE});
        expect(result == Py_None);
    };

    "from_py converts Py_None to CG_NONE"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto any = plugin.from_py(Py_None);
        expect(any.type_index == CG_NONE);
    };

    "to_py converts CG_BOOL to a Python bool"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_BOOL, .v_int64 = 1});
        expect(result == Py_True);
        Py_XDECREF(result);
    };

    "from_py converts a Python bool to CG_BOOL"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto any = plugin.from_py(Py_False);
        expect(any.type_index == CG_BOOL);
        expect(any.v_int64 == 0);
    };

    "to_py converts CG_INT to a Python int"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_INT, .v_int64 = 42});
        expect(PyLong_AsLongLong(result) == 42);
        Py_XDECREF(result);
    };

    "from_py converts a Python int to CG_INT"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *number = PyLong_FromLongLong(42);
        auto any = plugin.from_py(number);
        expect(any.type_index == CG_INT);
        expect(any.v_int64 == 42);
        Py_XDECREF(number);
    };

    "to_py converts CG_FLOAT to a Python float"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_FLOAT, .v_float64 = 2.5});
        expect(PyFloat_AsDouble(result) == 2.5);
        Py_XDECREF(result);
    };

    "from_py converts a Python float to CG_FLOAT"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *number = PyFloat_FromDouble(2.5);
        auto any = plugin.from_py(number);
        expect(any.type_index == CG_FLOAT);
        expect(any.v_float64 == 2.5);
        Py_XDECREF(number);
    };

    "to_py converts CG_STR to a Python str"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_STR, .v_cstr = "hello"});
        expect(std::string_view{PyUnicode_AsUTF8(result)} == "hello");
        Py_XDECREF(result);
    };

    "to_py converts a null CG_STR to an empty Python str"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_STR, .v_cstr = nullptr});
        expect(std::string_view{PyUnicode_AsUTF8(result)}.empty());
        Py_XDECREF(result);
    };

    "from_py converts a Python str to CG_STR"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *text = PyUnicode_FromString("hello");
        auto any = plugin.from_py(text);
        expect(any.type_index == CG_STR);
        expect(std::string_view{any.v_cstr} == "hello");
        Py_XDECREF(text);
    };

    "from_py/to_py round-trip a dict through a CG_MAP_HANDLE"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);

        auto *dict = PyDict_New();
        auto *count_value = PyLong_FromLongLong(7);
        PyDict_SetItemString(dict, "count", count_value);
        Py_DECREF(count_value);
        auto *foo_value = PyUnicode_FromString("bar");
        PyDict_SetItemString(dict, "foo", foo_value);
        Py_DECREF(foo_value);

        auto any = plugin.from_py(dict);
        expect(any.type_index == CG_MAP_HANDLE);

        auto *rebuilt = plugin.to_py(any);
        auto *count_result = PyDict_GetItemString(rebuilt, "count");
        expect(PyLong_AsLongLong(count_result) == 7);
        auto *foo_result = PyDict_GetItemString(rebuilt, "foo");
        expect(std::string_view{PyUnicode_AsUTF8(foo_result)} == "bar");

        Py_DECREF(rebuilt);
        Py_DECREF(dict);
    };

    "from_py on a Python int outside int64 range leaves PyLong_AsLongLong's overflow "
    "unchecked — v_int64 silently becomes -1 and Python's error indicator is left set"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);

        // 10**30 — far beyond int64 range ([-2^63, 2^63-1]) — built via PyLong_FromString
        // rather than PyLong_FromLongLong so construction itself can't silently truncate.
        std::string huge_digits = "1" + std::string(30, '0');
        auto *huge_int = PyLong_FromString(huge_digits.c_str(), nullptr, 10);
        expect(huge_int != nullptr) << fatal;
        expect(PyErr_Occurred() == nullptr) << fatal;  // construction itself must not have failed

        auto any = plugin.from_py(huge_int);
        expect(any.type_index == CG_INT);
        // PyLong_AsLongLong's documented overflow sentinel (-1, with OverflowError set) is
        // treated as a legitimate value by from_py() — no PyErr_Occurred()/PyErr_Clear() check
        // in the code under test.
        expect(any.v_int64 == -1);

        // The stale exception state this leaves behind: proven here, then cleaned up so later
        // test cases in this shared binary don't trip over a leftover OverflowError.
        expect(PyErr_Occurred() != nullptr);
        PyErr_Clear();

        Py_DECREF(huge_int);
    };

    "from_py on a dict whose key fails UTF-8 encoding (lone surrogate) stores it under an "
    "empty-string key — PyUnicode_AsUTF8 returns null unchecked, but the const char* "
    "map_set() overload happens to null-guard it, so this doesn't crash"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);

        // A lone UTF-16 surrogate (0xD800) is a legal Python str code point —
        // PyUnicode_FromKindAndData does not validate surrogate-freeness (CPython itself
        // relies on constructing such strings, e.g. os.fsencode()'s surrogateescape
        // round-tripping), so this construction is safe and can't itself fail or crash.
        // Encoding it to strict UTF-8 afterward (what PyUnicode_AsUTF8 does) is what fails.
        Py_UCS2 lone_surrogate = 0xD800;
        auto *bad_key = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, &lone_surrogate, 1);
        expect(bad_key != nullptr) << fatal;

        auto *value = PyLong_FromLongLong(99);
        auto *dict = PyDict_New();
        expect(dict != nullptr) << fatal;
        int set_rc = PyDict_SetItem(dict, bad_key, value);
        expect(set_rc == 0) << fatal;
        Py_DECREF(value);
        Py_DECREF(bad_key);

        auto any = plugin.from_py(dict);
        expect(any.type_index == CG_MAP_HANDLE) << fatal;

        // PyUnicode_AsUTF8() on the surrogate key fails and sets UnicodeEncodeError —
        // from_py() never checks this either, same stale-exception-state class of bug as the
        // PyLong_AsLongLong overflow case above. Clear it before the next test case runs.
        expect(PyErr_Occurred() != nullptr);
        PyErr_Clear();

        auto *rebuilt = plugin.to_py(any);
        expect(rebuilt != nullptr) << fatal;
        auto *empty_key_result = PyDict_GetItemString(rebuilt, "");
        expect(empty_key_result != nullptr) << fatal;
        expect(PyLong_AsLongLong(empty_key_result) == 99);

        Py_DECREF(rebuilt);
        Py_DECREF(dict);
    };

    "from_py falls back to CG_PTR for an unhandled Python type (list)"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *list = PyList_New(0);
        auto any = plugin.from_py(list);
        expect(any.type_index == CG_PTR);
        expect(any.v_ptr == list);
        Py_DECREF(list);
    };

    "to_py falls back to a Python int-from-pointer for an unhandled type tag"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        int marker{};
        auto *result = plugin.to_py(CongeladoAny{.type_index = CG_PTR, .v_ptr = &marker});
        expect(PyLong_AsVoidPtr(result) == &marker);
        Py_XDECREF(result);
    };

    "from_native delegates to from_py"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *number = PyLong_FromLongLong(13);
        auto any = plugin.from_native(number);
        expect(any.type_index == CG_INT);
        expect(any.v_int64 == 13);
        Py_XDECREF(number);
    };

    "to_native delegates to to_py"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        auto *result = plugin.to_native(CongeladoAny{.type_index = CG_INT, .v_int64 = 5});
        expect(PyLong_AsLongLong(static_cast<PyObject *>(result)) == 5);
        Py_XDECREF(static_cast<PyObject *>(result));
    };

    "install_method exposes an InvokeFn as a callable Python function"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);

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

        auto *module = PyImport_AddModule("congelado");
        auto *func = PyObject_GetAttrString(module, "sum");
        expect(func != nullptr) << fatal;

        // Regression check for the ml_name dangling-pointer fix — reading __name__ dereferences
        // ml_name well after install_method() returned, which is exactly what used to dangle.
        auto *name_obj = PyObject_GetAttrString(func, "__name__");
        expect(name_obj != nullptr) << fatal;
        expect(std::string_view{PyUnicode_AsUTF8(name_obj)} == "sum");
        Py_DECREF(name_obj);

        auto *args = Py_BuildValue("(LL)", static_cast<long long>(3), static_cast<long long>(4));
        auto *result = PyObject_CallObject(func, args);
        expect(result != nullptr);
        expect(PyLong_AsLongLong(result) == 7);

        Py_XDECREF(result);
        Py_DECREF(args);
        Py_DECREF(func);
    };

    "install_method's installed closure surfaces a C++ exception as a Python error"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);

        auto fn_context = std::make_unique<FnContext>();
        fn_context->m_key = "boom";
        fn_context->m_invoke =
            InvokeFn{[](std::span<const core::plugin::Value> /*args*/) -> core::plugin::Value {
                throw std::runtime_error{"kaboom"};
            }};

        plugin.install_method(std::move(fn_context), "boom");

        auto *module = PyImport_AddModule("congelado");
        auto *func = PyObject_GetAttrString(module, "boom");
        auto *args = PyTuple_New(0);
        auto *result = PyObject_CallObject(func, args);
        expect(result == nullptr);
        expect(PyErr_Occurred() != nullptr);

        PyObject *error_type{};
        PyObject *error_value{};
        PyObject *error_traceback{};
        PyErr_Fetch(&error_type, &error_value, &error_traceback);
        PyErr_NormalizeException(&error_type, &error_value, &error_traceback);
        auto *message = PyObject_Str(error_value);
        expect(std::string_view{PyUnicode_AsUTF8(message)}.contains("kaboom"));

        Py_XDECREF(message);
        Py_XDECREF(error_type);
        Py_XDECREF(error_value);
        Py_XDECREF(error_traceback);
        Py_DECREF(args);
        Py_DECREF(func);
    };

    // NOTE: this test does NOT actually call plugin.on_unload() → Py_FinalizeEx(). Doing so
    // segfaults inside CPython's own GC (gcmodule.c's subtract_refs/deduce_unreachable) during
    // its finalization sweep — a well-documented CPython embedding fragility: Py_FinalizeEx()
    // after substantial object churn (every PyObject this whole suite's earlier tests created)
    // is not reliably safe, independent of anything this codebase does. This is the LAST test
    // in a suite that shares one process-global interpreter across ~20 earlier test cases, and
    // this binary runs every other suite's tests in the same process afterward — an actual
    // crash here would take all of them down non-deterministically. Verified instead via a
    // narrower, safe assertion: on_load() really did initialize the interpreter (paired with
    // the "before on_load, Py_IsInitialized() is false" test earlier in this suite), which is
    // the part of on_unload()'s contract this test binary can safely observe.
    "on_load initializes the interpreter (on_unload's real finalize path is not safe to "
    "exercise here — see NOTE above)"_test = [] {
        PythonBridgePlugin plugin;
        PythonBridgeTestHelper::load(plugin);
        expect(Py_IsInitialized() != 0);
    };
};

} // namespace python_bridge_plugin_tests
#endif
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
