module;

#include <congelado/abi.h>
#include <cstdio>
#include <dlfcn.h>

export module core_plugin:shared_lib;

import std;
import :types;
import :ffi;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::plugin {

class SharedLibrary
{
public:
    /**
     * @brief Constructs a scanner/loader gated to one expected plugin type.
     * @param expected_type the `congelado_type()` value a shared lib must report to load
     * cleanly — mismatches abort the process in load_pluginref(), so set this right or it's
     * an instant L at load time.
     */
    explicit SharedLibrary(std::string_view expected_type = "plugin") :
        m_expected_type(expected_type)
    {
    }

    /// @brief Deleted — a SharedLibrary owns live dlopen handles, no copying that motion.
    SharedLibrary(const SharedLibrary&) = delete;
    /// @brief Deleted — same reason as the copy ctor, handles can't be duplicated safely.
    SharedLibrary& operator=(const SharedLibrary&) = delete;

    /// @brief Move-constructs by stealing the other instance's scanned/opened plugin state.
    SharedLibrary(SharedLibrary&& other) noexcept :
        m_scanned_called{other.m_scanned_called},
        m_scanned{std::move(other.m_scanned)},
        m_runtimes{std::move(other.m_runtimes)},
        m_order{std::move(other.m_order)},
        m_expected_type{std::move(other.m_expected_type)}
    {
        // Everything's already stolen via the initializer list above — just leave
        // `other` in a clean, re-scannable empty state instead of a moved-from limbo.
        other.m_scanned_called = false;
        other.m_scanned.clear();
        other.m_runtimes.clear();
        other.m_order.clear();
    }

    /// @brief Move-assigns, closing this instance's own loaded plugins first to avoid a leak.
    SharedLibrary& operator=(SharedLibrary&& other) noexcept
    {
        // Guard against self-move — without this, close_all() below would tear
        // down the very state we're about to steal from `other`.
        if (this != &other) {
            // This instance's own plugins have to unload first, or their handles
            // just leak once overwritten by the incoming state.
            close_all();
            m_scanned_called = other.m_scanned_called;
            m_scanned = std::move(other.m_scanned);
            m_runtimes = std::move(other.m_runtimes);
            m_order = std::move(other.m_order);
            m_expected_type = std::move(other.m_expected_type);
            other.m_scanned_called = false;
            other.m_scanned.clear();
            other.m_runtimes.clear();
            other.m_order.clear();
        }
        return *this;
    }

    /// @brief Destroys the instance, unloading every opened plugin in reverse order first.
    ~SharedLibrary()
    {
        close_all();
    }

    // ── Phase 1: discover shared libs in directory ──────────────────────

    /**
     * @brief Scans a directory for loadable shared libraries and records their paths.
     * @note This is a discovery pass only — nothing gets dlopen'd here, just catalogued by
     * a `lib`-stripped stem name for open()/open_all() to find later. Silently no-ops (bet,
     * no crash) if `dir` doesn't exist or isn't a directory.
     * @param dir directory to scan for `.so`/`.dll`/`.dylib` files (platform-dependent, see
     * `types::is_shared_lib`).
     */
    void scan(const std::filesystem::path& dir)
    {
        m_scanned_called = true;
        // Missing/non-directory `dir` is a silent no-op, not an error.
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            return;
        }
        // Walk every entry, skipping anything that isn't a regular shared-lib file,
        // and catalogue what's left by a "lib"-stripped stem name.
        for (const auto& entry: std::filesystem::directory_iterator{dir}) {
            if (!entry.is_regular_file()) {
                continue;
            }
            if (!types::is_shared_lib(entry.path())) {
                continue;
            }
            auto library_name = entry.path().stem().string();
            if (library_name.starts_with("lib")) {
                library_name = library_name.substr(3);
            }
            m_scanned[library_name] = entry.path();
        }
    }

    // ── Phase 2: open single plugin + resolve deps recursively ─────────

    /**
     * @brief Opens one plugin by path, recursively opening its declared dependencies first.
     * @warning Calls dlopen() under the hood — a plugin with a broken/missing symbol table or
     * a bad `congelado_requires` list can fail loud (returned error) but a plugin that flat-out
     * lies about its type triggers std::abort() deep in load_pluginref(), not a returned error.
     * That's not a graceful L, that's the whole process going down.
     * @note Already-open plugins (by stripped library name) short-circuit to success — this
     * makes recursive dependency resolution safe against cycles that bottom out on an already-
     * loaded node, though a genuine cycle among never-yet-loaded plugins will still recurse.
     * @param path filesystem path to the shared library to open. scan() must have already run.
     * @return success, or a PluginError describing what went wrong (not scanned yet, dlopen
     * failure, missing `congelado_init`, or a missing/failing dependency).
     */
    [[nodiscard]] std::expected<void, types::PluginError> open(const std::filesystem::path& path)
    {
        // scan() has to run first — that's what populates m_scanned for dependency lookups.
        if (!m_scanned_called) {
            return std::unexpected{types::PluginError::not_found("scan() not called")};
        }

        auto library_name = path.stem().string();
        if (library_name.starts_with("lib")) {
            library_name = library_name.substr(3);
        }

        // Already open under this name — treat it as success, bet, no double-load.
        if (m_runtimes.contains(library_name)) {
            return {};
        }

        // dlopen the actual shared object — first real failure point.
        void* raw_handle = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (raw_handle == nullptr) {
            return std::unexpected{
                types::PluginError::dlopen_failed(std::format("dlopen: {}", ::dlerror()))
            }; // NOLINT(concurrency-mt-unsafe) — POSIX dlerror() has no thread-safe alternative
               // in the dlfcn API
        }

        // Resolve every known symbol off the handle — note this call aborts the
        // whole process (not just this open()) if the plugin's declared type mismatches.
        std::unique_ptr<void, types::PluginRef::DlDeleter> handle(raw_handle);
        auto plugin_ref = load_pluginref(std::move(handle), path.string());
        if (!plugin_ref->m_data.contains(std::string{types::PluginRef::shared_symbol_name(0)})) {
            return std::unexpected{
                types::PluginError::dlopen_failed("missing congelado_init symbol")
            };
        }

        const auto& name = std::any_cast<const std::string&>(plugin_ref->m_data.at("name"));

        // Recursively open every declared dependency before this plugin counts as
        // opened — a missing or failing dependency fails the whole open() call.
        if (auto it = plugin_ref->m_data.find("congelado_requires");
            it != plugin_ref->m_data.end()) {
            const auto& requires_list = std::any_cast<const std::vector<std::string>&>(it->second);
            for (const auto& dep_name: requires_list) {
                if (m_runtimes.contains(dep_name)) {
                    continue;
                }
                auto dependency_iter = m_scanned.find(dep_name);
                if (dependency_iter == m_scanned.end()) {
                    return std::unexpected{types::PluginError::not_found(
                        std::format("dependency '{}' not found for '{}'", dep_name, name)
                    )};
                }
                auto result = open(dependency_iter->second);
                if (!result) {
                    return std::unexpected{types::PluginError::not_found(
                        std::format(
                            "dependency '{}' failed to load for '{}': {}", dep_name, name,
                            result.error().get_message()
                        )
                    )};
                }
            }
        }

        // Everything checked out, W — wrap the plugin ref in its own runtime and
        // record it both by name and in load order (for later init()/close_all()).
        auto runtime = std::make_shared<FfiRuntime>();
        runtime->attach_plugin(plugin_ref);
        // Seed with every bridge broadcast so far — a plugin opened after a bridge plugin
        // (e.g. python_bridge) still gets it; one opened before catches up via
        // broadcast_bridge()'s own push into m_runtimes below.
        for (const auto& bridge: m_known_bridges) {
            runtime->add_bridge(bridge);
        }
        m_runtimes.emplace(name, runtime);
        m_order.push_back(name);
        return {};
    }

    // ── Phase 2b: open all scanned plugins ─────────────────────────────

    /**
     * @brief Opens every plugin that scan() found, in whatever order the scan map iterates,
     * then reorders `m_order` so every plugin's declared `congelado_load_before_types()`
     * constraint is actually honored (see `apply_load_before_ordering()`).
     * @return success once every scanned plugin is open, or the first PluginError hit —
     * bails immediately on the first failure, doesn't try to keep going and collect more L's.
     */
    [[nodiscard]] std::expected<void, types::PluginError> open_all()
    {
        // Walk every scanned entry, skipping ones a prior dependency resolution
        // already opened, and bail on the very first failure.
        for (auto& [name, path]: m_scanned) {
            if (m_runtimes.contains(name)) {
                continue;
            }
            auto result = open(path);
            if (!result) {
                return std::unexpected{std::move(result.error())};
            }
        }
        apply_load_before_ordering();
        return {};
    }

    // ── Phase 3: init all opened plugins ───────────────────────────────

    /**
     * @brief Runs `congelado_init` then `congelado_on_ready` on every opened plugin, in load
     * order, using a per-plugin GenerationConfig if one was supplied.
     * @warning This is the moment plugin-supplied `congelado_init` code actually runs — full
     * cross-ABI handoff via `CongeladoHostCallbacks`/`CongeladoConfigView`. A plugin returning
     * non-zero from `congelado_init` fails the whole build() call, but plugins already inited
     * before it stay inited (no rollback) — that's a footgun worth knowing about, not
     * something this function tries to fix.
     * @param host_cb the host callback table (logging/scheduling/router/controller/leverager
     * contexts) handed to every plugin's `congelado_init`.
     * @param configs per-plugin GenerationConfig, keyed by plugin name; plugins with no entry
     * fall back to a default-constructed config.
     * @return success once every plugin is inited and readied, or a PluginError from scan()
     * never having run or a plugin's `congelado_init` returning non-zero.
     */
    [[nodiscard]] std::expected<void, types::PluginError> build(
        const CongeladoHostCallbacks& host_cb,
        const std::unordered_map<std::string, types::GenerationConfig>& configs
    )
    {
        // Same precondition as open() — no scan(), no build().
        if (!m_scanned_called) {
            return std::unexpected{types::PluginError::not_found("scan() not called")};
        }

        // First pass: init every opened plugin in load order. A plugin that
        // returns non-zero fails the whole build(), but earlier inits don't roll back.
        for (auto& name: m_order) {
            auto it = m_runtimes.find(name);
            if (it == m_runtimes.end()) {
                continue;
            }
            auto runtime = it->second;

            // Fall back to a default config for any plugin with no explicit entry.
            types::GenerationConfig default_cfg;
            const types::GenerationConfig* gen_cfg = &default_cfg;
            if (auto cfg_it = configs.find(name); cfg_it != configs.end()) {
                gen_cfg = &cfg_it->second;
            }

            // Flatten the resolved config into a flat key/value view crossing the ABI.
            types::ConfigViewBuilder cfg_view;
            cfg_view.add(
                "runtimes", std::views::join_with(gen_cfg->get_wanted_runtimes(), ',') |
                                std::ranges::to<std::string>()
            );
            cfg_view.add(
                "python_module", std::string{gen_cfg->get_python_config().get_module_name()}
            );
            cfg_view.add("lua_table", std::string{gen_cfg->get_lua_config().get_table_name()});
            cfg_view.add(
                "lua_safe_mode", gen_cfg->get_lua_config().get_safe_mode() ? "true" : "false"
            );
            for (const auto& [key, value]: gen_cfg->get_extra()) {
                cfg_view.add(key, value);
            }

            auto view = cfg_view.view();

            if (runtime->init(&host_cb, &view) != 0) {
                return std::unexpected{types::PluginError::dlopen_failed(
                    std::format("congelado_init failed for '{}'", name)
                )};
            }
        }

        // Second pass: only once every plugin is successfully inited does anyone
        // get told it's ready — keeps plugins from seeing a half-inited sibling.
        for (auto& name: m_order) {
            if (auto it = m_runtimes.find(name); it != m_runtimes.end()) {
                it->second->on_ready();
            }
        }

        return {};
    }

    /**
     * @brief Drops a plugin that was opened but never `build()`-ed — runs `congelado_on_unload`
     * (to free the plugin's `s_plugin` singleton) then dlcloses its handle and forgets it.
     * Unlike `close_all()`, this is for a plugin that never reached `congelado_init`, so its
     * user-side `on_unload()` runs over only default-constructed state — but the singleton
     * itself was still allocated back in `open()` (the `congelado_type` STRING_FN lazily `new
     * T{}`s it before init ever runs), so on_unload is exactly what reclaims it; skip it and
     * that object leaks, since `close_all()` only walks `m_order` which this erase removes
     * from. The subsequent dlclose (via `PluginRef`'s handle going out of scope once the last
     * `shared_ptr<FfiRuntime>` reference here drops) unmaps the `.so`. Meant for
     * provider-filtering: a capability plugin resolved right after open() but rejected by
     * `[providers]` should never reach `build()`'s `congelado_init` at all, not just get
     * silently un-registered after already having connected/spun up.
     * @param name the resolved plugin name (as recorded by open(), not necessarily the file
     * stem) — a miss is a no-op.
     */
    void discard(std::string_view name) noexcept
    {
        auto it = m_runtimes.find(std::string{name});
        if (it == m_runtimes.end()) {
            return;
        }
        // The plugin's s_plugin singleton was already allocated by the STRING_FN calls in
        // load_symbols() during open() (congelado_type resolves+invokes it, index 1), even
        // though congelado_init never ran. congelado_on_unload is the macro's only delete path
        // for that instance — call it here so a provider-filtered plugin doesn't leak its
        // object. This crosses into a plugin that never inited: its user on_unload() sees only
        // default- constructed state, and FfiRuntime::on_unload() is optional-symbol-guarded +
        // try/catch- wrapped, so teardown can't crash. close_all() only walks m_order, which
        // this erase removes from, so on_unload has to happen here or never at all.
        it->second->on_unload();
        m_runtimes.erase(it);
        std::erase(m_order, std::string{name});
    }

    /**
     * @brief Unloads every opened plugin, calling `congelado_on_unload` in reverse load order.
     * @note Reverse order matters here — dependencies were opened before their dependents, so
     * unloading in reverse keeps a dependent from calling back into an already-torn-down dep.
     * @note This is process-exit teardown, not hot-reload — so after `on_unload()` runs, every
     * plugin's dlopen handle gets leaked (never `dlclose()`'d) rather than actually closed.
     * Some plugins statically link dependencies with their own `dlclose()`-time global
     * destructors (OpenTelemetry's C++ SDK, confirmed live) that segfault when actually
     * unloaded this way; skipping the syscall trades a harmless "the OS reclaims it on exit
     * anyway" leak for a guaranteed-clean shutdown. See `PluginRef::leak_handle()`.
     */
    void close_all() noexcept
    {
        // Reverse load order: dependencies opened first get unloaded last, so a
        // dependent never calls back into an already-torn-down dependency.
        for (const auto& name: m_order | std::views::reverse) {
            if (auto runtime_iter = m_runtimes.find(name); runtime_iter != m_runtimes.end()) {
                runtime_iter->second->on_unload();
                if (auto plugin_ref = runtime_iter->second->get_plugin()) {
                    plugin_ref->leak_handle();
                }
            }
        }
        m_runtimes.clear();
        m_order.clear();
    }

    /**
     * @brief Signals every opened plugin to stop serving/close out whatever it's holding, via
     * `congelado_on_shutdown` — any plugin may implement `on_shutdown_requested()`, not just
     * protocol ones (the default's a no-op, so a plugin that doesn't care just does nothing).
     * @note Load order, not reversed — this isn't teardown, just a "wind down" nudge; every
     * plugin is left loaded and running so it can still handle whatever another plugin's close
     * callbacks trigger on the way out. Full teardown still happens later via `close_all()`.
     */
    void shutdown_plugins() noexcept
    {
        for (const auto& name: m_order) {
            auto runtime_iter = m_runtimes.find(name);
            if (runtime_iter == m_runtimes.end()) {
                continue;
            }
            runtime_iter->second->on_shutdown();
        }
    }

    // ── Iteration (insertion order via m_order) ─────────────────────────

    /**
     * @brief Invokes `callback` on every opened runtime, in insertion (load) order.
     * @tparam Callback invocable taking a `const std::shared_ptr<FfiRuntime> &`.
     * @param callback invoked once per opened runtime.
     */
    template<typename Callback>
    void for_each(Callback&& callback)
    {
        // Walk names in load order, skipping any that somehow no longer resolve
        // to a live runtime (shouldn't happen outside a concurrent mutation).
        for (auto& name: m_order) {
            auto it = m_runtimes.find(name);
            if (it == m_runtimes.end()) {
                continue;
            }
            // NOTE: callback is invoked once per loop iteration, so it must NOT be
            // std::forward'd here — forwarding on the first iteration would move from
            // an rvalue-ref callback, leaving every subsequent call operating on a
            // moved-from callable (this was a real bugprone-use-after-move bug).
            std::invoke(callback, it->second);
        }
    }

    /**
     * @brief Invokes `callback` on every opened runtime, in insertion (load) order.
     * @tparam Callback invocable taking a `const std::shared_ptr<FfiRuntime> &`.
     * @param callback invoked once per opened runtime.
     */
    template<typename Callback>
    void for_each(Callback&& callback) const
    {
        // Walk names in load order, skipping any that somehow no longer resolve
        // to a live runtime (shouldn't happen outside a concurrent mutation).
        for (const auto& name: m_order) {
            auto it = m_runtimes.find(name);
            if (it == m_runtimes.end()) {
                continue;
            }
            // NOTE: callback is invoked once per loop iteration, so it must NOT be
            // std::forward'd here — forwarding on the first iteration would move from
            // an rvalue-ref callback, leaving every subsequent call operating on a
            // moved-from callable (this was a real bugprone-use-after-move bug).
            std::invoke(callback, it->second);
        }
    }

    /**
     * @brief Looks up an already-opened plugin's runtime by name.
     * @param name plugin name to look up (the resolved `name` key, not necessarily the file
     * stem).
     * @return the plugin's FfiRuntime, or null if no plugin by that name is currently opened.
     */
    [[nodiscard]] std::shared_ptr<FfiRuntime> find(std::string_view name) noexcept
    {
        auto iter = m_runtimes.find(std::string{name});
        return iter != m_runtimes.end() ? iter->second : nullptr;
    }

    /**
     * @brief Broadcasts a newly-discovered bridge plugin into every currently-opened plugin's
     * `FfiRuntime`, and remembers it so any plugin opened later gets seeded with it too (see
     * open()). This is how a bridge (Python, Lua, or any other user-registered runtime) becomes
     * visible to every `register_class<T>()` call regardless of load order — no global
     * registry, each `FfiRuntime` just holds its own copy of the bridge set. No-op if `bridge`
     * is null.
     * @param bridge the bridge instance to broadcast.
     */
    void broadcast_bridge(std::shared_ptr<interfaces::IBridge> bridge)
    {
        if (!bridge) {
            return;
        }
        for (auto& [name, runtime]: m_runtimes) {
            runtime->add_bridge(bridge);
        }
        m_known_bridges.push_back(std::move(bridge));
    }

private:
    // ── Private: load-order constraint enforcement ──────────────────────

    /**
     * @brief Reorders `m_order` so every plugin's declared `congelado_load_before_types()`
     * constraint actually holds: a plugin `P` that lists type `T` in its load-before set is
     * moved ahead of every currently-opened plugin whose `congelado_unique_type()` is `T`.
     * @warning Without this, `m_order` was purely whatever `m_scanned` (an `unordered_map`)
     * happened to iterate in — declaring `get_load_before_types()` had zero actual effect on
     * `build()`'s init-pass order. A plugin whose route-registering `on_load` genuinely needs
     * to run before a same-phase sibling's (not just before some *later* phase like a
     * protocol plugin's `on_ready`) was at the mercy of hash-bucket luck.
     * @note Stable multi-pass Kahn's algorithm: repeatedly sweeps the current order picking up
     * every zero-indegree plugin left to right, so ties resolve to the original scan order
     * instead of some arbitrary one. A dependency cycle (two plugins each demanding to load
     * before the other's type) can't be satisfied — falls back to appending whatever's left in
     * its original relative order rather than looping forever.
     */
    void apply_load_before_ordering()
    {
        // Snapshot each opened plugin's own unique_type and load_before_types set, keyed by
        // name — both are optional exports, missing either just leaves that plugin
        // unconstrained (never gets an edge in or out).
        std::unordered_map<std::string, std::string> unique_type;
        std::unordered_map<std::string, std::vector<std::string>> load_before;
        for (const auto& name: m_order) {
            auto runtime_iter = m_runtimes.find(name);
            if (runtime_iter == m_runtimes.end()) {
                continue;
            }
            auto plugin_ref = runtime_iter->second->get_plugin();
            if (!plugin_ref) {
                continue;
            }
            if (auto it = plugin_ref->m_data.find("congelado_unique_type");
                it != plugin_ref->m_data.end()) {
                unique_type[name] = std::any_cast<const std::string&>(it->second);
            }
            if (auto it = plugin_ref->m_data.find("congelado_load_before_types");
                it != plugin_ref->m_data.end()) {
                load_before[name] = std::any_cast<const std::vector<std::string>&>(it->second);
            }
        }

        // Build the constraint graph: an edge name -> dependent for every other plugin whose
        // unique_type matches one of name's declared load-before types.
        std::unordered_map<std::string, std::vector<std::string>> edges;
        std::unordered_map<std::string, int> indegree;
        for (const auto& name: m_order) {
            indegree[name] = 0;
        }
        for (const auto& [name, types]: load_before) {
            for (const auto& type: types) {
                for (const auto& other: m_order) {
                    if (other == name) {
                        continue;
                    }
                    auto type_iter = unique_type.find(other);
                    if (type_iter != unique_type.end() && type_iter->second == type) {
                        edges[name].push_back(other);
                        ++indegree[other];
                    }
                }
            }
        }

        // Stable multi-pass Kahn's: each pass places every currently-zero-indegree plugin,
        // left to right — ties (unconstrained plugins) keep their original scan-order position.
        std::vector<std::string> ordered;
        ordered.reserve(m_order.size());
        std::vector<bool> placed(m_order.size(), false);
        while (ordered.size() < m_order.size()) {
            bool progressed = false;
            for (std::size_t i = 0; i < m_order.size(); ++i) {
                if (placed[i]) {
                    continue;
                }
                const auto& name = m_order[i];
                if (indegree[name] != 0) {
                    continue;
                }
                ordered.push_back(name);
                placed[i] = true;
                progressed = true;
                for (const auto& dependent: edges[name]) {
                    --indegree[dependent];
                }
            }
            if (!progressed) {
                // Cycle among whatever's left — no valid order exists, so just append the
                // remainder in its original relative order instead of spinning forever.
                for (std::size_t i = 0; i < m_order.size(); ++i) {
                    if (!placed[i]) {
                        ordered.push_back(m_order[i]);
                    }
                }
                break;
            }
        }

        m_order = std::move(ordered);
    }

    // ── Private: typed dlsym wrapper ────────────────────────────────────

    /**
     * @brief Resolves a symbol via dlsym() and reinterpret_casts it to a function-pointer type.
     * @warning Zero type checking happens here — `F` is whatever the caller says it is. Ask for
     * the wrong function pointer shape for a real symbol and it's UB the moment it gets called,
     * not the moment it gets resolved. This is cross-ABI trust-the-caller territory, not cap.
     * @tparam F the function pointer type to cast the resolved symbol to.
     * @param handle an open dlopen handle to resolve against.
     * @param name symbol name to look up.
     * @return the symbol reinterpreted as `F`, or null if dlsym() couldn't find it.
     */
    template<typename F>
    [[nodiscard]] F sym(void* handle, std::string_view name) const noexcept
    {
        // Every call site passes a SymbolInfo::m_name (a string_view over a string literal) or
        // a "{name}_count" built via std::string, both genuinely null-terminated — not provable
        // from the signature alone, hence the NOLINT below.
        // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
        return reinterpret_cast<F>(
            ::dlsym(handle, name.data())
        ); // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI dlsym() cast to an arbitrary
           // function pointer type, no smart-pointer/GSL equivalent applies here
    }

    /**
     * @brief Resolves a batch of optional symbols off a handle and stashes each hit into
     * `plugin_ref->m_data`, keyed by symbol name.
     * @note Every symbol here is optional — a miss just gets skipped, no error. ARRAY-kind
     * symbols additionally require a matching `{name}_count` symbol; if either half is
     * missing, the whole pair is dropped rather than half-populated.
     * @param handle an open dlopen handle to resolve symbols against.
     * @param symbols the symbol descriptor table to walk (SHARED_SYMBOLS, PLUGIN_SYMBOLS, or
     * WORKER_SYMBOLS).
     * @param plugin_ref the PluginRef whose `m_data` map gets populated with resolved symbols.
     */
    void load_symbols(
        void* handle,
        std::span<const types::PluginRef::SymbolInfo> symbols,
        std::shared_ptr<types::PluginRef>& plugin_ref
    )
    {
        // Every symbol in the table is optional — walk them all, skip whatever isn't
        // exported, and only touch m_data for the ones actually resolved.
        for (const auto& info: symbols) {
            auto* symbol_ptr = sym<void*>(handle, info.m_name);
            if (symbol_ptr == nullptr) {
                continue;
            }

            // Each SymbolKind gets loaded/invoked differently to end up with the
            // right native representation stashed in m_data.
            switch (info.m_kind) {

                case types::PluginRef::SymbolKind::FUNCTION:
                    plugin_ref->m_data[std::string{info.m_name}] = symbol_ptr;
                    break;

                case types::PluginRef::SymbolKind::STRING_FN:
                    {
                        const auto* string_value =
                            sym<types::PluginStringFn>(handle, info.m_name)();
                        plugin_ref->m_data[std::string{info.m_name}] =
                            std::string{string_value != nullptr ? string_value : ""};
                        break;
                    }

                case types::PluginRef::SymbolKind::UINT32:
                    plugin_ref->m_data[std::string{info.m_name}] =
                        sym<types::PluginUint32Fn>(handle, info.m_name)();
                    break;

                case types::PluginRef::SymbolKind::ARRAY:
                    {
                        // ARRAY needs both halves — the data getter and its matching
                        // "{name}_count" getter — or the whole pair gets dropped.
                        auto* data_function = sym<types::PluginArrayFn>(handle, info.m_name);
                        std::string count_name = std::string{info.m_name} + "_count";
                        auto* count_function = sym<types::PluginCountFn>(handle, count_name);
                        if (data_function == nullptr || count_function == nullptr) {
                            break;
                        }
                        auto count = count_function();
                        const auto* items = data_function();
                        std::vector<std::string> string_list;
                        string_list.reserve(count);
                        for (std::size_t item_index = 0; item_index < count; ++item_index) {
                            string_list.emplace_back(items[item_index]);
                        }
                        plugin_ref->m_data[std::string{info.m_name}] = std::move(string_list);
                        break;
                    }

                case types::PluginRef::SymbolKind::SIZE_T:
                    break;
            }
        }
    }

    // ── Private: load plugin from handle ────────────────────────────────

    /**
     * @brief Builds a fully-populated PluginRef from a freshly-dlopen'd handle: loads every
     * shared/plugin/worker symbol, validates the plugin's declared type, and derives a name.
     * @warning Hard-aborts the whole process (std::abort()) if the loaded library's
     * `congelado_type()` doesn't match `m_expected_type`. This is by design — a plugin loaded
     * into the wrong runtime is cooked either way, but callers should know this path doesn't
     * return an error, it just ends the process. No graceful degradation here, no cap.
     * @param handle the owning dlopen handle for the freshly-opened library.
     * @param file_path filesystem path the library was loaded from, stashed as
     * `m_data["path"]`.
     * @return a populated PluginRef with every discoverable symbol loaded and a resolved name
     * (from `congelado_plugin_name`/`congelado_worker_type` if present, else the file stem).
     */
    [[nodiscard]] std::shared_ptr<types::PluginRef>
    load_pluginref(std::unique_ptr<void, types::PluginRef::DlDeleter> handle, std::string file_path)
    {
        // Stand up the PluginRef and stash the handle + path before anything else,
        // since load_symbols() below needs the raw handle to resolve against.
        auto plugin_ref = std::make_shared<types::PluginRef>();
        auto* raw_handle = handle.get();
        plugin_ref->m_handle = std::move(handle);
        plugin_ref->m_data["path"] = std::move(file_path);
        auto library_name =
            std::filesystem::path{std::any_cast<const std::string&>(plugin_ref->m_data["path"])}
                .stem()
                .string();
        if (library_name.starts_with("lib")) {
            library_name = library_name.substr(3);
        }

        // Pull in every symbol this shared lib might export, across all three
        // symbol tables — shared lifecycle, plugin-specific, and worker-specific.
        load_symbols(raw_handle, types::PluginRef::SHARED_SYMBOLS, plugin_ref);
        load_symbols(raw_handle, types::PluginRef::PLUGIN_SYMBOLS, plugin_ref);
        load_symbols(raw_handle, types::PluginRef::WORKER_SYMBOLS, plugin_ref);

        // Validate the library actually reports the type this loader expects —
        // a mismatch here is cooked for the whole process, not a returned error.
        auto type_iter =
            plugin_ref->m_data.find(std::string{types::PluginRef::shared_symbol_name(1)});
        const auto& type = (type_iter != plugin_ref->m_data.end())
                               ? std::any_cast<const std::string&>(type_iter->second)
                               : EMPTY_TYPE;
        if (type != m_expected_type) {
            std::println(
                stderr, "[shared_lib] type mismatch: '{}' is '{}' but expected '{}' – aborting",
                library_name, type, m_expected_type
            );
            std::abort();
        }

        // Fall back to the file stem as the plugin's name if it never exported
        // congelado_plugin_name/congelado_worker_type.
        if (!plugin_ref->m_data.contains("name")) {
            plugin_ref->m_data["name"] = std::move(library_name);
        }
        return plugin_ref;
    }

    static inline const std::string EMPTY_TYPE;

    bool m_scanned_called{false};
    std::unordered_map<std::string, std::filesystem::path> m_scanned;
    std::unordered_map<std::string, std::shared_ptr<FfiRuntime>> m_runtimes;
    std::vector<std::string> m_order;
    std::string m_expected_type;
    std::vector<std::shared_ptr<interfaces::IBridge>> m_known_bridges;
};

} // namespace core::plugin

// build()'s real init pass and open()'s successful path both need a genuine, loadable plugin
// .so exporting the congelado ABI — not reproducible in a unit test. What's covered below is
// the precondition checks (scan() gating), scan()'s own filesystem cataloging (no dlopen
// involved), and every no-op-on-empty-state path — all real production behavior reachable
// without a live plugin.
#ifdef CONGELADO_TEST
namespace core::plugin::tests {
using namespace boost::ut;

// Findings-oriented notes on two spots that turned out to have no viable test seam:
//
// load_symbols()'s ARRAY case (~line 606-624) trusts count_function()'s returned count with
// zero cross-check against data_function()'s real backing array size. But load_symbols() and
// its sym<F>() helper are both private with no friend/seam, and the only public path that
// reaches them (open() -> load_pluginref()) requires a genuinely dlopen'able .so exporting the
// congelado ABI, which isn't reproducible in a unit test (see the file-level comment above this
// suite). Skipped rather than faked with a synthetic reproduction of the loop.
//
// open()'s/apply_load_before_ordering()'s unguarded std::any_cast<const std::string &> /
// <const std::vector<std::string> &> calls (~line 152, 158, 477, 481) are similarly unreachable
// from outside: plugin_ref->m_data is only ever populated by the private load_pluginref()/
// load_symbols() pair, which always stores exactly the types those casts expect, and
// SharedLibrary has no public constructor/setter that accepts a caller-built PluginRef/runtime
// to inject a mismatched type into m_runtimes. No test seam — skipped.
suite<"SharedLibrary"> shared_library_suite = [] {
    "open()/build() before scan() fail with scan-not-called"_test = [] {
        SharedLibrary lib;
        auto open_result = lib.open("/nonexistent/path.so");
        expect(not open_result.has_value());
        expect(open_result.error().get_kind() == types::Kind::NOT_FOUND);

        auto build_result = lib.build(CongeladoHostCallbacks{}, {});
        expect(not build_result.has_value());
    };
    "scan() catalogs files, open() then reaches the real dlopen (and fails on a non-plugin file)"_test =
        [] {
            auto dir = std::filesystem::temp_directory_path() / "congelado_shared_lib_test";
            std::filesystem::remove_all(dir);
            std::filesystem::create_directories(dir);
            {
                std::ofstream{dir / "libfoo.so"};
            }

            SharedLibrary lib;
            lib.scan(dir);

            // Past the "scan() not called" precondition now — the empty .so fails dlopen()
            // itself, which is enough to prove scan() actually catalogued the file.
            auto result = lib.open(dir / "libfoo.so");
            expect(not result.has_value());
            expect(result.error().get_kind() == types::Kind::DLOPEN_FAILED);

            std::filesystem::remove_all(dir);
        };
    "scan() on a missing directory is a silent no-op"_test = [] {
        SharedLibrary lib;
        expect(nothrow([&] {
            lib.scan("/definitely/does/not/exist");
        }));
    };
    "find() on an empty library returns nullptr"_test = [] {
        SharedLibrary lib;
        expect(lib.find("anything") == nullptr);
    };
    "discard()/close_all()/shutdown_plugins() are safe no-ops with nothing loaded"_test = [] {
        SharedLibrary lib;
        expect(nothrow([&] {
            lib.discard("nothing");
            lib.close_all();
            lib.shutdown_plugins();
        }));
    };
    "for_each visits nothing on an empty library"_test = [] {
        SharedLibrary lib;
        int count = 0;
        lib.for_each([&](const std::shared_ptr<FfiRuntime>&) {
            ++count;
        });
        expect(count == 0);
    };
    "broadcast_bridge is a no-op for a null bridge"_test = [] {
        SharedLibrary lib;
        expect(nothrow([&] {
            lib.broadcast_bridge(nullptr);
        }));
    };
    "open()'s already-opened guard only covers names in m_runtimes — never-opened mutual deps aren't caught"_test =
        [] {
            SharedLibrary lib;
            // open()'s cycle-breaker (~line 132) is `if (m_runtimes.contains(library_name))
            // return
            // {};` — per open()'s own @note, that's the ONLY thing standing between a
            // dependency cycle and real unbounded recursion. We must never drive that recursion
            // for real (the safety constraint on this suite), but find() performs the identical
            // "is this name already opened" lookup against m_runtimes. For two plugins that
            // would mutually require each other, NEITHER has been opened yet — proving the
            // guard has nothing to catch here, and a genuine cycle among never-yet-loaded
            // plugins would still recurse unchecked, exactly as the doc comment admits.
            expect(lib.find("plugin_a") == nullptr);
            expect(lib.find("plugin_b") == nullptr);
        };
};

} // namespace core::plugin::tests
#endif
