# SDK Rework: Workers & Plugins Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move workers and plugins into a clean `sdk/` directory with module-first C++26 API, single-header entry points, and a pure-C++ static-registry task discovery mechanism (replacing ELF section `void*` factories). Framework-owned default `main()` with `run_worker()` escape hatch.

**Architecture:** `sdk/worker/` and `sdk/plugin/` each provide a C++26 module (`congelado_worker`, `congelado_plugin`) plus a single `<congelado/worker.h>` / `<congelado/plugin.h>` header for macros. `CONGELADO_TASK(T)` generates a static-init registration into `congelado::detail::TaskRegistry` (Meyers singleton — thread-safe, no platform tricks). `CONGELADO_PLUGIN(T)` is unchanged (already generates `extern "C"` symbols). Both SDK modules are compiled into `congelado_lib`. `sdk/worker/runtime.cc` defines `congelado::run_worker(argc, argv)`, discovering tasks via `TaskRegistry::instance().all()` instead of ELF section iteration.

**Tech Stack:** C++26 modules (clang), xmake build system, Meyers singleton registry for task discovery, libffi plugin bridge.

---

## File Map

### New files
| Path | Purpose |
|------|---------|
| `sdk/worker/congelado_worker.cppm` | Module: `ITask`, `TaskInput`, `TaskOutput`, `TaskRegistry` |
| `sdk/worker/include/congelado/worker.h` | Header: `CONGELADO_TASK` (registry-based), `run_worker` decl |
| `sdk/worker/runtime.cc` | Defines `congelado::run_worker` — uses `TaskRegistry` |
| `sdk/worker/main.cc` | Default `main()` → `return congelado::run_worker(argc, argv)` |
| `sdk/plugin/congelado_plugin.cppm` | Module: `Plugin`, `HostCallbacks`, `ConfigView` aliases |
| `sdk/plugin/include/congelado/plugin.h` | Header: includes `plugin_api.hpp`, adds `congelado::*` aliases |

### Modified files
| Path | Change |
|------|--------|
| `include/worker/task_worker.cppm` | Add `data_map()` accessor to `TaskInput` |
| `xmake.lua` | Add SDK `.cppm` + `runtime.cc` to `congelado_lib`; SDK include paths |
| `xmake/worker.lua` | Point main at `sdk/worker/main.cc`; user-override detection |
| `xmake/plugin.lua` | Update comment only (include paths already inherited via congelado_lib) |
| `workers/echo/echo_task.cc` | `import congelado_worker` + `congelado::ITask` + `type()`/`run()` |
| `plugins/file_logger/file_logger.cc` | `import congelado_plugin` + `congelado::Plugin`/`HostCallbacks`/`ConfigView` |
| `plugins/http2/http2.cc` | Same as file_logger |

### Deleted files
| Path | Replaced by |
|------|------------|
| `src/worker_main.cc` | `sdk/worker/runtime.cc` + `sdk/worker/main.cc` |
| `include/worker/task_worker.h` | `CONGELADO_TASK` macro moves to `sdk/worker/include/congelado/worker.h` |

---

## Task 1: Add `data_map()` to `worker::TaskInput`

**Files:**
- Modify: `include/worker/task_worker.cppm:50` (after `get()` method)

`runtime.cc` needs to read `TaskInput` key-value pairs to forward task data. `data_map()` exposes the internal map as a const reference.

- [ ] **Step 1: Open `include/worker/task_worker.cppm`. After the closing `}` of `get()` (around line 50), add:**

```cpp
    [[nodiscard]] std::unordered_map<std::string, std::string> const &data_map() const noexcept {
        return m_data;
    }
```

- [ ] **Step 2: Commit**

```bash
git add include/worker/task_worker.cppm
git commit -m "feat(worker): expose TaskInput::data_map() for SDK runtime bridge"
```

---

## Task 2: Create `sdk/worker/congelado_worker.cppm`

**Files:**
- Create: `sdk/worker/congelado_worker.cppm`

`congelado::ITask` is a real class (not a type alias) that bridges clean names `type()`/`run()` to the internal `get_task_type()`/`execute()` interface. `TaskRegistry` is a Meyers singleton — safe against static-init order issues, thread-safe in C++11+.

`register_task` takes a factory lambda, immediately constructs one singleton instance, and stores it by type name. Workers only ever need one live instance per task type.

- [ ] **Step 1: Create the file**

```cpp
export module congelado_worker;

export import worker:task_worker;
import std;

export namespace congelado {

using TaskInput  = worker::TaskInput;
using TaskOutput = worker::TaskOutput;

class ITask : public worker::ITaskWorker {
  public:
    [[nodiscard]] virtual std::string_view type() const noexcept = 0;
    [[nodiscard]] virtual TaskOutput       run(TaskInput const &)  = 0;

  private:
    [[nodiscard]] std::string_view get_task_type() const noexcept final { return type(); }
    [[nodiscard]] TaskOutput       execute(TaskInput const &in)   final { return run(in); }
};

namespace detail {

class TaskRegistry {
  public:
    using Factory = std::function<ITask *()>;

    static TaskRegistry &instance() noexcept {
        static TaskRegistry s_instance;
        return s_instance;
    }

    void register_task(Factory factory) {
        std::unique_ptr<ITask> instance(factory());
        auto key = std::string(instance->type());
        m_tasks.emplace(std::move(key), std::move(instance));
    }

    [[nodiscard]] std::vector<ITask *> all() const {
        std::vector<ITask *> result;
        result.reserve(m_tasks.size());
        for (auto const &[_, task] : m_tasks)
            result.push_back(task.get());
        return result;
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<ITask>> m_tasks;

    TaskRegistry()  = default;
};

} // namespace detail
} // namespace congelado
```

- [ ] **Step 2: Commit**

```bash
git add sdk/worker/congelado_worker.cppm
git commit -m "feat(sdk): add congelado_worker module — ITask, TaskInput, TaskOutput, TaskRegistry"
```

---

## Task 3: Create `sdk/worker/include/congelado/worker.h`

**Files:**
- Create: `sdk/worker/include/congelado/worker.h`

`CONGELADO_TASK(T)` generates a static-init lambda that registers `T` in the `TaskRegistry`. The anonymous namespace prevents ODR issues if the header is included in multiple TUs. `[[maybe_unused]]` suppresses -Wunused-variable. No ELF section attributes, no `void*` factories.

This header requires `import congelado_worker;` to have been done before the macro is expanded (so `congelado::ITask` and `congelado::detail::TaskRegistry` are available).

- [ ] **Step 1: Create the file**

```cpp
// NOLINTBEGIN
#pragma once

#ifdef __cplusplus
namespace congelado {
int run_worker(int argc, char **argv);
} // namespace congelado

/* CONGELADO_TASK(T): register a congelado::ITask subclass with the worker runtime.
   T must publicly inherit congelado::ITask. Drop exactly once per task class
   at file scope in the task .cc file, after the class definition.
   Requires: import congelado_worker; (provides congelado::ITask and TaskRegistry). */
#define CONGELADO_TASK(T)                                                                          \
    namespace { /* NOLINT(cert-dcl59-cpp) */                                                      \
    [[maybe_unused]] bool const _congelado_registered_##T = []() noexcept -> bool {              \
        congelado::detail::TaskRegistry::instance().register_task(                                \
            []() -> congelado::ITask * { return new T{}; } /* NOLINT */                           \
        );                                                                                        \
        return true;                                                                              \
    }();                                                                                          \
    } /* anonymous namespace */

#endif // __cplusplus
// NOLINTEND
```

- [ ] **Step 2: Commit**

```bash
git add sdk/worker/include/congelado/worker.h
git commit -m "feat(sdk): add congelado/worker.h with registry-based CONGELADO_TASK and run_worker decl"
```

---

## Task 4: Create `sdk/worker/runtime.cc`

**Files:**
- Create: `sdk/worker/runtime.cc`

Replaces `src/worker_main.cc`. Discovers tasks via `TaskRegistry::instance().all()` — no ELF section machinery. `ITask*` is implicitly convertible to `worker::ITaskWorker*` (single-inheritance chain), so `ctx.add_task_worker(w)` works directly.

- [ ] **Step 1: Create the file**

```cpp
#include <csignal>
#include "congelado/worker.h"
import congelado_worker;
import worker;
import std;

namespace {
std::atomic<bool> g_running{true};
extern "C" void on_signal(int) noexcept { g_running.store(false, std::memory_order_relaxed); }
} // namespace

namespace congelado {

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

    // 2. Get all registered task workers (populated via CONGELADO_TASK static-init)
    auto task_workers = detail::TaskRegistry::instance().all();

    // 3. Fail-fast: every registered task type must have a [[tasks]] config entry
    for (auto *w : task_workers) {
        bool found = std::ranges::any_of(cfg.tasks, [w](auto const &t) {
            return t.worker_type == w->type();
        });
        if (!found) {
            std::println(std::cerr,
                "worker: task type '{}' registered but missing [[tasks]] entry in '{}'",
                w->type(), config_path);
            return 1;
        }
    }

    // 4. Fail-fast: every [[tasks]] entry must have a registered handler
    for (auto const &task_cfg : cfg.tasks) {
        bool found = std::ranges::any_of(task_workers, [&task_cfg](auto *w) {
            return w->type() == task_cfg.worker_type;
        });
        if (!found) {
            std::println(std::cerr,
                "worker: config entry '{}' (type '{}') has no registered handler",
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

    // 6. Build WorkerContext — ITask* is ITaskWorker* (single-inheritance), add_task_worker accepts it
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

- [ ] **Step 2: Commit**

```bash
git add sdk/worker/runtime.cc
git commit -m "feat(sdk): add run_worker() using TaskRegistry — no ELF section machinery"
```

---

## Task 5: Create `sdk/worker/main.cc`

**Files:**
- Create: `sdk/worker/main.cc`

Thin wrapper. Users who need custom startup write their own `main.cc` in the worker directory; xmake detects it and skips this file.

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

## Task 6: Create `sdk/plugin/congelado_plugin.cppm`

**Files:**
- Create: `sdk/plugin/congelado_plugin.cppm`

`PluginBase` is defined in a plain header. Use the C++26 global module fragment (`module;` section) to include it before the module purview starts.

- [ ] **Step 1: Create the file**

```cpp
module;
// Global module fragment — includes non-modular header before module purview
#include "core/ffi/plugin_api.hpp"

export module congelado_plugin;

export namespace congelado {

using Plugin        = PluginBase;
using HostCallbacks = ::CongeladoHostCallbacks;
using ConfigView    = ::CongeladoConfigView;

} // namespace congelado
```

- [ ] **Step 2: Commit**

```bash
git add sdk/plugin/congelado_plugin.cppm
git commit -m "feat(sdk): add congelado_plugin module — Plugin, HostCallbacks, ConfigView"
```

---

## Task 7: Create `sdk/plugin/include/congelado/plugin.h`

**Files:**
- Create: `sdk/plugin/include/congelado/plugin.h`

Single include for plugin authors who prefer headers or need `CONGELADO_PLUGIN` / `CONGELADO_CAP_*` macros (macros cannot be exported from modules).

- [ ] **Step 1: Create the file**

```cpp
#pragma once
// Provides: congelado::PluginBase (via plugin_api.hpp), CONGELADO_PLUGIN macro,
//           CONGELADO_CAP_* macros, CongeladoHostCallbacks, CongeladoConfigView.
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
git commit -m "feat(sdk): add congelado/plugin.h header — Plugin/HostCallbacks/ConfigView aliases"
```

---

## Task 8: Update `xmake.lua`

**Files:**
- Modify: `xmake.lua`

Three additions to `congelado_lib`:
1. Compile SDK `.cppm` module files.
2. Compile `sdk/worker/runtime.cc` so `run_worker()` is linked into every worker binary.
3. Expose SDK include dirs publicly so dependent targets resolve `<congelado/worker.h>` and `<congelado/plugin.h>`.

- [ ] **Step 1: Find these two lines (around line 138–139) and replace them**

Find:
```lua
add_files("include/**.cppm", { public = true })
add_files("src/**.cc")
```

Replace with:
```lua
add_files("include/**.cppm", { public = true })
add_files("sdk/worker/congelado_worker.cppm", { public = true })
add_files("sdk/plugin/congelado_plugin.cppm", { public = true })
add_files("src/**.cc")
add_files("sdk/worker/runtime.cc")
```

- [ ] **Step 2: Find this line (around line 153) and replace it**

Find:
```lua
add_includedirs("include", { public = true })
```

Replace with:
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

## Task 9: Update `xmake/worker.lua`

**Files:**
- Modify: `xmake/worker.lua`

Point injected `main.cc` at `sdk/worker/main.cc`. Add user-override detection: if the worker directory contains its own `main.cc`, skip the SDK default.

- [ ] **Step 1: Rewrite `xmake/worker.lua`**

```lua
-- Worker targets: compiled against congelado_lib, register tasks via CONGELADO_TASK(T).
-- Bundle mode:   worker("payments_worker", "tasks/payment.cc", "tasks/refund.cc")
-- Per-task mode: worker("email_worker",    "tasks/email.cc")
-- Override main: add a main.cc in the worker directory — SDK default is skipped.
rule("congelado.worker")
on_load(function(target)
    target:add("deps", "congelado_lib")
    if not os.isfile(path.join(target:scriptdir(), "main.cc")) then
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

## Task 10: Update `xmake/plugin.lua`

**Files:**
- Modify: `xmake/plugin.lua`

SDK include dirs are already inherited via `congelado_lib` (added in Task 8). Only update the comment.

- [ ] **Step 1: Replace the comment block at the top of the file**

Find:
```lua
-- All plugins: compiled with module support, linked against congelado_lib, loaded at runtime via dlopen.
-- Each exposes congelado_get_plugin/congelado_destroy_plugin (see include/core/ffi/plugin_api.h).
-- C++ plugins inherit congelado::PluginBase and use CONGELADO_PLUGIN(T) (plugin_api.hpp).
```

Replace with:
```lua
-- All plugins: compiled with module support, linked against congelado_lib, loaded at runtime via dlopen.
-- SDK entry point: import congelado_plugin; + #include <congelado/plugin.h>
-- C++ plugins inherit congelado::Plugin and use CONGELADO_PLUGIN(T).
```

- [ ] **Step 2: Commit**

```bash
git add xmake/plugin.lua
git commit -m "build(plugin): update comment to reflect SDK entry points"
```

---

## Task 11: Delete old files

**Files:**
- Delete: `src/worker_main.cc`
- Delete: `include/worker/task_worker.h`

`src/worker_main.cc` is replaced by `sdk/worker/runtime.cc` + `sdk/worker/main.cc`.
`include/worker/task_worker.h` is replaced by `sdk/worker/include/congelado/worker.h` (the old `CONGELADO_TASK` macro lives there now).

- [ ] **Step 1: Delete both files**

```bash
git rm src/worker_main.cc include/worker/task_worker.h
git commit -m "chore: delete worker_main.cc and task_worker.h — replaced by SDK"
```

- [ ] **Step 2: Verify congelado_lib still builds**

```bash
xmake build congelado_lib
```

Expected: success. Worker example targets will fail until Task 12.

---

## Task 12: Migrate `workers/echo/echo_task.cc`

**Files:**
- Modify: `workers/echo/echo_task.cc`

Switch to `import congelado_worker` + `congelado::ITask` with `type()`/`run()` methods. `CONGELADO_TASK` now registers via static-init, not ELF section.

- [ ] **Step 1: Replace entire file contents**

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

Expected: compiles and links.

- [ ] **Step 3: Smoke test**

```bash
xmake run echo workers/echo/echo.toml
```

Expected: `worker 'echo' running: N thread(s), 1 task type(s)`. Press Ctrl+C → `worker 'echo' shutting down...`, exits 0.

- [ ] **Step 4: Commit**

```bash
git add workers/echo/echo_task.cc
git commit -m "feat(echo): migrate to congelado_worker SDK — ITask, type(), run(), registry"
```

---

## Task 13: Migrate `plugins/file_logger/file_logger.cc`

**Files:**
- Modify: `plugins/file_logger/file_logger.cc`

Switch `PluginBase` → `congelado::Plugin`, `CongeladoHostCallbacks` → `congelado::HostCallbacks`, `CongeladoConfigView` → `congelado::ConfigView`.

- [ ] **Step 1: Replace the top of the file**

Change:
```cpp
#include <stdio.h>
#include "core/ffi/plugin_api.hpp"

import std;
```

To:
```cpp
#include <stdio.h>
import congelado_plugin;
#include <congelado/plugin.h>
import std;
```

- [ ] **Step 2: Update the class declaration**

Change:
```cpp
class FileLogger final : public congelado::PluginBase {
```

To:
```cpp
class FileLogger final : public congelado::Plugin {
```

- [ ] **Step 3: Update `on_load` signature**

Change:
```cpp
void on_load(const CongeladoHostCallbacks & /*host*/, const CongeladoConfigView *cfg) override {
```

To:
```cpp
void on_load(const congelado::HostCallbacks & /*host*/, const congelado::ConfigView *cfg) override {
```

- [ ] **Step 4: Build**

```bash
xmake build file_logger
```

Expected: compiles to `file_logger.so`.

- [ ] **Step 5: Commit**

```bash
git add plugins/file_logger/file_logger.cc
git commit -m "feat(file_logger): migrate to congelado_plugin SDK"
```

---

## Task 14: Migrate `plugins/http2/http2.cc`

**Files:**
- Modify: `plugins/http2/http2.cc`

- [ ] **Step 1: Replace the top of the file**

Change:
```cpp
#include "core/ffi/plugin_api.hpp"

#include <memory>
```

To:
```cpp
#include <memory>
import congelado_plugin;
#include <congelado/plugin.h>
```

- [ ] **Step 2: Update the class declaration**

Change:
```cpp
class Http2Plugin final : public congelado::PluginBase {
```

To:
```cpp
class Http2Plugin final : public congelado::Plugin {
```

- [ ] **Step 3: Update `on_load` signature**

Change:
```cpp
void on_load(const CongeladoHostCallbacks &host, const CongeladoConfigView *cfg_view) override {
```

To:
```cpp
void on_load(const congelado::HostCallbacks &host, const congelado::ConfigView *cfg_view) override {
```

- [ ] **Step 4: Build**

```bash
xmake build http2
```

Expected: compiles to `http2.so`.

- [ ] **Step 5: Commit**

```bash
git add plugins/http2/http2.cc
git commit -m "feat(http2): migrate to congelado_plugin SDK"
```

---

## Task 15: Full build + verification

**Files:** none

- [ ] **Step 1: Full clean build**

```bash
xmake clean && xmake
```

Expected: all targets succeed — `congelado`, `congelado_lib`, `echo`, `file_logger.so`, `http2.so`.

- [ ] **Step 2: Echo worker smoke test**

```bash
xmake run echo workers/echo/echo.toml &
WPID=$!
sleep 1
kill $WPID
wait $WPID
echo "exit: $?"
```

Expected: prints running message, then shutting down, exit 0.

- [ ] **Step 3: Final commit**

```bash
git commit --allow-empty -m "chore: SDK rework complete — pure C++ registry, no ELF sections, sdk/ boundary"
```
