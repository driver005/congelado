export module congelado_api_dto;

import std;
import serde;

export namespace congelado_api_dto {

class InputMapping {
  public:
    InputMapping() = default;

    void setSource(std::string value) { m_source = std::move(value); }
    void setTarget(std::string value) { m_target = std::move(value); }

    [[nodiscard]] const std::string &getSource() const noexcept { return m_source; }
    [[nodiscard]] const std::string &getTarget() const noexcept { return m_target; }

  private:
    std::string m_source;
    std::string m_target;
};

class TaskEdge {
  public:
    TaskEdge() = default;

    void setFrom(std::string value) { m_from = std::move(value); }
    void setTo(std::string value) { m_to = std::move(value); }
    void setCondition(std::optional<std::string> value) { m_condition = std::move(value); }
    void setMappings(std::vector<InputMapping> value) { m_mappings = std::move(value); }

    [[nodiscard]] const std::string &getFrom() const noexcept { return m_from; }
    [[nodiscard]] const std::string &getTo() const noexcept { return m_to; }
    [[nodiscard]] const std::optional<std::string> &getCondition() const noexcept { return m_condition; }
    [[nodiscard]] const std::vector<InputMapping> &getMappings() const noexcept { return m_mappings; }

  private:
    std::string m_from;
    std::string m_to;
    std::optional<std::string> m_condition;
    std::vector<InputMapping> m_mappings;
};

class TaskNode {
  public:
    TaskNode() = default;

    void setTaskDefName(std::string value) { m_task_def_name = std::move(value); }
    void setEdges(std::vector<TaskEdge> value) { m_edges = std::move(value); }

    [[nodiscard]] const std::string &getTaskDefName() const noexcept { return m_task_def_name; }
    [[nodiscard]] const std::vector<TaskEdge> &getEdges() const noexcept { return m_edges; }

  private:
    std::string m_task_def_name;
    std::vector<TaskEdge> m_edges;
};

class ExecutionTimings {
  public:
    ExecutionTimings() = default;

    void setScheduledAt(std::optional<std::string> value) { m_scheduled_at = std::move(value); }
    void setStartedAt(std::optional<std::string> value) { m_started_at = std::move(value); }
    void setCompletedAt(std::optional<std::string> value) { m_completed_at = std::move(value); }

    [[nodiscard]] const std::optional<std::string> &getScheduledAt() const noexcept { return m_scheduled_at; }
    [[nodiscard]] const std::optional<std::string> &getStartedAt() const noexcept { return m_started_at; }
    [[nodiscard]] const std::optional<std::string> &getCompletedAt() const noexcept { return m_completed_at; }

  private:
    std::optional<std::string> m_scheduled_at;
    std::optional<std::string> m_started_at;
    std::optional<std::string> m_completed_at;
};

class TaskInstance {
  public:
    TaskInstance() = default;

    void setTaskId(std::string value) { m_task_id = std::move(value); }
    void setInputData(std::string value) { m_input_data = std::move(value); }
    void setWorkflowExecId(std::string value) { m_workflow_exec_id = std::move(value); }
    void setDefName(std::string value) { m_def_name = std::move(value); }
    void setSeq(std::int64_t value) { m_seq = std::move(value); }
    void setStatus(std::string value) { m_status = std::move(value); }
    void setRetryCount(std::int64_t value) { m_retry_count = std::move(value); }
    void setOutputData(std::string value) { m_output_data = std::move(value); }
    void setTimings(ExecutionTimings value) { m_timings = std::move(value); }

    [[nodiscard]] const std::string &getTaskId() const noexcept { return m_task_id; }
    [[nodiscard]] const std::string &getInputData() const noexcept { return m_input_data; }
    [[nodiscard]] const std::string &getWorkflowExecId() const noexcept { return m_workflow_exec_id; }
    [[nodiscard]] const std::string &getDefName() const noexcept { return m_def_name; }
    [[nodiscard]] const std::int64_t &getSeq() const noexcept { return m_seq; }
    [[nodiscard]] const std::string &getStatus() const noexcept { return m_status; }
    [[nodiscard]] const std::int64_t &getRetryCount() const noexcept { return m_retry_count; }
    [[nodiscard]] const std::string &getOutputData() const noexcept { return m_output_data; }
    [[nodiscard]] const ExecutionTimings &getTimings() const noexcept { return m_timings; }

  private:
    std::string m_task_id;
    std::string m_input_data;
    std::string m_workflow_exec_id;
    std::string m_def_name;
    std::int64_t m_seq;
    std::string m_status;
    std::int64_t m_retry_count;
    std::string m_output_data;
    ExecutionTimings m_timings;
};

class OutputMapping {
  public:
    OutputMapping() = default;

    void setSource(std::string value) { m_source = std::move(value); }
    void setTarget(std::string value) { m_target = std::move(value); }

    [[nodiscard]] const std::string &getSource() const noexcept { return m_source; }
    [[nodiscard]] const std::string &getTarget() const noexcept { return m_target; }

  private:
    std::string m_source;
    std::string m_target;
};

class RateLimitPolicy {
  public:
    RateLimitPolicy() = default;

    void setMaxConcurrent(std::int64_t value) { m_max_concurrent = std::move(value); }
    void setRateLimitPerSecond(std::int64_t value) { m_rate_limit_per_second = std::move(value); }

    [[nodiscard]] const std::int64_t &getMaxConcurrent() const noexcept { return m_max_concurrent; }
    [[nodiscard]] const std::int64_t &getRateLimitPerSecond() const noexcept { return m_rate_limit_per_second; }

  private:
    std::int64_t m_max_concurrent;
    std::int64_t m_rate_limit_per_second;
};

class TimeoutPolicy {
  public:
    TimeoutPolicy() = default;

    void setTimeoutMs(std::int64_t value) { m_timeout_ms = std::move(value); }
    void setAction(std::string value) { m_action = std::move(value); }

    [[nodiscard]] const std::int64_t &getTimeoutMs() const noexcept { return m_timeout_ms; }
    [[nodiscard]] const std::string &getAction() const noexcept { return m_action; }

  private:
    std::int64_t m_timeout_ms;
    std::string m_action;
};

class WorkflowDef {
  public:
    WorkflowDef() = default;

    void setName(std::string value) { m_name = std::move(value); }
    void setInputParams(std::vector<std::string> value) { m_input_params = std::move(value); }
    void setOutputMappings(std::vector<OutputMapping> value) { m_output_mappings = std::move(value); }
    void setVersion(std::int64_t value) { m_version = std::move(value); }
    void setNodes(std::vector<TaskNode> value) { m_nodes = std::move(value); }
    void setFailureWorkflow(std::optional<std::string> value) { m_failure_workflow = std::move(value); }
    void setTimeout(std::optional<TimeoutPolicy> value) { m_timeout = std::move(value); }

    [[nodiscard]] const std::string &getName() const noexcept { return m_name; }
    [[nodiscard]] const std::vector<std::string> &getInputParams() const noexcept { return m_input_params; }
    [[nodiscard]] const std::vector<OutputMapping> &getOutputMappings() const noexcept { return m_output_mappings; }
    [[nodiscard]] const std::int64_t &getVersion() const noexcept { return m_version; }
    [[nodiscard]] const std::vector<TaskNode> &getNodes() const noexcept { return m_nodes; }
    [[nodiscard]] const std::optional<std::string> &getFailureWorkflow() const noexcept { return m_failure_workflow; }
    [[nodiscard]] const std::optional<TimeoutPolicy> &getTimeout() const noexcept { return m_timeout; }

  private:
    std::string m_name;
    std::vector<std::string> m_input_params;
    std::vector<OutputMapping> m_output_mappings;
    std::int64_t m_version;
    std::vector<TaskNode> m_nodes;
    std::optional<std::string> m_failure_workflow;
    std::optional<TimeoutPolicy> m_timeout;
};

class WorkflowExecution {
  public:
    WorkflowExecution() = default;

    void setExecId(std::string value) { m_exec_id = std::move(value); }
    void setStatus(std::string value) { m_status = std::move(value); }
    void setDefName(std::string value) { m_def_name = std::move(value); }
    void setDefVersion(std::int64_t value) { m_def_version = std::move(value); }
    void setTaskInstances(std::vector<TaskInstance> value) { m_task_instances = std::move(value); }
    void setCorrelationId(std::optional<std::string> value) { m_correlation_id = std::move(value); }
    void setVariables(std::string value) { m_variables = std::move(value); }
    void setTimings(ExecutionTimings value) { m_timings = std::move(value); }

    [[nodiscard]] const std::string &getExecId() const noexcept { return m_exec_id; }
    [[nodiscard]] const std::string &getStatus() const noexcept { return m_status; }
    [[nodiscard]] const std::string &getDefName() const noexcept { return m_def_name; }
    [[nodiscard]] const std::int64_t &getDefVersion() const noexcept { return m_def_version; }
    [[nodiscard]] const std::vector<TaskInstance> &getTaskInstances() const noexcept { return m_task_instances; }
    [[nodiscard]] const std::optional<std::string> &getCorrelationId() const noexcept { return m_correlation_id; }
    [[nodiscard]] const std::string &getVariables() const noexcept { return m_variables; }
    [[nodiscard]] const ExecutionTimings &getTimings() const noexcept { return m_timings; }

  private:
    std::string m_exec_id;
    std::string m_status;
    std::string m_def_name;
    std::int64_t m_def_version;
    std::vector<TaskInstance> m_task_instances;
    std::optional<std::string> m_correlation_id;
    std::string m_variables;
    ExecutionTimings m_timings;
};

class RetryPolicy {
  public:
    RetryPolicy() = default;

    void setMaxAttempts(std::int64_t value) { m_max_attempts = std::move(value); }
    void setBackoff(std::string value) { m_backoff = std::move(value); }
    void setIntervalMs(std::int64_t value) { m_interval_ms = std::move(value); }

    [[nodiscard]] const std::int64_t &getMaxAttempts() const noexcept { return m_max_attempts; }
    [[nodiscard]] const std::string &getBackoff() const noexcept { return m_backoff; }
    [[nodiscard]] const std::int64_t &getIntervalMs() const noexcept { return m_interval_ms; }

  private:
    std::int64_t m_max_attempts;
    std::string m_backoff;
    std::int64_t m_interval_ms;
};

class TaskDef {
  public:
    TaskDef() = default;

    void setName(std::string value) { m_name = std::move(value); }
    void setType(std::string value) { m_type = std::move(value); }
    void setWorkerType(std::string value) { m_worker_type = std::move(value); }
    void setInputKeys(std::vector<std::string> value) { m_input_keys = std::move(value); }
    void setOutputKeys(std::vector<std::string> value) { m_output_keys = std::move(value); }
    void setRateLimit(std::optional<RateLimitPolicy> value) { m_rate_limit = std::move(value); }
    void setRetry(RetryPolicy value) { m_retry = std::move(value); }
    void setTimeout(TimeoutPolicy value) { m_timeout = std::move(value); }

    [[nodiscard]] const std::string &getName() const noexcept { return m_name; }
    [[nodiscard]] const std::string &getType() const noexcept { return m_type; }
    [[nodiscard]] const std::string &getWorkerType() const noexcept { return m_worker_type; }
    [[nodiscard]] const std::vector<std::string> &getInputKeys() const noexcept { return m_input_keys; }
    [[nodiscard]] const std::vector<std::string> &getOutputKeys() const noexcept { return m_output_keys; }
    [[nodiscard]] const std::optional<RateLimitPolicy> &getRateLimit() const noexcept { return m_rate_limit; }
    [[nodiscard]] const RetryPolicy &getRetry() const noexcept { return m_retry; }
    [[nodiscard]] const TimeoutPolicy &getTimeout() const noexcept { return m_timeout; }

  private:
    std::string m_name;
    std::string m_type;
    std::string m_worker_type;
    std::vector<std::string> m_input_keys;
    std::vector<std::string> m_output_keys;
    std::optional<RateLimitPolicy> m_rate_limit;
    RetryPolicy m_retry;
    TimeoutPolicy m_timeout;
};

} // namespace congelado_api_dto

template <>
struct serde::Serializable<congelado_api_dto::InputMapping> {
    static constexpr auto fields() {
        using congelado_api_dto::InputMapping;
        return std::tuple{
            serde::FieldDesc<"source", &InputMapping::getSource, &InputMapping::setSource>{},
            serde::FieldDesc<"target", &InputMapping::getTarget, &InputMapping::setTarget>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::TaskEdge> {
    static constexpr auto fields() {
        using congelado_api_dto::TaskEdge;
        return std::tuple{
            serde::FieldDesc<"from", &TaskEdge::getFrom, &TaskEdge::setFrom>{},
            serde::FieldDesc<"to", &TaskEdge::getTo, &TaskEdge::setTo>{},
            serde::FieldDesc<"condition", &TaskEdge::getCondition, &TaskEdge::setCondition>{},
            serde::FieldDesc<"mappings", &TaskEdge::getMappings, &TaskEdge::setMappings>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::TaskNode> {
    static constexpr auto fields() {
        using congelado_api_dto::TaskNode;
        return std::tuple{
            serde::FieldDesc<"task_def_name", &TaskNode::getTaskDefName, &TaskNode::setTaskDefName>{},
            serde::FieldDesc<"edges", &TaskNode::getEdges, &TaskNode::setEdges>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::ExecutionTimings> {
    static constexpr auto fields() {
        using congelado_api_dto::ExecutionTimings;
        return std::tuple{
            serde::FieldDesc<"scheduled_at", &ExecutionTimings::getScheduledAt, &ExecutionTimings::setScheduledAt>{},
            serde::FieldDesc<"started_at", &ExecutionTimings::getStartedAt, &ExecutionTimings::setStartedAt>{},
            serde::FieldDesc<"completed_at", &ExecutionTimings::getCompletedAt, &ExecutionTimings::setCompletedAt>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::TaskInstance> {
    static constexpr auto fields() {
        using congelado_api_dto::TaskInstance;
        return std::tuple{
            serde::FieldDesc<"task_id", &TaskInstance::getTaskId, &TaskInstance::setTaskId>{},
            serde::FieldDesc<"input_data", &TaskInstance::getInputData, &TaskInstance::setInputData>{},
            serde::FieldDesc<"workflow_exec_id", &TaskInstance::getWorkflowExecId, &TaskInstance::setWorkflowExecId>{},
            serde::FieldDesc<"def_name", &TaskInstance::getDefName, &TaskInstance::setDefName>{},
            serde::FieldDesc<"seq", &TaskInstance::getSeq, &TaskInstance::setSeq>{},
            serde::FieldDesc<"status", &TaskInstance::getStatus, &TaskInstance::setStatus>{},
            serde::FieldDesc<"retry_count", &TaskInstance::getRetryCount, &TaskInstance::setRetryCount>{},
            serde::FieldDesc<"output_data", &TaskInstance::getOutputData, &TaskInstance::setOutputData>{},
            serde::FieldDesc<"timings", &TaskInstance::getTimings, &TaskInstance::setTimings>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::OutputMapping> {
    static constexpr auto fields() {
        using congelado_api_dto::OutputMapping;
        return std::tuple{
            serde::FieldDesc<"source", &OutputMapping::getSource, &OutputMapping::setSource>{},
            serde::FieldDesc<"target", &OutputMapping::getTarget, &OutputMapping::setTarget>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::RateLimitPolicy> {
    static constexpr auto fields() {
        using congelado_api_dto::RateLimitPolicy;
        return std::tuple{
            serde::FieldDesc<"max_concurrent", &RateLimitPolicy::getMaxConcurrent, &RateLimitPolicy::setMaxConcurrent>{},
            serde::FieldDesc<"rate_limit_per_second", &RateLimitPolicy::getRateLimitPerSecond, &RateLimitPolicy::setRateLimitPerSecond>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::TimeoutPolicy> {
    static constexpr auto fields() {
        using congelado_api_dto::TimeoutPolicy;
        return std::tuple{
            serde::FieldDesc<"timeout_ms", &TimeoutPolicy::getTimeoutMs, &TimeoutPolicy::setTimeoutMs>{},
            serde::FieldDesc<"action", &TimeoutPolicy::getAction, &TimeoutPolicy::setAction>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::WorkflowDef> {
    static constexpr auto fields() {
        using congelado_api_dto::WorkflowDef;
        return std::tuple{
            serde::FieldDesc<"name", &WorkflowDef::getName, &WorkflowDef::setName>{},
            serde::FieldDesc<"input_params", &WorkflowDef::getInputParams, &WorkflowDef::setInputParams>{},
            serde::FieldDesc<"output_mappings", &WorkflowDef::getOutputMappings, &WorkflowDef::setOutputMappings>{},
            serde::FieldDesc<"version", &WorkflowDef::getVersion, &WorkflowDef::setVersion>{},
            serde::FieldDesc<"nodes", &WorkflowDef::getNodes, &WorkflowDef::setNodes>{},
            serde::FieldDesc<"failure_workflow", &WorkflowDef::getFailureWorkflow, &WorkflowDef::setFailureWorkflow>{},
            serde::FieldDesc<"timeout", &WorkflowDef::getTimeout, &WorkflowDef::setTimeout>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::WorkflowExecution> {
    static constexpr auto fields() {
        using congelado_api_dto::WorkflowExecution;
        return std::tuple{
            serde::FieldDesc<"exec_id", &WorkflowExecution::getExecId, &WorkflowExecution::setExecId>{},
            serde::FieldDesc<"status", &WorkflowExecution::getStatus, &WorkflowExecution::setStatus>{},
            serde::FieldDesc<"def_name", &WorkflowExecution::getDefName, &WorkflowExecution::setDefName>{},
            serde::FieldDesc<"def_version", &WorkflowExecution::getDefVersion, &WorkflowExecution::setDefVersion>{},
            serde::FieldDesc<"task_instances", &WorkflowExecution::getTaskInstances, &WorkflowExecution::setTaskInstances>{},
            serde::FieldDesc<"correlation_id", &WorkflowExecution::getCorrelationId, &WorkflowExecution::setCorrelationId>{},
            serde::FieldDesc<"variables", &WorkflowExecution::getVariables, &WorkflowExecution::setVariables>{},
            serde::FieldDesc<"timings", &WorkflowExecution::getTimings, &WorkflowExecution::setTimings>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::RetryPolicy> {
    static constexpr auto fields() {
        using congelado_api_dto::RetryPolicy;
        return std::tuple{
            serde::FieldDesc<"max_attempts", &RetryPolicy::getMaxAttempts, &RetryPolicy::setMaxAttempts>{},
            serde::FieldDesc<"backoff", &RetryPolicy::getBackoff, &RetryPolicy::setBackoff>{},
            serde::FieldDesc<"interval_ms", &RetryPolicy::getIntervalMs, &RetryPolicy::setIntervalMs>{},
        };
    }
};

template <>
struct serde::Serializable<congelado_api_dto::TaskDef> {
    static constexpr auto fields() {
        using congelado_api_dto::TaskDef;
        return std::tuple{
            serde::FieldDesc<"name", &TaskDef::getName, &TaskDef::setName>{},
            serde::FieldDesc<"type", &TaskDef::getType, &TaskDef::setType>{},
            serde::FieldDesc<"worker_type", &TaskDef::getWorkerType, &TaskDef::setWorkerType>{},
            serde::FieldDesc<"input_keys", &TaskDef::getInputKeys, &TaskDef::setInputKeys>{},
            serde::FieldDesc<"output_keys", &TaskDef::getOutputKeys, &TaskDef::setOutputKeys>{},
            serde::FieldDesc<"rate_limit", &TaskDef::getRateLimit, &TaskDef::setRateLimit>{},
            serde::FieldDesc<"retry", &TaskDef::getRetry, &TaskDef::setRetry>{},
            serde::FieldDesc<"timeout", &TaskDef::getTimeout, &TaskDef::setTimeout>{},
        };
    }
};

