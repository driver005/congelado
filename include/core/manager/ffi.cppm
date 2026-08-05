module;

#include <congelado/abi.h>

export module core_plugin:ffi;

import std;
import :value;
import :types;
import :plugin_ref;
import interfaces;
import core_events;
import core_ffi;

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

        auto call = [instance, key, ARG_SIZE](std::span<const Value> args) -> Value {
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

    /**
     * @brief Registers a loaded bridge plugin against this runtime, keyed by its own
     * `runtime_name()` — bridges are plain instance data here, not a global registry; each
     * `FfiRuntime` (one per opened plugin, see `PluginStore::open()`) holds its own set,
     * broadcast into it by `PluginStore::broadcast_bridge()` as bridge plugins load. No-op if
     * `bridge` is null.
     * @param bridge the bridge instance to add.
     */
    void add_bridge(std::shared_ptr<interfaces::IBridge> bridge) {
        if (bridge) {
            m_bridges[std::string{bridge->runtime_name()}] = std::move(bridge);
        }
    }

    /**
     * @brief Looks up a bridge previously added via add_bridge(), by runtime name (e.g.
     * `"python"`, `"lua"`, or any other user-registered bridge's own name).
     * @param runtime_name the runtime to look up.
     * @return the matching bridge, or `nullptr` if none was added for it — i.e. that bridge
     * plugin simply isn't loaded.
     */
    [[nodiscard]] interfaces::IBridge *get_bridge(std::string_view runtime_name) const noexcept {
        auto it = m_bridges.find(std::string{runtime_name});
        return it != m_bridges.end() ? it->second.get() : nullptr;
    }

    /**
     * @brief Finds whichever added bridge self-reports handling `script_extension` (e.g.
     * `".py"`, `".lua"`) — for a caller holding a script path and nothing else, so it never has
     * to hardcode which extension belongs to which runtime name.
     * @param script_extension the file extension to match (dot included).
     * @return the matching bridge, or `nullptr` if none of the added bridges handle it.
     */
    [[nodiscard]] interfaces::IBridge *
    find_bridge_for_extension(std::string_view script_extension) const noexcept {
        for (const auto &[name, bridge] : m_bridges) {
            if (bridge->script_extension() == script_extension) {
                return bridge.get();
            }
        }
        return nullptr;
    }

    /**
     * @brief Registers every method listed in `T`'s `core::ffi::Exported<T>` specialization as
     * a callable FnEntry bound against `core::ffi::Exported<T>::instance()`, wiring up a
     * Python/Lua bridge for each wanted runtime in `cfg`.
     * @note No C++26 static reflection here on purpose — this toolchain doesn't support it
     * (`__cpp_reflection` is undefined, no experimental flag enables it either). `T` opts in by
     * hand, listing exactly which methods to export via `Exported<T>::methods()`, the same
     * reflection-free convention `serde::Serializable<T>`/`FieldDesc` already uses for
     * serialization.
     * TODO(reflection): GCC 16.1 actually implements P2996 today via `-std=c++26
     * -freflection` (confirmed directly — `std::meta::accessible_members_of`/`^^T` both
     * compile and run) — clang doesn't yet, only via the separate bloomberg/clang-p2996
     * fork. If this project's toolchain ever moves off mainline clang for real reflection,
     * restore a `template <typename T> void register_class(...)` overload here that walks
     * `std::meta::accessible_members_of(^^T, ctx)` directly (filtering ctors/dtors/operators/
     * overloads, same as the version this replaced) instead of requiring a hand-written
     * `core::ffi::Exported<T>` — and delete `core::ffi::Exported<T>`/`MethodDesc`
     * (include/core/ffi/ffi.cppm) plus every hand-written specialization of it once nothing
     * needs the fallback anymore.
     * @tparam T a type with a `core::ffi::Exported<T>` specialization.
     * @param cfg which runtimes (Python/Lua) to build bridges for and how to configure them.
     * @param prefix namespace prefix used to build both the registered key (`prefix.method`)
     * and the bridge-facing name (`prefix_method`) — required, since there's no reflection to
     * pull `T`'s own name from.
     */
    template <core::ffi::IsExported T>
    void register_class(const types::GenerationConfig &cfg, std::string_view prefix) {
        // 1. Look up already-loaded bridges for whichever runtimes the caller wants — an open
        // set of names (cfg.get_wanted_runtimes()), not a fixed pair. No bridge added for a
        // wanted runtime means that runtime simply isn't available, same as an unregistered
        // serde format.
        std::vector<interfaces::IBridge *> bridges;
        for (const auto &runtime_name : cfg.get_wanted_runtimes()) {
            if (auto *bridge = get_bridge(runtime_name)) {
                bridges.push_back(bridge);
            }
        }

        // 2. Register every method listed in Exported<T>::methods(), bound against the
        // single shared instance the specialization itself provides.
        auto &instance = core::ffi::Exported<T>::instance();
        std::apply(
            [&](auto... method_desc) {
                (
                    [&] {
                        auto registered_key =
                            std::string{prefix} + "." + std::string{method_desc.name.string_view()};
                        auto lang_name =
                            std::string{prefix} + "_" + std::string{method_desc.name.string_view()};

                        // Create and store the FnEntry, bound against the real instance —
                        // fixes the previous reflection-era code's always-nullptr bind.
                        auto entry = FnEntry::from_method<method_desc.member>(&instance, registered_key);
                        auto invoke_function = entry.get_invoke_fn();
                        register_entry(std::move(entry));

                        // Create one FnContext per bridge (each bridge owns its own)
                        for (auto &bridge : bridges) {
                            auto fn_context = std::make_unique<FnContext>(std::any{invoke_function},
                                                                          registered_key);
                            bridge->install_method(std::move(fn_context), lang_name);
                        }
                    }(),
                    ...);
            },
            core::ffi::Exported<T>::methods());
    }

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
            core::events::publish("ffi.plugin.init_failed");
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
            core::events::publish("ffi.plugin.on_ready_failed");
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
            core::events::publish("ffi.plugin.on_unload_failed");
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
            core::events::publish("ffi.call.failed",
                                  {{"key", std::string{key, key_len}}, {"error", m_last_error}});
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

    /**
     * @brief Gets a loaded bridge's native interpreter handle (e.g. Lua's `lua_State*`, as
     * `void*` — core_plugin doesn't include `<lua.hpp>` any more than it includes `<Python.h>`,
     * bridges are plugins now). A caller running a user script in that language needs this
     * exact handle, not a freshly-created one, or it won't see the registered table/module.
     * @param runtime_name which bridge to query (e.g. `"python"`, `"lua"`).
     * @return the native handle as `void*` (the caller casts it to the concrete type it
     * expects), or `nullptr` if that bridge isn't loaded or doesn't expose one (Python's
     * interpreter is process-global, `PythonBridgePlugin` never has one to give back).
     */
    [[nodiscard]] void *get_bridge_native_handle(std::string_view runtime_name) const noexcept {
        auto *bridge = get_bridge(runtime_name);
        return bridge != nullptr ? bridge->native_handle() : nullptr;
    }

  private:
    types::GenerationConfig m_cfg;
    std::unordered_map<std::string, FnEntry> m_entries;
    std::unordered_map<std::string, std::shared_ptr<interfaces::IBridge>> m_bridges;
    std::string m_last_error;
    std::shared_ptr<types::PluginRef> m_plugin;
};

} // namespace core::plugin
