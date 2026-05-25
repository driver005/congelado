# Generic Plugin System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the logger-focused C++ vtable plugin ABI with a language-agnostic C-struct ABI, fix the segfault in `FfiBridge::write()`, support multi-logger fan-out, and support background/custom plugins with zero C hand-written by plugin authors.

**Architecture:** `plugin_api.h` defines a pure-C struct ABI (`CongeladoPlugin`, `CongeladoLoggerCap`, `CongeladoCap`). `plugin_api.hpp` wraps it in a C++ `PluginBase` class + `CONGELADO_PLUGIN` macro so C++ plugins write zero C. `FfiBridge` wraps `CongeladoPlugin*` and caches capability pointers; null guards on all dispatch methods fix the segfault. `LoggerRegistry` becomes a fan-out vector.

**Tech Stack:** C++26 modules (Clang), C11 ABI layer, libffi for host callbacks, xmake build system, Catch2 (for config tests), dlopen/dlclose.

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `include/core/ffi/plugin_api.h` | Rewrite | Pure-C ABI contract — `CongeladoPlugin`, caps, vtables |
| `include/core/ffi/plugin_api.hpp` | Create | C++ `PluginBase` + `CONGELADO_PLUGIN` macro |
| `include/core/ffi/bridge.cppm` | Rewrite | RAII wrapper around `CongeladoPlugin*`, null-safe dispatch |
| `include/core/logger/registry.cppm` | Rewrite | Multi-logger vector, `has_logger()`, `all()` |
| `include/core/logger/logger.cppm` | Edit | Fix noexcept/throw, fan-out to all loggers |
| `include/core/manager/handler.cppm` | Edit | Remove `IPluginHandler` references |
| `include/core/heart/app.cppm` | Edit | Register all logger caps, abort-if-none after loop |
| `plugins/file_logger/file_logger.cc` | Rewrite | Migrate to `PluginBase` + `CONGELADO_PLUGIN` |
| `plugins/http2/http2.cc` | Rewrite | Migrate to `PluginBase` + `CONGELADO_PLUGIN` |

---

## Task 1: Rewrite `plugin_api.h` — pure-C ABI contract

**Files:**
- Modify: `include/core/ffi/plugin_api.h`

- [ ] **Step 1: Replace the entire file**

```c
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Capability IDs — must stay in sync with core::ffi::Cap in bridge.cppm */
typedef enum CongeladoCap {
    CONGELADO_CAP_LOGGER   = 0,
    CONGELADO_CAP_PROTOCOL = 1,
    CONGELADO_CAP_CUSTOM   = 2,
} CongeladoCap;

/* Host-side callbacks given to the plugin at on_load time.
   Built as libffi closures — real callable C function pointers. */
typedef struct CongeladoHostCallbacks {
    void  (*log)(void* ctx, int level, const char* msg, size_t len);
    void  (*schedule)(void* ctx);
    void* ctx;
} CongeladoHostCallbacks;

/* Read-only view of one plugin's config section.
   Valid only for the duration of on_load(). Do not store pointers. */
typedef struct CongeladoConfigView {
    const char* const* keys;
    const char* const* values;
    size_t count;
} CongeladoConfigView;

/* Logger capability vtable — returned by get_capability(CONGELADO_CAP_LOGGER) */
typedef struct CongeladoLoggerCap {
    void (*write)(void* self, int level, const char* msg, size_t len);
    void (*write_error)(void* self, const char* msg, size_t len);
    void* self;
} CongeladoLoggerCap;

/* Protocol capability vtable — placeholder, defined when IProtocol is C-ified.
   For now get_capability(CONGELADO_CAP_PROTOCOL) returns interfaces::IProtocol* cast to void*. */
typedef struct CongeladoProtocolCap CongeladoProtocolCap;

/* Main plugin descriptor — any language that can produce a .so can fill this */
typedef struct CongeladoPlugin {
    const char* name;
    const char* version;
    void  (*on_load)(void* self, const CongeladoHostCallbacks*, const CongeladoConfigView*);
    void  (*on_unload)(void* self);
    /* Returns pointer to cap-specific vtable, or NULL if capability not supported.
       cap_id is one of CongeladoCap. */
    void* (*get_capability)(void* self, uint32_t cap_id);
    void* self;
} CongeladoPlugin;

/* Every plugin .so must export these two symbols (extern "C" for dlsym).
   C++ plugins use plugin_api.hpp + CONGELADO_PLUGIN(ClassName) — zero hand-written C. */
/* CongeladoPlugin* congelado_get_plugin();                  */
/* void             congelado_destroy_plugin(CongeladoPlugin*); */

#ifdef __cplusplus
}
#endif
```

- [ ] **Step 2: Build to verify it compiles (will have downstream errors — expected)**

```bash
xmake build congelado_lib 2>&1 | head -40
```

Expected: errors referencing `IPluginHandler` in bridge.cppm, file_logger.cc, http2.cc. The header itself compiles cleanly.

- [ ] **Step 3: Commit**

```bash
git add include/core/ffi/plugin_api.h
git commit -m "feat(ffi): rewrite plugin_api.h as pure-C ABI contract"
```

---

## Task 2: Create `plugin_api.hpp` — C++ convenience layer

**Files:**
- Create: `include/core/ffi/plugin_api.hpp`

- [ ] **Step 1: Create the file**

```cpp
#pragma once
#include "plugin_api.h"
#include <string_view>
#include <cstring>

namespace congelado {

/* C++ base class for plugin authors. Inherit this, override virtual methods,
   drop CONGELADO_PLUGIN(YourClass) at bottom of .cc — zero C required. */
class PluginBase {
public:
    virtual ~PluginBase() = default;

    virtual std::string_view name()    const noexcept = 0;
    virtual std::string_view version() const noexcept = 0;
    virtual void on_load(const CongeladoHostCallbacks&, const CongeladoConfigView*) {}
    virtual void on_unload() {}

    /* Override to expose logger capability. Return non-null if this plugin is a logger. */
    virtual CongeladoLoggerCap*   logger_cap()   noexcept { return nullptr; }
    /* Override to expose protocol capability. Return non-null if this plugin is a protocol.
       For now returns interfaces::IProtocol* reinterpret_cast'd — placeholder. */
    virtual CongeladoProtocolCap* protocol_cap() noexcept { return nullptr; }

    /* Builds a CongeladoPlugin C-struct backed by this object.
       The returned struct holds a raw pointer to this — caller must keep this alive. */
    CongeladoPlugin to_c_plugin() noexcept {
        CongeladoPlugin p{};
        p.name    = name().data();
        p.version = version().data();
        p.self    = this;
        p.on_load = [](void* self, const CongeladoHostCallbacks* cb, const CongeladoConfigView* cfg) {
            static_cast<PluginBase*>(self)->on_load(*cb, cfg);
        };
        p.on_unload = [](void* self) {
            static_cast<PluginBase*>(self)->on_unload();
        };
        p.get_capability = [](void* self, uint32_t cap_id) -> void* {
            auto* pb = static_cast<PluginBase*>(self);
            switch (cap_id) {
            case CONGELADO_CAP_LOGGER:   return pb->logger_cap();
            case CONGELADO_CAP_PROTOCOL: return pb->protocol_cap();
            default:                     return nullptr;
            }
        };
        return p;
    }
};

} // namespace congelado

/* Drop exactly once at the bottom of your plugin .cc.
   Generates the two extern "C" symbols dlopen looks for. */
#define CONGELADO_PLUGIN(T)                                                 \
    extern "C" CongeladoPlugin* congelado_get_plugin() {                    \
        auto* p = new T{};                                                  \
        auto* desc = new CongeladoPlugin(p->to_c_plugin());                 \
        return desc;                                                        \
    }                                                                       \
    extern "C" void congelado_destroy_plugin(CongeladoPlugin* desc) {       \
        delete static_cast<T*>(desc->self);                                 \
        delete desc;                                                        \
    }
```

- [ ] **Step 2: Build to confirm no syntax errors in the header itself**

```bash
xmake build congelado_lib 2>&1 | grep "plugin_api.hpp" | head -10
```

Expected: no errors from `plugin_api.hpp` (errors from bridge/plugins still expected).

- [ ] **Step 3: Commit**

```bash
git add include/core/ffi/plugin_api.hpp
git commit -m "feat(ffi): add plugin_api.hpp C++ convenience layer and CONGELADO_PLUGIN macro"
```

---

## Task 3: Rewrite `bridge.cppm` — wrap `CongeladoPlugin*`, fix segfault

**Files:**
- Modify: `include/core/ffi/bridge.cppm`

- [ ] **Step 1: Replace the entire file**

```cpp
module;

#include <ffi.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "core/ffi/plugin_api.h"

export module core_ffi:bridge;

import std;
import shared;
import interfaces;
import core_config;

export namespace core::ffi {

// Host-side capability enum — values mirror CongeladoCap in plugin_api.h.
enum class Cap : std::uint32_t {
    Logger   = CONGELADO_CAP_LOGGER,
    Server   = 10u,  // future
    Client   = 11u,  // future
    Protocol = CONGELADO_CAP_PROTOCOL,
};

struct LoadError {
    std::string detail;
};

// FfiBridge: RAII wrapper around one loaded CongeladoPlugin*.
//
// Loading:
//   1. dlopen the .so
//   2. Probe "congelado_get_plugin" (direct cast, signature known) → CongeladoPlugin*
//   3. Build libffi closures for HostCallbacks (log, schedule)
//   4. Call plugin->on_load(self, &callbacks, cfg)
//   5. Probe capabilities via get_capability(self, cap_id)
//
// Dispatch:
//   All interface calls go through cached cap vtable pointers (m_logger_cap etc.).
//   Always null-checked — never segfaults on missing capability.
class FfiBridge : public shared::HandlerBase, public interfaces::ILogger {
  public:
    [[nodiscard]] static std::expected<std::shared_ptr<FfiBridge>, LoadError>
    load(const std::filesystem::path &path, const core::config::PluginConfig *plugin_cfg = nullptr) {
        void *lib =
#if defined(_WIN32)
            static_cast<void *>(LoadLibraryA(path.string().c_str()));
#else
            dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
        if (!lib)
            return std::unexpected(LoadError{std::format("dlopen failed: {}", path.string())});

        auto bridge = std::shared_ptr<FfiBridge>(new FfiBridge{});
        bridge->m_lib = lib;

        auto *get_fn      = bridge->probe("congelado_get_plugin");
        auto *destroy_sym = bridge->probe("congelado_destroy_plugin");
        if (!get_fn || !destroy_sym)
            return std::unexpected(LoadError{"missing 'congelado_get_plugin' / 'congelado_destroy_plugin'"});

        bridge->m_destroy_sym = destroy_sym;
        bridge->m_plugin = reinterpret_cast<CongeladoPlugin *(*)()>(get_fn)();
        if (!bridge->m_plugin)
            return std::unexpected(LoadError{"congelado_get_plugin returned null"});

        bridge->m_lib_name = bridge->m_plugin->name;

        // Build libffi closures for host callbacks
        HostCallbacksInternal callbacks = bridge->make_host_callbacks();
        CongeladoHostCallbacks c_callbacks{};
        c_callbacks.log      = reinterpret_cast<CongeladoHostCallbacks::log_t>(callbacks.log_fn_code);
        c_callbacks.schedule = reinterpret_cast<CongeladoHostCallbacks::schedule_t>(callbacks.sched_fn_code);
        c_callbacks.ctx      = bridge.get();

        std::vector<const char *> pcv_keys, pcv_vals;
        if (plugin_cfg) {
            for (auto &[k, v] : plugin_cfg->fields) {
                pcv_keys.push_back(k.c_str());
                pcv_vals.push_back(v.c_str());
            }
        }
        CongeladoConfigView pcv{
            plugin_cfg ? pcv_keys.data() : nullptr,
            plugin_cfg ? pcv_vals.data() : nullptr,
            plugin_cfg ? pcv_keys.size() : 0,
        };
        bridge->m_plugin->on_load(bridge->m_plugin->self,
                                  &c_callbacks,
                                  plugin_cfg ? &pcv : nullptr);

        bridge->discover_caps();

        return bridge;
    }

    ~FfiBridge() {
        release_plugin();
        if (m_log_closure)   ffi_closure_free(m_log_closure);
        if (m_sched_closure) ffi_closure_free(m_sched_closure);
        if (m_lib) {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(m_lib));
#else
            dlclose(m_lib);
#endif
        }
    }

    FfiBridge(const FfiBridge &) = delete;
    FfiBridge &operator=(const FfiBridge &) = delete;

    [[nodiscard]] bool has(Cap cap) const noexcept {
        return (m_caps & std::to_underlying(cap)) != 0;
    }

    [[nodiscard]] std::shared_ptr<interfaces::IProtocol> get_protocol() const noexcept {
        return m_protocol;
    }

    // shared::HandlerBase + interfaces::ILogger — name() satisfies both.
    [[nodiscard]] std::string_view name() const noexcept override { return m_lib_name; }

    // Null-guarded — never segfaults even if plugin has no logger cap.
    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        if (!m_logger_cap) return;
        m_logger_cap->write(m_logger_cap->self,
                            static_cast<int>(level),
                            msg.data(), msg.size());
    }

    void error(std::string_view msg) override {
        if (!m_logger_cap) return;
        m_logger_cap->write_error(m_logger_cap->self, msg.data(), msg.size());
    }

    [[nodiscard]] shared::WorkerFunction on_execute() override { return nullptr; }

    [[nodiscard]] shared::ReleaseFunction on_released() noexcept override {
        return [this] { release_plugin(); };
    }

    [[nodiscard]] shared::ErrorHandler on_error() override {
        return [this](std::exception_ptr eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception &e) {
                std::println(stderr, "[ffi::{}] error: {}", m_lib_name, e.what());
            } catch (...) {
                std::println(stderr, "[ffi::{}] unknown error", m_lib_name);
            }
        };
    }

  private:
    FfiBridge() = default;

    struct HostCallbacksInternal {
        void *log_fn_code   = nullptr;
        void *sched_fn_code = nullptr;
    };

    void *m_lib         = nullptr;
    void *m_destroy_sym = nullptr;
    CongeladoPlugin *m_plugin    = nullptr;
    CongeladoLoggerCap *m_logger_cap = nullptr;
    std::string m_lib_name;
    std::uint32_t m_caps = 0;
    std::shared_ptr<interfaces::IProtocol> m_protocol;

    ffi_closure *m_log_closure   = nullptr;
    void        *m_log_fn_code   = nullptr;
    ffi_cif      m_cif_log{};
    ffi_type    *m_log_args[4]{};

    ffi_closure *m_sched_closure  = nullptr;
    void        *m_sched_fn_code  = nullptr;
    ffi_cif      m_cif_sched{};
    ffi_type    *m_sched_args[1]{};

    void release_plugin() noexcept {
        if (!m_plugin) return;
        m_plugin->on_unload(m_plugin->self);
        using DestroyFn = void (*)(CongeladoPlugin *);
        reinterpret_cast<DestroyFn>(m_destroy_sym)(m_plugin);
        m_plugin     = nullptr;
        m_logger_cap = nullptr;
    }

    [[nodiscard]] void *probe(const char *sym) const noexcept {
#if defined(_WIN32)
        return reinterpret_cast<void *>(GetProcAddress(static_cast<HMODULE>(m_lib), sym));
#else
        return dlsym(m_lib, sym);
#endif
    }

    static ffi_type *size_ffi_type() noexcept {
        return sizeof(std::size_t) == 8 ? &ffi_type_uint64 : &ffi_type_uint32;
    }

    static void log_fn(ffi_cif *, void *, void **args, void *data) noexcept {
        auto *self = static_cast<FfiBridge *>(data);
        int level         = *static_cast<int *>(args[1]);
        const char *ptr   = *static_cast<const char **>(args[2]);
        std::size_t len   = *static_cast<std::size_t *>(args[3]);
        std::println(stderr, "[plugin::{}] log({}): {}",
                     self->m_lib_name, level, std::string_view{ptr, len});
    }

    static void schedule_fn(ffi_cif *, void *, void **, void *data) noexcept {
        auto *self = static_cast<FfiBridge *>(data);
        std::println(stderr, "[plugin::{}] schedule requested", self->m_lib_name);
    }

    [[nodiscard]] HostCallbacksInternal make_host_callbacks() {
        m_log_args[0] = &ffi_type_pointer;
        m_log_args[1] = &ffi_type_sint;
        m_log_args[2] = &ffi_type_pointer;
        m_log_args[3] = size_ffi_type();
        ffi_prep_cif(&m_cif_log, FFI_DEFAULT_ABI, 4, &ffi_type_void, m_log_args);
        m_log_closure = static_cast<ffi_closure *>(
            ffi_closure_alloc(sizeof(ffi_closure), &m_log_fn_code));
        ffi_prep_closure_loc(m_log_closure, &m_cif_log, log_fn, this, m_log_fn_code);

        m_sched_args[0] = &ffi_type_pointer;
        ffi_prep_cif(&m_cif_sched, FFI_DEFAULT_ABI, 1, &ffi_type_void, m_sched_args);
        m_sched_closure = static_cast<ffi_closure *>(
            ffi_closure_alloc(sizeof(ffi_closure), &m_sched_fn_code));
        ffi_prep_closure_loc(m_sched_closure, &m_cif_sched, schedule_fn, this, m_sched_fn_code);

        return {m_log_fn_code, m_sched_fn_code};
    }

    void discover_caps() {
        if (!m_plugin) return;

        // Logger capability
        auto *lc = static_cast<CongeladoLoggerCap *>(
            m_plugin->get_capability(m_plugin->self, CONGELADO_CAP_LOGGER));
        if (lc) {
            m_logger_cap = lc;
            m_caps |= std::to_underlying(Cap::Logger);
        }

        // Protocol capability — placeholder: plugin returns interfaces::IProtocol* as void*
        auto *proto_raw = m_plugin->get_capability(m_plugin->self, CONGELADO_CAP_PROTOCOL);
        if (proto_raw) {
            auto *proto = static_cast<interfaces::IProtocol *>(proto_raw);
            m_protocol  = std::shared_ptr<interfaces::IProtocol>(proto, [](interfaces::IProtocol *) {});
            m_caps |= std::to_underlying(Cap::Protocol);
        }
    }
};

} // namespace core::ffi
```

Note: `CongeladoHostCallbacks` in `plugin_api.h` needs two typedefs for the function pointer types. Add them in the next step.

- [ ] **Step 2: Add function pointer typedefs to `plugin_api.h`**

In `plugin_api.h`, inside `CongeladoHostCallbacks`, add typedefs so the cast in bridge.cppm compiles:

```c
typedef struct CongeladoHostCallbacks {
    typedef void (*log_t)(void* ctx, int level, const char* msg, size_t len);
    typedef void (*schedule_t)(void* ctx);
    void  (*log)(void* ctx, int level, const char* msg, size_t len);
    void  (*schedule)(void* ctx);
    void* ctx;
} CongeladoHostCallbacks;
```

Wait — C doesn't support typedefs inside structs. Instead, add them at file scope before the struct:

```c
/* Function pointer types for CongeladoHostCallbacks */
typedef void (*CongeladoLogFn)(void* ctx, int level, const char* msg, size_t len);
typedef void (*CongeladoScheduleFn)(void* ctx);

typedef struct CongeladoHostCallbacks {
    CongeladoLogFn      log;
    CongeladoScheduleFn schedule;
    void* ctx;
} CongeladoHostCallbacks;
```

Update `plugin_api.h` — replace the `CongeladoHostCallbacks` definition with the above.

Also update `plugin_api.hpp` — the `on_load` thunk now receives `const CongeladoHostCallbacks*`, which is already correct. No change needed there.

Update `bridge.cppm` — replace the cast lines:
```cpp
c_callbacks.log      = reinterpret_cast<CongeladoHostCallbacks::log_t>(...);
c_callbacks.schedule = reinterpret_cast<CongeladoHostCallbacks::schedule_t>(...);
```
with:
```cpp
c_callbacks.log      = reinterpret_cast<CongeladoLogFn>(callbacks.log_fn_code);
c_callbacks.schedule = reinterpret_cast<CongeladoScheduleFn>(callbacks.sched_fn_code);
```

- [ ] **Step 3: Build congelado_lib — expect errors only from plugins, not from bridge**

```bash
xmake build congelado_lib 2>&1 | grep -v "plugins/" | head -30
```

Expected: `congelado_lib` builds cleanly. Errors only come from `plugins/` (not yet migrated).

- [ ] **Step 4: Commit**

```bash
git add include/core/ffi/plugin_api.h include/core/ffi/bridge.cppm
git commit -m "feat(ffi): migrate FfiBridge to CongeladoPlugin* C-struct ABI, fix null-guard segfault"
```

---

## Task 4: Rewrite `LoggerRegistry` — multi-logger fan-out

**Files:**
- Modify: `include/core/logger/registry.cppm`

- [ ] **Step 1: Replace the entire file**

```cpp
export module core_logger:registry;

import std;
import interfaces;

export namespace core::logger {

class LoggerRegistry {
    static inline std::vector<std::shared_ptr<interfaces::ILogger>> loggers;

  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    static void register_logger(std::shared_ptr<interfaces::ILogger> logger) {
        if (logger) loggers.push_back(std::move(logger));
    }

    [[nodiscard]] static bool has_logger() noexcept { return !loggers.empty(); }

    [[nodiscard]] static const std::vector<std::shared_ptr<interfaces::ILogger>> &all() noexcept {
        return loggers;
    }
};

} // namespace core::logger
```

- [ ] **Step 2: Build to verify**

```bash
xmake build congelado_lib 2>&1 | grep "registry" | head -10
```

Expected: no errors from `registry.cppm`.

- [ ] **Step 3: Commit**

```bash
git add include/core/logger/registry.cppm
git commit -m "feat(logger): multi-logger registry with fan-out vector, remove single-logger assumption"
```

---

## Task 5: Fix `logger.cppm` — noexcept/throw mismatch + fan-out

**Files:**
- Modify: `include/core/logger/logger.cppm`

- [ ] **Step 1: Replace `write_to_plugin` and the export namespace block**

Replace the entire file with:

```cpp
export module core_logger;

import std;
import interfaces;

export import :registry;

namespace core::logger {

// Never throws. Falls back to stderr before any logger is registered.
// After registration, fans out to all registered loggers.
inline void write_to_plugin(interfaces::LogLevel level, std::string_view message) noexcept {
    const auto &loggers = LoggerRegistry::all();
    if (loggers.empty()) {
        std::println(stderr, "[pre-logger] {}", message);
        if (level == interfaces::LogLevel::Fatal) std::abort();
        return;
    }
    for (const auto &logger : loggers) {
        if (level == interfaces::LogLevel::Error || level == interfaces::LogLevel::Fatal) {
            logger->error(message);
        } else {
            logger->write(level, message);
        }
    }
    if (level == interfaces::LogLevel::Fatal) std::abort();
}

} // namespace core::logger

export namespace core::logger {

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void log(interfaces::LogLevel level, std::format_string<Args...> fmt, Args &&...args) noexcept {
    write_to_plugin(level, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Info, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Debug, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Warning, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Error, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Fatal, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

} // namespace core::logger

export namespace core::logger::unnamed {

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Warning, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Error, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Fatal, fmt, std::forward<Args>(args)...);
}

} // namespace core::logger::unnamed

export namespace core::logger::named {
using namespace core::logger;
} // namespace core::logger::named
```

- [ ] **Step 2: Build to verify**

```bash
xmake build congelado_lib 2>&1 | grep "logger" | head -10
```

Expected: no errors from `logger.cppm` or `registry.cppm`.

- [ ] **Step 3: Commit**

```bash
git add include/core/logger/logger.cppm
git commit -m "fix(logger): noexcept write_to_plugin with stderr fallback, fan-out to all loggers"
```

---

## Task 6: Update `App::load_plugins` — register all logger caps

**Files:**
- Modify: `include/core/heart/app.cppm`

- [ ] **Step 1: Update `load_plugins` method**

Replace the entire `load_plugins` method body (lines 83–120 in current file):

```cpp
bool load_plugins(const core::config::Config &cfg, std::vector<core::plugin::PluginHandle> &handles,
                  std::shared_ptr<interfaces::IProtocol> &proto) {
    for (auto &[name, plugin_cfg] : cfg.plugins) {
#if defined(_WIN32)
        auto so = m_plugin_dir / (name + ".dll");
#else
        auto so = m_plugin_dir / ("lib" + name + ".so");
#endif
        if (!std::filesystem::exists(so)) {
            std::println(stderr, "[heart] plugin '{}' not found at '{}'", name, so.string());
            continue;
        }

        auto result = core::plugin::load(so, &plugin_cfg);
        if (!result) {
            std::println(stderr, "[heart] plugin '{}' failed to load: {}", name, result.error().detail);
            continue;
        }

        std::println("[heart] loaded plugin '{}'", name);
        handles.push_back(*result);

        // Register every logger-capable plugin — fan-out handles multiple loggers.
        if (auto logger = core::plugin::make_logger(*result))
            core::logger::LoggerRegistry::register_logger(std::move(logger));

        // First protocol plugin wins.
        if (!proto)
            proto = core::plugin::make_protocol(*result);
    }

    return core::logger::LoggerRegistry::has_logger();
}
```

- [ ] **Step 2: Update `run()` — replace logger_up check**

In `run()`, replace:
```cpp
if (!plugin_logger) {
    std::println("[heart] no plugin logger found");
    std::abort();
}
```
with:
```cpp
if (!plugin_logger) {
    std::println(stderr, "[heart] no logger plugin found — aborting");
    std::abort();
}
```
(same semantics, stderr is correct target for pre-logger abort message)

- [ ] **Step 3: Build congelado_lib**

```bash
xmake build congelado_lib 2>&1 | grep -v "plugins/" | head -20
```

Expected: `congelado_lib` builds cleanly.

- [ ] **Step 4: Commit**

```bash
git add include/core/heart/app.cppm
git commit -m "feat(heart): register all logger-cap plugins, use has_logger() abort check"
```

---

## Task 7: Migrate `file_logger` plugin to new C-struct ABI

**Files:**
- Modify: `plugins/file_logger/file_logger.cc`

- [ ] **Step 1: Replace the entire file**

```cpp
#include "core/ffi/plugin_api.hpp"

import std;

namespace {

constexpr std::string_view level_str(int level) noexcept {
    switch (level) {
    case 0: return "DEBUG";
    case 1: return "INFO";
    case 2: return "WARNING";
    case 3: return "ERROR";
    case 4: return "FATAL";
    default: return "UNKNOWN";
    }
}

class FileLogger final : public congelado::PluginBase {
  public:
    std::string_view name()    const noexcept override { return "FileLogger"; }
    std::string_view version() const noexcept override { return "1.0.0"; }

    void on_load(const CongeladoHostCallbacks &host, const CongeladoConfigView *cfg) override {
        m_host = host;
        const char *log_file = "congelado.log";
        if (cfg) {
            for (std::size_t i = 0; i < cfg->count; ++i) {
                if (std::string_view{cfg->keys[i]} == "file") {
                    log_file = cfg->values[i];
                    break;
                }
            }
        }
        m_stream.open(log_file, std::ios::app);
        if (!m_stream.is_open()) {
            std::println(stderr, "FileLogger: failed to open {}", log_file);
            std::abort();
        }
        // Build the logger cap vtable once
        m_cap.write       = &FileLogger::cap_write;
        m_cap.write_error = &FileLogger::cap_write_error;
        m_cap.self        = this;
        // Log startup message via cap directly (logger not yet registered on host)
        write_line("INFO", std::format("FileLogger: writing to {}", log_file));
    }

    void on_unload() override {
        if (m_stream.is_open()) m_stream.close();
    }

    CongeladoLoggerCap *logger_cap() noexcept override { return &m_cap; }

  private:
    CongeladoHostCallbacks m_host{};
    CongeladoLoggerCap     m_cap{};
    std::ofstream          m_stream;

    void write_line(std::string_view level, std::string_view msg) {
        auto now  = std::chrono::system_clock::now();
        auto time = std::chrono::current_zone()->to_local(now);
        auto line = std::format("[{:%H:%M:%S}] [{}]: {}", time, level, msg);
        std::println("{}", line);
        if (m_stream.is_open()) {
            m_stream << line << '\n';
            m_stream.flush();
        }
    }

    static void cap_write(void *self, int level, const char *msg, size_t len) noexcept {
        try {
            static_cast<FileLogger *>(self)->write_line(level_str(level), {msg, len});
        } catch (...) { std::abort(); }
    }

    static void cap_write_error(void *self, const char *msg, size_t len) noexcept {
        try {
            static_cast<FileLogger *>(self)->write_line("ERROR", {msg, len});
        } catch (...) { std::abort(); }
    }
};

} // namespace

CONGELADO_PLUGIN(FileLogger)
```

- [ ] **Step 2: Build file_logger plugin**

```bash
xmake build file_logger 2>&1 | head -20
```

Expected: builds cleanly, produces `build/linux/x86_64/debug/libfile_logger.so`.

- [ ] **Step 3: Commit**

```bash
git add plugins/file_logger/file_logger.cc
git commit -m "feat(plugins): migrate file_logger to C-struct ABI via PluginBase + CONGELADO_PLUGIN"
```

---

## Task 8: Migrate `http2` plugin to new C-struct ABI

**Files:**
- Modify: `plugins/http2/http2.cc`

- [ ] **Step 1: Replace the entire file**

```cpp
#include "core/ffi/plugin_api.hpp"

import std;
import interfaces;
import io_layer_http2;
import io_shared;
import core_config;
import core_server;

namespace {

struct RouterImpl {
    core::server::Server<io::shared::http::Protocol> server;
    explicit RouterImpl(core::server::RouterContext<io::shared::http::Protocol> ctx)
        : server{core::server::ServerBuilder<io::shared::http::Protocol>{}.build(std::move(ctx))} {}
};

class Http2Plugin final : public congelado::PluginBase {
  public:
    std::string_view name()    const noexcept override { return "http2"; }
    std::string_view version() const noexcept override { return "1.0.0"; }

    void on_load(const CongeladoHostCallbacks &, const CongeladoConfigView *cfg_view) override {
        core::config::PluginConfig cfg;
        if (cfg_view) {
            for (std::size_t i = 0; i < cfg_view->count; ++i)
                cfg.fields[std::string{cfg_view->keys[i]}] = std::string{cfg_view->values[i]};
        }
        m_protocol = std::make_unique<io::layer::http2::Http2Protocol>(cfg_view ? &cfg : nullptr);

        core::server::RouterContext<io::shared::http::Protocol> ctx;
        ctx.add_route(core::server::Route<io::shared::http::Protocol>{"/hello"}.get(
            [](interfaces::IRequest<io::shared::http::Protocol> &,
               interfaces::IResponse<io::shared::http::Protocol> &res) noexcept {
                constexpr std::string_view BODY = R"({"hello":"world"})";
                std::vector<std::byte> body;
                body.reserve(BODY.size());
                for (char c : BODY)
                    body.push_back(static_cast<std::byte>(c));
                res.set_status(interfaces::Status::OK);
                res.add_header(io::shared::http::Token::CONTENT_TYPE, "application/json");
                res.set_body(std::move(body));
            }));

        m_router.reset(new RouterImpl{std::move(ctx)});
        RouterImpl *router = m_router.get();

        m_protocol->set_router([router](interfaces::IRequest<io::shared::http::Protocol> &req,
                                        interfaces::IResponse<io::shared::http::Protocol> &res) {
            auto http_method = io::shared::http::parse_method(req.get_method());
            core::server::Method method;
            switch (http_method) {
            case io::shared::http::HttpMethod::GET:     method = core::server::Method::GET;     break;
            case io::shared::http::HttpMethod::POST:    method = core::server::Method::POST;    break;
            case io::shared::http::HttpMethod::PUT:     method = core::server::Method::PUT;     break;
            case io::shared::http::HttpMethod::DELETE:  method = core::server::Method::DELETE;  break;
            case io::shared::http::HttpMethod::PATCH:   method = core::server::Method::PATCH;   break;
            case io::shared::http::HttpMethod::HEAD:    method = core::server::Method::HEAD;    break;
            case io::shared::http::HttpMethod::OPTIONS: method = core::server::Method::OPTIONS; break;
            default: return;
            }
            try {
                router->server.match(method, req.get_target(), req, res);
            } catch (const std::runtime_error &) {}
        });
    }

    void on_unload() override {
        m_protocol.reset();
        m_router.reset();
    }

    // Protocol cap — returns interfaces::IProtocol* as void* (placeholder until CongeladoProtocolCap defined)
    CongeladoProtocolCap *protocol_cap() noexcept override {
        return reinterpret_cast<CongeladoProtocolCap *>(m_protocol.get());
    }

  private:
    std::unique_ptr<io::layer::http2::Http2Protocol> m_protocol;
    std::unique_ptr<RouterImpl>                       m_router;
};

Http2Plugin g_plugin;

} // namespace

CONGELADO_PLUGIN(Http2Plugin)
```

Note: `CONGELADO_PLUGIN` allocates a new instance via `new T{}`, but `g_plugin` is a global. Remove `g_plugin` — the macro manages the instance. The static `g_plugin` is no longer needed.

Revised — remove `Http2Plugin g_plugin;` line entirely from the file above (it's already absent in the version shown — confirm before applying).

- [ ] **Step 2: Build http2 plugin**

```bash
xmake build http2 2>&1 | head -30
```

Expected: builds cleanly. If linker errors about `congelado_get_handler`, that symbol is gone — confirm the old symbols are removed.

- [ ] **Step 3: Build all targets**

```bash
xmake build 2>&1 | tail -20
```

Expected: all targets build cleanly.

- [ ] **Step 4: Run smoke test**

```bash
xmake run congelado 2>&1 &
sleep 2
curl -k --http2 https://localhost:8080/hello 2>&1
kill %1
```

Expected: `{"hello":"world"}` response, no segfault, FileLogger writes to `congelado.log`.

- [ ] **Step 5: Commit**

```bash
git add plugins/http2/http2.cc
git commit -m "feat(plugins): migrate http2 to C-struct ABI via PluginBase + CONGELADO_PLUGIN"
```

---

## Task 9: Update `xmake.lua` comment — remove stale IPluginHandler reference

**Files:**
- Modify: `xmake.lua`

- [ ] **Step 1: Update the third-party plugin comment**

Find line (currently ~193):
```lua
-- Pure C++ — no C ABI header. extern "C" only on the two factory functions.
-- NOT linked into the main binary; loaded at runtime via dlopen.
```

Replace with:
```lua
-- C++ plugins include core/ffi/plugin_api.hpp and use CONGELADO_PLUGIN(ClassName).
-- NOT linked into the main binary; loaded at runtime via dlopen.
```

- [ ] **Step 2: Build and final smoke test**

```bash
xmake build 2>&1 | tail -5
```

Expected: `build ok!`

- [ ] **Step 3: Final commit**

```bash
git add xmake.lua
git commit -m "docs(xmake): update plugin target comment for new C-struct ABI"
```

---

## Self-Review Checklist

- [x] **Spec goal 1** (replace logger-focused system): Tasks 1–3 implement the C-ABI, Tasks 7–8 migrate plugins
- [x] **Spec goal 2** (background/custom plugins): `CONGELADO_CAP_CUSTOM` defined in Task 1; `FfiBridge` in Task 3 handles null caps naturally — no code needed for custom plugins beyond loading
- [x] **Spec goal 3** (fix segfault): Task 3 adds null guard on `m_logger_cap` before all dispatch
- [x] **Spec goal 4** (noexcept/throw): Task 5 makes `write_to_plugin` `noexcept` with stderr fallback
- [x] **Spec goal 5** (multi-logger): Tasks 4–5–6 implement vector fan-out and register-all-logger-caps
- [x] **Spec goal 6** (minimize C): Tasks 1–2 confine C to `plugin_api.h`; `plugin_api.hpp` + macro covers all C++ plugin authors
- [x] **Type consistency**: `CongeladoPlugin*`, `CongeladoLoggerCap*`, `congelado::PluginBase` used consistently across all tasks
- [x] **Protocol placeholder**: Task 3 uses `void*` cast to `interfaces::IProtocol*`; Task 8 casts back — consistent placeholder approach
