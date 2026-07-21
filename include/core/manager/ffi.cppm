module;

#include <congelado/abi.h>

export module core_plugin:ffi;

import std;
import :value;
import :types;
import :plugin_ref;
import :bridge;
import interfaces;

export namespace core::plugin {
using std::runtime_error;


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
    static constexpr std::size_t ARG_COUNT = sizeof...(A);
};

class FnEntry {
  public:
    /// @brief Default-constructs an empty, uncallable FnEntry — mostly for container storage.
    FnEntry() = default;
    /**
     * @brief Constructs an FnEntry directly from its parts.
     * @param key the registered lookup key for this entry.
     * @param arity expected argument count for `call`.
     * @param call the type-erased invoke callable.
     */
    FnEntry(std::string key, std::size_t arity, std::function<Value(std::span<const Value>)> call)
        : m_key{std::move(key)}, m_arity{arity}, m_call{std::move(call)} {}

    /**
     * @brief Builds an FnEntry that dispatches to a bound member function pointer, marshaling
     * `Value` args in and out via `ValueTraits`.
     * @warning Zero runtime arity/type checking on the args passed through `ValueTraits::from_
     * value` beyond the plain count check — a `Value` holding the wrong alternative for a
     * given parameter type throws from inside `ValueTraits`, not from here. `instance` must
     * outlive every call through the returned FnEntry, since it's captured raw, not owned.
     * @tparam MemFn the pointer-to-member-function to bind (non-type template param).
     * @tparam T the class `MemFn` belongs to, deduced from `instance`.
     * @param instance the object `MemFn` gets invoked on for every call.
     * @param key the registered lookup key for the resulting entry.
     * @return an FnEntry wrapping the bound method, ready to register_entry() or dispatch via
     * call().
     */
    template <auto MemFn, typename T>
    [[nodiscard]] static FnEntry from_method(T *instance, std::string key) {
        using Info = FunctionInfo<decltype(MemFn)>;
        using Params = Info::Args;
        using Ret = Info::Ret;
        constexpr std::size_t ARG_SIZE = Info::ARG_COUNT;

        auto call = [instance, key](std::span<const Value> args) -> Value {
            // Bail loud if the caller didn't pass the exact arity this member expects —
            // no partial application, no cap.
            if (args.size() != ARG_SIZE) {
                throw runtime_error{
                    std::format("{}: expected {} args, got {}", key, ARG_SIZE, args.size())};
            }

            // Unpack the arg span positionally via an index sequence, converting each
            // Value to its parameter type and invoking the bound member function.
            return [&]<std::size_t... I>(std::index_sequence<I...>) -> Value {
                if constexpr (std::is_void_v<Ret>) {
                    // Void-returning members have nothing to marshal back — hand back None.
                    (instance->*MemFn)(
                        ValueTraits<std::tuple_element_t<I, Params>>::from_value(args[I])...);  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                    return None{};
                } else {
                    // Non-void: convert the return value back into a Value for the caller.
                    return ValueTraits<Ret>::to_value((instance->*MemFn)(
                        ValueTraits<std::tuple_element_t<I, Params>>::from_value(args[I])...));  // FIXME(clang-tidy): unchecked operator[], consider .at(); non-constant array index
                }
            }(std::make_index_sequence<Info::ARG_COUNT>{});
        };

        return FnEntry{std::move(key), ARG_SIZE, std::move(call)};
    }

    /**
     * @brief Calls the wrapped function with the given args.
     * @param args arguments to pass through to the underlying callable.
     * @return the callable's result, marshaled back as a Value.
     * @throws std::runtime_error if the underlying callable rejects the arg count or types
     * (thrown from inside `m_call`, e.g. the arity check in from_method()'s generated lambda).
     */
    [[nodiscard]] Value invoke(std::span<const Value> args) const { return m_call(args); }

    /// @brief Gets the registered lookup key.
    /// @return this entry's key.
    [[nodiscard]] std::string_view get_key() const noexcept { return m_key; }
    /// @brief Gets the expected argument count.
    /// @return this entry's arity.
    [[nodiscard]] std::size_t get_arity() const noexcept { return m_arity; }
    /// @brief Gets the underlying type-erased invoke callable.
    /// @return a reference to the wrapped `std::function`.
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
    // Count every accessible member sharing this function's name — more than one
    // hit means it's overloaded and register_class() should skip it.
    std::size_t overload_count = 0;
    for (auto member : std::meta::accessible_members_of(^^T, ctx))
        if (std::meta::is_function(member) && std::meta::identifier_of(member) == name)
            ++overload_count;
    return overload_count > 1;
}
#endif

class FfiRuntime {
  public:
    /// @brief Default-constructs an FfiRuntime with a default GenerationConfig and no plugin.
    FfiRuntime() = default;
    /// @brief Constructs with an explicit GenerationConfig and no plugin attached yet.
    /// @param cfg the runtime generation config to use.
    explicit FfiRuntime(types::GenerationConfig cfg) : m_cfg{std::move(cfg)} {}
    /**
     * @brief Constructs with both a GenerationConfig and an already-loaded plugin attached.
     * @param cfg the runtime generation config to use.
     * @param plugin the loaded plugin this runtime drives lifecycle calls (init/on_ready/
     * on_unload) against.
     */
    FfiRuntime(types::GenerationConfig cfg, std::shared_ptr<types::PluginRef> plugin)
        : m_cfg{std::move(cfg)}, m_plugin{std::move(plugin)} {}

    /// @brief Deleted — an FfiRuntime owns bridges and a plugin ref, no copying that motion.
    FfiRuntime(const FfiRuntime &) = delete;
    /// @brief Deleted — same reason as the copy ctor.
    FfiRuntime &operator=(const FfiRuntime &) = delete;
    /// @brief Move-constructs, transferring ownership of bridges/entries/plugin ref.
    FfiRuntime(FfiRuntime &&) = default;
    /// @brief Move-assigns, transferring ownership of bridges/entries/plugin ref.
    FfiRuntime &operator=(FfiRuntime &&) = default;
    /// @brief Default dtor — every member is a normal RAII type, nothing needs manual teardown.
    ~FfiRuntime() = default;

    /// @brief Sets (replaces) the runtime generation config.
    /// @param cfg the new config.
    void set_config(types::GenerationConfig cfg) { m_cfg = std::move(cfg); }
    /// @brief Attaches (or replaces) the loaded plugin this runtime drives lifecycle calls against.
    /// @param plugin the plugin ref to attach.
    void attach_plugin(std::shared_ptr<types::PluginRef> plugin) { m_plugin = std::move(plugin); }

    /**
     * @brief Registers (or overwrites) a callable entry, keyed by its own registered key.
     * @param entry the FnEntry to register — an existing entry with the same key gets replaced.
     */
    void register_entry(FnEntry entry) {
        m_entries.insert_or_assign(std::string{entry.get_key()}, std::move(entry));
    }

#ifdef __cpp_reflection
    /**
     * @brief Reflects over `T`'s member functions and registers every eligible one as a
     * callable FnEntry, wiring up a Python/Lua bridge for each wanted runtime in `cfg`.
     * @warning Only compiled when `__cpp_reflection` is available — the non-reflection build
     * falls back to the no-op overload below. Overloaded member functions get silently
     * skipped via is_overloaded() (no ambiguity resolution attempted), and only functions
     * passing is_registerable() (non-static, non-ctor/dtor, non-operator, non-special-member)
     * get registered at all.
     * @tparam T the reflected class to register methods from.
     * @param cfg which runtimes (Python/Lua) to build bridges for and how to configure them.
     * @param prefix namespace prefix used to build both the registered key (`prefix.method`)
     * and the bridge-facing name (`prefix_method`); defaults to `T`'s own identifier.
     */
    template <typename T>
    void register_class(const types::GenerationConfig &cfg = {},
                        std::string_view prefix = std::meta::identifier_of(^^T)) {
        // 1. Create bridges based on config
        std::vector<std::unique_ptr<interfaces::IBridge>> bridges;

        // Only stand up a Python bridge if the caller actually wants Python.
        if (cfg.wants(types::Runtime::PYTHON)) {
            if (auto bridge = bridge::PythonBridge::setup(m_handles,
                                                    cfg.get_python_config().get_module_name()))
                bridges.push_back(std::move(bridge));
        }
        // Same deal for Lua — both runtimes can be wanted at once.
        if (cfg.wants(types::Runtime::LUA)) {
            if (auto bridge = bridge::LuaBridge::setup(m_handles,
                                                  cfg.get_lua_config().get_table_name()))
                bridges.push_back(std::move(bridge));
        }

        // 2. Register each reflected method
        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto method : std::meta::accessible_members_of(^^T, ctx)) {
            if constexpr (is_registerable(method) && !is_overloaded<T>(method)) {
                constexpr auto name = std::meta::identifier_of(method);
                auto registered_key = std::string{prefix} + "." + std::string{name};
                auto lang_name = std::string{prefix} + "_" + std::string{name};

                // Create and store FnEntry
                auto entry = FnEntry::from_method<[:method:]>(nullptr, std::string{registered_key});
                auto invoke_function = entry.get_invoke_fn();
                register_entry(std::move(entry));

                // Create one FnContext per bridge (each bridge owns its own)
                for (auto &bridge : bridges) {
                    auto fn_context =
                        std::make_unique<FnContext>(std::any{invoke_function}, std::string{registered_key});
                    bridge->install_method(std::move(fn_context), lang_name);
                }
            }
        }

        // 3. Move bridges into this runtime's ownership
        for (auto &bridge : bridges)
            m_bridges.push_back(std::move(bridge));
    }
#else
    /**
     * @brief No-op fallback when `__cpp_reflection` isn't available — reflection-based class
     * registration simply isn't possible without it, so this compiles clean and does nothing.
     * @tparam T the class that would have been reflected over, had reflection been available.
     */
    template <typename T>
    void register_class([[maybe_unused]] const types::GenerationConfig &cfg = {},
                        [[maybe_unused]] std::string_view prefix = "") {}
#endif

    // ── Plugin lifecycle helpers (now own plugin ref) ─────────────────────

    /**
     * @brief Calls the attached plugin's `congelado_init` entry point across the C ABI.
     * @warning Reinterpret-casts a resolved `void *` symbol straight to `types::InitFn` and
     * calls through it — this is a hard trust boundary. If the plugin's actual `congelado_init`
     * signature doesn't match `InitFn`, that's UB on the call, not a caught error, no cap.
     * @param host_cb host callback table handed to the plugin.
     * @param view config key/value view handed to the plugin.
     * @return the plugin's own return code, or -1 if no plugin is attached, the `congelado_init`
     * symbol wasn't resolved, or the call threw (caught here and swallowed into -1).
     */
    int init(const CongeladoHostCallbacks *host_cb, const CongeladoConfigView *view) noexcept {
        // No plugin attached — nothing to init.
        if (!m_plugin) {
            return -1;
        }
        try {
            // Look up the resolved congelado_init symbol and, if present, jump across
            // the ABI boundary through it.
            auto symbol_name =
                std::string{types::PluginRef::shared_symbol_name(0)}; // index 0 == congelado_init
            if (auto it = m_plugin->m_data.find(symbol_name); it != m_plugin->m_data.end()) {
                auto init_function = reinterpret_cast<types::InitFn>(std::any_cast<void *>(it->second));  // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI cast of a dlsym'd void* back to its known function pointer type
                return init_function(host_cb, view);
            }
            return -1;
        } catch (...) {
            // Never let a plugin-side throw escape across the ABI boundary — report
            // failure through the return code instead.
            return -1;
        }
    }

    /**
     * @brief Calls the attached plugin's optional `congelado_on_ready` lifecycle hook, if present.
     * @note No-op (bet, safely) if no plugin is attached or the symbol was never resolved —
     * `congelado_on_ready` is optional per the plugin ABI, unlike `congelado_init`.
     */
    void on_ready() noexcept {
        // No plugin attached — nothing to notify.
        if (!m_plugin) {
            return;
        }
        // Optional hook, lowkey best-effort — only call through if the plugin
        // actually exported it. Wrapped in try/catch like init() above — this crosses the ABI
        // into arbitrary plugin code just the same, so a plugin-side throw here shouldn't be
        // allowed to terminate the process either.
        try {
            auto symbol_name = std::string{
                types::PluginRef::shared_symbol_name(3)}; // index 3 == congelado_on_ready
            if (auto it = m_plugin->m_data.find(symbol_name); it != m_plugin->m_data.end()) {
                reinterpret_cast<types::PluginReadyFn>(std::any_cast<void *>(it->second))();  // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI cast of a dlsym'd void* back to its known function pointer type
            }
        } catch (...) { // NOLINT(bugprone-empty-catch) — deliberate: never let a plugin-side throw escape across the ABI boundary
        }
    }

    /**
     * @brief Calls the attached plugin's optional `congelado_on_unload` lifecycle hook, if present.
     * @note No-op if no plugin is attached or the symbol was never resolved — like on_ready(),
     * this hook is optional per the plugin ABI.
     */
    void on_unload() noexcept {
        // No plugin attached — nothing to tear down.
        if (!m_plugin) {
            return;
        }
        // Optional hook — only call through if the plugin actually exported it. Wrapped in
        // try/catch for the same reason as on_ready() above: this crosses the ABI into
        // arbitrary plugin code, and a plugin-side throw shouldn't terminate the process.
        try {
            auto symbol_name = std::string{
                types::PluginRef::shared_symbol_name(2)}; // index 2 == congelado_on_unload
            if (auto it = m_plugin->m_data.find(symbol_name); it != m_plugin->m_data.end()) {
                reinterpret_cast<types::PluginUnloadFn>(std::any_cast<void *>(it->second))();  // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI cast of a dlsym'd void* back to its known function pointer type
            }
        } catch (...) { // NOLINT(bugprone-empty-catch) — deliberate: never let a plugin-side throw escape across the ABI boundary
        }
    }

    // ── Function dispatch ───────────────────────────────────────────────────

    /**
     * @brief Looks up a registered entry by key and invokes it with the given C-ABI args.
     * @note This is the actual cross-language dispatch entry point — args/result cross the
     * ABI as raw `CongeladoAny` arrays and get marshaled to/from `Value` via AnyConverter.
     * Any failure (not found, wrong arity, thrown exception during invoke) is reported through
     * the return code plus get_last_error(), never as a thrown exception across this boundary.
     * @param key pointer to the registered lookup key (not necessarily null-terminated).
     * @param key_len length of `key` in bytes.
     * @param args C-ABI argument array.
     * @param num_args number of entries in `args`.
     * @param result out-param written with the call's result on success; untouched on failure.
     * @return 0 on success, -1 on failure (key not found or the call threw) — check
     * get_last_error() for details.
     */
    int call(const char *key, size_t key_len, const CongeladoAny *args, int num_args,
             CongeladoAny *result) noexcept {
        try {
            // Unknown key — record why and bail before touching anything else.
            auto it = m_entries.find(std::string{key, key_len});
            if (it == m_entries.end()) {
                m_last_error = std::format("not found: {}", std::string_view{key, key_len});
                return -1;
            }
            // Convert every cross-ABI arg into a Value, dispatch to the registered
            // entry, then convert the result back out through the out-param.
            auto dispatch_args =
                std::views::iota(0, num_args) |
                std::views::transform([&](int arg_index) { return AnyConverter::from_any(args[arg_index]); }) |
                std::ranges::to<std::vector>();
            auto return_value = it->second.invoke(dispatch_args);
            *result = AnyConverter::to_any(return_value);
            return 0;
        } catch (const std::exception &exception) {
            // Arity mismatch, bad marshaling, or a plugin-side throw — report it
            // through get_last_error() instead of letting it cross the ABI boundary.
            m_last_error = exception.what();
            return -1;
        }
    }

    /**
     * @brief Gets the message from the most recent failed call().
     * @return the last error message, or the literal string `"no error"` if nothing has
     * failed yet (or `m_last_error` was never set).
     */
    [[nodiscard]] const char *get_last_error() const noexcept {
        return m_last_error.empty() ? "no error" : m_last_error.c_str();
    }

    // ── Accessors ─────────────────────────────────────────────────────────[...]

    /// @brief Gets how many entries are currently registered.
    /// @return the registered entry count.
    [[nodiscard]] std::size_t get_size() const noexcept { return m_entries.size(); }
    /// @brief Gets the runtime's generation config.
    /// @return the configured GenerationConfig.
    [[nodiscard]] const types::GenerationConfig &get_config() const noexcept { return m_cfg; }
    /// @brief Gets the attached plugin ref, if any.
    /// @return the attached PluginRef, or null if none is attached.
    [[nodiscard]] std::shared_ptr<types::PluginRef> get_plugin() const noexcept { return m_plugin; }
    /// @brief Gets the shared handle table used for map/array handles crossing the FFI.
    /// @return a reference to the runtime's HandleTable.
    [[nodiscard]] HandleTable &get_handles() noexcept { return m_handles; }

  private:
    types::GenerationConfig m_cfg;
    HandleTable m_handles;
    std::unordered_map<std::string, FnEntry> m_entries;
    std::vector<std::unique_ptr<interfaces::IBridge>> m_bridges;
    std::string m_last_error;
    std::shared_ptr<types::PluginRef> m_plugin;
};

} // namespace core::plugin
