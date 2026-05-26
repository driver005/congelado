# Workflow Model Design

**Date:** 2026-05-26
**Project:** congelado

## Context

Congelado is a C++26 HTTP/2 plugin server. Plugins communicate via a pure C ABI boundary (libffi). The workflow model is a pure data layer — no execution logic — living under `include/model/`. The execution engine, worker dispatch, and queue are separate concerns built on top of this layer.

Inspired by Conductor OSS and n8n. `stduuid 1.2` is already a project dependency.

## Scope

12 C++ module files under `include/model/`. Pure data definitions: structs, enums, type aliases. No execution logic, no I/O, no engine concerns.

## Design Decisions

### Task independence
Tasks are independent units with no inherent order. Order is only imposed when a task appears as a node inside a `WorkflowDef`. This means `TaskDef` is a pure definition reusable across multiple workflows.

### Execution model
Worker-pull through the core server and context manager. Workers filter the queue by `worker_type`. The model layer does not model the queue itself — that belongs in the engine layer.

### Workflow topology
Full DAG with switch/decision branching. `WorkflowDef` holds a `vector<TaskNode>`, where each `TaskNode` carries outgoing `TaskEdge`s. Edges carry optional conditions (for SWITCH) and input mappings.

### Persistence
In-memory first. If a plugin implementing `IDatabase`/`ICache` is loaded, the persistence layer wraps model structs with `AuditRecord`. The model layer itself has no DB dependency — `AuditRecord` is a separate mixin never touched by the engine.

### Data flow between tasks
Dual model: explicit `InputMapping` declared in `WorkflowDef` edges (source path → target key) plus a shared `variables` map on `WorkflowExecution`. Engine resolves mappings against `variables` at task dispatch time. `source` uses JSONPath-like notation (`$.task_name.output.key`, `$.workflow.input.key`).

## File Structure

```
include/model/
  common/
    identifiers.cppm
    timestamps.cppm
    audit.cppm
    policies.cppm
  task/
    task_status.cppm
    task_def.cppm
    task_instance.cppm
  workflow/
    workflow_status.cppm
    workflow_dag.cppm
    workflow_def.cppm
    workflow_exec.cppm
    workflow_event.cppm
```

## Module Specifications

### `common/identifiers.cppm`

```cpp
using WorkflowId     = uuids::uuid;
using TaskId         = uuids::uuid;
using ExecutionId    = uuids::uuid;
using CorrelationId  = uuids::uuid;

ExecutionId generate_id();  // uses uuids::uuid_system_generator
```

### `common/timestamps.cppm`

Runtime-only. Never persisted directly.

```cpp
struct ExecutionTimings {
    std::optional<time_point> scheduled_at;
    std::optional<time_point> started_at;
    std::optional<time_point> completed_at;
};
```

### `common/audit.cppm`

DB-only mixin. Engine never instantiates this. Persistence layer composes it with model structs.

```cpp
struct AuditRecord {
    time_point created_at;
    time_point updated_at;
    uint32_t   version;     // optimistic lock counter
};
```

### `common/policies.cppm`

```cpp
enum class RetryBackoff { FIXED, EXPONENTIAL };

struct RetryPolicy {
    uint32_t     max_attempts;
    RetryBackoff backoff;
    uint32_t     interval_ms;
};

enum class TimeoutAction { RETRY, FAIL_WORKFLOW, ALERT_ONLY };

struct TimeoutPolicy {
    uint32_t      timeout_ms;
    TimeoutAction action;
};

struct RateLimitPolicy {
    uint32_t max_concurrent;
    uint32_t rate_limit_per_second;
};
```

### `task/task_status.cppm`

```cpp
enum class TaskStatus {
    SCHEDULED, IN_PROGRESS, COMPLETED, FAILED,
    TIMED_OUT, SKIPPED, CANCELED
};

enum class TaskType {
    SIMPLE, FORK, JOIN, SWITCH, SUB_WORKFLOW
};

enum class TaskResult {
    SUCCESS, FAILURE, TIMEOUT, SKIPPED
};

bool is_terminal(TaskStatus) noexcept;
```

### `task/task_def.cppm`

Pure definition. No timestamps, no runtime state.

```cpp
struct TaskDef {
    std::string                  name;           // unique registry key
    TaskType                     type;
    std::string                  worker_type;    // worker pool selector
    std::vector<std::string>     input_keys;     // declared inputs
    std::vector<std::string>     output_keys;    // declared outputs
    RetryPolicy                  retry;
    TimeoutPolicy                timeout;
    std::optional<RateLimitPolicy> rate_limit;
};
```

### `task/task_instance.cppm`

Runtime snapshot of one task execution within a workflow.

```cpp
struct TaskInstance {
    TaskId                                    task_id;
    std::string                               def_name;         // ref to TaskDef by name
    ExecutionId                               workflow_exec_id;
    TaskStatus                                status;
    uint32_t                                  seq;              // deterministic replay order
    std::unordered_map<std::string, std::string> input_data;
    std::unordered_map<std::string, std::string> output_data;
    ExecutionTimings                          timings;
    uint32_t                                  retry_count;
};
```

### `workflow/workflow_status.cppm`

```cpp
enum class WorkflowStatus {
    RUNNING, COMPLETED, FAILED,
    TIMED_OUT, PAUSED, TERMINATED
};

bool is_terminal(WorkflowStatus) noexcept;
```

### `workflow/workflow_dag.cppm`

Graph primitives for `WorkflowDef`.

```cpp
struct InputMapping {
    std::string source;  // "$.task_name.output.key" or "$.workflow.input.key"
    std::string target;  // input_key name on receiving task
};

struct OutputMapping {
    std::string source;  // "$.task_name.output.key"
    std::string target;  // workflow output key name
};

struct TaskEdge {
    std::string                  from;
    std::string                  to;
    std::optional<std::string>   condition;  // expression string; null on non-SWITCH edges
    std::vector<InputMapping>    mappings;
};

struct TaskNode {
    std::string            task_def_name;
    std::vector<TaskEdge>  edges;
};
```

### `workflow/workflow_def.cppm`

```cpp
struct WorkflowDef {
    std::string                      name;
    uint32_t                         version;
    std::vector<TaskNode>            nodes;
    std::vector<std::string>         input_params;
    std::vector<OutputMapping>       output_mappings;
    std::optional<std::string>       failure_workflow;  // spawned on terminal FAILED
    std::optional<TimeoutPolicy>     timeout;
};
```

### `workflow/workflow_exec.cppm`

Runtime state of one workflow execution. No `AuditRecord` — in-memory; persistence layer wraps externally.

```cpp
struct WorkflowExecution {
    ExecutionId                                   exec_id;
    std::string                                   def_name;
    uint32_t                                      def_version;   // pinned at start
    WorkflowStatus                                status;
    std::optional<CorrelationId>                  correlation_id;
    std::unordered_map<std::string, std::string>  variables;     // shared execution context
    std::vector<TaskInstance>                     task_instances;
    ExecutionTimings                              timings;
};
```

### `workflow/workflow_event.cppm`

External signals into a running workflow.

```cpp
enum class WorkflowEventType {
    PAUSE, RESUME, TERMINATE, RESTART, SIGNAL
};

struct WorkflowEvent {
    ExecutionId                exec_id;
    WorkflowEventType          type;
    std::optional<std::string> payload;
    time_point                 issued_at;
};
```

## What This Layer Does NOT Include

- Queue implementation (engine layer)
- Worker registration or dispatch (engine layer)
- Expression evaluation for `condition` and `InputMapping.source` (engine layer)
- Persistence adapters (plugin layer wrapping `IDatabase`/`ICache`)
- Workflow registry / `TaskDef` registry (engine layer)
