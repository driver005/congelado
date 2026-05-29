# SDK Rework: Workers & Plugins

**Date:** 2026-05-29
**Scope:** Workers and plugins become a clean, installable SDK for internal and external developers.

---

## Problem

Current pain points (all four must be resolved):

1. **Messy includes** — workers need `import worker;` + `#include "worker/task_worker.h"` (two things); plugins need `#include "core/ffi/plugin_api.hpp"` which leaks raw C internals.
2. **Misplaced main** — `src/worker_main.cc` lives inside engine internals, not in the worker SDK.
3. **No SDK boundary** — external devs cannot tell which headers are theirs vs engine-internal.
4. **No C API** — tasks and plugins can only be authored in C++.

---

## Approach

**Option A + C**: SDK restructure with pure-C macro support. Designed so an install target (Option B) slots in later without further restructuring.

---

## Directory Layout

```
congelado/
├── sdk/
│   ├── worker/
│   │   ├── include/
│   │   │   └── congelado/
│   │   │       └── worker.h          ← single header; C and C++ via #ifdef __cplusplus
│   │   ├── congelado_worker.cppm     ← import congelado_worker;
│   │   └── main.cc                   ← default main() + congelado::run_worker(argc, argv)
│   └── plugin/
│       ├── include/
│       │   └── congelado/
│       │       └── plugin.h          ← single header; C and C++ via macros
│       └── congelado_plugin.cppm     ← import congelado_plugin;
├── include/                          ← engine internals (unchanged)
│   ├── engine/
│   ├── core/ffi/
│   ├── worker/
│   └── ...
├── src/
│   ├── main.cc                       ← engine binary (unchanged)
│   └── worker_main.cc                ← DELETED — replaced by sdk/worker/main.cc
├── workers/echo/                     ← updated to new SDK
├── plugins/file_logger/              ← updated to new SDK
└── xmake/
    ├── worker.lua                    ← updated
    └── plugin.lua                    ← updated
```

Engine internals (`include/worker/`, `include/core/ffi/`) stay in place. The SDK modules re-export internal types under the `congelado::` namespace via `using` declarations and namespace aliases — not copies or reimplementations.

---

## Public API Surface

### Worker SDK

```cpp
// C++ (module-first)
import congelado_worker;

class PayTask : public congelado::ITask {
    std::string_view type() const noexcept override { return "pay"; }
    congelado::TaskOutput run(congelado::TaskInput const&) override { ... }
};
CONGELADO_TASK(PayTask)
```

```c
// Pure C — macro generates flat C factory symbol
#include <congelado/worker.h>

// C structs exposed via worker.h in pure-C mode:
//   CongeladoTaskInput  { const char *const *keys; const char *const *values; size_t count; }
//   CongeladoTaskOutput { char **keys; char **values; size_t count; }  (owned by framework)
static CongeladoTaskOutput pay_run(CongeladoTaskInput *in) { ... }

// Expands to: extern "C" void *congelado_task_factory_pay() returning a C-compatible vtable stub.
// In C++ mode, CONGELADO_TASK(T) is used instead.
CONGELADO_TASK_C(pay, pay_run)
```

```cpp
// Escape hatch: user writes own main
#include <congelado/worker.h>
int main(int argc, char **argv) {
    // custom pre-init
    return congelado::run_worker(argc, argv);
}
```

### Plugin SDK

```cpp
// C++
import congelado_plugin;

class MyPlugin : public congelado::Plugin {
    std::string_view name()    const noexcept override { return "my_plugin"; }
    std::string_view version() const noexcept override { return "1.0.0"; }
    uint32_t capabilities()    const noexcept override { return CONGELADO_CAP_LOGGER; }
    void on_load(congelado::HostCallbacks const&, congelado::ConfigView const*) override { ... }
};
CONGELADO_PLUGIN(MyPlugin)
```

### Renames (intentional breaking changes)

| Old (internal)              | New (SDK public)              |
|-----------------------------|-------------------------------|
| `worker::ITaskWorker`       | `congelado::ITask`            |
| `worker::TaskInput`         | `congelado::TaskInput`        |
| `worker::TaskOutput`        | `congelado::TaskOutput`       |
| `congelado::PluginBase`     | `congelado::Plugin`           |
| `CongeladoHostCallbacks`    | `congelado::HostCallbacks` (C++ wrapper) |
| `CongeladoConfigView`       | `congelado::ConfigView` (C++ wrapper)    |

All macros (`CONGELADO_TASK`, `CONGELADO_PLUGIN`, `CONGELADO_CAP_*`) stay identical.

---

## Build Rules

**xmake/worker.lua:**
```lua
rule("congelado.worker")
on_load(function(target)
    target:add("deps", "congelado_lib")
    target:add("files", "$(projectdir)/sdk/worker/main.cc")  -- was src/worker_main.cc
    target:add("includedirs", "$(projectdir)/sdk/worker/include")
end)
rule_end()
```

User override: xmake checks `os.isfile(path.join(target:scriptdir(), "main.cc"))` — if found, skips adding `sdk/worker/main.cc`. This check lives in the `on_load` callback of the `congelado.worker` rule.

**xmake/plugin.lua:**
```lua
rule("congelado.plugin")
on_load(function(target)
    target:add("deps", "congelado_lib")
    target:add("includedirs", "$(projectdir)/sdk/plugin/include")
end)
rule_end()
```

**xmake.lua (congelado_lib)** — SDK modules added:
```lua
add_files("sdk/worker/congelado_worker.cppm")
add_files("sdk/plugin/congelado_plugin.cppm")
```

**Future install target (Option B):** `sdk/worker/include/` and `sdk/plugin/include/` are already isolated — `xmake install` copies those two trees. No further restructuring needed.

---

## Migration

**Existing workers:**
```cpp
// Before
import std;
import worker;
#include "worker/task_worker.h"
class EchoTask : public worker::ITaskWorker { ... };
CONGELADO_TASK(EchoTask)

// After
import congelado_worker;
class EchoTask : public congelado::ITask { ... };
CONGELADO_TASK(EchoTask)
```

**Existing plugins:**
```cpp
// Before
#include "core/ffi/plugin_api.hpp"
import std;
class FileLogger : public congelado::PluginBase { ... };
CONGELADO_PLUGIN(FileLogger)

// After
import congelado_plugin;
class FileLogger : public congelado::Plugin { ... };
CONGELADO_PLUGIN(FileLogger)
```

Internal engine code is untouched.

---

## Error Handling

No new error handling surface. `run_worker(argc, argv)` mirrors current `main()` return semantics: 0 on clean shutdown, 1 on config/registration failure.

---

## Testing

- `workers/echo/` updated as reference worker — must compile and run smoke test
- `plugins/file_logger/` updated as reference plugin — must compile and load
- `plugins/http2/` updated as reference protocol plugin
- Build must succeed with both `import congelado_worker;` and `#include <congelado/worker.h>`
