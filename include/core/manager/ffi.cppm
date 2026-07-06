module;

#include <congelado/abi.h>

export module core_plugin:ffi;

import std;
import :value;
import :types;
import :bridge;
import interfaces;

// ── FunctionInfo — compile-time function signature decomposition ─────────────

export namespace core::plugin {

template <typename F>
struct FunctionInfo;
template <typename C, typename R, typename... A>
struct FunctionInfo<R (C::*)(A...)> : FunctionInfo<R(A...)> {
    using Class = C;
};
template <typename C, typename R, typename... A>
struct FunctionInfo<R (C::*)(A...) const> : FunctionInfo<R(A...)> {
    using Class = C;
};
template <typename C, typename R, typename... A>
struct FunctionInfo<R (C::*)(A...) noexcept> : FunctionInfo<R(A...)> {
    using Class = C;
};
template <typename C, typename R, typename... A>
struct FunctionInfo<R (C::*)(A...) const noexcept> : FunctionInfo<R(A...)> {
    using Class = C;
};
template <typename R, typename... A>
struct FunctionInfo<R(A...)> {
    using Ret = R;
    using Args = std::tuple<A...>;
    static constexpr std::size_t N = sizeof...(A);
};

class FnEntry {
  public:
    FnEntry() = default;
    FnEntry(std::string key, std::size_t arity, std::function<Value(std::span<const Value>)> call)
        : m_key{std::move(key)}, m_arity{arity}, m_call{std::move(call)} {}

    template <auto MemFn, typename T>
    [[nodiscard]] static FnEntry from_method(T *instance, std::string key) {
        using Info = FunctionInfo<decltype(MemFn)>;
        using Params = typename Info::Args;
        using Ret = typename Info::Ret;
        constexpr std::size_t ar = Info::N;

        auto call = [instance, key](std::span<const Value> args) -> Value {
            if (args.size() != ar)
                throw std::runtime_error{
                    std::format("{}: expected {} args, got {}", key, ar, args.size())};
            return [&]<std::size_t... I>(std::index_sequence<I...>) -> Value {
                if constexpr (std::is_void_v<Ret>) {
                    (instance->*MemFn)(
                        ValueTraits<std::tuple_element_t<I, Params>>::from_value(args[I])...);
                    return None{};
                } else {
                    return ValueTraits<Ret>::to_value((instance->*MemFn)(
                        ValueTraits<std::tuple_element_t<I, Params>>::from_value(args[I])...));
                }
            }(std::make_index_sequence<Info::N>{});
        };
        return FnEntry{std::move(key), ar, std::move(call)};
    }

    [[nodiscard]] Value invoke(std::span<const Value> args) const { return m_call(args); }

    [[nodiscard]] std::string_view get_key() const noexcept { return m_key; }
    [[nodiscard]] std::size_t get_arity() const noexcept { return m_arity; }
    [[nodiscard]] const std::function<Value(std::span<const Value>)> &
    get_invoke_fn() const noexcept {
        return m_call;
    }

  private:
    std::string m_key;
    std::size_t m_arity{0};
    std::function<Value(std::span<const Value>)> m_call;
};

#ifdef __cpp_reflection
template <typename T>
consteval bool is_registerable(std::meta::info r) noexcept {
    return std::meta::is_function(r) && std::meta::is_nonstatic_member(r) &&
           !std::meta::is_constructor(r) && !std::meta::is_destructor(r) &&
           !std::meta::is_operator_function(r) && !std::meta::is_special_member(r);
}
template <typename T>
consteval bool is_overloaded(std::meta::info fn) noexcept {
    auto ctx = std::meta::access_context::current();
    auto name = std::meta::identifier_of(fn);
    std::size_t n = 0;
    for (auto m : std::meta::accessible_members_of(^^T, ctx))
        if (std::meta::is_function(m) && std::meta::identifier_of(m) == name)
            ++n;
    return n > 1;
}
#endif

class FfiRuntime {
  public:
    FfiRuntime() = default;
    explicit FfiRuntime(types::GenerationConfig cfg) : m_cfg{std::move(cfg)} {}
    FfiRuntime(types::GenerationConfig cfg, std::shared_ptr<types::PluginRef> plugin)
        : m_cfg{std::move(cfg)}, m_plugin{std::move(plugin)} {}

    FfiRuntime(const FfiRuntime &) = delete;
    FfiRuntime &operator=(const FfiRuntime &) = delete;
    FfiRuntime(FfiRuntime &&) = default;
    FfiRuntime &operator=(FfiRuntime &&) = default;

    void set_config(types::GenerationConfig cfg) { m_cfg = std::move(cfg); }
    void attach_plugin(std::shared_ptr<types::PluginRef> plugin) { m_plugin = std::move(plugin); }

    void register_entry(FnEntry e) { m_entries.insert_or_assign(std::string{e.get_key()}, std::move(e)); }

#ifdef __cpp_reflection
    template <typename T>
    void register_class(const types::GenerationConfig &cfg = {},
                        std::string_view prefix = std::meta::identifier_of(^^T)) {
        // 1. Create bridges based on config
        std::vector<std::unique_ptr<interfaces::IBridge>> bridges;

        if (cfg.wants(types::Runtime::PYTHON)) {
            if (auto b = bridge::PythonBridge::setup(cfg.get_python_config().get_module_name()))
                bridges.push_back(std::move(b));
        }
        if (cfg.wants(types::Runtime::LUA)) {
            if (auto b = bridge::LuaBridge::setup(cfg.get_lua_config().get_table_name()))
                bridges.push_back(std::move(b));
        }

        // 2. Register each reflected method
        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto method : std::meta::accessible_members_of(^^T, ctx)) {
            if constexpr (is_registerable(method) && !is_overloaded<T>(method)) {
                constexpr auto name = std::meta::identifier_of(method);
                auto reg_key = std::string{prefix} + "." + std::string{name};
                auto lang_name = std::string{prefix} + "_" + std::string{name};

                // Create and store FnEntry
                auto entry = FnEntry::from_method<[:method:]>(nullptr, std::string{reg_key});
                auto invoke_fn = entry.get_invoke_fn();
                register_entry(std::move(entry));

                // Create one FnContext per bridge (each bridge owns its own)
                for (auto &b : bridges) {
                    auto fn_ctx = std::make_unique<FnContext>(std::any{invoke_fn}, std::string{reg_key});
                    b->install_method(std::move(fn_ctx), lang_name);
                }
            }
        }

        // 3. Move bridges into this runtime's ownership
        for (auto &b : bridges)
            m_bridges.push_back(std::move(b));
    }
#else
    template <typename T>
    void register_class(const types::GenerationConfig & = {}, std::string_view = "") {}
#endif

    // ── Plugin lifecycle helpers (now own plugin ref) ─────────────────────

    int init(const CongeladoHostCallbacks *host_cb, const CongeladoConfigView *view) noexcept {
        if (!m_plugin)
            return -1;
        try {
            auto key = std::string{types::PluginRef::shared_symbol_name(0)}; // index 0 == congelado_init
            if (auto it = m_plugin->m_data.find(key); it != m_plugin->m_data.end()) {
                auto fn = reinterpret_cast<InitFn>(std::any_cast<void *>(it->second));
                return fn(host_cb, view);
            }
            return -1;
        } catch (...) {
            return -1;
        }
    }

    void on_ready() noexcept {
        if (!m_plugin)
            return;
        auto key = std::string{types::PluginRef::shared_symbol_name(3)}; // index 3 == congelado_on_ready
        if (auto it = m_plugin->m_data.find(key); it != m_plugin->m_data.end())
            reinterpret_cast<PluginReadyFn>(std::any_cast<void *>(it->second))();
    }

    void on_unload() noexcept {
        if (!m_plugin)
            return;
        auto key = std::string{types::PluginRef::shared_symbol_name(2)}; // index 2 == congelado_on_unload
        if (auto it = m_plugin->m_data.find(key); it != m_plugin->m_data.end())
            reinterpret_cast<PluginUnloadFn>(std::any_cast<void *>(it->second))();
    }

    // ── Function dispatch ───────────────────────────────────────────────────

    int call(const char *key, size_t key_len, const CongeladoAny *args, int num_args,
             CongeladoAny *result) noexcept {
        try {
            auto it = m_entries.find(std::string{key, key_len});
            if (it == m_entries.end()) {
                m_last_error = std::format("not found: {}", std::string_view{key, key_len});
                return -1;
            }
            auto dargs =
                std::views::iota(0, num_args) |
                std::views::transform([&](int i) { return AnyConverter::from_any(args[i]); }) |
                std::ranges::to<std::vector>();
            auto ret = it->second.invoke(dargs);
            *result = AnyConverter::to_any(ret);
            return 0;
        } catch (const std::exception &ex) {
            m_last_error = ex.what();
            return -1;
        }
    }

    [[nodiscard]] const char *get_last_error() const noexcept {
        return m_last_error.empty() ? "no error" : m_last_error.c_str();
    }

    // ── Accessors ─────────────────────────────────────────────────────────[...]

    [[nodiscard]] std::size_t get_size() const noexcept { return m_entries.size(); }
    [[nodiscard]] const types::GenerationConfig &get_config() const noexcept { return m_cfg; }
    [[nodiscard]] std::shared_ptr<types::PluginRef> get_plugin() const noexcept { return m_plugin; }

  private:
    types::GenerationConfig m_cfg;
    std::unordered_map<std::string, FnEntry> m_entries;
    std::vector<std::unique_ptr<interfaces::IBridge>> m_bridges;
    std::string m_last_error;
    std::shared_ptr<types::PluginRef> m_plugin;
};

} // namespace core::plugin
