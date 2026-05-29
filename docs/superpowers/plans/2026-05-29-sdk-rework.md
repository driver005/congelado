# SDK Rework: Workers & Plugins Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move workers and plugins into a clean `sdk/` directory with module-first C++26 API, single-header entry points, C-style task macro support, and a framework-owned default `main()` with override escape hatch.

**Architecture:** `sdk/worker/` and `sdk/plugin/` each provide a C++26 module (`congelado_worker`, `congelado_plugin`) plus a single `<congelado/worker.h>` / `<congelado/plugin.h>` header for macros and C-compat. Both SDK modules are compiled into `congelado_lib`. The default `main()` lives in `sdk/worker/main.cc`; `run_worker()` logic lives in `sdk/worker/runtime.cc` (also in `congelado_lib`) so users can call it from a custom `main`.

**Tech Stack:** C++26 modules (clang), xmake build system, ELF linker sections for task discovery, libffi plugin bridge.

---

## File Map

### New files
| Path | Purpose |
|------|---------|
| `sdk/worker/congelado_worker.cppm` | Module: exports `congelado::ITask`, `TaskInput`, `TaskOutput` |
| `sdk/worker/include/congelado/worker.h` | Header: `CONGELADO_TASK`, `CONGELADO_TASK_C`, `run_worker` decl |
| `sdk/worker/runtime.cc` | Defines `congelado::run_worker(argc, argv)` — all task-discovery logic |
| `sdk/worker/main.cc` | Default `main()` → `return congelado::run_worker(argc, argv)` |
| `sdk/plugin/congelado_plugin.cppm` | Module: exports `congelado::Plugin`, `HostCallbacks`, `ConfigView` |
| `sdk/plugin/include/congelado/plugin.h` | Header: includes `plugin_api.hpp`, adds `congelado::Plugin`/`HostCallbacks`/`ConfigView` aliases |

### Modified files
| Path | Change |
|------|--------|
| `xmake.lua` | Add SDK `.cppm` files + `runtime.cc` to `congelado_lib`; add SDK include paths |
| `xmake/worker.lua` | Point main at `sdk/worker/main.cc`; add SDK include path; user-override detection |
| `xmake/plugin.lua` | Add SDK plugin include path |
| `workers/echo/echo_task.cc` | `import congelado_worker` + `congelado::ITask` + `type()`/`run()` |
| `plugins/file_logger/file_logger.cc` | `import congelado_plugin` + `congelado::Plugin`/`HostCallbacks`/`ConfigView` |
| `plugins/http2/http2.cc` | Same as file_logger |

### Deleted files
| Path | Replaced by |
|------|------------|
| `src/worker_main.cc` | `sdk/worker/runtime.cc` + `sdk/worker/main.cc` |

---

## Task 1: Create `sdk/worker/congelado_worker.cppm`

**Files:**
- Create: `sdk/worker/congelado_worker.cppm`

`congelado::ITask` is a new class (not a type alias) that inherits `worker::ITaskWorker` and bridges clean method names (`type()`, `run()`) to the internal virtual interface (`get_task_type()`, `execute()`). This keeps internal code unchanged while presenting a clean API.

- [ ] **Step 1: Create the file**

The global module fragment (`module;` … `export module`) is where non-modular headers live. We include `<congelado/worker.h>` there to get `CongeladoCTaskFn` for the `call_c_task` declaration. The macros in that header reference `congelado::ITask` only when _expanded_ in user code, not when the header is _included_ here — so there is no circular dependency.

```cpp
module;
// Global module fragment — non-modular headers go here.
// Pulls in CongeladoCTaskFn / CongeladoTaskInputC / CongeladoTaskOutputC for call_c_task.
// Macros in worker.h are safe here: they reference congelado::ITask only when expanded,
// not at include time.
#include <congelado/worker.h>

export module congelado_worker;

export import worker:task_worker;

export namespace congelado {

using TaskInput  = worker::TaskInput;
using TaskOutput = worker::TaskOutput;

class ITask : public worker::ITaskWorker {
  public:
    [[nodiscard]] virtual std::string_view type()  const noexcept = 0;
    [[nodiscard]] virtual TaskOutput       run(TaskInput const &) = 0;

  private:
    [[nodiscard]] std::string_view get_task_type() const noexcept final { return type(); }
    [[nodiscard]] TaskOutput       execute(TaskInput const &in)  final { return run(in); }
};

namespace detail {
// Defined in sdk/worker/runtime.cc. Bridges a C task function into the C++ TaskOutput type.
TaskOutput call_c_task(CongeladoCTaskFn fn, TaskInput const &input);
} // namespace detail

} // namespace congelado
```

- [ ] **Step 2: Commit**

```bash
git add sdk/worker/congelado_worker.cppm
git commit -m "feat(sdk): add congelado_worker module with ITask, TaskInput, TaskOutput, call_c_task"
```

---

## Task 2: Create `sdk/worker/include/congelado/worker.h`

**Files:**
- Create: `sdk/worker/include/congelado/worker.h`

This header is the only file task authors need besides `import congelado_worker;`. It provides:
- `CongeladoTaskFactory` typedef (used by the ELF section)
- `CONGELADO_TASK(T)` — C++ task registration (updated: casts via `congelado::ITask*` not `worker::ITaskWorker*`)
- `CONGELADO_TASK_C(name, fn)` — C-style task (fn is a plain C function; macro generates an adapter class)
- C structs `CongeladoTaskInputC` / `CongeladoTaskOutputC` for C-style tasks
- `congelado::run_worker(int, char**)` declaration

`CONGELADO_TASK` casts `T*` through `congelado::ITask*` before going to `void*`. This is safe: single-inheritance chain `T → congelado::ITask → worker::ITaskWorker`, no virtual bases. `runtime.cc` casts `void*` back to `worker::ITaskWorker*`.

`CONGELADO_TASK_C` generates an inline adapter class that wraps a C function pointer and calls `congelado::detail::call_c_task()`, which is defined in `runtime.cc`. This keeps the conversion logic out of the header.

- [ ] **Step 1: Create the file**

```cpp
// NOLINTBEGIN
#pragma once
#include <stddef.h>

#ifdef __cplusplus
using CongeladoTaskFactory = void *(*)();
#else
typedef void *(*CongeladoTaskFactory)();
#endif

/* C-compatible task structs — used with CONGELADO_TASK_C */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct CongeladoTaskInputC {
    const char *const *keys;
    const char *const *values;
    size_t             count;
} CongeladoTaskInputC;

typedef struct CongeladoTaskOutputC {
    const char **keys;
    const char **values;
    size_t       count;
} CongeladoTaskOutputC;

typedef CongeladoTaskOutputC (*CongeladoCTaskFn)(CongeladoTaskInputC *);

#ifdef __cplusplus
} // extern "C"
#endif

/* C++ declarations */
#ifdef __cplusplus
namespace congelado {

int run_worker(int argc, char **argv);

// congelado::detail::call_c_task is declared in the congelado_worker module (congelado_worker.cppm).
// It is NOT declared here — it requires congelado::TaskInput / TaskOutput which come from the module.
// CONGELADO_TASK_C will only compile after: import congelado_worker;

} // namespace congelado
#endif

/* CONGELADO_TASK(T): C++ task registration.
   T must publicly inherit congelado::ITask. Single inheritance required.
   Drop once per task class at the bottom of the task .cc file. */
#ifdef __cplusplus
#define CONGELADO_TASK(T)                                                                              \
    static T *s_task_##T = nullptr; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */  \
    static void *_congelado_task_create_##T() noexcept {                                              \
        if (s_task_##T == nullptr)                                                                    \
            s_task_##T = new T{}; /* NOLINT(cppcoreguidelines-owning-memory) */                       \
        return static_cast<void *>(static_cast<congelado::ITask *>(s_task_##T));                      \
    }                                                                                                  \
    static void _congelado_task_destroy_##T() noexcept {                                             \
        delete s_task_##T; /* NOLINT(cppcoreguidelines-owning-memory) */                              \
        s_task_##T = nullptr;                                                                         \
    }                                                                                                  \
    [[gnu::section("congelado_tasks"), gnu::used]]                                                    \
    static CongeladoTaskFactory const _task_factory_##T = &_congelado_task_create_##T; /* NOLINT */
#endif

/* CONGELADO_TASK_C(task_name, fn): C-style task registration.
   fn must have signature: CongeladoTaskOutputC fn(CongeladoTaskInputC *).
   Requires: import congelado_worker; (or equivalent) for congelado::ITask to be defined.
   Drop once per C task at file scope. */
#ifdef __cplusplus
#define CONGELADO_TASK_C(task_name, fn)                                                               \
    namespace {                                                                                       \
    class _CTask_##task_name final : public congelado::ITask {                                       \
      public:                                                                                         \
        [[nodiscard]] std::string_view type() const noexcept override { return #task_name; }         \
        [[nodiscard]] congelado::TaskOutput run(congelado::TaskInput const &in) override;            \
    };                                                                                               \
    congelado::TaskOutput _CTask_##task_name::run(congelado::TaskInput const &in) {                  \
        return congelado::detail::call_c_task((fn), in);                                             \
    }                                                                                                \
    } /* anonymous namespace */                                                                      \
    CONGELADO_TASK(_CTask_##task_name)
#endif

// NOLINTEND
```

- [ ] **Step 2: Commit**

```bash
git add sdk/worker/include/congelado/worker.h
git commit -m "feat(sdk): add congelado/worker.h with CONGELADO_TASK, CONGELADO_TASK_C, run_worker"
```

---

## Task 3: Create `sdk/worker/runtime.cc`

**Files:**
- Create: `sdk/worker/runtime.cc`

This replaces `src/worker_main.cc`. The `main()` logic becomes `congelado::run_worker(int argc, char **argv)`. Also defines `congelado::detail::call_c_task` for C-style task bridging.

- [ ] **Step 1: Create the file**

```cpp
#include <csignal>
#include "congelado/worker.h"
import worker;
import std;

// Linux/ELF: linker generates __start_congelado_tasks and __stop_congelado_tasks to bound
// the "congelado_tasks" section, which is populated by CONGELADO_TASK(T) in task .cc files.
// TODO(macos): use getsectbyname("__DATA", "congelado_tasks") instead.
extern "C" {
    using CTaskFactory = void *(*)();
    extern CTaskFactory __start_congelado_tasks[];
    extern CTaskFactory __stop_congelado_tasks[];
}

namespace {
std::atomic<bool> g_running{true};
extern "C" void on_signal(int) noexcept { g_running.store(false, std::memory_order_relaxed); }
} // namespace

namespace congelado {

namespace detail {

TaskOutput call_c_task(CongeladoCTaskFn fn, TaskInput const &input) {
    // Build CongeladoTaskInputC from TaskInput's internal data
    auto const &data = input.data_map();
    std::vector<const char *> keys;
    std::vector<const char *> values;
    keys.reserve(data.size());
    values.reserve(data.size());
    for (auto const &[k, v] : data) {
        keys.push_back(k.c_str());
        values.push_back(v.c_str());
    }
    CongeladoTaskInputC c_in{keys.data(), values.data(), keys.size()};
    CongeladoTaskOutputC c_out = fn(&c_in);
    TaskOutput out{};
    for (std::size_t i = 0; i < c_out.count; ++i) {
        out.set(std::string{c_out.keys[i]}, std::string_view{c_out.values[i]});
    }
    return out;
}

} // namespace detail

int run_worker(int argc, char **argv) {
    std::string_view config_path = argc > 1 ? argv[1] : "worker.toml";

    // 1. Parse worker.toml
    worker::WorkerConfig cfg;
    try {
        cfg = worker::WorkerConfig::from_file(config_path);
    } catch (std::exception const &e) {
        std::println(std::cerr, "worker: failed to load '{}': {}", config_path, e.what());
        return 1;
    }

    // 2. Enumerate compiled-in task types from ELF linker section
    std::vector<worker::ITaskWorker *> task_workers;
    for (CTaskFactory *f = __start_congelado_tasks; f != __stop_congelado_tasks; ++f) {
        task_workers.push_back(static_cast<worker::ITaskWorker *>((*f)()));
    }

    // 3. Fail-fast: every compiled-in task type must have a [[tasks]] config entry
    for (auto *w : task_workers) {
        bool found = std::ranges::any_of(cfg.tasks, [w](auto const &t) {
            return t.worker_type == w->get_task_type();
        });
        if (!found) {
            std::println(std::cerr,
                "worker: task type '{}' compiled in but missing [[tasks]] entry in '{}'",
                w->get_task_type(), config_path);
            return 1;
        }
    }

    // 4. Fail-fast: every [[tasks]] entry must have a compiled-in handler
    for (auto const &task_cfg : cfg.tasks) {
        bool found = std::ranges::any_of(task_workers, [&task_cfg](auto *w) {
            return w->get_task_type() == task_cfg.worker_type;
        });
        if (!found) {
            std::println(std::cerr,
                "worker: config entry '{}' (type '{}') has no compiled-in handler",
                task_cfg.name, task_cfg.worker_type);
            return 1;
        }
    }

    // 5. Register task defs with engine (stub: always succeeds)
    worker::EngineClient client{cfg.engine_url};
    for (auto const &task_cfg : cfg.tasks) {
        if (!client.upsert_task_def(task_cfg)) {
            std::println(std::cerr, "worker: failed to register task '{}' with engine at '{}'",
                task_cfg.name, cfg.engine_url);
            return 1;
        }
    }

    // 6. Build WorkerContext — singletons owned by TU-statics; context holds non-owning raw ptrs
    worker::WorkerContext ctx;
    ctx.set_worker_id(cfg.worker_id);
    for (auto *w : task_workers) {
        ctx.add_task_worker(w);
    }

    // 7. Register signals before starting poll threads
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    // 8. Start poll threads
    std::uint32_t concurrency =
        cfg.concurrency == 0 ? std::thread::hardware_concurrency() : cfg.concurrency;
    worker::Poller poller{ctx, client, concurrency};
    poller.start();

    std::println("worker '{}' running: {} thread(s), {} task type(s)",
        cfg.worker_id, concurrency, cfg.tasks.size());
    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::println("worker '{}' shutting down...", cfg.worker_id);
    poller.stop();
    poller.join();
    return 0;
}

} // namespace congelado
```

**Note:** `input.data_map()` does not exist yet on `worker::TaskInput`. Task 3, Step 2 below adds it.

- [ ] **Step 2: Add `data_map()` accessor to `worker::TaskInput` in `include/worker/task_worker.cppm`**

Open `include/worker/task_worker.cppm`. In the `TaskInput` class, add this public accessor after the existing `get()` method (around line 50):

```cpp
    [[nodiscard]] std::unordered_map<std::string, std::string> const &data_map() const noexcept {
        return m_data;
    }
```

- [ ] **Step 3: Commit**

```bash
git add sdk/worker/runtime.cc include/worker/task_worker.cppm
git commit -m "feat(sdk): add run_worker() runtime and call_c_task bridge; expose TaskInput::data_map()"
```

---

## Task 4: Create `sdk/worker/main.cc`

**Files:**
- Create: `sdk/worker/main.cc`

Thin wrapper. When users override `main()`, they add their own `main.cc` to the worker target; xmake detects it and skips this file.

- [ ] **Step 1: Create the file**

```cpp
#include "congelado/worker.h"

int main(int argc, char **argv) {
    return congelado::run_worker(argc, argv);
}
```

- [ ] **Step 2: Commit**

```bash
git add sdk/worker/main.cc
git commit -m "feat(sdk): add default worker main() delegating to run_worker()"
```

---

## Task 5: Create `sdk/plugin/congelado_plugin.cppm`

**Files:**
- Create: `sdk/plugin/congelado_plugin.cppm`

`PluginBase` is defined in `include/core/ffi/plugin_api.hpp` (a plain header). We include it via the global module fragment (the section before `export module` in a C++26 module interface unit).

- [ ] **Step 1: Create the file**

```cpp
module;
// Global module fragment — non-modular headers go here
#include "core/ffi/plugin_api.hpp"

export module congelado_plugin;

export namespace congelado {

using Plugin         = PluginBase;
using HostCallbacks  = ::CongeladoHostCallbacks;
using ConfigView     = ::CongeladoConfigView;

} // namespace congelado
```

- [ ] **Step 2: Commit**

```bash
git add sdk/plugin/congelado_plugin.cppm
git commit -m "feat(sdk): add congelado_plugin module with Plugin, HostCallbacks, ConfigView"
```

---

## Task 6: Create `sdk/plugin/include/congelado/plugin.h`

**Files:**
- Create: `sdk/plugin/include/congelado/plugin.h`

Provides all macros (`CONGELADO_PLUGIN`, `CONGELADO_CAP_*`) and C++ type aliases in a single include. When `import congelado_plugin;` is used alongside this header, the `using` declarations here are redundant but harmless.

- [ ] **Step 1: Create the file**

```cpp
#pragma once
// Provides: PluginBase (as congelado::PluginBase), CONGELADO_PLUGIN macro,
//           CONGELADO_CAP_* macros, CongeladoHostCallbacks, CongeladoConfigView
#include "core/ffi/plugin_api.hpp"

#ifdef __cplusplus
namespace congelado {

using Plugin        = PluginBase;
using HostCallbacks = ::CongeladoHostCallbacks;
using ConfigView    = ::CongeladoConfigView;

} // namespace congelado
#endif
```

- [ ] **Step 2: Commit**

```bash
git add sdk/plugin/include/congelado/plugin.h
git commit -m "feat(sdk): add congelado/plugin.h header with Plugin/HostCallbacks/ConfigView aliases"
```

---

## Task 7: Update `xmake.lua`

**Files:**
- Modify: `xmake.lua` (the `congelado_lib` target section)

Three changes to `congelado_lib`:
1. Add SDK `.cppm` files so worker/plugin binaries can `import` them.
2. Add `sdk/worker/runtime.cc` so `run_worker()` is linked into all worker binaries.
3. Add SDK include dirs as public so any dependent target can `#include <congelado/worker.h>`.

- [ ] **Step 1: In `xmake.lua`, update the `congelado_lib` target**

Find the line (around line 138):
```lua
add_files("include/**.cppm", { public = true })
add_files("src/**.cc")
```

Change to:
```lua
add_files("include/**.cppm", { public = true })
add_files("sdk/worker/congelado_worker.cppm", { public = true })
add_files("sdk/plugin/congelado_plugin.cppm", { public = true })
add_files("src/**.cc")
add_files("sdk/worker/runtime.cc")
```

- [ ] **Step 2: Add SDK include dirs to `congelado_lib`**

Find the line (around line 153):
```lua
add_includedirs("include", { public = true })
```

Change to:
```lua
add_includedirs("include", { public = true })
add_includedirs("sdk/worker/include", { public = true })
add_includedirs("sdk/plugin/include", { public = true })
```

- [ ] **Step 3: Commit**

```bash
git add xmake.lua
git commit -m "build: add SDK modules and runtime to congelado_lib"
```

---

## Task 8: Update `xmake/worker.lua`

**Files:**
- Modify: `xmake/worker.lua`

Two changes:
1. Point the injected `main.cc` at `sdk/worker/main.cc` instead of `src/worker_main.cc`.
2. Add user-override detection: if the worker's own directory contains a `main.cc`, skip injecting the SDK default.

- [ ] **Step 1: Rewrite `xmake/worker.lua`**

```lua
-- Worker targets: compiled against congelado_lib, register tasks via CONGELADO_TASK(T).
-- Bundle mode:   worker("payments_worker", "tasks/payment.cc", "tasks/refund.cc")
-- Per-task mode: worker("email_worker",    "tasks/email.cc")
-- Override main: add a main.cc in the worker's directory — SDK default is skipped.
rule("congelado.worker")
on_load(function(target)
    target:add("deps", "congelado_lib")
    -- Inject SDK default main unless the worker provides its own
    local worker_dir = target:scriptdir()
    if not os.isfile(path.join(worker_dir, "main.cc")) then
        target:add("files", "$(projectdir)/sdk/worker/main.cc")
    end
end)
rule_end()

function worker(name, ...)
    target(name)
    set_kind("binary")
    set_languages("c++26")
    set_policy("build.c++.modules", true)
    add_rules("congelado.worker")
    add_files(...)
    add_includedirs("$(projectdir)/include")
    if is_plat("linux", "macosx") then
        add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
    end
    target_end()
end

local workers = {}
for _, f in ipairs(os.files(path.join(os.projectdir(), "workers/**/*.cc"))) do
    local name = path.basename(path.directory(f))
    workers[name] = workers[name] or {}
    table.insert(workers[name], f)
end
for name, files in pairs(workers) do
    worker(name, table.unpack(files))
end
```

- [ ] **Step 2: Commit**

```bash
git add xmake/worker.lua
git commit -m "build(worker): use sdk/worker/main.cc; detect user main override"
```

---

## Task 9: Update `xmake/plugin.lua`

**Files:**
- Modify: `xmake/plugin.lua`

Add `sdk/plugin/include` to plugin include dirs so plugins can `#include <congelado/plugin.h>`.

Note: `sdk/plugin/include` is already a public include dir of `congelado_lib` (added in Task 7). Since plugins depend on `congelado_lib`, they inherit it. This task is a no-op in practice, but we update the comment for clarity.

- [ ] **Step 1: Update comment in `xmake/plugin.lua`**

Replace the existing first-line comment:
```lua
-- All plugins: compiled with module support, linked against congelado_lib, loaded at runtime via dlopen.
-- Each exposes congelado_get_plugin/congelado_destroy_plugin (see include/core/ffi/plugin_api.h).
-- C++ plugins inherit congelado::PluginBase and use CONGELADO_PLUGIN(T) (plugin_api.hpp).
```

With:
```lua
-- All plugins: compiled with module support, linked against congelado_lib, loaded at runtime via dlopen.
-- SDK entry point: import congelado_plugin; + #include <congelado/plugin.h>
-- C++ plugins inherit congelado::Plugin and use CONGELADO_PLUGIN(T).
```

- [ ] **Step 2: Commit**

```bash
git add xmake/plugin.lua
git commit -m "build(plugin): update comment to SDK entry points"
```

---

## Task 10: Delete `src/worker_main.cc`

**Files:**
- Delete: `src/worker_main.cc`

`runtime.cc` (Task 3) and `main.cc` (Task 4) replace it entirely.

- [ ] **Step 1: Delete the file**

```bash
git rm src/worker_main.cc
git commit -m "chore: delete src/worker_main.cc — replaced by sdk/worker/runtime.cc + main.cc"
```

- [ ] **Step 2: Verify build still works (SDK not yet wired to examples)**

```bash
xmake build congelado_lib
```

Expected output: build succeeds. Worker and plugin example targets will fail to compile until Tasks 11–13.

---

## Task 11: Migrate `workers/echo/echo_task.cc`

**Files:**
- Modify: `workers/echo/echo_task.cc`

Update to use `import congelado_worker` + new API (`congelado::ITask`, `type()`, `run()`).

- [ ] **Step 1: Replace file contents**

```cpp
import congelado_worker;
#include <congelado/worker.h>

class EchoTask : public congelado::ITask {
  public:
    [[nodiscard]] std::string_view type() const noexcept override {
        return "echo_worker";
    }

    [[nodiscard]] congelado::TaskOutput run(congelado::TaskInput const &input) override {
        congelado::TaskOutput out{};
        if (auto msg = input.get<std::string>("message")) {
            out.set("echo", *msg);
        }
        return out;
    }
};

CONGELADO_TASK(EchoTask)
```

- [ ] **Step 2: Build the echo worker**

```bash
xmake build echo
```

Expected: compiles and links successfully.

- [ ] **Step 3: Smoke test — run the echo worker with its config**

```bash
xmake run echo workers/echo/echo.toml
```

Expected: prints `worker 'echo' running: N thread(s), 1 task type(s)`, then waits. Press Ctrl+C — it should print `worker 'echo' shutting down...` and exit 0.

- [ ] **Step 4: Commit**

```bash
git add workers/echo/echo_task.cc
git commit -m "feat(echo): migrate to congelado_worker SDK (ITask, type(), run())"
```

---

## Task 12: Migrate `plugins/file_logger/file_logger.cc`

**Files:**
- Modify: `plugins/file_logger/file_logger.cc`

Update to use `import congelado_plugin` + `congelado::Plugin` / `congelado::HostCallbacks` / `congelado::ConfigView`.

- [ ] **Step 1: Replace the include and base class**

Change the top of the file from:
```cpp
#include <stdio.h>
#include "core/ffi/plugin_api.hpp"

import std;
```
to:
```cpp
#include <stdio.h>
import congelado_plugin;
#include <congelado/plugin.h>
import std;
```

Change the class declaration from:
```cpp
class FileLogger final : public congelado::PluginBase {
```
to:
```cpp
class FileLogger final : public congelado::Plugin {
```

Change the `on_load` signature from:
```cpp
void on_load(const CongeladoHostCallbacks & /*host*/, const CongeladoConfigView *cfg) override {
```
to:
```cpp
void on_load(const congelado::HostCallbacks & /*host*/, const congelado::ConfigView *cfg) override {
```

Leave the rest of the file unchanged.

- [ ] **Step 2: Build the plugin**

```bash
xmake build file_logger
```

Expected: compiles and links to `file_logger.so`.

- [ ] **Step 3: Commit**

```bash
git add plugins/file_logger/file_logger.cc
git commit -m "feat(file_logger): migrate to congelado_plugin SDK (Plugin, HostCallbacks, ConfigView)"
```

---

## Task 13: Migrate `plugins/http2/http2.cc`

**Files:**
- Modify: `plugins/http2/http2.cc`

Same pattern as Task 12.

- [ ] **Step 1: Replace the include and base class**

Change the top of the file from:
```cpp
#include "core/ffi/plugin_api.hpp"

#include <memory>

import std;
```
to:
```cpp
#include <memory>
import congelado_plugin;
#include <congelado/plugin.h>
import std;
```

Change the class declaration from:
```cpp
class Http2Plugin final : public congelado::PluginBase {
```
to:
```cpp
class Http2Plugin final : public congelado::Plugin {
```

Change the `on_load` signature from:
```cpp
void on_load(const CongeladoHostCallbacks &host, const CongeladoConfigView *cfg_view) override {
```
to:
```cpp
void on_load(const congelado::HostCallbacks &host, const congelado::ConfigView *cfg_view) override {
```

Leave the rest of the file unchanged.

- [ ] **Step 2: Build the plugin**

```bash
xmake build http2
```

Expected: compiles and links to `http2.so`.

- [ ] **Step 3: Commit**

```bash
git add plugins/http2/http2.cc
git commit -m "feat(http2): migrate to congelado_plugin SDK (Plugin, HostCallbacks, ConfigView)"
```

---

## Task 14: Full build + final verification

**Files:** none

- [ ] **Step 1: Full clean build**

```bash
xmake clean && xmake
```

Expected: all targets build without errors — `congelado` (engine binary), `congelado_lib`, `echo` (worker binary), `file_logger.so`, `http2.so`.

- [ ] **Step 2: Verify echo worker smoke test still passes**

```bash
xmake run echo workers/echo/echo.toml &
sleep 1
kill %1
```

Expected: prints running message, then shutting down, exits 0.

- [ ] **Step 3: Verify header-only usage compiles**

Create a temporary file to confirm `#include <congelado/worker.h>` without `import congelado_worker;` works for the CONGELADO_TASK macro resolution (macros don't require the module):

```bash
cat > /tmp/test_header_only.cc << 'EOF'
import congelado_worker;
#include <congelado/worker.h>

class TestTask : public congelado::ITask {
    std::string_view type() const noexcept override { return "test"; }
    congelado::TaskOutput run(congelado::TaskInput const &) override { return {}; }
};
CONGELADO_TASK(TestTask)
EOF
xmake run --test /tmp/test_header_only.cc 2>&1 | head -5
```

If xmake doesn't support ad-hoc compilation, skip this step — the echo worker migration in Task 11 already validates the full path.

- [ ] **Step 4: Final commit with summary**

```bash
git commit --allow-empty -m "chore: SDK rework complete — sdk/worker + sdk/plugin replace src/worker_main.cc"
```
