# Plugin Dependencies & Uniqueness — Design Spec

**Date:** 2026-06-03
**Status:** Approved

## Summary

Add two capabilities to the congelado plugin system:
1. **Uniqueness** — a plugin declares a type tag; only one plugin per tag loads.
2. **Dependencies** — a plugin declares required plugin names; load order is auto-sorted.

Both are declared in plugin author code (not config). The framework enforces them at startup.

---

## Section 1 — Plugin SDK

### `congelado_plugin.cppm` — two new virtual methods on `Plugin`

```cpp
// Returns a uniqueness type tag. Empty string = not unique.
// Example: "logger", "protocol", "engine"
// If two plugins share the same tag, only the first in config order loads.
[[nodiscard]] virtual std::string_view get_unique_type() const noexcept { return {}; }

// Returns names of plugins this plugin requires to be loaded before it.
// Names must match get_name() of the required plugins exactly.
// Example: { "http2", "logger" }
[[nodiscard]] virtual std::span<const std::string_view> get_requires() const noexcept { return {}; }
```

Both have default no-op implementations so existing plugins require no changes.

### `plugin.h` — three new C ABI symbols added to `CONGELADO_PLUGIN(T)`

```c
// Returns unique type tag. "" if not unique.
extern "C" const char* congelado_unique_type() noexcept;

// Returns pointer to array of required plugin name C-strings.
extern "C" const char* const* congelado_requires() noexcept;

// Returns length of the congelado_requires() array.
extern "C" size_t congelado_requires_count() noexcept;
```

`CONGELADO_PLUGIN(T)` emits these symbols automatically. They delegate to the plugin instance's `get_unique_type()` and `get_requires()`.

---

## Section 2 — FfiBridge Split

`FfiBridge::load()` is split into two methods to allow metadata inspection before activation.

### `FfiBridge::open(path, plugin_cfg)` (replaces load)
- `dlopen` the .so
- Resolve all symbols (including the three new ones)
- Read and store: name, version, caps, unique_type, requires list
- Does **not** call `congelado_on_load`
- Returns `std::expected<std::shared_ptr<FfiBridge>, LoadError>`

### `FfiBridge::activate(router_ctx, controller_ctx, leverager_ctx)`
- Builds `CongeladoHostCallbacks`
- Calls `congelado_on_load`
- Calls `discover_caps()` to populate protocol/storage pointers

### New accessors

```cpp
[[nodiscard]] std::string_view                  get_unique_type() const noexcept;
[[nodiscard]] std::span<const std::string_view> get_requires()    const noexcept;
```

### `PluginSymbols` additions

```cpp
using UniqueTypeFn    = const char *(*)() noexcept;
using RequiresFn      = const char *const *(*)() noexcept;
using RequiresCountFn = std::size_t (*)() noexcept;

UniqueTypeFn    unique_type{nullptr};
RequiresFn      requires_get{nullptr};
RequiresCountFn requires_count{nullptr};
```

All three are optional — `resolve_symbols()` probes them with `dlsym`; missing = safe defaults (empty string, null, 0). Older plugins without these symbols load unchanged.

---

## Section 3 — App::load_plugins() Rewrite

Four phases replace the current single-pass loop.

### Phase 1 — Probe
Call `FfiBridge::open()` for every plugin listed in config order. Collect into `probed: vector<pair<name, bridge>>`. Log failures but continue (same behavior as today for missing .so files).

### Phase 2 — Uniqueness Filter
```
seen_types: map<string_view, string_view>   // type_tag → first plugin name
```
For each probed plugin:
- If `unique_type` is empty → keep unconditionally
- If `unique_type` not in `seen_types` → keep, record in map
- If `unique_type` already in `seen_types` → print warning, remove from list

```
[heart] plugin 'my-logger' skipped — unique type 'logger' already claimed by 'file-logger'
```

### Phase 3 — Dependency Sort (Kahn's Algorithm)

Build:
- `name_map: map<string, bridge>` from surviving plugins
- `in_degree: map<string, int>` per plugin
- `dependents: map<string, vector<string>>` — who depends on whom

**Missing dependency check:** For each plugin, for each name in `get_requires()`:
- If name not in `name_map` → print error + `abort()`

```
[heart] plugin 'engine' requires 'http2' which is not loaded — aborting
```

**Cycle detection:** Run Kahn's algorithm. If the sorted output is smaller than the input set → cycle exists → print error + `abort()`.

```
[heart] plugin dependency cycle detected — aborting
```

Output: `sorted: vector<bridge>` in valid activation order.

### Phase 4 — Activate
Call `bridge->activate(router_ctx, controller_ctx, leverager_ctx)` for each bridge in sorted order. Then register loggers and find the first protocol — identical logic to today.

---

## Error Table

| Violation | Action |
|---|---|
| Plugin .so not found | warn + skip (existing behavior) |
| Required plugin not in config | `stderr` + `abort()` |
| Dependency cycle | `stderr` + `abort()` |
| Duplicate unique type | warn + skip second plugin |

---

## Files Changed

| File | Change |
|---|---|
| `sdk/plugin/congelado_plugin.cppm` | Add `get_unique_type()`, `get_requires()` to `Plugin` |
| `sdk/plugin/include/congelado/plugin.h` | Add three C ABI symbols to `CONGELADO_PLUGIN(T)` |
| `include/core/ffi/bridge.cppm` | Split `load()` into `open()` + `activate()`; new symbol slots + accessors |
| `include/core/heart/app.cppm` | Rewrite `load_plugins()` with four-phase algorithm |

No changes to `IProtocol`, config types, or any plugin consumer code.
