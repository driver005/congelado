# Workflow Model Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the 12-file pure data model layer for the workflow engine under `include/model/`, covering identifiers, timestamps, policies, task definitions, task instances, and workflow DAG/execution/event types.

**Architecture:** All model types live in the `model` C++ module (partitioned). No execution logic — pure data definitions with two behavioral functions (`is_terminal` for both status enums, `generate_id` for UUID creation). The top-level `model.cppm` re-exports all partitions so callers `import model;`.

**Tech Stack:** C++26 modules, stduuid 1.2.3 (`uuids::uuid_system_generator`), Catch2 3.7.1 for tests, xmake build system.

---

### Task 1: xmake — add stduuid to congelado_lib and enable model test target

**Files:**
- Modify: `xmake.lua:154-170` (add stduuid to congelado_lib packages)
- Modify: `xmake.lua:229-239` (add model_test target, currently commented out)

- [ ] **Step 1: Add stduuid to congelado_lib packages**

In `xmake.lua`, find the `add_packages(...)` block inside `target("congelado_lib")` (around line 154). Add `"stduuid"` before `{ public = true }`:

```lua
add_packages(
    "fmt",
    "simdjson",
    "tomlplusplus",
    "grpc",
    "protobuf",
    "asio",
    "openssl",
    "nghttp2",
    "ngtcp2",
    "nghttp3",
    "backward",
    "libffi",
    "microsoft-gsl",
    "range-v3",
    "stduuid",
    { public = true }
)
```

- [ ] **Step 2: Add model_test target**

Append after `target_end()` for `congelado` (before the commented-out benchmark block):

```lua
target("model_test")
    set_kind("binary")
    set_languages("c++26")
    set_policy("build.c++.modules", true)
    add_files("tests/model/model_test.cc")
    add_deps("congelado_lib")
    add_packages("catch2")
    add_cxflags("-fpermissive")
    if is_plat("linux", "macosx") then
        add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
    end
    add_tests("default")
target_end()
```

- [ ] **Step 3: Verify xmake config parses**

```bash
xmake f --check
```

Expected: no errors about unknown packages or syntax.

- [ ] **Step 4: Commit**

```bash
git add xmake.lua
git commit -m "build: add stduuid to congelado_lib, add model_test target"
```

---

### Task 2: model:identifiers — UUID type aliases and generate_id()

**Files:**
- Create: `include/model/common/identifiers.cppm`

- [ ] **Step 1: Create model.cppm stub**

Create `include/model/model.cppm` (empty shell — will grow each task):

```cpp
export module model;

export import :identifiers;
```

- [ ] **Step 2: Write the failing test stub**

Create `tests/model/model_test.cc`. The `#include <uuid.h>` must come before `import model;` so the compiler knows `uuids::uuid` member functions (`.is_nil()`, `operator!=`) when it processes the test:

```cpp
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>
#include <catch2/catch_test_macros.hpp>
import model;

TEST_CASE("generate_id produces non-nil UUIDs") {
    auto id1 = model::generate_id();
    auto id2 = model::generate_id();
    CHECK_FALSE(id1.is_nil());
    CHECK_FALSE(id2.is_nil());
    CHECK(id1 != id2);
}
```

- [ ] **Step 3: Verify test fails to build (identifiers partition doesn't exist yet)**

```bash
xmake build model_test 2>&1 | head -5
```

Expected: error about `model:identifiers` partition not found.

- [ ] **Step 4: Create include/model/common/identifiers.cppm**

```cpp
module;
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

export module model:identifiers;

import std;

export namespace model {

using WorkflowId    = uuids::uuid;
using TaskId        = uuids::uuid;
using ExecutionId   = uuids::uuid;
using CorrelationId = uuids::uuid;

inline ExecutionId generate_id() {
    static uuids::uuid_system_generator gen{};
    return gen();
}

} // namespace model
```

- [ ] **Step 5: Build model_test**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds.

- [ ] **Step 6: Run test**

```bash
xmake run model_test
```

Expected:
```
All tests passed (3 assertions in 1 test case)
```

- [ ] **Step 7: Commit**

```bash
git add include/model/model.cppm include/model/common/identifiers.cppm tests/model/model_test.cc
git commit -m "feat(model): add identifiers partition with UUID type aliases and generate_id"
```

---

### Task 3: model:timestamps and model:audit — time tracking structs

**Files:**
- Create: `include/model/common/timestamps.cppm`
- Create: `include/model/common/audit.cppm`

- [ ] **Step 1: Create include/model/common/timestamps.cppm**

```cpp
export module model:timestamps;

import std;

export namespace model {

struct ExecutionTimings {
    std::optional<std::chrono::system_clock::time_point> scheduled_at;
    std::optional<std::chrono::system_clock::time_point> started_at;
    std::optional<std::chrono::system_clock::time_point> completed_at;
};

} // namespace model
```

- [ ] **Step 2: Create include/model/common/audit.cppm**

```cpp
export module model:audit;

import std;

export namespace model {

struct AuditRecord {
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::uint32_t version{0};
};

} // namespace model
```

- [ ] **Step 3: Add partitions to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
```

- [ ] **Step 4: Build to verify**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds, existing tests still pass.

- [ ] **Step 5: Commit**

```bash
git add include/model/common/timestamps.cppm include/model/common/audit.cppm include/model/model.cppm
git commit -m "feat(model): add timestamps and audit partitions"
```

---

### Task 4: model:policies — retry, timeout, rate-limit policies

**Files:**
- Create: `include/model/common/policies.cppm`

- [ ] **Step 1: Create include/model/common/policies.cppm**

```cpp
export module model:policies;

import std;

export namespace model {

enum class RetryBackoff : std::uint8_t {
    FIXED,
    EXPONENTIAL,
};

struct RetryPolicy {
    std::uint32_t max_attempts{3};
    RetryBackoff  backoff{RetryBackoff::FIXED};
    std::uint32_t interval_ms{1000};
};

enum class TimeoutAction : std::uint8_t {
    RETRY,
    FAIL_WORKFLOW,
    ALERT_ONLY,
};

struct TimeoutPolicy {
    std::uint32_t timeout_ms{30000};
    TimeoutAction action{TimeoutAction::FAIL_WORKFLOW};
};

struct RateLimitPolicy {
    std::uint32_t max_concurrent{10};
    std::uint32_t rate_limit_per_second{100};
};

} // namespace model
```

- [ ] **Step 2: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
```

- [ ] **Step 3: Build**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds.

- [ ] **Step 4: Commit**

```bash
git add include/model/common/policies.cppm include/model/model.cppm
git commit -m "feat(model): add policies partition (RetryPolicy, TimeoutPolicy, RateLimitPolicy)"
```

---

### Task 5: model:task_status — task enums and is_terminal

**Files:**
- Create: `include/model/task/task_status.cppm`

- [ ] **Step 1: Add is_terminal(TaskStatus) test to model_test.cc**

Append to `tests/model/model_test.cc`:

```cpp
TEST_CASE("is_terminal(TaskStatus)") {
    using enum model::TaskStatus;
    CHECK(model::is_terminal(COMPLETED));
    CHECK(model::is_terminal(FAILED));
    CHECK(model::is_terminal(TIMED_OUT));
    CHECK(model::is_terminal(SKIPPED));
    CHECK(model::is_terminal(CANCELED));
    CHECK_FALSE(model::is_terminal(SCHEDULED));
    CHECK_FALSE(model::is_terminal(IN_PROGRESS));
}
```

- [ ] **Step 2: Verify test fails to build (task_status not defined yet)**

```bash
xmake build model_test 2>&1 | head -5
```

Expected: compile error about `model::TaskStatus`.

- [ ] **Step 3: Create include/model/task/task_status.cppm**

```cpp
export module model:task_status;

import std;

export namespace model {

enum class TaskStatus : std::uint8_t {
    SCHEDULED,
    IN_PROGRESS,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    SKIPPED,
    CANCELED,
};

enum class TaskType : std::uint8_t {
    SIMPLE,
    FORK,
    JOIN,
    SWITCH,
    SUB_WORKFLOW,
};

enum class TaskResult : std::uint8_t {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    SKIPPED,
};

[[nodiscard]] constexpr bool is_terminal(TaskStatus s) noexcept {
    using enum TaskStatus;
    return s == COMPLETED || s == FAILED || s == TIMED_OUT
        || s == SKIPPED    || s == CANCELED;
}

} // namespace model
```

- [ ] **Step 4: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
```

- [ ] **Step 5: Build and run tests**

```bash
xmake build model_test && xmake run model_test
```

Expected: all tests pass.

- [ ] **Step 6: Commit**

```bash
git add include/model/task/task_status.cppm include/model/model.cppm tests/model/model_test.cc
git commit -m "feat(model): add task_status partition with TaskStatus/TaskType/TaskResult enums and is_terminal"
```

---

### Task 6: model:task_def — pure task definition

**Files:**
- Create: `include/model/task/task_def.cppm`

- [ ] **Step 1: Create include/model/task/task_def.cppm**

```cpp
export module model:task_def;

import std;
import :task_status;
import :policies;

export namespace model {

struct TaskDef {
    std::string                    name;
    TaskType                       type{TaskType::SIMPLE};
    std::string                    worker_type;
    std::vector<std::string>       input_keys;
    std::vector<std::string>       output_keys;
    RetryPolicy                    retry{};
    TimeoutPolicy                  timeout{};
    std::optional<RateLimitPolicy> rate_limit;
};

} // namespace model
```

- [ ] **Step 2: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
```

- [ ] **Step 3: Build**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds, all existing tests pass.

- [ ] **Step 4: Commit**

```bash
git add include/model/task/task_def.cppm include/model/model.cppm
git commit -m "feat(model): add task_def partition"
```

---

### Task 7: model:task_instance — runtime task snapshot

**Files:**
- Create: `include/model/task/task_instance.cppm`

- [ ] **Step 1: Create include/model/task/task_instance.cppm**

```cpp
export module model:task_instance;

import std;
import :identifiers;
import :timestamps;
import :task_status;

export namespace model {

struct TaskInstance {
    TaskId                                        task_id;
    std::string                                   def_name;
    ExecutionId                                   workflow_exec_id;
    TaskStatus                                    status{TaskStatus::SCHEDULED};
    std::uint32_t                                 seq{0};
    std::unordered_map<std::string, std::string>  input_data;
    std::unordered_map<std::string, std::string>  output_data;
    ExecutionTimings                              timings{};
    std::uint32_t                                 retry_count{0};
};

} // namespace model
```

- [ ] **Step 2: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
```

- [ ] **Step 3: Build**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds.

- [ ] **Step 4: Commit**

```bash
git add include/model/task/task_instance.cppm include/model/model.cppm
git commit -m "feat(model): add task_instance partition"
```

---

### Task 8: model:workflow_status — workflow status enum and is_terminal

**Files:**
- Create: `include/model/workflow/workflow_status.cppm`

- [ ] **Step 1: Add is_terminal(WorkflowStatus) test**

Append to `tests/model/model_test.cc`:

```cpp
TEST_CASE("is_terminal(WorkflowStatus)") {
    using enum model::WorkflowStatus;
    CHECK(model::is_terminal(COMPLETED));
    CHECK(model::is_terminal(FAILED));
    CHECK(model::is_terminal(TIMED_OUT));
    CHECK(model::is_terminal(TERMINATED));
    CHECK_FALSE(model::is_terminal(RUNNING));
    CHECK_FALSE(model::is_terminal(PAUSED));
}
```

- [ ] **Step 2: Verify test fails to build**

```bash
xmake build model_test 2>&1 | head -5
```

Expected: error about `model::WorkflowStatus`.

- [ ] **Step 3: Create include/model/workflow/workflow_status.cppm**

```cpp
export module model:workflow_status;

import std;

export namespace model {

enum class WorkflowStatus : std::uint8_t {
    RUNNING,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    PAUSED,
    TERMINATED,
};

[[nodiscard]] constexpr bool is_terminal(WorkflowStatus s) noexcept {
    using enum WorkflowStatus;
    return s == COMPLETED || s == FAILED || s == TIMED_OUT || s == TERMINATED;
}

} // namespace model
```

- [ ] **Step 4: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
```

- [ ] **Step 5: Build and run tests**

```bash
xmake build model_test && xmake run model_test
```

Expected: all tests pass.

- [ ] **Step 6: Commit**

```bash
git add include/model/workflow/workflow_status.cppm include/model/model.cppm tests/model/model_test.cc
git commit -m "feat(model): add workflow_status partition with is_terminal"
```

---

### Task 9: model:workflow_dag — DAG graph primitives

**Files:**
- Create: `include/model/workflow/workflow_dag.cppm`

- [ ] **Step 1: Create include/model/workflow/workflow_dag.cppm**

```cpp
export module model:workflow_dag;

import std;

export namespace model {

struct InputMapping {
    std::string source; // "$.task_name.output.key" or "$.workflow.input.key"
    std::string target; // input_key name on the receiving task
};

struct OutputMapping {
    std::string source; // "$.task_name.output.key"
    std::string target; // workflow output key name
};

struct TaskEdge {
    std::string                  from;
    std::string                  to;
    std::optional<std::string>   condition; // expression string; null on non-SWITCH edges
    std::vector<InputMapping>    mappings;
};

struct TaskNode {
    std::string           task_def_name;
    std::vector<TaskEdge> edges;
};

} // namespace model
```

- [ ] **Step 2: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
export import :workflow_dag;
```

- [ ] **Step 3: Build**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds.

- [ ] **Step 4: Commit**

```bash
git add include/model/workflow/workflow_dag.cppm include/model/model.cppm
git commit -m "feat(model): add workflow_dag partition (TaskNode, TaskEdge, InputMapping, OutputMapping)"
```

---

### Task 10: model:workflow_def — workflow definition

**Files:**
- Create: `include/model/workflow/workflow_def.cppm`

- [ ] **Step 1: Create include/model/workflow/workflow_def.cppm**

```cpp
export module model:workflow_def;

import std;
import :workflow_dag;
import :policies;

export namespace model {

struct WorkflowDef {
    std::string                  name;
    std::uint32_t                version{1};
    std::vector<TaskNode>        nodes;
    std::vector<std::string>     input_params;
    std::vector<OutputMapping>   output_mappings;
    std::optional<std::string>   failure_workflow; // name of fallback WorkflowDef; spawned on terminal FAILED
    std::optional<TimeoutPolicy> timeout;
};

} // namespace model
```

- [ ] **Step 2: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
export import :workflow_dag;
export import :workflow_def;
```

- [ ] **Step 3: Build**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds.

- [ ] **Step 4: Commit**

```bash
git add include/model/workflow/workflow_def.cppm include/model/model.cppm
git commit -m "feat(model): add workflow_def partition"
```

---

### Task 11: model:workflow_exec — runtime workflow execution state

**Files:**
- Create: `include/model/workflow/workflow_exec.cppm`

- [ ] **Step 1: Create include/model/workflow/workflow_exec.cppm**

```cpp
export module model:workflow_exec;

import std;
import :identifiers;
import :timestamps;
import :workflow_status;
import :task_instance;

export namespace model {

struct WorkflowExecution {
    ExecutionId                                   exec_id;
    std::string                                   def_name;
    std::uint32_t                                 def_version{1};
    WorkflowStatus                                status{WorkflowStatus::RUNNING};
    std::optional<CorrelationId>                  correlation_id;
    std::unordered_map<std::string, std::string>  variables;    // shared execution context; both explicit mappings and task outputs land here
    std::vector<TaskInstance>                     task_instances;
    ExecutionTimings                              timings{};
};

} // namespace model
```

- [ ] **Step 2: Add to model.cppm**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
export import :workflow_dag;
export import :workflow_def;
export import :workflow_exec;
```

- [ ] **Step 3: Build**

```bash
xmake build model_test 2>&1
```

Expected: build succeeds.

- [ ] **Step 4: Commit**

```bash
git add include/model/workflow/workflow_exec.cppm include/model/model.cppm
git commit -m "feat(model): add workflow_exec partition"
```

---

### Task 12: model:workflow_event — external workflow signals

**Files:**
- Create: `include/model/workflow/workflow_event.cppm`

- [ ] **Step 1: Create include/model/workflow/workflow_event.cppm**

```cpp
export module model:workflow_event;

import std;
import :identifiers;

export namespace model {

enum class WorkflowEventType : std::uint8_t {
    PAUSE,
    RESUME,
    TERMINATE,
    RESTART,
    SIGNAL,
};

struct WorkflowEvent {
    ExecutionId                          exec_id;
    WorkflowEventType                    type;
    std::optional<std::string>           payload;
    std::chrono::system_clock::time_point issued_at;
};

} // namespace model
```

- [ ] **Step 2: Complete model.cppm with all partitions**

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
export import :workflow_dag;
export import :workflow_def;
export import :workflow_exec;
export import :workflow_event;
```

- [ ] **Step 3: Build and run all tests**

```bash
xmake build model_test && xmake run model_test
```

Expected: all tests pass.

- [ ] **Step 4: Commit**

```bash
git add include/model/workflow/workflow_event.cppm include/model/model.cppm
git commit -m "feat(model): add workflow_event partition, complete model re-export"
```

---

### Task 13: Add structural construction tests

**Files:**
- Modify: `tests/model/model_test.cc`

Verify the complete model can be constructed end-to-end (catches type mismatch bugs).

- [ ] **Step 1: Add construction tests to model_test.cc**

Append to `tests/model/model_test.cc`:

```cpp
TEST_CASE("TaskDef construction") {
    model::TaskDef def{
        .name        = "send_email",
        .type        = model::TaskType::SIMPLE,
        .worker_type = "email_worker",
        .input_keys  = {"to", "subject", "body"},
        .output_keys = {"message_id"},
        .retry       = {.max_attempts = 3, .backoff = model::RetryBackoff::EXPONENTIAL, .interval_ms = 500},
        .timeout     = {.timeout_ms = 5000, .action = model::TimeoutAction::FAIL_WORKFLOW},
    };
    CHECK(def.name == "send_email");
    CHECK(def.type == model::TaskType::SIMPLE);
    CHECK(def.input_keys.size() == 3);
    CHECK_FALSE(def.rate_limit.has_value());
}

TEST_CASE("WorkflowDef DAG construction") {
    model::WorkflowDef wf{
        .name    = "order_pipeline",
        .version = 1,
        .nodes   = {
            model::TaskNode{
                .task_def_name = "validate_order",
                .edges = {
                    model::TaskEdge{
                        .from     = "validate_order",
                        .to       = "charge_payment",
                        .condition = std::nullopt,
                        .mappings = {
                            model::InputMapping{
                                .source = "$.validate_order.output.order_id",
                                .target = "order_id"
                            }
                        }
                    }
                }
            },
            model::TaskNode{
                .task_def_name = "charge_payment",
                .edges         = {}
            }
        },
        .input_params = {"order_id", "customer_id"},
    };
    CHECK(wf.nodes.size() == 2);
    CHECK(wf.nodes[0].edges[0].to == "charge_payment");
    CHECK_FALSE(wf.failure_workflow.has_value());
}

TEST_CASE("WorkflowExecution initial state") {
    model::WorkflowExecution exec{
        .exec_id     = model::generate_id(),
        .def_name    = "order_pipeline",
        .def_version = 1,
    };
    CHECK_FALSE(exec.exec_id.is_nil());
    CHECK(exec.status == model::WorkflowStatus::RUNNING);
    CHECK(exec.task_instances.empty());
    CHECK(exec.variables.empty());
    CHECK_FALSE(model::is_terminal(exec.status));
}
```

- [ ] **Step 2: Build and run all tests**

```bash
xmake build model_test && xmake run model_test
```

Expected:
```
All tests passed (N assertions in 6 test cases)
```

- [ ] **Step 3: Commit**

```bash
git add tests/model/model_test.cc
git commit -m "test(model): add construction and structural integration tests"
```

---

### Task 14: Full build verification

- [ ] **Step 1: Build the full project**

```bash
xmake build
```

Expected: `congelado_lib`, `http2`, `congelado`, and `model_test` all build with no errors or warnings.

- [ ] **Step 2: Run model tests**

```bash
xmake run model_test
```

Expected: all test cases pass.

- [ ] **Step 3: Verify model files in congelado_lib**

```bash
xmake build congelado_lib -v 2>&1 | grep "model/" | head -20
```

Expected: all 13 `.cppm` files under `include/model/` appear in the compilation list.
