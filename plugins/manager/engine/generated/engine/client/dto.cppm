export module congelado_api_dto;

import std;
import serde;

export namespace congelado_api_dto {

class OutputMapping
{
public:
    OutputMapping() = default;

    void setSource(std::string value)
    {
        m_source = std::move(value);
    }

    void setTarget(std::string value)
    {
        m_target = std::move(value);
    }

    [[nodiscard]] const std::string& getSource() const noexcept
    {
        return m_source;
    }

    [[nodiscard]] const std::string& getTarget() const noexcept
    {
        return m_target;
    }

private:
    std::string m_source;
    std::string m_target;
};

class EventAction
{
public:
    EventAction() = default;

    void setType(std::string value)
    {
        m_type = std::move(value);
    }

    void setPayload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& getType() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const std::string& getPayload() const noexcept
    {
        return m_payload;
    }

private:
    std::string m_type;
    std::string m_payload;
};

class WorkflowSchedule
{
public:
    WorkflowSchedule() = default;

    void setName(std::string value)
    {
        m_name = std::move(value);
    }

    void setWorkflowName(std::string value)
    {
        m_workflow_name = std::move(value);
    }

    void setWorkflowVersion(std::int64_t value)
    {
        m_workflow_version = std::move(value);
    }

    void setPaused(bool value)
    {
        m_paused = std::move(value);
    }

    void setLastFiredAt(std::optional<std::string> value)
    {
        m_last_fired_at = std::move(value);
    }

    void setCronExpression(std::string value)
    {
        m_cron_expression = std::move(value);
    }

    void setSeedVariables(std::string value)
    {
        m_seed_variables = std::move(value);
    }

    void setEnabled(bool value)
    {
        m_enabled = std::move(value);
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& getWorkflowName() const noexcept
    {
        return m_workflow_name;
    }

    [[nodiscard]] const std::int64_t& getWorkflowVersion() const noexcept
    {
        return m_workflow_version;
    }

    [[nodiscard]] const bool& getPaused() const noexcept
    {
        return m_paused;
    }

    [[nodiscard]] const std::optional<std::string>& getLastFiredAt() const noexcept
    {
        return m_last_fired_at;
    }

    [[nodiscard]] const std::string& getCronExpression() const noexcept
    {
        return m_cron_expression;
    }

    [[nodiscard]] const std::string& getSeedVariables() const noexcept
    {
        return m_seed_variables;
    }

    [[nodiscard]] const bool& getEnabled() const noexcept
    {
        return m_enabled;
    }

private:
    std::string m_name;
    std::string m_workflow_name;
    std::int64_t m_workflow_version;
    bool m_paused;
    std::optional<std::string> m_last_fired_at;
    std::string m_cron_expression;
    std::string m_seed_variables;
    bool m_enabled;
};

class InputMapping
{
public:
    InputMapping() = default;

    void setSource(std::string value)
    {
        m_source = std::move(value);
    }

    void setTarget(std::string value)
    {
        m_target = std::move(value);
    }

    [[nodiscard]] const std::string& getSource() const noexcept
    {
        return m_source;
    }

    [[nodiscard]] const std::string& getTarget() const noexcept
    {
        return m_target;
    }

private:
    std::string m_source;
    std::string m_target;
};

class TaskEdge
{
public:
    TaskEdge() = default;

    void setFrom(std::string value)
    {
        m_from = std::move(value);
    }

    void setTo(std::string value)
    {
        m_to = std::move(value);
    }

    void setCondition(std::optional<std::string> value)
    {
        m_condition = std::move(value);
    }

    void setMappings(std::vector<InputMapping> value)
    {
        m_mappings = std::move(value);
    }

    [[nodiscard]] const std::string& getFrom() const noexcept
    {
        return m_from;
    }

    [[nodiscard]] const std::string& getTo() const noexcept
    {
        return m_to;
    }

    [[nodiscard]] const std::optional<std::string>& getCondition() const noexcept
    {
        return m_condition;
    }

    [[nodiscard]] const std::vector<InputMapping>& getMappings() const noexcept
    {
        return m_mappings;
    }

private:
    std::string m_from;
    std::string m_to;
    std::optional<std::string> m_condition;
    std::vector<InputMapping> m_mappings;
};

class TaskNode
{
public:
    TaskNode() = default;

    void setTaskDefName(std::string value)
    {
        m_task_def_name = std::move(value);
    }

    void setRefName(std::string value)
    {
        m_ref_name = std::move(value);
    }

    void setEdges(std::vector<TaskEdge> value)
    {
        m_edges = std::move(value);
    }

    void setJoinOn(std::vector<std::string> value)
    {
        m_join_on = std::move(value);
    }

    void setJoinType(std::string value)
    {
        m_join_type = std::move(value);
    }

    void setDynamicTasksInputKey(std::optional<std::string> value)
    {
        m_dynamic_tasks_input_key = std::move(value);
    }

    void setLoopBody(std::vector<std::string> value)
    {
        m_loop_body = std::move(value);
    }

    void setLoopCondition(std::optional<std::string> value)
    {
        m_loop_condition = std::move(value);
    }

    [[nodiscard]] const std::string& getTaskDefName() const noexcept
    {
        return m_task_def_name;
    }

    [[nodiscard]] const std::string& getRefName() const noexcept
    {
        return m_ref_name;
    }

    [[nodiscard]] const std::vector<TaskEdge>& getEdges() const noexcept
    {
        return m_edges;
    }

    [[nodiscard]] const std::vector<std::string>& getJoinOn() const noexcept
    {
        return m_join_on;
    }

    [[nodiscard]] const std::string& getJoinType() const noexcept
    {
        return m_join_type;
    }

    [[nodiscard]] const std::optional<std::string>& getDynamicTasksInputKey() const noexcept
    {
        return m_dynamic_tasks_input_key;
    }

    [[nodiscard]] const std::vector<std::string>& getLoopBody() const noexcept
    {
        return m_loop_body;
    }

    [[nodiscard]] const std::optional<std::string>& getLoopCondition() const noexcept
    {
        return m_loop_condition;
    }

private:
    std::string m_task_def_name;
    std::string m_ref_name;
    std::vector<TaskEdge> m_edges;
    std::vector<std::string> m_join_on;
    std::string m_join_type;
    std::optional<std::string> m_dynamic_tasks_input_key;
    std::vector<std::string> m_loop_body;
    std::optional<std::string> m_loop_condition;
};

class ExecutionTimings
{
public:
    ExecutionTimings() = default;

    void setScheduledAt(std::optional<std::string> value)
    {
        m_scheduled_at = std::move(value);
    }

    void setStartedAt(std::optional<std::string> value)
    {
        m_started_at = std::move(value);
    }

    void setCompletedAt(std::optional<std::string> value)
    {
        m_completed_at = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& getScheduledAt() const noexcept
    {
        return m_scheduled_at;
    }

    [[nodiscard]] const std::optional<std::string>& getStartedAt() const noexcept
    {
        return m_started_at;
    }

    [[nodiscard]] const std::optional<std::string>& getCompletedAt() const noexcept
    {
        return m_completed_at;
    }

private:
    std::optional<std::string> m_scheduled_at;
    std::optional<std::string> m_started_at;
    std::optional<std::string> m_completed_at;
};

class TaskInstance
{
public:
    TaskInstance() = default;

    void setTaskId(std::string value)
    {
        m_task_id = std::move(value);
    }

    void setInputData(std::string value)
    {
        m_input_data = std::move(value);
    }

    void setWorkflowExecId(std::string value)
    {
        m_workflow_exec_id = std::move(value);
    }

    void setDefName(std::string value)
    {
        m_def_name = std::move(value);
    }

    void setSeq(std::int64_t value)
    {
        m_seq = std::move(value);
    }

    void setNodeRef(std::string value)
    {
        m_node_ref = std::move(value);
    }

    void setOutputData(std::string value)
    {
        m_output_data = std::move(value);
    }

    void setStatus(std::string value)
    {
        m_status = std::move(value);
    }

    void setRetryCount(std::int64_t value)
    {
        m_retry_count = std::move(value);
    }

    void setDeadlineAt(std::optional<std::string> value)
    {
        m_deadline_at = std::move(value);
    }

    void setTimings(ExecutionTimings value)
    {
        m_timings = std::move(value);
    }

    void setNextRetryAt(std::optional<std::string> value)
    {
        m_next_retry_at = std::move(value);
    }

    void setSubWorkflowExecId(std::optional<std::string> value)
    {
        m_sub_workflow_exec_id = std::move(value);
    }

    [[nodiscard]] const std::string& getTaskId() const noexcept
    {
        return m_task_id;
    }

    [[nodiscard]] const std::string& getInputData() const noexcept
    {
        return m_input_data;
    }

    [[nodiscard]] const std::string& getWorkflowExecId() const noexcept
    {
        return m_workflow_exec_id;
    }

    [[nodiscard]] const std::string& getDefName() const noexcept
    {
        return m_def_name;
    }

    [[nodiscard]] const std::int64_t& getSeq() const noexcept
    {
        return m_seq;
    }

    [[nodiscard]] const std::string& getNodeRef() const noexcept
    {
        return m_node_ref;
    }

    [[nodiscard]] const std::string& getOutputData() const noexcept
    {
        return m_output_data;
    }

    [[nodiscard]] const std::string& getStatus() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::int64_t& getRetryCount() const noexcept
    {
        return m_retry_count;
    }

    [[nodiscard]] const std::optional<std::string>& getDeadlineAt() const noexcept
    {
        return m_deadline_at;
    }

    [[nodiscard]] const ExecutionTimings& getTimings() const noexcept
    {
        return m_timings;
    }

    [[nodiscard]] const std::optional<std::string>& getNextRetryAt() const noexcept
    {
        return m_next_retry_at;
    }

    [[nodiscard]] const std::optional<std::string>& getSubWorkflowExecId() const noexcept
    {
        return m_sub_workflow_exec_id;
    }

private:
    std::string m_task_id;
    std::string m_input_data;
    std::string m_workflow_exec_id;
    std::string m_def_name;
    std::int64_t m_seq;
    std::string m_node_ref;
    std::string m_output_data;
    std::string m_status;
    std::int64_t m_retry_count;
    std::optional<std::string> m_deadline_at;
    ExecutionTimings m_timings;
    std::optional<std::string> m_next_retry_at;
    std::optional<std::string> m_sub_workflow_exec_id;
};

class WorkflowExecution
{
public:
    WorkflowExecution() = default;

    void setExecId(std::string value)
    {
        m_exec_id = std::move(value);
    }

    void setStatus(std::string value)
    {
        m_status = std::move(value);
    }

    void setDynamicNodes(std::vector<TaskNode> value)
    {
        m_dynamic_nodes = std::move(value);
    }

    void setDefName(std::string value)
    {
        m_def_name = std::move(value);
    }

    void setDefVersion(std::int64_t value)
    {
        m_def_version = std::move(value);
    }

    void setTaskInstances(std::vector<TaskInstance> value)
    {
        m_task_instances = std::move(value);
    }

    void setCorrelationId(std::optional<std::string> value)
    {
        m_correlation_id = std::move(value);
    }

    void setVariables(std::string value)
    {
        m_variables = std::move(value);
    }

    void setTimings(ExecutionTimings value)
    {
        m_timings = std::move(value);
    }

    void setParentExecId(std::optional<std::string> value)
    {
        m_parent_exec_id = std::move(value);
    }

    [[nodiscard]] const std::string& getExecId() const noexcept
    {
        return m_exec_id;
    }

    [[nodiscard]] const std::string& getStatus() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::vector<TaskNode>& getDynamicNodes() const noexcept
    {
        return m_dynamic_nodes;
    }

    [[nodiscard]] const std::string& getDefName() const noexcept
    {
        return m_def_name;
    }

    [[nodiscard]] const std::int64_t& getDefVersion() const noexcept
    {
        return m_def_version;
    }

    [[nodiscard]] const std::vector<TaskInstance>& getTaskInstances() const noexcept
    {
        return m_task_instances;
    }

    [[nodiscard]] const std::optional<std::string>& getCorrelationId() const noexcept
    {
        return m_correlation_id;
    }

    [[nodiscard]] const std::string& getVariables() const noexcept
    {
        return m_variables;
    }

    [[nodiscard]] const ExecutionTimings& getTimings() const noexcept
    {
        return m_timings;
    }

    [[nodiscard]] const std::optional<std::string>& getParentExecId() const noexcept
    {
        return m_parent_exec_id;
    }

private:
    std::string m_exec_id;
    std::string m_status;
    std::vector<TaskNode> m_dynamic_nodes;
    std::string m_def_name;
    std::int64_t m_def_version;
    std::vector<TaskInstance> m_task_instances;
    std::optional<std::string> m_correlation_id;
    std::string m_variables;
    ExecutionTimings m_timings;
    std::optional<std::string> m_parent_exec_id;
};

class SignalBody
{
public:
    SignalBody() = default;

    void setNodeRef(std::string value)
    {
        m_node_ref = std::move(value);
    }

    void setPayload(std::optional<std::string> value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& getNodeRef() const noexcept
    {
        return m_node_ref;
    }

    [[nodiscard]] const std::optional<std::string>& getPayload() const noexcept
    {
        return m_payload;
    }

private:
    std::string m_node_ref;
    std::optional<std::string> m_payload;
};

class PollData
{
public:
    PollData() = default;

    void setWorkerType(std::string value)
    {
        m_worker_type = std::move(value);
    }

    void setLastPollAt(std::string value)
    {
        m_last_poll_at = std::move(value);
    }

    [[nodiscard]] const std::string& getWorkerType() const noexcept
    {
        return m_worker_type;
    }

    [[nodiscard]] const std::string& getLastPollAt() const noexcept
    {
        return m_last_poll_at;
    }

private:
    std::string m_worker_type;
    std::string m_last_poll_at;
};

class RetryPolicy
{
public:
    RetryPolicy() = default;

    void setMaxAttempts(std::int64_t value)
    {
        m_max_attempts = std::move(value);
    }

    void setBackoff(std::string value)
    {
        m_backoff = std::move(value);
    }

    void setIntervalMs(std::int64_t value)
    {
        m_interval_ms = std::move(value);
    }

    [[nodiscard]] const std::int64_t& getMaxAttempts() const noexcept
    {
        return m_max_attempts;
    }

    [[nodiscard]] const std::string& getBackoff() const noexcept
    {
        return m_backoff;
    }

    [[nodiscard]] const std::int64_t& getIntervalMs() const noexcept
    {
        return m_interval_ms;
    }

private:
    std::int64_t m_max_attempts;
    std::string m_backoff;
    std::int64_t m_interval_ms;
};

class BulkExecIdsBody
{
public:
    BulkExecIdsBody() = default;

    void setExecIds(std::vector<std::string> value)
    {
        m_exec_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& getExecIds() const noexcept
    {
        return m_exec_ids;
    }

private:
    std::vector<std::string> m_exec_ids;
};

class BulkResult
{
public:
    BulkResult() = default;

    void setExecId(std::string value)
    {
        m_exec_id = std::move(value);
    }

    void setSuccess(bool value)
    {
        m_success = std::move(value);
    }

    [[nodiscard]] const std::string& getExecId() const noexcept
    {
        return m_exec_id;
    }

    [[nodiscard]] const bool& getSuccess() const noexcept
    {
        return m_success;
    }

private:
    std::string m_exec_id;
    bool m_success;
};

class EventHandler
{
public:
    EventHandler() = default;

    void setName(std::string value)
    {
        m_name = std::move(value);
    }

    void setEvent(std::string value)
    {
        m_event = std::move(value);
    }

    void setCondition(std::optional<std::string> value)
    {
        m_condition = std::move(value);
    }

    void setActions(std::vector<EventAction> value)
    {
        m_actions = std::move(value);
    }

    void setActive(bool value)
    {
        m_active = std::move(value);
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& getEvent() const noexcept
    {
        return m_event;
    }

    [[nodiscard]] const std::optional<std::string>& getCondition() const noexcept
    {
        return m_condition;
    }

    [[nodiscard]] const std::vector<EventAction>& getActions() const noexcept
    {
        return m_actions;
    }

    [[nodiscard]] const bool& getActive() const noexcept
    {
        return m_active;
    }

private:
    std::string m_name;
    std::string m_event;
    std::optional<std::string> m_condition;
    std::vector<EventAction> m_actions;
    bool m_active;
};

class ScheduleNextRun
{
public:
    ScheduleNextRun() = default;

    void setAt(std::string value)
    {
        m_at = std::move(value);
    }

    [[nodiscard]] const std::string& getAt() const noexcept
    {
        return m_at;
    }

private:
    std::string m_at;
};

class QueueUpdateBody
{
public:
    QueueUpdateBody() = default;

    void setExecId(std::string value)
    {
        m_exec_id = std::move(value);
    }

    void setStatus(std::string value)
    {
        m_status = std::move(value);
    }

    void setNodeRef(std::string value)
    {
        m_node_ref = std::move(value);
    }

    void setOutputData(std::string value)
    {
        m_output_data = std::move(value);
    }

    [[nodiscard]] const std::string& getExecId() const noexcept
    {
        return m_exec_id;
    }

    [[nodiscard]] const std::string& getStatus() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::string& getNodeRef() const noexcept
    {
        return m_node_ref;
    }

    [[nodiscard]] const std::string& getOutputData() const noexcept
    {
        return m_output_data;
    }

private:
    std::string m_exec_id;
    std::string m_status;
    std::string m_node_ref;
    std::string m_output_data;
};

class SearchRequestBody
{
public:
    SearchRequestBody() = default;

    void setQuery(std::string value)
    {
        m_query = std::move(value);
    }

    void setSort(std::string value)
    {
        m_sort = std::move(value);
    }

    void setFreeText(std::string value)
    {
        m_free_text = std::move(value);
    }

    void setSize(std::int64_t value)
    {
        m_size = std::move(value);
    }

    void setStart(std::int64_t value)
    {
        m_start = std::move(value);
    }

    [[nodiscard]] const std::string& getQuery() const noexcept
    {
        return m_query;
    }

    [[nodiscard]] const std::string& getSort() const noexcept
    {
        return m_sort;
    }

    [[nodiscard]] const std::string& getFreeText() const noexcept
    {
        return m_free_text;
    }

    [[nodiscard]] const std::int64_t& getSize() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] const std::int64_t& getStart() const noexcept
    {
        return m_start;
    }

private:
    std::string m_query;
    std::string m_sort;
    std::string m_free_text;
    std::int64_t m_size;
    std::int64_t m_start;
};

class TimeoutPolicy
{
public:
    TimeoutPolicy() = default;

    void setTimeoutMs(std::int64_t value)
    {
        m_timeout_ms = std::move(value);
    }

    void setAction(std::string value)
    {
        m_action = std::move(value);
    }

    [[nodiscard]] const std::int64_t& getTimeoutMs() const noexcept
    {
        return m_timeout_ms;
    }

    [[nodiscard]] const std::string& getAction() const noexcept
    {
        return m_action;
    }

private:
    std::int64_t m_timeout_ms;
    std::string m_action;
};

class WorkflowDef
{
public:
    WorkflowDef() = default;

    void setName(std::string value)
    {
        m_name = std::move(value);
    }

    void setInputParams(std::vector<std::string> value)
    {
        m_input_params = std::move(value);
    }

    void setOutputMappings(std::vector<OutputMapping> value)
    {
        m_output_mappings = std::move(value);
    }

    void setVersion(std::int64_t value)
    {
        m_version = std::move(value);
    }

    void setNodes(std::vector<TaskNode> value)
    {
        m_nodes = std::move(value);
    }

    void setFailureWorkflow(std::optional<std::string> value)
    {
        m_failure_workflow = std::move(value);
    }

    void setTimeout(std::optional<TimeoutPolicy> value)
    {
        m_timeout = std::move(value);
    }

    void setRestartable(bool value)
    {
        m_restartable = std::move(value);
    }

    void setWorkflowStatusListenerEnabled(bool value)
    {
        m_workflow_status_listener_enabled = std::move(value);
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::vector<std::string>& getInputParams() const noexcept
    {
        return m_input_params;
    }

    [[nodiscard]] const std::vector<OutputMapping>& getOutputMappings() const noexcept
    {
        return m_output_mappings;
    }

    [[nodiscard]] const std::int64_t& getVersion() const noexcept
    {
        return m_version;
    }

    [[nodiscard]] const std::vector<TaskNode>& getNodes() const noexcept
    {
        return m_nodes;
    }

    [[nodiscard]] const std::optional<std::string>& getFailureWorkflow() const noexcept
    {
        return m_failure_workflow;
    }

    [[nodiscard]] const std::optional<TimeoutPolicy>& getTimeout() const noexcept
    {
        return m_timeout;
    }

    [[nodiscard]] const bool& getRestartable() const noexcept
    {
        return m_restartable;
    }

    [[nodiscard]] const bool& getWorkflowStatusListenerEnabled() const noexcept
    {
        return m_workflow_status_listener_enabled;
    }

private:
    std::string m_name;
    std::vector<std::string> m_input_params;
    std::vector<OutputMapping> m_output_mappings;
    std::int64_t m_version;
    std::vector<TaskNode> m_nodes;
    std::optional<std::string> m_failure_workflow;
    std::optional<TimeoutPolicy> m_timeout;
    bool m_restartable;
    bool m_workflow_status_listener_enabled;
};

class RerunBody
{
public:
    RerunBody() = default;

    void setNodeRef(std::string value)
    {
        m_node_ref = std::move(value);
    }

    void setInput(std::string value)
    {
        m_input = std::move(value);
    }

    [[nodiscard]] const std::string& getNodeRef() const noexcept
    {
        return m_node_ref;
    }

    [[nodiscard]] const std::string& getInput() const noexcept
    {
        return m_input;
    }

private:
    std::string m_node_ref;
    std::string m_input;
};

class AdminConfig
{
public:
    AdminConfig() = default;

    void setDbConfigured(bool value)
    {
        m_db_configured = std::move(value);
    }

    void setSweepIntervalSeconds(std::int64_t value)
    {
        m_sweep_interval_seconds = std::move(value);
    }

    void setLuaBridgeConfigured(bool value)
    {
        m_lua_bridge_configured = std::move(value);
    }

    [[nodiscard]] const bool& getDbConfigured() const noexcept
    {
        return m_db_configured;
    }

    [[nodiscard]] const std::int64_t& getSweepIntervalSeconds() const noexcept
    {
        return m_sweep_interval_seconds;
    }

    [[nodiscard]] const bool& getLuaBridgeConfigured() const noexcept
    {
        return m_lua_bridge_configured;
    }

private:
    bool m_db_configured;
    std::int64_t m_sweep_interval_seconds;
    bool m_lua_bridge_configured;
};

class TaskSubmitBody
{
public:
    TaskSubmitBody() = default;

    void setResult(std::string value)
    {
        m_result = std::move(value);
    }

    void setOutputData(std::string value)
    {
        m_output_data = std::move(value);
    }

    [[nodiscard]] const std::string& getResult() const noexcept
    {
        return m_result;
    }

    [[nodiscard]] const std::string& getOutputData() const noexcept
    {
        return m_output_data;
    }

private:
    std::string m_result;
    std::string m_output_data;
};

class RateLimitPolicy
{
public:
    RateLimitPolicy() = default;

    void setMaxConcurrent(std::int64_t value)
    {
        m_max_concurrent = std::move(value);
    }

    void setRateLimitPerSecond(std::int64_t value)
    {
        m_rate_limit_per_second = std::move(value);
    }

    [[nodiscard]] const std::int64_t& getMaxConcurrent() const noexcept
    {
        return m_max_concurrent;
    }

    [[nodiscard]] const std::int64_t& getRateLimitPerSecond() const noexcept
    {
        return m_rate_limit_per_second;
    }

private:
    std::int64_t m_max_concurrent;
    std::int64_t m_rate_limit_per_second;
};

class TaskDef
{
public:
    TaskDef() = default;

    void setName(std::string value)
    {
        m_name = std::move(value);
    }

    void setWorkerType(std::string value)
    {
        m_worker_type = std::move(value);
    }

    void setMaskedFields(std::vector<std::string> value)
    {
        m_masked_fields = std::move(value);
    }

    void setInputKeys(std::vector<std::string> value)
    {
        m_input_keys = std::move(value);
    }

    void setType(std::string value)
    {
        m_type = std::move(value);
    }

    void setOutputKeys(std::vector<std::string> value)
    {
        m_output_keys = std::move(value);
    }

    void setRateLimit(std::optional<RateLimitPolicy> value)
    {
        m_rate_limit = std::move(value);
    }

    void setInputSchema(std::optional<std::string> value)
    {
        m_input_schema = std::move(value);
    }

    void setWaitDurationMs(std::optional<std::int64_t> value)
    {
        m_wait_duration_ms = std::move(value);
    }

    void setOutputSchema(std::optional<std::string> value)
    {
        m_output_schema = std::move(value);
    }

    void setTimeout(TimeoutPolicy value)
    {
        m_timeout = std::move(value);
    }

    void setDynamicTaskParam(std::optional<std::string> value)
    {
        m_dynamic_task_param = std::move(value);
    }

    void setDomain(std::optional<std::string> value)
    {
        m_domain = std::move(value);
    }

    void setRetry(RetryPolicy value)
    {
        m_retry = std::move(value);
    }

    void setEnforceSchema(bool value)
    {
        m_enforce_schema = std::move(value);
    }

    [[nodiscard]] const std::string& getName() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& getWorkerType() const noexcept
    {
        return m_worker_type;
    }

    [[nodiscard]] const std::vector<std::string>& getMaskedFields() const noexcept
    {
        return m_masked_fields;
    }

    [[nodiscard]] const std::vector<std::string>& getInputKeys() const noexcept
    {
        return m_input_keys;
    }

    [[nodiscard]] const std::string& getType() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const std::vector<std::string>& getOutputKeys() const noexcept
    {
        return m_output_keys;
    }

    [[nodiscard]] const std::optional<RateLimitPolicy>& getRateLimit() const noexcept
    {
        return m_rate_limit;
    }

    [[nodiscard]] const std::optional<std::string>& getInputSchema() const noexcept
    {
        return m_input_schema;
    }

    [[nodiscard]] const std::optional<std::int64_t>& getWaitDurationMs() const noexcept
    {
        return m_wait_duration_ms;
    }

    [[nodiscard]] const std::optional<std::string>& getOutputSchema() const noexcept
    {
        return m_output_schema;
    }

    [[nodiscard]] const TimeoutPolicy& getTimeout() const noexcept
    {
        return m_timeout;
    }

    [[nodiscard]] const std::optional<std::string>& getDynamicTaskParam() const noexcept
    {
        return m_dynamic_task_param;
    }

    [[nodiscard]] const std::optional<std::string>& getDomain() const noexcept
    {
        return m_domain;
    }

    [[nodiscard]] const RetryPolicy& getRetry() const noexcept
    {
        return m_retry;
    }

    [[nodiscard]] const bool& getEnforceSchema() const noexcept
    {
        return m_enforce_schema;
    }

private:
    std::string m_name;
    std::string m_worker_type;
    std::vector<std::string> m_masked_fields;
    std::vector<std::string> m_input_keys;
    std::string m_type;
    std::vector<std::string> m_output_keys;
    std::optional<RateLimitPolicy> m_rate_limit;
    std::optional<std::string> m_input_schema;
    std::optional<std::int64_t> m_wait_duration_ms;
    std::optional<std::string> m_output_schema;
    TimeoutPolicy m_timeout;
    std::optional<std::string> m_dynamic_task_param;
    std::optional<std::string> m_domain;
    RetryPolicy m_retry;
    bool m_enforce_schema;
};

} // namespace congelado_api_dto

template<>
struct serde::Serializable<congelado_api_dto::OutputMapping>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::OutputMapping;
        return std::tuple{
            serde::FieldDesc<"source", &OutputMapping::getSource, &OutputMapping::setSource>{},
            serde::FieldDesc<"target", &OutputMapping::getTarget, &OutputMapping::setTarget>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::EventAction>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::EventAction;
        return std::tuple{
            serde::FieldDesc<"type", &EventAction::getType, &EventAction::setType>{},
            serde::FieldDesc<"payload", &EventAction::getPayload, &EventAction::setPayload>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::WorkflowSchedule>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::WorkflowSchedule;
        return std::tuple{
            serde::FieldDesc<"name", &WorkflowSchedule::getName, &WorkflowSchedule::setName>{},
            serde::FieldDesc<
                "workflow_name", &WorkflowSchedule::getWorkflowName,
                &WorkflowSchedule::setWorkflowName>{},
            serde::FieldDesc<
                "workflow_version", &WorkflowSchedule::getWorkflowVersion,
                &WorkflowSchedule::setWorkflowVersion>{},
            serde::FieldDesc<
                "paused", &WorkflowSchedule::getPaused, &WorkflowSchedule::setPaused>{},
            serde::FieldDesc<
                "last_fired_at", &WorkflowSchedule::getLastFiredAt,
                &WorkflowSchedule::setLastFiredAt>{},
            serde::FieldDesc<
                "cron_expression", &WorkflowSchedule::getCronExpression,
                &WorkflowSchedule::setCronExpression>{},
            serde::FieldDesc<
                "seed_variables", &WorkflowSchedule::getSeedVariables,
                &WorkflowSchedule::setSeedVariables>{},
            serde::FieldDesc<
                "enabled", &WorkflowSchedule::getEnabled, &WorkflowSchedule::setEnabled>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::InputMapping>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::InputMapping;
        return std::tuple{
            serde::FieldDesc<"source", &InputMapping::getSource, &InputMapping::setSource>{},
            serde::FieldDesc<"target", &InputMapping::getTarget, &InputMapping::setTarget>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::TaskEdge>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::TaskEdge;
        return std::tuple{
            serde::FieldDesc<"from", &TaskEdge::getFrom, &TaskEdge::setFrom>{},
            serde::FieldDesc<"to", &TaskEdge::getTo, &TaskEdge::setTo>{},
            serde::FieldDesc<"condition", &TaskEdge::getCondition, &TaskEdge::setCondition>{},
            serde::FieldDesc<"mappings", &TaskEdge::getMappings, &TaskEdge::setMappings>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::TaskNode>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::TaskNode;
        return std::tuple{
            serde::FieldDesc<
                "task_def_name", &TaskNode::getTaskDefName, &TaskNode::setTaskDefName>{},
            serde::FieldDesc<"ref_name", &TaskNode::getRefName, &TaskNode::setRefName>{},
            serde::FieldDesc<"edges", &TaskNode::getEdges, &TaskNode::setEdges>{},
            serde::FieldDesc<"join_on", &TaskNode::getJoinOn, &TaskNode::setJoinOn>{},
            serde::FieldDesc<"join_type", &TaskNode::getJoinType, &TaskNode::setJoinType>{},
            serde::FieldDesc<
                "dynamic_tasks_input_key", &TaskNode::getDynamicTasksInputKey,
                &TaskNode::setDynamicTasksInputKey>{},
            serde::FieldDesc<"loop_body", &TaskNode::getLoopBody, &TaskNode::setLoopBody>{},
            serde::FieldDesc<
                "loop_condition", &TaskNode::getLoopCondition, &TaskNode::setLoopCondition>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::ExecutionTimings>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::ExecutionTimings;
        return std::tuple{
            serde::FieldDesc<
                "scheduled_at", &ExecutionTimings::getScheduledAt,
                &ExecutionTimings::setScheduledAt>{},
            serde::FieldDesc<
                "started_at", &ExecutionTimings::getStartedAt, &ExecutionTimings::setStartedAt>{},
            serde::FieldDesc<
                "completed_at", &ExecutionTimings::getCompletedAt,
                &ExecutionTimings::setCompletedAt>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::TaskInstance>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::TaskInstance;
        return std::tuple{
            serde::FieldDesc<"task_id", &TaskInstance::getTaskId, &TaskInstance::setTaskId>{},
            serde::FieldDesc<
                "input_data", &TaskInstance::getInputData, &TaskInstance::setInputData>{},
            serde::FieldDesc<
                "workflow_exec_id", &TaskInstance::getWorkflowExecId,
                &TaskInstance::setWorkflowExecId>{},
            serde::FieldDesc<"def_name", &TaskInstance::getDefName, &TaskInstance::setDefName>{},
            serde::FieldDesc<"seq", &TaskInstance::getSeq, &TaskInstance::setSeq>{},
            serde::FieldDesc<"node_ref", &TaskInstance::getNodeRef, &TaskInstance::setNodeRef>{},
            serde::FieldDesc<
                "output_data", &TaskInstance::getOutputData, &TaskInstance::setOutputData>{},
            serde::FieldDesc<"status", &TaskInstance::getStatus, &TaskInstance::setStatus>{},
            serde::FieldDesc<
                "retry_count", &TaskInstance::getRetryCount, &TaskInstance::setRetryCount>{},
            serde::FieldDesc<
                "deadline_at", &TaskInstance::getDeadlineAt, &TaskInstance::setDeadlineAt>{},
            serde::FieldDesc<"timings", &TaskInstance::getTimings, &TaskInstance::setTimings>{},
            serde::FieldDesc<
                "next_retry_at", &TaskInstance::getNextRetryAt, &TaskInstance::setNextRetryAt>{},
            serde::FieldDesc<
                "sub_workflow_exec_id", &TaskInstance::getSubWorkflowExecId,
                &TaskInstance::setSubWorkflowExecId>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::WorkflowExecution>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::WorkflowExecution;
        return std::tuple{
            serde::FieldDesc<
                "exec_id", &WorkflowExecution::getExecId, &WorkflowExecution::setExecId>{},
            serde::FieldDesc<
                "status", &WorkflowExecution::getStatus, &WorkflowExecution::setStatus>{},
            serde::FieldDesc<
                "dynamic_nodes", &WorkflowExecution::getDynamicNodes,
                &WorkflowExecution::setDynamicNodes>{},
            serde::FieldDesc<
                "def_name", &WorkflowExecution::getDefName, &WorkflowExecution::setDefName>{},
            serde::FieldDesc<
                "def_version", &WorkflowExecution::getDefVersion,
                &WorkflowExecution::setDefVersion>{},
            serde::FieldDesc<
                "task_instances", &WorkflowExecution::getTaskInstances,
                &WorkflowExecution::setTaskInstances>{},
            serde::FieldDesc<
                "correlation_id", &WorkflowExecution::getCorrelationId,
                &WorkflowExecution::setCorrelationId>{},
            serde::FieldDesc<
                "variables", &WorkflowExecution::getVariables, &WorkflowExecution::setVariables>{},
            serde::FieldDesc<
                "timings", &WorkflowExecution::getTimings, &WorkflowExecution::setTimings>{},
            serde::FieldDesc<
                "parent_exec_id", &WorkflowExecution::getParentExecId,
                &WorkflowExecution::setParentExecId>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::SignalBody>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::SignalBody;
        return std::tuple{
            serde::FieldDesc<"node_ref", &SignalBody::getNodeRef, &SignalBody::setNodeRef>{},
            serde::FieldDesc<"payload", &SignalBody::getPayload, &SignalBody::setPayload>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::PollData>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::PollData;
        return std::tuple{
            serde::FieldDesc<"worker_type", &PollData::getWorkerType, &PollData::setWorkerType>{},
            serde::FieldDesc<"last_poll_at", &PollData::getLastPollAt, &PollData::setLastPollAt>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::RetryPolicy>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::RetryPolicy;
        return std::tuple{
            serde::FieldDesc<
                "max_attempts", &RetryPolicy::getMaxAttempts, &RetryPolicy::setMaxAttempts>{},
            serde::FieldDesc<"backoff", &RetryPolicy::getBackoff, &RetryPolicy::setBackoff>{},
            serde::FieldDesc<
                "interval_ms", &RetryPolicy::getIntervalMs, &RetryPolicy::setIntervalMs>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::BulkExecIdsBody>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::BulkExecIdsBody;
        return std::tuple{
            serde::FieldDesc<
                "exec_ids", &BulkExecIdsBody::getExecIds, &BulkExecIdsBody::setExecIds>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::BulkResult>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::BulkResult;
        return std::tuple{
            serde::FieldDesc<"exec_id", &BulkResult::getExecId, &BulkResult::setExecId>{},
            serde::FieldDesc<"success", &BulkResult::getSuccess, &BulkResult::setSuccess>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::EventHandler>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::EventHandler;
        return std::tuple{
            serde::FieldDesc<"name", &EventHandler::getName, &EventHandler::setName>{},
            serde::FieldDesc<"event", &EventHandler::getEvent, &EventHandler::setEvent>{},
            serde::FieldDesc<
                "condition", &EventHandler::getCondition, &EventHandler::setCondition>{},
            serde::FieldDesc<"actions", &EventHandler::getActions, &EventHandler::setActions>{},
            serde::FieldDesc<"active", &EventHandler::getActive, &EventHandler::setActive>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::ScheduleNextRun>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::ScheduleNextRun;
        return std::tuple{
            serde::FieldDesc<"at", &ScheduleNextRun::getAt, &ScheduleNextRun::setAt>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::QueueUpdateBody>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::QueueUpdateBody;
        return std::tuple{
            serde::FieldDesc<"exec_id", &QueueUpdateBody::getExecId, &QueueUpdateBody::setExecId>{},
            serde::FieldDesc<"status", &QueueUpdateBody::getStatus, &QueueUpdateBody::setStatus>{},
            serde::FieldDesc<
                "node_ref", &QueueUpdateBody::getNodeRef, &QueueUpdateBody::setNodeRef>{},
            serde::FieldDesc<
                "output_data", &QueueUpdateBody::getOutputData, &QueueUpdateBody::setOutputData>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::SearchRequestBody>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::SearchRequestBody;
        return std::tuple{
            serde::FieldDesc<"query", &SearchRequestBody::getQuery, &SearchRequestBody::setQuery>{},
            serde::FieldDesc<"sort", &SearchRequestBody::getSort, &SearchRequestBody::setSort>{},
            serde::FieldDesc<
                "free_text", &SearchRequestBody::getFreeText, &SearchRequestBody::setFreeText>{},
            serde::FieldDesc<"size", &SearchRequestBody::getSize, &SearchRequestBody::setSize>{},
            serde::FieldDesc<"start", &SearchRequestBody::getStart, &SearchRequestBody::setStart>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::TimeoutPolicy>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::TimeoutPolicy;
        return std::tuple{
            serde::FieldDesc<
                "timeout_ms", &TimeoutPolicy::getTimeoutMs, &TimeoutPolicy::setTimeoutMs>{},
            serde::FieldDesc<"action", &TimeoutPolicy::getAction, &TimeoutPolicy::setAction>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::WorkflowDef>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::WorkflowDef;
        return std::tuple{
            serde::FieldDesc<"name", &WorkflowDef::getName, &WorkflowDef::setName>{},
            serde::FieldDesc<
                "input_params", &WorkflowDef::getInputParams, &WorkflowDef::setInputParams>{},
            serde::FieldDesc<
                "output_mappings", &WorkflowDef::getOutputMappings,
                &WorkflowDef::setOutputMappings>{},
            serde::FieldDesc<"version", &WorkflowDef::getVersion, &WorkflowDef::setVersion>{},
            serde::FieldDesc<"nodes", &WorkflowDef::getNodes, &WorkflowDef::setNodes>{},
            serde::FieldDesc<
                "failure_workflow", &WorkflowDef::getFailureWorkflow,
                &WorkflowDef::setFailureWorkflow>{},
            serde::FieldDesc<"timeout", &WorkflowDef::getTimeout, &WorkflowDef::setTimeout>{},
            serde::FieldDesc<
                "restartable", &WorkflowDef::getRestartable, &WorkflowDef::setRestartable>{},
            serde::FieldDesc<
                "workflow_status_listener_enabled", &WorkflowDef::getWorkflowStatusListenerEnabled,
                &WorkflowDef::setWorkflowStatusListenerEnabled>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::RerunBody>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::RerunBody;
        return std::tuple{
            serde::FieldDesc<"node_ref", &RerunBody::getNodeRef, &RerunBody::setNodeRef>{},
            serde::FieldDesc<"input", &RerunBody::getInput, &RerunBody::setInput>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::AdminConfig>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::AdminConfig;
        return std::tuple{
            serde::FieldDesc<
                "db_configured", &AdminConfig::getDbConfigured, &AdminConfig::setDbConfigured>{},
            serde::FieldDesc<
                "sweep_interval_seconds", &AdminConfig::getSweepIntervalSeconds,
                &AdminConfig::setSweepIntervalSeconds>{},
            serde::FieldDesc<
                "lua_bridge_configured", &AdminConfig::getLuaBridgeConfigured,
                &AdminConfig::setLuaBridgeConfigured>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::TaskSubmitBody>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::TaskSubmitBody;
        return std::tuple{
            serde::FieldDesc<"result", &TaskSubmitBody::getResult, &TaskSubmitBody::setResult>{},
            serde::FieldDesc<
                "output_data", &TaskSubmitBody::getOutputData, &TaskSubmitBody::setOutputData>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::RateLimitPolicy>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::RateLimitPolicy;
        return std::tuple{
            serde::FieldDesc<
                "max_concurrent", &RateLimitPolicy::getMaxConcurrent,
                &RateLimitPolicy::setMaxConcurrent>{},
            serde::FieldDesc<
                "rate_limit_per_second", &RateLimitPolicy::getRateLimitPerSecond,
                &RateLimitPolicy::setRateLimitPerSecond>{},
        };
    }
};

template<>
struct serde::Serializable<congelado_api_dto::TaskDef>
{
    static constexpr auto fields()
    {
        using congelado_api_dto::TaskDef;
        return std::tuple{
            serde::FieldDesc<"name", &TaskDef::getName, &TaskDef::setName>{},
            serde::FieldDesc<"worker_type", &TaskDef::getWorkerType, &TaskDef::setWorkerType>{},
            serde::FieldDesc<
                "masked_fields", &TaskDef::getMaskedFields, &TaskDef::setMaskedFields>{},
            serde::FieldDesc<"input_keys", &TaskDef::getInputKeys, &TaskDef::setInputKeys>{},
            serde::FieldDesc<"type", &TaskDef::getType, &TaskDef::setType>{},
            serde::FieldDesc<"output_keys", &TaskDef::getOutputKeys, &TaskDef::setOutputKeys>{},
            serde::FieldDesc<"rate_limit", &TaskDef::getRateLimit, &TaskDef::setRateLimit>{},
            serde::FieldDesc<"input_schema", &TaskDef::getInputSchema, &TaskDef::setInputSchema>{},
            serde::FieldDesc<
                "wait_duration_ms", &TaskDef::getWaitDurationMs, &TaskDef::setWaitDurationMs>{},
            serde::FieldDesc<
                "output_schema", &TaskDef::getOutputSchema, &TaskDef::setOutputSchema>{},
            serde::FieldDesc<"timeout", &TaskDef::getTimeout, &TaskDef::setTimeout>{},
            serde::FieldDesc<
                "dynamic_task_param", &TaskDef::getDynamicTaskParam,
                &TaskDef::setDynamicTaskParam>{},
            serde::FieldDesc<"domain", &TaskDef::getDomain, &TaskDef::setDomain>{},
            serde::FieldDesc<"retry", &TaskDef::getRetry, &TaskDef::setRetry>{},
            serde::FieldDesc<
                "enforce_schema", &TaskDef::getEnforceSchema, &TaskDef::setEnforceSchema>{},
        };
    }
};
