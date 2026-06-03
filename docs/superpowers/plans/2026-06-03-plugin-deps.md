# Plugin Dependencies & Uniqueness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add plugin uniqueness type-tags and dependency declarations to the congelado plugin system, with automatic topological load-order sorting.

**Architecture:** Plugins declare `get_unique_type()` and `get_requires()` as virtual methods on `congelado::Plugin`. The `CONGELADO_PLUGIN(T)` macro exposes them as C ABI symbols. `FfiBridge::load()` is split into `open()` (probe, no `on_load`) + `activate()` (closures + `on_load`). `App::load_plugins()` probes all first, filters unique-type conflicts, topologically sorts by dependencies, then activates in order.

**Tech Stack:** C++26 modules, xmake, libffi, dlopen/dlsym

---

## File Map

| File | Change |
|---|---|
| `sdk/plugin/congelado_plugin.cppm` | Add `get_unique_type()` + `get_requires()` to `Plugin` |
| `sdk/plugin/include/congelado/plugin.h` | Add 3 C ABI symbols + their entries to `CONGELADO_PLUGIN(T)` |
| `include/core/ffi/bridge.cppm` | Add symbol slots; split `load()` → `open()` + `activate()`; add metadata members + accessors |
| `include/core/manager/loader.cppm` | Rename `load()` → `open()` (probe only); remove context-pointer params |
| `include/core/heart/app.cppm` | Rewrite `load_plugins()` with 4-phase algorithm |
| `defaults/plugins/file_logger/file_logger.cc` | Add `get_unique_type() = "logger"` |
| `defaults/plugins/http2/http2.cc` | Add `get_unique_type() = "protocol"`, `get_requires() = {"FileLogger"}` |
| `defaults/plugins/engine/engine_plugin.cc` | Add `get_requires() = {"http2"}` |

---

## Task 1: Add virtual methods to `Plugin` base class

**Files:**
- Modify: `sdk/plugin/congelado_plugin.cppm`

- [ ] **Step 1: Add `get_unique_type()` and `get_requires()` to `Plugin`**

In `sdk/plugin/congelado_plugin.cppm`, replace the `Plugin` class body's existing virtual methods section (after `storage_get`) with:

```cpp
    // IMPORTANT: returned string_view::data() is used as a raw const char*.
    // Implementations MUST return a view into a string literal or stable member.
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;
    [[nodiscard]] virtual std::string_view get_version() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t capabilities() const noexcept { return 0; }

    // Returns a type tag for uniqueness enforcement. Empty string = not unique.
    // Only one plugin with a given tag loads; the first in config order wins.
    // MUST return a view into a string literal.
    [[nodiscard]] virtual std::string_view get_unique_type() const noexcept { return {}; }

    // Returns the names of plugins that must be loaded before this plugin.
    // Each name must exactly match get_name() of the required plugin.
    // MUST return a span over a static array of string literals.
    [[nodiscard]] virtual std::span<const std::string_view> get_requires() const noexcept {
        return {};
    }

    virtual void on_load(HostCallbacks const & /*host*/, ConfigView const & /*cfg*/) {}
    virtual void on_unload() {}
    virtual void logger_write(int /*level*/, std::string_view /*msg*/) noexcept {}
    virtual void *protocol_get() noexcept { return nullptr; }
    // Returns interfaces::IDatabase* cast to void*. Override with CONGELADO_CAP_STORAGE.
    virtual void *storage_get() noexcept { return nullptr; }
```

- [ ] **Step 2: Build to verify SDK compiles**

```bash
xmake build congelado_plugin 2>&1 | tail -5
```
Expected: no errors. The two new methods have default impls so nothing downstream breaks.

- [ ] **Step 3: Commit**

```bash
git add sdk/plugin/congelado_plugin.cppm
git commit -m "feat(sdk): add get_unique_type() and get_requires() to Plugin base"
```

---

## Task 2: Add C ABI symbols to `CONGELADO_PLUGIN(T)` macro

**Files:**
- Modify: `sdk/plugin/include/congelado/plugin.h`

- [ ] **Step 1: Append three new symbol generators to the macro**

At the end of `CONGELADO_PLUGIN(T)`, before the closing backslash of `congelado_storage_get`, add:

```cpp
    extern "C" const char *congelado_unique_type() noexcept {                                \
        if (s_plugin == nullptr) s_plugin = new T{};                                         \
        return s_plugin->get_unique_type().data();                                            \
    }                                                                                         \
    extern "C" const char *const *congelado_requires() noexcept {                            \
        if (s_plugin == nullptr) s_plugin = new T{};                                         \
        static std::vector<const char *> s_cache;                                            \
        s_cache.clear();                                                                      \
        for (auto sv : s_plugin->get_requires())                                             \
            s_cache.push_back(sv.data());                                                     \
        return s_cache.data();                                                                \
    }                                                                                         \
    extern "C" std::size_t congelado_requires_count() noexcept {                             \
        if (s_plugin == nullptr) s_plugin = new T{};                                         \
        return s_plugin->get_requires().size();                                               \
    }
```

The full end of the macro (last three `extern "C"` blocks) must look like:

```cpp
    extern "C" void *congelado_protocol_get() noexcept {                                                 \
        return s_plugin != nullptr ? s_plugin->protocol_get() : nullptr;                                 \
    }                                                                                                     \
    extern "C" void *congelado_storage_get() noexcept {                                                  \
        return s_plugin != nullptr ? s_plugin->storage_get() : nullptr;                                  \
    }                                                                                                     \
    extern "C" const char *congelado_unique_type() noexcept {                                            \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_unique_type().data();                                                        \
    }                                                                                                     \
    extern "C" const char *const *congelado_requires() noexcept {                                        \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        static std::vector<const char *> s_cache; /* NOLINT */                                           \
        s_cache.clear();                                                                                  \
        for (auto sv : s_plugin->get_requires())                                                         \
            s_cache.push_back(sv.data());                                                                 \
        return s_cache.data();                                                                            \
    }                                                                                                     \
    extern "C" std::size_t congelado_requires_count() noexcept {                                         \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_requires().size();                                                           \
    }
```

- [ ] **Step 2: Build all default plugins**

```bash
xmake build file_logger http2 engine 2>&1 | tail -10
```
Expected: all three compile with no errors. New symbols are emitted automatically.

- [ ] **Step 3: Commit**

```bash
git add sdk/plugin/include/congelado/plugin.h
git commit -m "feat(sdk): expose congelado_unique_type/requires/requires_count via macro"
```

---

## Task 3: Extend `FfiBridge` — new symbol slots, metadata storage, accessors

**Files:**
- Modify: `include/core/ffi/bridge.cppm`

This task adds the metadata infrastructure. The load/activate split is Task 4.

- [ ] **Step 1: Add three new function-pointer types to `PluginSymbols`**

In the `PluginSymbols` struct (private section of `core::ffi`), after `StorageGetFn`:

```cpp
    using UniqueTypeFn    = const char *(*)() noexcept;
    using RequiresFn      = const char *const *(*)() noexcept;
    using RequiresCountFn = std::size_t (*)() noexcept;

    UniqueTypeFn    unique_type{nullptr};
    RequiresFn      requires_get{nullptr};
    RequiresCountFn requires_count{nullptr};
```

- [ ] **Step 2: Probe the three new symbols in `resolve_symbols()`**

After the line `m_syms.storage_get = probe<PluginSymbols::StorageGetFn>("congelado_storage_get");`, add:

```cpp
        m_syms.unique_type    = probe<PluginSymbols::UniqueTypeFn>("congelado_unique_type");
        m_syms.requires_get   = probe<PluginSymbols::RequiresFn>("congelado_requires");
        m_syms.requires_count = probe<PluginSymbols::RequiresCountFn>("congelado_requires_count");
```

All three are optional — `probe()` returns `nullptr` if symbol is absent, which is the safe default.

- [ ] **Step 3: Add metadata members to `FfiBridge`**

In the `// ── Members ───` section, after `m_lib_name`:

```cpp
    std::string              m_unique_type;
    std::vector<std::string> m_requires;
```

- [ ] **Step 4: Add a `read_metadata()` private method**

Add this private method to `FfiBridge`:

```cpp
    void read_metadata() noexcept {
        m_unique_type = (m_syms.unique_type != nullptr) ? std::string{m_syms.unique_type()} : "";

        if (m_syms.requires_count != nullptr && m_syms.requires_get != nullptr) {
            const auto count = m_syms.requires_count();
            const auto *arr  = m_syms.requires_get();
            if (arr != nullptr) {
                m_requires.reserve(count);
                for (std::size_t i = 0; i < count; ++i)
                    m_requires.emplace_back(arr[i]);
            }
        }
    }
```

- [ ] **Step 5: Add public accessors**

In the `export namespace core::ffi` public section of `FfiBridge`, after `get_name()`:

```cpp
    [[nodiscard]] std::string_view          get_unique_type() const noexcept { return m_unique_type; }
    [[nodiscard]] std::span<const std::string> get_requires() const noexcept { return m_requires; }
```

- [ ] **Step 6: Build to verify**

```bash
xmake build congelado 2>&1 | tail -10
```
Expected: compiles. `read_metadata()` is defined but not yet called — that's fine.

- [ ] **Step 7: Commit**

```bash
git add include/core/ffi/bridge.cppm
git commit -m "feat(ffi): add unique_type/requires symbol slots, metadata members, and accessors"
```

---

## Task 4: Split `FfiBridge::load()` into `open()` + `activate()`

**Files:**
- Modify: `include/core/ffi/bridge.cppm`

- [ ] **Step 1: Add `open()` static method — probe only, no `on_load`**

Replace the existing `load()` static method with `open()`. The full replacement:

```cpp
    // Phase 1 of the two-phase load. Opens the .so, resolves symbols, reads metadata.
    // Does NOT call congelado_on_load. Call activate() after sorting.
    [[nodiscard]] static std::expected<std::shared_ptr<FfiBridge>, LoadError>
    open(const std::filesystem::path &path,
         const core::config::PluginConfig *plugin_cfg = nullptr) {
        void *lib = open_lib(path);
        if (lib == nullptr)
            return std::unexpected(LoadError{std::format("dlopen failed: {}", path.string())});

        auto bridge = std::shared_ptr<FfiBridge>(new FfiBridge{lib});

        if (auto err = bridge->resolve_symbols(); !err.empty())
            return std::unexpected(LoadError{std::move(err)});

        bridge->m_lib_name = bridge->m_syms.name();
        bridge->build_config_view(plugin_cfg);
        bridge->read_metadata();
        return bridge;
    }
```

- [ ] **Step 2: Add `activate()` instance method — closures + `on_load` + `discover_caps`**

Add this public method to `FfiBridge`:

```cpp
    // Phase 2. Builds libffi closures, calls congelado_on_load, discovers caps.
    // Must be called exactly once per bridge, after open().
    void activate(void *router_ctx = nullptr, void *controller_ctx = nullptr,
                  void *leverager_ctx = nullptr) {
        try {
            m_log_closure = std::make_unique<Closure>(
                std::initializer_list<ffi_type *>{&ffi_type_pointer, &ffi_type_sint,
                                                  &ffi_type_pointer, size_ffi_type()},
                &FfiBridge::log_thunk, this);
            m_sched_closure =
                std::make_unique<Closure>(std::initializer_list<ffi_type *>{&ffi_type_pointer},
                                          &FfiBridge::schedule_thunk, this);
        } catch (const std::exception &ex) {
            std::println(stderr, "[ffi::{}] closure setup failed: {}", m_lib_name, ex.what());
            return;
        }

        CongeladoHostCallbacks callbacks{
            .log      = reinterpret_cast<congelado_log_fn>(m_log_closure->get()),
            .schedule = reinterpret_cast<congelado_sched_fn>(m_sched_closure->get()),
            .router_ctx      = router_ctx,
            .controller_ctx  = controller_ctx,
            .leverager_ctx   = leverager_ctx,
            .ctx             = this,
        };

        if (m_syms.on_load != nullptr)
            m_syms.on_load(&callbacks, &m_cfg_view);

        discover_caps();
    }
```

- [ ] **Step 3: Update the doc comment on `FfiBridge`**

Replace the old loading-sequence comment with:

```cpp
// RAII wrapper around one loaded plugin .so.
//
// Two-phase load:
//   open()     — dlopen + resolve symbols + read metadata (name, unique_type, requires)
//   activate() — build libffi closures, call congelado_on_load, discover caps
```

- [ ] **Step 4: Build to verify**

```bash
xmake build congelado 2>&1 | tail -10
```
Expected: `open()` exists, `activate()` exists. `loader.cppm` still calls the old `load()` — it will fail to compile. Proceed to Task 5 immediately to fix it.

---

## Task 5: Update `loader.cppm` — expose `open()`, remove context params

**Files:**
- Modify: `include/core/manager/loader.cppm`

- [ ] **Step 1: Change `load()` to `open()` and remove context-pointer parameters**

Replace the entire function in `loader.cppm` with:

```cpp
export module core_plugin:loader;

import std;
import core_ffi;
import core_config;
import :handle;

export namespace core::plugin {

// Probe phase: opens the .so and reads metadata. Does NOT call on_load.
// Call bridge->activate() after dependency sorting.
[[nodiscard]]
inline std::expected<PluginHandle, LoadError>
open(const std::filesystem::path &path,
     const core::config::PluginConfig *plugin_cfg = nullptr) {
    return core::ffi::FfiBridge::open(path, plugin_cfg);
}

} // namespace core::plugin
```

- [ ] **Step 2: Build to verify**

```bash
xmake build congelado 2>&1 | tail -10
```
Expected: `loader.cppm` compiles. `app.cppm` will fail because it still calls `core::plugin::load()`. Proceed to Task 6.

---

## Task 6: Rewrite `App::load_plugins()` — four-phase algorithm

**Files:**
- Modify: `include/core/heart/app.cppm`

- [ ] **Step 1: Replace `load_plugins()` with the four-phase implementation**

Replace the entire `load_plugins()` method body. The full new method:

```cpp
    // Probes, filters, sorts, and activates all plugins from config.
    // Phase 1: probe (open .so, read metadata).
    // Phase 2: uniqueness filter — skip duplicates by type tag.
    // Phase 3: dependency sort (Kahn's algorithm) — abort on missing dep or cycle.
    // Phase 4: activate in sorted order — register loggers, find protocol.
    // Returns true if any logger plugin registered.
    bool load_plugins(const core::config::Config &cfg, AppContext &ctx,
                      std::vector<core::plugin::PluginHandle> &handles,
                      std::shared_ptr<interfaces::IProtocol> &proto) {
        // ── Phase 1: probe ────────────────────────────────────────────────────
        std::vector<core::plugin::PluginHandle> probed;

        for (auto &[name, plugin_cfg] : cfg.get_plugins()) {
#if defined(_WIN32)
            auto so = m_plugin_dir / (name + ".dll");
#else
            auto so = m_plugin_dir / ("lib" + name + ".so");
#endif
            if (!std::filesystem::exists(so)) {
                std::println(stderr, "[heart] plugin '{}' not found at '{}'", name, so.string());
                continue;
            }

            auto result = core::plugin::open(so, &plugin_cfg);
            if (!result) {
                std::println(stderr, "[heart] plugin '{}' failed to open: {}", name,
                             result.error().get_detail());
                continue;
            }

            probed.push_back(std::move(*result));
        }

        // ── Phase 2: uniqueness filter ────────────────────────────────────────
        std::unordered_map<std::string, std::string> seen_types; // type_tag → first plugin name
        std::vector<core::plugin::PluginHandle> surviving;

        for (auto &bridge : probed) {
            auto unique_type = std::string{bridge->get_unique_type()};
            if (unique_type.empty()) {
                surviving.push_back(bridge);
                continue;
            }
            if (!seen_types.contains(unique_type)) {
                seen_types[unique_type] = std::string{bridge->get_name()};
                surviving.push_back(bridge);
            } else {
                std::println(stderr,
                             "[heart] plugin '{}' skipped — unique type '{}' already claimed by '{}'",
                             bridge->get_name(), unique_type, seen_types[unique_type]);
            }
        }

        // ── Phase 3: dependency sort (Kahn's algorithm) ───────────────────────
        std::unordered_map<std::string, core::plugin::PluginHandle> name_map;
        for (auto &bridge : surviving)
            name_map[std::string{bridge->get_name()}] = bridge;

        // Verify all declared requirements are present.
        for (auto &bridge : surviving) {
            for (auto &req : bridge->get_requires()) {
                if (!name_map.contains(req)) {
                    std::println(stderr,
                                 "[heart] plugin '{}' requires '{}' which is not loaded — aborting",
                                 bridge->get_name(), req);
                    std::abort();
                }
            }
        }

        // Build in-degree and adjacency list.
        std::unordered_map<std::string, int>                     in_degree;
        std::unordered_map<std::string, std::vector<std::string>> dependents;

        for (auto &bridge : surviving) {
            auto name = std::string{bridge->get_name()};
            in_degree.try_emplace(name, 0);
            for (auto &req : bridge->get_requires()) {
                dependents[req].push_back(name);
                ++in_degree[name];
            }
        }

        std::queue<std::string> ready;
        for (auto &[name, deg] : in_degree) {
            if (deg == 0)
                ready.push(name);
        }

        std::vector<core::plugin::PluginHandle> sorted;
        sorted.reserve(surviving.size());
        while (!ready.empty()) {
            auto name = ready.front();
            ready.pop();
            sorted.push_back(name_map[name]);
            for (auto &dependent : dependents[name]) {
                if (--in_degree[dependent] == 0)
                    ready.push(dependent);
            }
        }

        if (sorted.size() != surviving.size()) {
            std::println(stderr, "[heart] plugin dependency cycle detected — aborting");
            std::abort();
        }

        // ── Phase 4: activate ─────────────────────────────────────────────────
        auto *router_ctx      = ctx.get_router();
        auto *controller_ctx  = &ctx.get_contract_group();
        auto *leverager_ctx   = &ctx.get_leverager();

        for (auto &bridge : sorted) {
            bridge->activate(router_ctx, controller_ctx, leverager_ctx);
            handles.push_back(bridge);
            std::println("[heart] loaded plugin '{}'", bridge->get_name());

            if (auto logger = core::plugin::make_logger(bridge)) {
                core::logger::LoggerRegistry::register_logger(std::move(logger));
            }
            if (!proto) {
                proto = core::plugin::make_protocol(bridge);
            }
        }

        return core::logger::LoggerRegistry::has_logger();
    }
```

- [ ] **Step 2: Build the full project**

```bash
xmake build 2>&1 | tail -15
```
Expected: full build succeeds.

- [ ] **Step 3: Commit**

```bash
git add include/core/ffi/bridge.cppm include/core/manager/loader.cppm include/core/heart/app.cppm
git commit -m "feat(heart): four-phase plugin load with uniqueness filter and dependency sort"
```

---

## Task 7: Wire up default plugins with metadata

**Files:**
- Modify: `defaults/plugins/file_logger/file_logger.cc`
- Modify: `defaults/plugins/http2/http2.cc`
- Modify: `defaults/plugins/engine/engine_plugin.cc`

- [ ] **Step 1: Add `get_unique_type()` to `FileLoggerPlugin`**

In `defaults/plugins/file_logger/file_logger.cc`, in the `FileLoggerPlugin` class after `get_version()`:

```cpp
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "logger"; }
```

- [ ] **Step 2: Add `get_unique_type()` and `get_requires()` to `Http2Plugin`**

In `defaults/plugins/http2/http2.cc`, in the `Http2Plugin` class after `get_version()`:

```cpp
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "protocol"; }

    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        static constexpr std::string_view reqs[] = {"FileLogger"};
        return reqs;
    }
```

- [ ] **Step 3: Add `get_requires()` to `EnginePlugin`**

In `defaults/plugins/engine/engine_plugin.cc`, in the `EnginePlugin` class after `get_version()`:

```cpp
    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        static constexpr std::string_view reqs[] = {"http2"};
        return reqs;
    }
```

- [ ] **Step 4: Build all plugins**

```bash
xmake build file_logger http2 engine 2>&1 | tail -10
```
Expected: all three compile cleanly.

- [ ] **Step 5: Commit**

```bash
git add defaults/plugins/file_logger/file_logger.cc \
        defaults/plugins/http2/http2.cc \
        defaults/plugins/engine/engine_plugin.cc
git commit -m "feat(plugins): declare unique types and dependencies in default plugins"
```

---

## Validation Checklist

After all tasks, manually verify the following behaviours by reading log output:

- [ ] **Normal load**: three plugins with `FileLogger` → `http2` → `engine` dep chain load in that order regardless of config order.
- [ ] **Duplicate unique type**: add a second `[plugins.other-logger]` entry pointing to a copy of `libFileLogger.so`; expect `[heart] plugin 'other-logger' skipped — unique type 'logger' already claimed by 'FileLogger'`.
- [ ] **Missing dep**: remove `FileLogger` from config while keeping `http2`; expect `[heart] plugin 'http2' requires 'FileLogger' which is not loaded — aborting`.
- [ ] **Cycle**: artificially add `get_requires() = {"engine"}` to `FileLogger` (temp edit), rebuild; expect `[heart] plugin dependency cycle detected — aborting`.
