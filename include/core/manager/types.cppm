module;
#include <congelado/abi.h>
#include <lua.hpp>

export module core_plugin:types;

import std;

export namespace core::plugin::types {

enum class Kind : std::uint8_t { NOT_FOUND, DLOPEN_FAILED, ALREADY_LOADED };

class PluginError {
  public:
    PluginError(Kind kind, std::string message) : m_kind{kind}, m_message{std::move(message)} {}

    [[nodiscard]] static PluginError not_found(std::string message) {
        return {Kind::NOT_FOUND, std::move(message)};
    }
    [[nodiscard]] static PluginError dlopen_failed(std::string message) {
        return {Kind::DLOPEN_FAILED, std::move(message)};
    }
    [[nodiscard]] static PluginError already_loaded(std::string message) {
        return {Kind::ALREADY_LOADED, std::move(message)};
    }

    [[nodiscard]] Kind get_kind() const noexcept { return m_kind; }
    [[nodiscard]] std::string_view get_message() const noexcept { return m_message; }

  private:
    Kind m_kind;
    std::string m_message;
};

enum class Runtime : std::uint32_t {
    NONE = 0,
    NATIVE = 1 << 0,
    PYTHON = 1 << 1,
    LUA = 1 << 2,
    WASM = 1 << 3,
};
constexpr Runtime operator|(Runtime rigth, Runtime left) noexcept {
    return static_cast<Runtime>(std::to_underlying(rigth) | std::to_underlying(left));
}
constexpr bool has(Runtime set, Runtime flag) noexcept {
    return (std::to_underlying(set) & std::to_underlying(flag)) != 0;
}

class PythonConfig {
  public:
    PythonConfig() : m_module_name{"congelado"} {};

    void set_module_name(std::string name) { m_module_name = std::move(name); }

    [[nodiscard]] std::string_view get_module_name() const noexcept { return m_module_name; }

  private:
    std::string m_module_name;
};

class LuaConfig {
  public:
    LuaConfig() : m_table_name{"congelado"}, m_safe_mode{true} {};

    void set_table_name(std::string name) { m_table_name = std::move(name); }
    void set_safe_mode(bool safe_mode) { m_safe_mode = safe_mode; }

    [[nodiscard]] std::string_view get_table_name() const noexcept { return m_table_name; }
    [[nodiscard]] bool get_safe_mode() const noexcept { return m_safe_mode; }

  private:
    std::string m_table_name;
    bool m_safe_mode;
};

class GenerationConfig {
  public:
    GenerationConfig() : GenerationConfig(Runtime::NATIVE) {};
    GenerationConfig(Runtime runtimes) : m_runtimes{runtimes} {}

    void set_runtimes(Runtime runtimes) { m_runtimes = runtimes; }
    void set_python_config(PythonConfig config) { m_python = std::move(config); }
    void set_lua_config(LuaConfig config) { m_lua = std::move(config); }
    void set_extra(std::unordered_map<std::string, std::string> extra) {
        m_extra = std::move(extra);
    }

    [[nodiscard]] const Runtime &get_runtimes() const noexcept { return m_runtimes; }
    [[nodiscard]] const PythonConfig &get_python_config() const noexcept { return m_python; }
    [[nodiscard]] const LuaConfig &get_lua_config() const noexcept { return m_lua; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_extra() const noexcept {
        return m_extra;
    }

    [[nodiscard]] bool wants(Runtime runtime) const noexcept { return has(m_runtimes, runtime); }

  private:
    Runtime m_runtimes;
    PythonConfig m_python;
    LuaConfig m_lua;
    std::unordered_map<std::string, std::string> m_extra;
};


// ── C ABI function pointer types for plugin entry points ────────────────────

using InitFn = int (*)(const CongeladoHostCallbacks *host, const CongeladoConfigView *cfg);
using PluginUnloadFn = void (*)();
using PluginReadyFn = void (*)();
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
template <typename T>
[[nodiscard]] T *leverager_ctx(const CongeladoHostCallbacks &host) noexcept {
    return static_cast<T *>(host.leverager_ctx);
}

inline std::optional<std::string> config_get(const CongeladoConfigView &cfg,
                                             std::string key) noexcept {
    for (std::size_t i = 0; i < cfg.count; ++i)
        if (std::string_view{cfg.keys[i]} == key)
            return std::string{cfg.values[i]};
    return std::nullopt;
}
template <typename Callback>
void config_for_each(const CongeladoConfigView &cfg, Callback &&callback) noexcept {
    for (std::size_t i = 0; i < cfg.count; ++i)
        std::forward<Callback>(callback)(std::string_view{cfg.keys[i]},
                                         std::string_view{cfg.values[i]});
}

// ── Config view builder ───────────────────────────────────────────────────

class ConfigViewBuilder {
  public:
    void add(std::string key, std::string value) {
        m_keys.push_back(std::move(key));
        m_values.push_back(std::move(value));
        m_key_ptrs.push_back(m_keys.back().c_str());
        m_val_ptrs.push_back(m_values.back().c_str());
    }

    [[nodiscard]] CongeladoConfigView view() const noexcept {
        return {m_key_ptrs.data(), m_val_ptrs.data(), m_key_ptrs.size()};
    }

  private:
    std::deque<std::string> m_keys;
    std::deque<std::string> m_values;
    std::vector<const char *> m_key_ptrs;
    std::vector<const char *> m_val_ptrs;
};

// ── Lua state ownership ──────────────────────────────────────────────────────

inline std::shared_ptr<lua_State> make_lua_state() {
    return std::shared_ptr<lua_State>{luaL_newstate(), [](lua_State *L) { lua_close(L); }};
}

} // namespace core::plugin::types
