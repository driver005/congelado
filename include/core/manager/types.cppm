module;
#include <congelado/abi.h>

export module core_plugin:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::plugin::types {

enum class Kind : std::uint8_t { NOT_FOUND, DLOPEN_FAILED, ALREADY_LOADED };

class PluginError {
  public:
    /**
     * @brief Constructs a PluginError directly from a kind and message.
     * @param kind the error category — prefer the not_found()/dlopen_failed()/already_loaded()
     * factories over calling this straight, they read clearer at the call site.
     * @param message human-readable detail describing what went wrong.
     */
    PluginError(Kind kind, std::string message) : m_kind{kind}, m_message{std::move(message)} {}

    /**
     * @brief Builds a NOT_FOUND-kind PluginError.
     * @param message detail on what wasn't found (missing dependency, unscanned plugin, etc).
     * @return the constructed PluginError.
     */
    [[nodiscard]] static PluginError not_found(std::string message) {
        return {Kind::NOT_FOUND, std::move(message)};
    }
    /**
     * @brief Builds a DLOPEN_FAILED-kind PluginError.
     * @param message detail on the dlopen()/dlsym() failure or ABI mismatch that caused it.
     * @return the constructed PluginError.
     */
    [[nodiscard]] static PluginError dlopen_failed(std::string message) {
        return {Kind::DLOPEN_FAILED, std::move(message)};
    }
    /**
     * @brief Builds an ALREADY_LOADED-kind PluginError.
     * @param message detail on which plugin was already loaded.
     * @return the constructed PluginError.
     */
    [[nodiscard]] static PluginError already_loaded(std::string message) {
        return {Kind::ALREADY_LOADED, std::move(message)};
    }

    /// @brief Gets the error's category.
    /// @return the Kind this error was constructed with.
    [[nodiscard]] Kind get_kind() const noexcept { return m_kind; }
    /// @brief Gets the human-readable detail message.
    /// @return the message this error was constructed with.
    [[nodiscard]] std::string_view get_message() const noexcept { return m_message; }

  private:
    Kind m_kind;
    std::string m_message;
};

class PythonConfig {
  public:
    /// @brief Constructs a PythonConfig defaulting the module name to `"congelado"`.
    PythonConfig() : m_module_name{"congelado"} {};

    /// @brief Sets the Python module name plugin-registered functions get installed under.
    /// @param name the new module name.
    void set_module_name(std::string name) { m_module_name = std::move(name); }

    /// @brief Gets the Python module name plugin-registered functions get installed under.
    /// @return the configured module name.
    [[nodiscard]] std::string_view get_module_name() const noexcept { return m_module_name; }

  private:
    std::string m_module_name;
};

class LuaConfig {
  public:
    /// @brief Constructs a LuaConfig defaulting the table name to `"congelado"` and safe mode on.
    LuaConfig() : m_table_name{"congelado"} {};

    /// @brief Sets the Lua global table name plugin-registered functions get installed under.
    /// @param name the new table name.
    void set_table_name(std::string name) { m_table_name = std::move(name); }
    /// @brief Sets whether the Lua state runs in safe mode.
    /// @param safe_mode true to enable safe mode, false to disable it.
    void set_safe_mode(bool safe_mode) { m_safe_mode = safe_mode; }

    /// @brief Gets the Lua global table name plugin-registered functions get installed under.
    /// @return the configured table name.
    [[nodiscard]] std::string_view get_table_name() const noexcept { return m_table_name; }
    /// @brief Gets whether the Lua state is configured to run in safe mode.
    /// @return true if safe mode is on, false otherwise.
    [[nodiscard]] bool get_safe_mode() const noexcept { return m_safe_mode; }

  private:
    std::string m_table_name;
    bool m_safe_mode{true};
};

class GenerationConfig {
  public:
    /// @brief Default-constructs, wanting no bridge runtimes (native-only).
    GenerationConfig() = default;

    /// @brief Adds a wanted runtime by name (e.g. `"python"`, `"lua"`, or any user-registered
    /// bridge's own `runtime_name()`) — open set, no fixed enum of supported languages. No-op
    /// if already present.
    /// @param runtime_name the runtime name to add.
    void add_runtime(std::string runtime_name) { m_wanted_runtimes.insert(std::move(runtime_name)); }
    /// @brief Sets the Python-specific config (module name, etc).
    /// @param config the new PythonConfig.
    void set_python_config(PythonConfig config) { m_python = std::move(config); }
    /// @brief Sets the Lua-specific config (table name, safe mode, etc).
    /// @param config the new LuaConfig.
    void set_lua_config(LuaConfig config) { m_lua = std::move(config); }
    /// @brief Sets free-form extra key/value config passed through to the plugin at init time.
    /// @param extra the replacement extra config map.
    void set_extra(std::unordered_map<std::string, std::string> extra) {
        m_extra = std::move(extra);
    }

    /// @brief Gets the configured set of wanted runtime names.
    /// @return the wanted runtime names.
    [[nodiscard]] const std::set<std::string> &get_wanted_runtimes() const noexcept {
        return m_wanted_runtimes;
    }
    /// @brief Gets the Python-specific config.
    /// @return the configured PythonConfig.
    [[nodiscard]] const PythonConfig &get_python_config() const noexcept { return m_python; }
    /// @brief Gets the Lua-specific config.
    /// @return the configured LuaConfig.
    [[nodiscard]] const LuaConfig &get_lua_config() const noexcept { return m_lua; }
    /// @brief Gets the free-form extra key/value config.
    /// @return the extra config map, forwarded to the plugin's `congelado_init` as key/value pairs.
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_extra() const noexcept {
        return m_extra;
    }

    /**
     * @brief Checks whether a given runtime name is in the wanted set.
     * @param runtime_name the runtime name to test for.
     * @return true if `runtime_name` is included in the configured set.
     */
    [[nodiscard]] bool wants(std::string_view runtime_name) const noexcept {
        return m_wanted_runtimes.contains(std::string{runtime_name});
    }

  private:
    std::set<std::string> m_wanted_runtimes;
    PythonConfig m_python;
    LuaConfig m_lua;
    std::unordered_map<std::string, std::string> m_extra;
};


// ── C ABI function pointer types for plugin entry points ────────────────────

using InitFn = int (*)(const CongeladoHostCallbacks *host, const CongeladoConfigView *cfg);
using PluginUnloadFn = void (*)();
using PluginReadyFn = void (*)();
using PluginShutdownFn = void (*)();
using PluginStringFn = const char *(*)();
using PluginUint32Fn = std::uint32_t (*)();
using PluginArrayFn = const char *const *(*)();
using PluginCountFn = std::size_t (*)();
using WorkerTypeFn = const char *(*)();
using WorkerExecuteFn = CongeladoConfigView (*)(const CongeladoConfigView *input);

// ── Convenience helpers on top of the C ABI structs ─────────────────────────

template <typename T>
[[nodiscard]] T *router_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.router_ctx);
}
template <typename T>
[[nodiscard]] T *controller_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.controller_ctx);
}
/**
 * @brief Gets the host's contract registry, if the host populated it — a plugin registers a
 * `Contract<>` it creates (via `controller_ctx`'s `ContractGroup`) into this so it gets released
 * automatically at host shutdown instead of the plugin managing its own release.
 * @tparam T the concrete type to cast to (almost always `core::contract::ContractRegistry`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if the host didn't populate it.
 */
template <typename T>
[[nodiscard]] T *registry_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.registry_ctx);
}
template <typename T>
[[nodiscard]] T *leverager_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.leverager_ctx);
}
/**
 * @brief Gets the resolved storage-capable plugin's interface pointer, if one was found before
 * this host callback table got built — a plugin's `on_load` reads this to wire a storage
 * backend into its own connector, since capability resolution normally happens too late (after
 * every plugin's `on_load` has already run) for that to work any other way.
 * @tparam T the concrete interface type to cast to (almost always `interfaces::IDatabase`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if no storage-capable plugin was found.
 */
template <typename T>
[[nodiscard]] T *database_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.database_ctx);
}
/**
 * @brief Gets the resolved "lua" runtime's interfaces::IBridge*, if a lua_bridge plugin was
 * found before this host callback table got built — same "resolve before build()" reasoning
 * (and same shape) as database_ctx() above, for a plugin (e.g. engine) whose on_load needs a
 * live Lua bridge to evaluate expressions with.
 * @tparam T the concrete interface type to cast to (almost always `interfaces::IBridge`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if no lua_bridge plugin was found.
 */
template <typename T>
[[nodiscard]] T *lua_bridge_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.lua_bridge_ctx);
}
/**
 * @brief Gets the resolved search-capable plugin's interfaces::ISearchProvider*, if one was
 * found before this host callback table got built — same "resolve before build()" reasoning
 * (and same shape) as database_ctx() above, for the engine plugin's on_load to wire a search
 * backend into its own terminal-transition projector.
 * @tparam T the concrete interface type to cast to (almost always `interfaces::ISearchProvider`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if no search-capable plugin was found.
 */
template <typename T>
[[nodiscard]] T *search_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.search_ctx);
}
/**
 * @brief Gets the resolved cache-capable plugin's interfaces::ICache*, if one was found before
 * this host callback table got built — same "resolve before build()" reasoning (and same shape)
 * as database_ctx() above, for the engine plugin's on_load to wire a cache backend into its own
 * Connector via set_cache().
 * @tparam T the concrete interface type to cast to (almost always `interfaces::ICache`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if no cache-capable plugin was found (Connector
 * falls back to its own in-process LocalCache in that case).
 */
template <typename T>
[[nodiscard]] T *cache_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.cache_ctx);
}
/**
 * @brief Gets the resolved cron-capable plugin's interfaces::ICron*, if one was found before this
 * host callback table got built — same "resolve before build()" reasoning (and same shape) as
 * search_ctx()/cache_ctx() above, for the engine plugin's on_load to install its fire callback and
 * seed existing schedules into the backend.
 * @tparam T the concrete interface type to cast to (almost always `interfaces::ICron`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if no cron-capable plugin was found (schedules then
 * never fire).
 */
template <typename T>
[[nodiscard]] T *cron_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.cron_ctx);
}
/**
 * @brief Gets the resolved worker-manager-capable plugin's interfaces::IWorkerManager*, if one was
 * found before this host callback table got built — same "resolve before build()" reasoning (and
 * same shape) as cron_ctx() above, for the host to drive worker lifecycle after plugins go ready.
 * @tparam T the concrete interface type to cast to (almost always `interfaces::IWorkerManager`).
 * @param host the host callback table handed to `on_load`.
 * @return the resolved pointer, or `nullptr` if no worker-manager-capable plugin was found.
 */
template <typename T>
[[nodiscard]] T *worker_manager_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.worker_manager_ctx);
}

inline std::optional<std::string> config_get(const CongeladoConfigView &cfg,
                                             const std::string &key) noexcept {
    // Linear scan over the parallel keys/values arrays — first match wins.
    for (std::size_t index = 0; index < cfg.count; ++index) {
        if (std::string_view{cfg.keys[index]} == key) {
            return std::string{cfg.values[index]};
        }
    }
    return std::nullopt;
}
template <typename Callback>
void config_for_each(const CongeladoConfigView &cfg, Callback &&callback) noexcept {  // NOLINT(cppcoreguidelines-missing-std-forward) — deliberately not forwarded, see below
    // Just fan out every key/value pair in the view to the caller's callback. callback is
    // invoked once per entry, so it must NOT be std::forward'd here — forwarding on the first
    // iteration would move from an rvalue-ref callback, leaving every later call operating on
    // a moved-from callable (this was a real bugprone-use-after-move bug).
    for (std::size_t index = 0; index < cfg.count; ++index) {
        callback(std::string_view{cfg.keys[index]}, std::string_view{cfg.values[index]});
    }
}

// ── Config view builder ───────────────────────────────────────────────────

class ConfigViewBuilder {
  public:
    /**
     * @brief Appends a key/value pair, keeping the backing strings alive for view().
     * @warning `m_keys`/`m_values` are `std::deque`s specifically because pointers into deque
     * elements stay stable across further push_back calls (unlike `std::vector`, which can
     * reallocate and dangle every pointer collected so far). Swap that container type out and
     * this whole class turns into a UB generator — don't be that guy.
     * @param key the config key.
     * @param value the config value.
     */
    void add(std::string key, std::string value) {
        // Own the strings in the stable deques first, then collect raw pointers
        // into them — order matters, bet, the pointer arrays borrow from the deques.
        m_keys.push_back(std::move(key));
        m_values.push_back(std::move(value));
        m_key_ptrs.push_back(m_keys.back().c_str());
        m_val_ptrs.push_back(m_values.back().c_str());
    }

    /**
     * @brief Builds a C-ABI CongeladoConfigView over everything added so far.
     * @warning The returned view borrows pointers into this builder's own deques — it's only
     * valid as long as this ConfigViewBuilder stays alive and untouched. Let the builder go
     * out of scope (or call add() again after taking the view) and those pointers are cooked.
     * @return a view exposing the accumulated keys/values as parallel C-string arrays.
     */
    [[nodiscard]] CongeladoConfigView view() const noexcept {
        return {.keys = m_key_ptrs.data(), .values = m_val_ptrs.data(), .count = m_key_ptrs.size()};
    }

  private:
    std::deque<std::string> m_keys;
    std::deque<std::string> m_values;
    std::vector<const char *> m_key_ptrs;
    std::vector<const char *> m_val_ptrs;
};

} // namespace core::plugin::types

#ifdef CONGELADO_TEST
namespace core::plugin::types::tests {
using namespace boost::ut;

suite<"PluginError"> plugin_error_suite = [] {
    "not_found factory sets kind and message"_test = [] {
        auto err = PluginError::not_found("missing plugin");
        expect(err.get_kind() == Kind::NOT_FOUND);
        expect(err.get_message() == "missing plugin");
    };
    "dlopen_failed factory sets kind and message"_test = [] {
        auto err = PluginError::dlopen_failed("dlopen: boom");
        expect(err.get_kind() == Kind::DLOPEN_FAILED);
        expect(err.get_message() == "dlopen: boom");
    };
    "already_loaded factory sets kind and message"_test = [] {
        auto err = PluginError::already_loaded("postgres already loaded");
        expect(err.get_kind() == Kind::ALREADY_LOADED);
        expect(err.get_message() == "postgres already loaded");
    };
};

suite<"PythonConfig"> python_config_suite = [] {
    "defaults the module name to congelado"_test = [] {
        PythonConfig cfg;
        expect(cfg.get_module_name() == "congelado");
    };
    "set_module_name overwrites the default"_test = [] {
        PythonConfig cfg;
        cfg.set_module_name("myapp");
        expect(cfg.get_module_name() == "myapp");
    };
};

suite<"LuaConfig"> lua_config_suite = [] {
    "defaults the table name to congelado and safe mode on"_test = [] {
        LuaConfig cfg;
        expect(cfg.get_table_name() == "congelado");
        expect(cfg.get_safe_mode());
    };
    "setters overwrite both defaults"_test = [] {
        LuaConfig cfg;
        cfg.set_table_name("myapp");
        cfg.set_safe_mode(false);
        expect(cfg.get_table_name() == "myapp");
        expect(not cfg.get_safe_mode());
    };
};

suite<"GenerationConfig"> generation_config_suite = [] {
    "starts with no wanted runtimes"_test = [] {
        GenerationConfig cfg;
        expect(cfg.get_wanted_runtimes().empty());
        expect(not cfg.wants("python"));
    };
    "add_runtime is de-duplicated and queryable via wants()"_test = [] {
        GenerationConfig cfg;
        cfg.add_runtime("python");
        cfg.add_runtime("lua");
        cfg.add_runtime("python");

        expect(cfg.get_wanted_runtimes().size() == 2);
        expect(cfg.wants("python"));
        expect(cfg.wants("lua"));
        expect(not cfg.wants("ruby"));
    };
    "python/lua sub-configs round-trip through their setters"_test = [] {
        GenerationConfig cfg;
        PythonConfig python;
        python.set_module_name("myapp");
        cfg.set_python_config(python);

        LuaConfig lua;
        lua.set_table_name("myapp_lua");
        cfg.set_lua_config(lua);

        expect(cfg.get_python_config().get_module_name() == "myapp");
        expect(cfg.get_lua_config().get_table_name() == "myapp_lua");
    };
    "extra config round-trips verbatim"_test = [] {
        GenerationConfig cfg;
        cfg.set_extra({{"key", "value"}});
        expect(cfg.get_extra().at("key") == "value");
    };
};

suite<"ConfigViewBuilder"> config_view_builder_suite = [] {
    "add() accumulates pairs, view() exposes them as parallel C arrays"_test = [] {
        ConfigViewBuilder builder;
        builder.add("threads", "4");
        builder.add("migrations_dir", "db/migrations");

        auto view = builder.view();
        expect(view.count == 2);
        expect(std::string_view{view.keys[0]} == "threads");  // FIXME(clang-tidy): non-constant array index
        expect(std::string_view{view.values[0]} == "4");  // FIXME(clang-tidy): non-constant array index
        expect(std::string_view{view.keys[1]} == "migrations_dir");  // FIXME(clang-tidy): non-constant array index
        expect(std::string_view{view.values[1]} == "db/migrations");  // FIXME(clang-tidy): non-constant array index
    };
};

suite<"config_get / config_for_each"> config_helpers_suite = [] {
    "config_get finds an existing key and misses a missing one"_test = [] {
        const char *keys[] = {"threads", "name"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {"4", "worker"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView view{.keys = keys, .values = values, .count = 2};

        expect(config_get(view, "threads").value() == "4");
        expect(config_get(view, "name").value() == "worker");
        expect(not config_get(view, "missing").has_value());
    };

    "config_for_each visits every key/value pair in order"_test = [] {
        const char *keys[] = {"a", "b", "c"};      // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {"1", "2", "3"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView view{.keys = keys, .values = values, .count = 3};

        std::vector<std::pair<std::string, std::string>> collected;
        config_for_each(view, [&](std::string_view key, std::string_view value) {
            collected.emplace_back(std::string{key}, std::string{value});
        });

        expect(collected.size() == 3);
        expect(collected[0].first == "a" && collected[0].second == "1");
        expect(collected[2].first == "c" && collected[2].second == "3");
    };
};

suite<"host callback ctx helpers"> ctx_helper_suite = [] {
    "router_ctx/database_ctx cast the matching field back to T*"_test = [] {
        int router_probe = 0;
        double database_probe = 0.0;
        CongeladoHostCallbacks host{.router_ctx = &router_probe, .database_ctx = &database_probe};

        expect(router_ctx<int>(host) == &router_probe);
        expect(database_ctx<double>(host) == &database_probe);
        expect(cache_ctx<int>(host) == nullptr);
    };
};

} // namespace core::plugin::types::tests
#endif
