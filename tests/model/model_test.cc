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

TEST_CASE("is_terminal(WorkflowStatus)") {
    using enum model::WorkflowStatus;
    CHECK(model::is_terminal(COMPLETED));
    CHECK(model::is_terminal(FAILED));
    CHECK(model::is_terminal(TIMED_OUT));
    CHECK(model::is_terminal(TERMINATED));
    CHECK_FALSE(model::is_terminal(RUNNING));
    CHECK_FALSE(model::is_terminal(PAUSED));
}

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

TEST_CASE("TaskInstance construction") {
    model::TaskInstance inst{
        .task_id          = model::generate_id(),
        .def_name         = "send_email",
        .workflow_exec_id = model::generate_id(),
    };
    CHECK_FALSE(inst.task_id.is_nil());
    CHECK(inst.status == model::TaskStatus::SCHEDULED);
    CHECK(inst.seq == 0);
    CHECK(inst.retry_count == 0);
    CHECK(inst.input_data.empty());
    CHECK(inst.output_data.empty());
}

TEST_CASE("WorkflowEvent construction") {
    model::WorkflowEvent ev{
        .exec_id   = model::generate_id(),
        .type      = model::WorkflowEventType::PAUSE,
        .payload   = std::nullopt,
        .issued_at = std::chrono::system_clock::now(),
    };
    CHECK_FALSE(ev.exec_id.is_nil());
    CHECK(ev.type == model::WorkflowEventType::PAUSE);
    CHECK_FALSE(ev.payload.has_value());
}
