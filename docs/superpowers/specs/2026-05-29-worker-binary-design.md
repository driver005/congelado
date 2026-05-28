# Worker Binary System — Design Spec

**Date:** 2026-05-29  
**Status:** Approved

---

## Goal

Implement the `worker` module infrastructure that makes `src/worker_main.cc` compile and link. Workers are standalone binaries that poll a workflow engine for tasks, execute them, and return results. The handler layer (PollHandler, ExecutionHandler, StatusHandler) already exists as stubs and is out of scope here.

---

## Architecture

### Deployment Modes

Two modes, one macro, one enumeration mechanism:

**Single-task binary:** one `CONGELADO_TASK(T)` in one `.cc` → section has 1 entry.  
**Bundle binary:** multiple `CONGELADO_TASK(T)` across multiple `.cc` files → section has N entries.

`worker_main.cc` always does the same thing: iterates the linker section. No special casing.

### Mirror of Plugin Pattern

| Plugin (dynamic `.so`) | Task (static binary) |
|---|---|
| `static T* s_plugin` | `static T* s_task_T` |
| `extern "C"` symbols | linker section entry in `congelado_tasks` |
| Bridge discovers via `dlsym` | `worker_main` enumerates `__start_congelado_tasks` / `__stop_congelado_tasks` |
| One plugin per `.so` | One or many tasks per binary |

---

## Components

### `include/worker/task_worker.h` (plain header)

Defines `ITaskWorker`, `TaskInput`, `TaskOutput`, `CongeladoTaskFactory`, and `CONGELADO_TASK(T)`.

This is a plain header (not a module) so the macro can reference `worker::ITaskWorker` from the user's `.cc` context, where `import worker;` has already been evaluated.

```cpp
// TODO(macos): on macOS/Mach-O, replace [[gnu::section("congelado_tasks")]] with
// __attribute__((section("__DATA,congelado_tasks"))) and use getsectbyname() in
// worker_main.cc instead of __start_congelado_tasks / __stop_congelado_tasks.

using CongeladoTaskFactory = void*(*)();

#define CONGELADO_TASK(T)                                                              \
    /* NOLINT(cppcoreguidelines-macro-usage) */                                        \
    static T *s_task_##T = nullptr;                                                    \
    /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */                   \
    static void *_congelado_task_create_##T() noexcept {                              \
        if (s_task_##T == nullptr) s_task_##T = new T{};                             \
        return static_cast<void *>(static_cast<::worker::ITaskWorker *>(s_task_##T)); \
    }                                                                                  \
    static void _congelado_task_destroy_##T() noexcept {               /* NOLINT */   \
        if (s_task_##T != nullptr) {                                                   \
            delete s_task_##T; /* NOLINT(cppcoreguidelines-owning-memory) */          \
            s_task_##T = nullptr;                                                      \
        }                                                                              \
    }                                                                                  \
    [[gnu::section("congelado_tasks"), gnu::used]]                                    \
    static CongeladoTaskFactory const _task_factory_##T = &_congelado_task_create_##T;
```

**User task file pattern:**
```cpp
import worker;
#include "worker/task_worker.h"

class EmailTask : public worker::ITaskWorker {
public:
    std::string_view get_task_type() const noexcept override { return "email_worker"; }
    worker::TaskOutput execute(worker::TaskInput const& input) override { ... }
};

CONGELADO_TASK(EmailTask)
```

---

### `include/worker/task_worker.cppm` — partition `worker:task_worker`

Defines the abstract interface and typed I/O wrappers.

**`ITaskWorker`:**
```cpp
class ITaskWorker {
public:
    virtual ~ITaskWorker() = default;
    [[nodiscard]] virtual std::string_view get_task_type() const noexcept = 0;
    [[nodiscard]] virtual TaskOutput execute(TaskInput const& input) = 0;
};
```

**`TaskInput`** — read-only view over `unordered_map<string,string>`:
- `get<T>(key) -> optional<T>` — parses string to T via `from_chars` / stream
- `has(key) -> bool`
- Supported T: `std::string`, `std::string_view`, `int`, `int64_t`, `double`, `bool`

**`TaskOutput`** — owned result map:
- `set<T>(key, val)` — serialises T to string via `to_string` / `format`
- `data() -> unordered_map<string,string> const&`

---

### `include/worker/config.cppm` — partition `worker:config`

Parses `worker.toml` using toml++ (same pattern as `core_config:loader`).

**TOML format:**
```toml
engine_url  = "http://localhost:8080"
worker_id   = "payments-worker-1"
concurrency = 4        # poll thread count; defaults to hardware_concurrency()

[[tasks]]
name        = "process_payment"   # TaskDef name registered on engine
worker_type = "payment_worker"    # must match ITaskWorker::get_task_type()
```

**Types:**
```cpp
struct TaskConfig {
    std::string name;
    std::string worker_type;
};

class WorkerConfig {
public:
    static WorkerConfig from_file(std::string_view path);  // throws on parse error

    std::string               engine_url;
    std::string               worker_id;
    std::uint32_t             concurrency{0};  // 0 = std::thread::hardware_concurrency()
    std::vector<TaskConfig>   tasks;
};
```

---

### `include/worker/engine_client.cppm` — partition `worker:engine_client`

Stub only. All methods return success. Networking not implemented.

```cpp
class EngineClient {
public:
    explicit EngineClient(std::string_view engine_url);

    // Stub: always returns true. Real impl: HTTP POST /api/v1/task-defs
    [[nodiscard]] bool upsert_task_def(TaskConfig const& cfg) noexcept;
};
```

---

### `include/worker/poller.cppm` — partition `worker:poller`

Stub only. `start()`/`stop()`/`join()` are no-ops.

```cpp
class Poller {
public:
    Poller(WorkerContext& ctx, EngineClient& client, std::uint32_t concurrency);
    void start();   // stub: no-op
    void stop();    // stub: no-op
    void join();    // stub: no-op
};
```

Real implementation (out of scope): spawns `concurrency` threads, each calling `PollHandler::poll()` in a loop.

---

### `include/worker/worker.cppm` — module root

Re-exports all partitions:
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

---

### `src/worker_main.cc` — rewritten

Uses linker section enumeration instead of `TaskRegistry`:

```cpp
import worker;
import std;

// Linux/ELF: linker generates these symbols bounding the congelado_tasks section.
// TODO(macos): replace with getsectbyname("__DATA", "congelado_tasks").
extern "C" {
    extern CongeladoTaskFactory __start_congelado_tasks[];
    extern CongeladoTaskFactory __stop_congelado_tasks[];
}
```

Startup sequence:
1. Parse `worker.toml` → `WorkerConfig`
2. Enumerate `congelado_tasks` section → build `vector<ITaskWorker*>`
3. Fail-fast: every compiled-in task type must have a `[[tasks]]` entry
4. Fail-fast: every `[[tasks]]` entry must have a compiled-in task type
5. `EngineClient::upsert_task_def()` for each task (stub: always succeeds)
6. Build `WorkerContext`, start `Poller`
7. Signal handler loop; clean shutdown on SIGINT/SIGTERM

---

### `xmake/worker.lua`

Already scaffolded. No changes needed — `src/worker_main.cc` links into every worker binary via the `congelado.worker` rule.

---

## File Map

| Action | Path | Notes |
|---|---|---|
| Create | `include/worker/task_worker.h` | Header: ITaskWorker, TaskInput, TaskOutput, CONGELADO_TASK macro |
| Create | `include/worker/task_worker.cppm` | Partition: same types exported as module |
| Create | `include/worker/config.cppm` | Partition: WorkerConfig, TaskConfig |
| Create | `include/worker/engine_client.cppm` | Partition: EngineClient stub |
| Create | `include/worker/poller.cppm` | Partition: Poller stub |
| Modify | `include/worker/worker.cppm` | Add all new re-exports |
| Rewrite | `src/worker_main.cc` | Linker section enumeration |
| No change | `include/worker/context.cppm` | Already correct |
| No change | `include/worker/handler/*.cppm` | Stubs, out of scope |
| No change | `xmake/worker.lua` | Already scaffolded |

---

## Success Criteria

- `xmake build congelado_lib` succeeds
- A minimal `workers/echo/echo_task.cc` with `CONGELADO_TASK(EchoTask)` compiles into `echo_worker` binary
- `echo_worker` starts, logs its task type, and shuts down cleanly on SIGINT
- No `TaskRegistry` type exists anywhere in the codebase
