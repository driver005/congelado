# Worker Binary System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the `worker` module infrastructure so `src/worker_main.cc` compiles and links, and a minimal echo worker binary can start and shut down cleanly.

**Architecture:** `CONGELADO_TASK(T)` mirrors `CONGELADO_PLUGIN(T)` — generates a static singleton + places a factory pointer in an ELF linker section (`congelado_tasks`). `worker_main.cc` enumerates `__start_congelado_tasks`/`__stop_congelado_tasks` to discover all compiled-in task types. No `TaskRegistry` singleton. `EngineClient` and `Poller` are stubs that compile but do nothing.

**Tech Stack:** C++26 modules (Clang), toml++ 3.4.0 (already in congelado_lib), Linux/ELF linker sections, xmake.

---

## File Map

| Action | Path | Responsibility |
|---|---|---|
| Create | `include/worker/task_worker.cppm` | `ITaskWorker`, `TaskInput`, `TaskOutput`, `CongeladoTaskFactory` |
| Create | `include/worker/task_worker.h` | `CONGELADO_TASK(T)` macro (plain header, no module) |
| Create | `include/worker/config.cppm` | `WorkerConfig`, `TaskConfig`, toml++ parsing |
| Create | `include/worker/engine_client.cppm` | `EngineClient` stub |
| Create | `include/worker/poller.cppm` | `Poller` stub |
| Modify | `include/worker/worker.cppm` | Add all new re-exports |
| Modify | `xmake/worker.lua` | Add language/policy flags to `worker()` function |
| Rewrite | `src/worker_main.cc` | Linker section enumeration; remove `TaskRegistry` |
| Create | `workers/echo/echo_task.cc` | Smoke-test task using `CONGELADO_TASK` |
| Create | `workers/echo/echo.toml` | Config for the echo worker binary |

---

## Task 1: Create `worker:task_worker` — ITaskWorker, TaskInput, TaskOutput

**Files:**
- Create: `include/worker/task_worker.cppm`

- [ ] **Step 1: Create the file**

```cpp
export module worker:task_worker;

import std;

export namespace worker {

using CongeladoTaskFactory = void *(*)();

class TaskInput {
  public:
    explicit TaskInput(std::unordered_map<std::string, std::string> const &data) noexcept
        : m_data(data) {}

    [[nodiscard]] bool has(std::string_view key) const noexcept {
        return m_data.contains(std::string(key));
    }

    template <typename T>
        requires(std::same_as<T, std::string> || std::same_as<T, std::string_view> ||
                 std::same_as<T, int> || std::same_as<T, std::int64_t> ||
                 std::same_as<T, double> || std::same_as<T, bool>)
    [[nodiscard]] std::optional<T> get(std::string_view key) const {
        auto it = m_data.find(std::string(key));
        if (it == m_data.end()) return std::nullopt;
        auto const &s = it->second;

        if constexpr (std::same_as<T, std::string>) {
            return s;
        } else if constexpr (std::same_as<T, std::string_view>) {
            return std::string_view{s};
        } else if constexpr (std::same_as<T, int>) {
            int val{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else if constexpr (std::same_as<T, std::int64_t>) {
            std::int64_t val{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else if constexpr (std::same_as<T, double>) {
            double val{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
            return ec == std::errc{} ? std::optional{val} : std::nullopt;
        } else {
            // bool
            if (s == "true") return true;
            if (s == "false") return false;
            return std::nullopt;
        }
    }

  private:
    std::unordered_map<std::string, std::string> const &m_data;
};

class TaskOutput {
  public:
    [[nodiscard]] std::unordered_map<std::string, std::string> const &data() const noexcept {
        return m_data;
    }

    template <typename T>
    void set(std::string const &key, T const &val) {
        if constexpr (std::same_as<T, std::string>) {
            m_data[key] = val;
        } else if constexpr (std::same_as<T, std::string_view>) {
            m_data[key] = std::string(val);
        } else if constexpr (std::same_as<T, bool>) {
            m_data[key] = val ? "true" : "false";
        } else {
            m_data[key] = std::format("{}", val);
        }
    }

  private:
    std::unordered_map<std::string, std::string> m_data;
};

class ITaskWorker {
  public:
    virtual ~ITaskWorker() = default;
    [[nodiscard]] virtual std::string_view get_task_type() const noexcept = 0;
    [[nodiscard]] virtual TaskOutput execute(TaskInput const &input) = 0;
};

} // namespace worker
```

- [ ] **Step 2: Build to verify partition compiles**

```bash
xmake build congelado_lib 2>&1 | tail -10
```

Expected: errors about missing `:config`, `:engine_client`, `:poller` re-exports (not yet created) but `worker:task_worker` itself should parse cleanly. If you see an error specifically about `task_worker.cppm`, fix it before continuing.

- [ ] **Step 3: Commit**

```bash
git add include/worker/task_worker.cppm
git commit -m "feat(worker): add task_worker partition (ITaskWorker, TaskInput, TaskOutput)"
```

---

## Task 2: Create `task_worker.h` — CONGELADO_TASK macro

**Files:**
- Create: `include/worker/task_worker.h`

This is a plain header (not a module). Users `import worker;` first, then `#include "worker/task_worker.h"` to get the macro. The macro references `::worker::ITaskWorker` which is visible after the import.

- [ ] **Step 1: Create the file**

```cpp
// NOLINTBEGIN
#pragma once

/* C-compatible factory function type placed in the "congelado_tasks" ELF section.
   Matches the void* return in _congelado_task_create_##T below. */
using CongeladoTaskFactory = void *(*)();

/* Drop exactly once per task class at the bottom of your task .cc file.
   Mirrors CONGELADO_PLUGIN: generates a static singleton + places a factory pointer
   in the "congelado_tasks" linker section. worker_main discovers all compiled-in
   task types by enumerating __start_congelado_tasks / __stop_congelado_tasks.

   Supports both modes:
     Single-task binary: one CONGELADO_TASK → section has 1 entry.
     Bundle binary:      multiple CONGELADO_TASK → section has N entries.

   Usage in a task .cc file:
     import worker;
     #include "worker/task_worker.h"
     class MyTask : public worker::ITaskWorker { ... };
     CONGELADO_TASK(MyTask)

   TODO(macos): replace [[gnu::section("congelado_tasks")]] with
     __attribute__((section("__DATA,congelado_tasks")))
   and use getsectbyname("__DATA", "congelado_tasks") in worker_main.cc
   instead of __start_congelado_tasks / __stop_congelado_tasks. */
#define CONGELADO_TASK(T)  /* NOLINT(cppcoreguidelines-macro-usage) */                               \
    static T *s_task_##T = nullptr; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */ \
    static void *_congelado_task_create_##T() noexcept {                                             \
        if (s_task_##T == nullptr)                                                                   \
            s_task_##T = new T{}; /* NOLINT(cppcoreguidelines-owning-memory) */                      \
        return static_cast<void *>(static_cast<::worker::ITaskWorker *>(s_task_##T));                \
    }                                                                                                 \
    static void _congelado_task_destroy_##T() noexcept {  /* NOLINT */                               \
        if (s_task_##T != nullptr) {                                                                  \
            delete s_task_##T; /* NOLINT(cppcoreguidelines-owning-memory) */                         \
            s_task_##T = nullptr;                                                                     \
        }                                                                                             \
    }                                                                                                 \
    [[gnu::section("congelado_tasks"), gnu::used]]                                                   \
    static CongeladoTaskFactory const _task_factory_##T = &_congelado_task_create_##T; /* NOLINT */
// NOLINTEND
```

- [ ] **Step 2: Commit**

```bash
git add include/worker/task_worker.h
git commit -m "feat(worker): add CONGELADO_TASK macro (ELF section self-registration)"
```

---

## Task 3: Create `worker:config` — WorkerConfig and TaskConfig

**Files:**
- Create: `include/worker/config.cppm`

Parses this TOML format:
```toml
engine_url  = "http://localhost:8080"
worker_id   = "payments-worker-1"
concurrency = 4

[[tasks]]
name        = "process_payment"
worker_type = "payment_worker"
```

- [ ] **Step 1: Create the file**

```cpp
module;
#include <toml++/toml.hpp>

export module worker:config;

import std;

export namespace worker {

struct TaskConfig {
    std::string name;
    std::string worker_type;
};

class WorkerConfig {
  public:
    std::string              engine_url;
    std::string              worker_id;
    std::uint32_t            concurrency{0}; // 0 = std::thread::hardware_concurrency()
    std::vector<TaskConfig>  tasks;

    // Throws std::runtime_error on parse failure.
    static WorkerConfig from_file(std::string_view path) {
        toml::table tbl;
        try {
            tbl = toml::parse_file(path);
        } catch (toml::parse_error const &e) {
            throw std::runtime_error(
                std::format("worker: failed to parse '{}': {}", path, e.what()));
        }

        WorkerConfig cfg;

        if (auto v = tbl["engine_url"].value<std::string>()) cfg.engine_url = std::move(*v);
        if (auto v = tbl["worker_id"].value<std::string>())  cfg.worker_id  = std::move(*v);
        if (auto v = tbl["concurrency"].value<std::uint32_t>()) cfg.concurrency = *v;

        if (auto *arr = tbl["tasks"].as_array()) {
            for (auto &elem : *arr) {
                auto *t = elem.as_table();
                if (t == nullptr) continue;
                TaskConfig tc;
                if (auto v = (*t)["name"].value<std::string>())        tc.name        = std::move(*v);
                if (auto v = (*t)["worker_type"].value<std::string>())  tc.worker_type = std::move(*v);
                cfg.tasks.push_back(std::move(tc));
            }
        }

        return cfg;
    }
};

} // namespace worker
```

- [ ] **Step 2: Commit**

```bash
git add include/worker/config.cppm
git commit -m "feat(worker): add config partition (WorkerConfig, TaskConfig, toml++ parsing)"
```

---

## Task 4: Create `worker:engine_client` — stub

**Files:**
- Create: `include/worker/engine_client.cppm`

- [ ] **Step 1: Create the file**

```cpp
export module worker:engine_client;

import std;
import :config;

export namespace worker {

// Stub: not implemented. All methods return success.
// Real implementation: HTTP calls to the engine's REST API.
class EngineClient {
  public:
    explicit EngineClient(std::string_view engine_url) : m_url(engine_url) {}

    // Stub: always returns true. Real impl: POST /api/v1/task-defs
    [[nodiscard]] bool upsert_task_def(TaskConfig const & /*cfg*/) noexcept { return true; }

  private:
    std::string m_url;
};

} // namespace worker
```

- [ ] **Step 2: Commit**

```bash
git add include/worker/engine_client.cppm
git commit -m "feat(worker): add engine_client stub"
```

---

## Task 5: Create `worker:poller` — stub

**Files:**
- Create: `include/worker/poller.cppm`

- [ ] **Step 1: Create the file**

```cpp
export module worker:poller;

import std;
import :context;
import :engine_client;

export namespace worker {

// Stub: not implemented. start/stop/join are no-ops.
// Real implementation: spawns `concurrency` threads each running PollHandler::poll() in a loop.
class Poller {
  public:
    Poller(WorkerContext & /*ctx*/, EngineClient & /*client*/,
           std::uint32_t /*concurrency*/) noexcept {}

    void start() noexcept {}
    void stop() noexcept {}
    void join() noexcept {}
};

} // namespace worker
```

- [ ] **Step 2: Commit**

```bash
git add include/worker/poller.cppm
git commit -m "feat(worker): add poller stub"
```

---

## Task 6: Update `worker.cppm` re-exports

**Files:**
- Modify: `include/worker/worker.cppm`

- [ ] **Step 1: Replace the entire file**

```cpp
export module worker;

export import :task_worker;
export import :config;
export import :engine_client;
export import :poller;
export import :context;
export import :poll_handler;
export import :execution_handler;
export import :status_handler;
```

- [ ] **Step 2: Build to verify**

```bash
xmake build congelado_lib 2>&1 | tail -10
```

Expected: no errors.

- [ ] **Step 3: Commit**

```bash
git add include/worker/worker.cppm
git commit -m "feat(worker): re-export all worker partitions from worker.cppm"
```

---

## Task 7: Update `xmake/worker.lua` — add build flags

**Files:**
- Modify: `xmake/worker.lua`

Worker binary targets need `c++26` and modules enabled explicitly (global settings don't propagate through rules in this project).

- [ ] **Step 1: Replace the entire file**

```lua
-- Worker targets: compiled against congelado_lib, register tasks via CONGELADO_TASK(T).
-- Bundle mode:   worker("payments_worker", "tasks/payment.cc", "tasks/refund.cc")
-- Per-task mode: worker("email_worker",    "tasks/email.cc")
rule("congelado.worker")
on_load(function(target)
    target:add("deps", "congelado_lib")
    target:add("files", "$(projectdir)/src/worker_main.cc")
end)
rule_end()

function worker(name, ...)
    target(name)
        set_kind("binary")
        set_languages("c++26")
        set_policy("build.c++.modules", true)
        add_rules("congelado.worker")
        add_files(...)
        add_includedirs("include")
        if is_plat("linux", "macosx") then
            add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
        end
    target_end()
end

for _, workerdir in ipairs(os.dirs("workers/*")) do
    local name = path.basename(workerdir)
    local files = os.files(path.join(workerdir, "**.cc"))
    if #files > 0 then
        worker(name, table.unpack(files))
    end
end
```

- [ ] **Step 2: Commit**

```bash
git add xmake/worker.lua
git commit -m "build(worker): add c++26, modules policy, includedirs to worker rule"
```

---

## Task 8: Rewrite `src/worker_main.cc`

**Files:**
- Rewrite: `src/worker_main.cc`

Replaces `TaskRegistry::global().create_all()` with ELF linker section enumeration.

- [ ] **Step 1: Replace the entire file**

```cpp
#include <csignal>
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

int main(int argc, char **argv) {
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

    // 6. Build WorkerContext — singletons owned by TU-statics; context holds non-owning refs
    worker::WorkerContext ctx;
    ctx.set_worker_id(cfg.worker_id);
    for (auto *w : task_workers) {
        ctx.add_task_worker(std::unique_ptr<worker::ITaskWorker>(w, [](worker::ITaskWorker *) noexcept {}));
    }

    // 7. Start poll threads (stub: no-op)
    std::uint32_t concurrency =
        cfg.concurrency == 0 ? std::thread::hardware_concurrency() : cfg.concurrency;
    worker::Poller poller{ctx, client, concurrency};
    poller.start();

    std::println("worker '{}' running: {} thread(s), {} task type(s)",
        cfg.worker_id, concurrency, cfg.tasks.size());

    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::println("worker '{}' shutting down...", cfg.worker_id);
    poller.stop();
    poller.join();
    return 0;
}
```

- [ ] **Step 2: Commit**

```bash
git add src/worker_main.cc
git commit -m "feat(worker): rewrite worker_main with ELF section task discovery, remove TaskRegistry"
```

---

## Task 9: Smoke test — echo worker

**Files:**
- Create: `workers/echo/echo_task.cc`
- Create: `workers/echo/echo.toml`

`echo_worker` is auto-discovered by `xmake/worker.lua` (scans `workers/*/`).

- [ ] **Step 1: Create `workers/echo/echo_task.cc`**

```cpp
import worker;
#include "worker/task_worker.h"

class EchoTask : public worker::ITaskWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override {
        return "echo_worker";
    }

    [[nodiscard]] worker::TaskOutput execute(worker::TaskInput const &input) override {
        worker::TaskOutput out{};
        if (auto msg = input.get<std::string>("message")) {
            out.set("echo", *msg);
        }
        return out;
    }
};

CONGELADO_TASK(EchoTask)
```

- [ ] **Step 2: Create `workers/echo/echo.toml`**

```toml
engine_url  = "http://localhost:8080"
worker_id   = "echo-worker-1"
concurrency = 2

[[tasks]]
name        = "echo_task"
worker_type = "echo_worker"
```

- [ ] **Step 3: Build the echo_worker binary**

```bash
xmake build echo_worker 2>&1
```

Expected: `echo_worker` binary produced with no errors.

- [ ] **Step 4: Run the binary (verifies linker section discovery + startup)**

```bash
timeout 1 xmake run echo_worker workers/echo/echo.toml; true
```

Expected output (before SIGTERM shuts it down after 1s):
```
worker 'echo-worker-1' running: 2 thread(s), 1 task type(s)
worker 'echo-worker-1' shutting down...
```

If you see `task type 'echo_worker' compiled in but missing [[tasks]] entry`, the config path is wrong — pass the absolute path or run from the project root.

- [ ] **Step 5: Commit**

```bash
git add workers/echo/echo_task.cc workers/echo/echo.toml
git commit -m "feat(worker): add echo_worker smoke test (EchoTask via CONGELADO_TASK)"
```

---

## Task 10: Full build verification

- [ ] **Step 1: Build everything**

```bash
xmake build 2>&1 | tail -15
```

Expected: `congelado_lib`, `congelado`, `http2`, `echo_worker` all build with no errors.

- [ ] **Step 2: Verify section entries in echo_worker ELF**

```bash
objdump -h $(find . -name echo_worker -type f | head -1) | grep congelado
```

Expected: one line showing the `congelado_tasks` section with non-zero size.

- [ ] **Step 3: Commit if any stray fixes were needed**

```bash
git add -p
git commit -m "fix(worker): build verification fixes"
```
