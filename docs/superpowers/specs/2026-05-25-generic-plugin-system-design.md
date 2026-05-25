# Generic Plugin System Design

**Date:** 2026-05-25  
**Status:** Approved

## Goals

1. Replace logger-focused plugin system with a generic, language-agnostic one.
2. Support background/custom plugins that start on `on_load` and run forever with no host involvement.
3. Fix segfault in `FfiBridge::write()` caused by null `m_logger_cap` dereference.
4. Fix `write_to_plugin` noexcept/throw mismatch.
5. Support multiple simultaneous logger plugins.
6. Minimize C code in plugin authoring — all C++ plugins write zero C.

## Non-Goals

- Multi-plugin capability registry (querying all plugins for a cap) — deferred.
- Changing the scheduler / `Registry<TController>` structure.
- Protocol cap vtable definition (placeholder only for now).

---

## Architecture

```
plugin_api.h        ← pure C ABI contract (boundary)
plugin_api.hpp      ← C++ convenience layer, all C++ plugins include only this

FfiBridge           ← RAII C++ wrapper around CongeladoPlugin*
LoggerRegistry      ← multi-logger fan-out, null-safe
App                 ← loads all plugins, registers all logger caps, aborts if none
```

---

## Section 1: ABI Layer

### `include/core/ffi/plugin_api.h` — full rewrite, pure C

```c
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CongeladoCap {
    CONGELADO_CAP_LOGGER   = 0,
    CONGELADO_CAP_PROTOCOL = 1,
    CONGELADO_CAP_CUSTOM   = 2,
} CongeladoCap;

typedef struct CongeladoHostCallbacks {
    void  (*log)(void* ctx, int level, const char* msg, size_t len);
    void  (*schedule)(void* ctx);
    void* ctx;
} CongeladoHostCallbacks;

typedef struct CongeladoConfigView {
    const char* const* keys;
    const char* const* values;
    size_t count;
} CongeladoConfigView;

typedef struct CongeladoLoggerCap {
    void (*write)(void* self, int level, const char* msg, size_t len);
    void (*write_error)(void* self, const char* msg, size_t len);
    void* self;
} CongeladoLoggerCap;

// Placeholder — defined when IProtocol is C-ified
typedef struct CongeladoProtocolCap CongeladoProtocolCap;

typedef struct CongeladoPlugin {
    const char* name;
    const char* version;
    void  (*on_load)(void* self, const CongeladoHostCallbacks*, const CongeladoConfigView*);
    void  (*on_unload)(void* self);
    void* (*get_capability)(void* self, uint32_t cap_id);
    void* self;
} CongeladoPlugin;

// Plugins must export these two symbols
// CongeladoPlugin* congelado_get_plugin();
// void             congelado_destroy_plugin(CongeladoPlugin*);

#ifdef __cplusplus
}
#endif
```

### `include/core/ffi/plugin_api.hpp` — C++ convenience layer (new file)

```cpp
#pragma once
#include "plugin_api.h"
#include <string_view>

namespace congelado {

class PluginBase {
public:
    virtual ~PluginBase() = default;
    virtual std::string_view name()    const noexcept = 0;
    virtual std::string_view version() const noexcept = 0;
    virtual void on_load(const CongeladoHostCallbacks&, const CongeladoConfigView*) {}
    virtual void on_unload() {}
    virtual CongeladoLoggerCap*   logger_cap()   noexcept { return nullptr; }
    virtual CongeladoProtocolCap* protocol_cap() noexcept { return nullptr; }

    CongeladoPlugin to_c_plugin() noexcept;
};

} // namespace congelado

// Drop at bottom of plugin .cc — generates extern "C" factory symbols
#define CONGELADO_PLUGIN(T)                                             \
    extern "C" CongeladoPlugin* congelado_get_plugin() {                \
        auto* p = new T{};                                              \
        return new CongeladoPlugin(p->to_c_plugin());                   \
    }                                                                   \
    extern "C" void congelado_destroy_plugin(CongeladoPlugin* p) {      \
        delete static_cast<T*>(p->self);                                \
        delete p;                                                       \
    }
```

`PluginBase::to_c_plugin()` fills function pointers using static thunks that cast `self` back to `PluginBase*` and dispatch to the virtual methods. Defined inline in `plugin_api.hpp`.

---

## Section 2: `FfiBridge` (bridge.cppm)

Replaces `IPluginHandler*` with `CongeladoPlugin*`. Still inherits `interfaces::ILogger` and `shared::HandlerBase`.

**Member changes:**
- `IPluginHandler* m_handler` → `CongeladoPlugin* m_plugin`
- `void* m_destroy_fn` → renamed `m_destroy_sym` (probed `congelado_destroy_plugin` pointer, stored as `void*`)
- `CongeladoLoggerCap* m_logger_cap = nullptr` — cached in `discover_caps()`

**`load()` changes:**
- Probe `congelado_get_plugin` / `congelado_destroy_plugin` (replace old symbols)
- Call `m_plugin->on_load(m_plugin->self, &callbacks, pcv_ptr)`
- Call `discover_caps()` after on_load

**`discover_caps()` changes:**
```cpp
void discover_caps() {
    auto* lc = static_cast<CongeladoLoggerCap*>(
        m_plugin->get_capability(m_plugin->self, CONGELADO_CAP_LOGGER));
    if (lc) {
        m_logger_cap = lc;
        m_caps |= to_underlying(Cap::Logger);
    }
    // protocol: similar
}
```

**`write()` and `error()` — fixes segfault:**
```cpp
void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
    if (!m_logger_cap) return;
    m_logger_cap->write(m_logger_cap->self, static_cast<int>(level), msg.data(), msg.size());
}
void error(std::string_view msg) override {
    if (!m_logger_cap) return;
    m_logger_cap->write_error(m_logger_cap->self, msg.data(), msg.size());
}
```

**`on_released()` lambda:**
```cpp
return [this] {
    if (m_plugin) {
        m_plugin->on_unload(m_plugin->self);
        reinterpret_cast<void(*)(CongeladoPlugin*)>(m_destroy_sym)(m_plugin);
        m_plugin = nullptr;
        m_logger_cap = nullptr;
    }
};
```

**`Cap` enum note:** `Cap` in `bridge.cppm` (host C++) mirrors `CongeladoCap` in `plugin_api.h` (plugin C). Values must stay in sync — both are the source of truth for their respective sides.

**Custom/background plugins:** `get_capability` returns null for all cap IDs. Host loads, calls `on_load`, stores handle. `m_logger_cap` stays null. No caps set. Plugin manages its own lifetime internally.

---

## Section 3: Logger fixes

### `include/core/logger/registry.cppm`

Multi-logger, null-safe:

```cpp
class LoggerRegistry {
    static inline std::vector<std::shared_ptr<interfaces::ILogger>> loggers;
public:
    static void register_logger(std::shared_ptr<interfaces::ILogger> logger) {
        if (logger) loggers.push_back(std::move(logger));
    }
    static bool has_logger() noexcept { return !loggers.empty(); }
    static const auto& all() noexcept { return loggers; }
};
```

### `include/core/logger/logger.cppm`

Fix noexcept/throw mismatch — `write_to_plugin` never throws:

```cpp
inline void write_to_plugin(interfaces::LogLevel level, std::string_view message) {
    auto& loggers = LoggerRegistry::all();
    if (loggers.empty()) {
        std::println(stderr, "[pre-logger] {}", message);
        return;
    }
    for (auto& logger : loggers) {
        if (level == interfaces::LogLevel::Error || level == interfaces::LogLevel::Fatal) {
            logger->error(message);
        } else {
            logger->write(level, message);
        }
    }
    if (level == interfaces::LogLevel::Fatal) std::abort();
}
```

---

## Section 4: App + manager

### `include/core/heart/app.cppm`

`load_plugins` registers **all** logger-capable plugins:

```cpp
for (auto& [name, plugin_cfg] : cfg.plugins) {
    // ... load bridge ...
    if (auto logger = core::plugin::make_logger(*result))
        core::logger::LoggerRegistry::register_logger(std::move(logger));
    if (!proto)
        proto = core::plugin::make_protocol(*result);
}

if (!core::logger::LoggerRegistry::has_logger()) {
    std::println(stderr, "[heart] no logger plugin found — aborting");
    std::abort();
}
```

Custom plugins with no logger/protocol cap are simply stored in `handles` — nothing else needed.

### `include/core/manager/handler.cppm`

`make_logger` / `make_protocol` signatures unchanged. Internals use `bridge->m_logger_cap` (via `has(Cap::Logger)` gate which is already correct).

### Existing plugins

All must export `congelado_get_plugin` / `congelado_destroy_plugin` instead of old symbols. C++ plugins use `plugin_api.hpp` + `CONGELADO_PLUGIN(ClassName)` macro — zero hand-written C.

---

## File Change Summary

| File | Change |
|---|---|
| `include/core/ffi/plugin_api.h` | Full rewrite — pure C structs |
| `include/core/ffi/plugin_api.hpp` | New — C++ convenience layer + `CONGELADO_PLUGIN` macro |
| `include/core/ffi/bridge.cppm` | Wrap `CongeladoPlugin*`, fix null guards, new symbols |
| `include/core/logger/registry.cppm` | Multi-logger vector, remove `get()` |
| `include/core/logger/logger.cppm` | Fix noexcept/throw, fan-out to all loggers |
| `include/core/manager/handler.cppm` | Update for new bridge API |
| `include/core/heart/app.cppm` | Register all logger caps, abort check after loop |
| Existing plugin `.cc` files | Export new symbols via `CONGELADO_PLUGIN` macro |

---

## Segfault Root Cause

`FfiBridge::write()` called `m_handler->logger_write(...)` with no null guard on `m_handler`. With the new design, `m_logger_cap` is always checked before use and no C++ vtable crosses the ABI boundary, eliminating the entire class of vtable-mismatch segfaults.
