# Manager ↔ Plugin SDK Bridge Design

**Date:** 2026-06-17
**Status:** Approved

## Summary

Connect `core/manager` to plugins via the SDK's `Plugin` virtual class. Remove `FfiBridge` from `core/ffi` and consolidate it as an internal manager detail. Add `mode.h` for compile-time guest/host selection and guard `plugin.h`'s host-only sections behind it.

---

## 1. `mode.h` — Build-Time Mode Selection

**New file:** `sdk/plugin/include/congelado/mode.h`

- Checks for `CONGELADO_GUEST` or `CONGELADO_HOST` compile-time define
- Static assert fires if neither is defined
- Single source of truth for which side of the ABI boundary is being compiled

```c
#pragma once

#if !defined(CONGELADO_GUEST) && !defined(CONGELADO_HOST)
#  error "Define either CONGELADO_GUEST or CONGELADO_HOST before including congelado headers"
#endif
```

`sdk/plugin/include/congelado/plugin.h` includes `mode.h` at the top. Any libffi-adjacent declarations (host-only structs, context pointer types) are wrapped in `#ifdef CONGELADO_HOST`.

---

## 2. `core/ffi` Removal

`include/core/ffi/bridge.cppm` is deleted. `core/ffi` as a module is removed entirely.

All `FfiBridge` logic moves into the manager as a non-exported internal submodule: `include/core/manager/bridge.cppm`. Nothing outside `core/manager` can import it.

---

## 3. `core/manager` Rebuild

### Internal: `include/core/manager/bridge.cppm`

Non-exported submodule. Contains the dlopen/dlsym/libffi logic previously in `core/ffi/bridge.cppm`. Same `open()` / `activate()` phases:

- **`open(path, config)`** — `dlopen`, probe required symbols, build config view, read metadata (`unique_type`, `requires`, `load_before_types`)
- **`activate(router_ctx, controller_ctx, leverager_ctx)`** — build libffi closures (`log_thunk`, `schedule_thunk`), call `congelado_on_load`

### Internal: `ManagedPlugin : Plugin`

Non-exported class. Wraps one `FfiBridge` instance and implements every virtual in the SDK's `Plugin` class:

| Plugin virtual | Delegates to |
|---|---|
| `get_name()` | `FfiBridge::get_name()` |
| `get_version()` | `FfiBridge::get_version()` |
| `capabilities()` | `FfiBridge::capabilities()` |
| `get_unique_type()` | `FfiBridge::get_unique_type()` |
| `get_requires()` | `FfiBridge::get_requires()` |
| `get_load_before_types()` | `FfiBridge::get_load_before_types()` |
| `on_load(host, cfg)` | `FfiBridge::activate(...)` |
| `on_unload()` | `FfiBridge::on_unload()` |

### Public: `PluginManager`

Exported in `include/core/manager/plugin.cppm`.

```
class PluginManager {
public:
    void addPlugin(std::string_view path, std::span<std::pair<std::string_view, std::string_view>> config);
    void activate(void* router_ctx, void* controller_ctx, void* leverager_ctx);
    Plugin* getByCapability(Cap capability) const;
    std::span<Plugin* const> getAll() const;

private:
    std::vector<ManagedPlugin> m_plugins;
};
```

**`addPlugin`:** takes raw config key-value pairs (stored internally). Calls `FfiBridge::open()`, constructs a `ManagedPlugin`, appends to `m_plugins`. `ConfigView` is constructed from stored pairs only at `activate()` time (it is valid only during `on_load`).

**`activate`:**
1. Uniqueness filter — one plugin per `get_unique_type()` value; error on duplicate
2. Kahn's topological sort on `get_requires()` + `get_load_before_types()` dependency graph
3. Call `on_load(host, cfg)` on each plugin in sorted order
4. On failure: call `on_unload()` on all previously activated plugins in reverse order

**`getByCapability`:** linear scan over `m_plugins`, returns first match or `nullptr`.

**`getAll`:** returns span over internal `Plugin*` pointers.

---

## 4. `App::load_plugins()` Rebuild

`include/core/heart/app.cppm` — replace stub with:

1. Construct `PluginManager`
2. Iterate plugin paths from loaded config → `manager.addPlugin(path, cfg_section)`
3. Call `manager.activate(router_ctx, controller_ctx, leverager_ctx)`
4. Get logger: `manager.getByCapability(Cap::LOGGER)` — exit if null
5. Get protocol: `manager.getByCapability(Cap::PROTOCOL)` — wire into server builder

All previously commented-out phases (probe → filter → sort → activate) are now inside `PluginManager::activate()`. `App` sees none of that complexity.

---

## 5. Defaults Plugins Update

`defaults/plugins/http2/http2.cc` (and all other defaults):

- Add `CONGELADO_GUEST` define before SDK include (or set via CMake/build system target)
- No structural changes to plugin logic

---

## Files Changed

| File | Action |
|---|---|
| `sdk/plugin/include/congelado/mode.h` | Create |
| `sdk/plugin/include/congelado/plugin.h` | Add `mode.h` include + `#ifdef CONGELADO_HOST` guards |
| `include/core/ffi/bridge.cppm` | Delete |
| `include/core/manager/bridge.cppm` | Create (FfiBridge, moved from core/ffi) |
| `include/core/manager/plugin.cppm` | Rebuild (PluginManager + ManagedPlugin) |
| `include/core/heart/app.cppm` | Rebuild load_plugins() |
| `defaults/plugins/http2/http2.cc` | Add CONGELADO_GUEST define |
| `defaults/plugins/file_logger/file_logger.cc` | Add CONGELADO_GUEST define |
| `defaults/plugins/postgres/postgres_plugin.cc` | Add CONGELADO_GUEST define |
| `defaults/plugins/engine/engine.cc` | Add CONGELADO_GUEST define |
