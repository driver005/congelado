export module congelado_api_routes;

import std;
import interfaces;
import serde;
import congelado_client;
import congelado_api_dto;

export namespace congelado_api::admin {

void get_config(std::function<void(congelado_api_dto::AdminConfig)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/admin/config");
    congelado::client::ClientRuntime::send<congelado_api_dto::AdminConfig>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_consistency(std::string_view exec_id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/admin/consistency/{}", exec_id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::admin

export namespace congelado_api::event_handlers {

void get(std::function<void(std::vector<congelado_api_dto::EventHandler>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/event_handlers");
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::EventHandler>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post(const congelado_api_dto::EventHandler &body, std::function<void(congelado_api_dto::EventHandler)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/event_handlers");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::EventHandler>(std::move(request), std::move(onResponse), std::move(onError));
}

void delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/event_handlers/{}", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void put(std::string_view name, const congelado_api_dto::EventHandler &body, std::function<void(congelado_api_dto::EventHandler)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("PUT").with_path(std::format("/api/v1/event_handlers/{}", name));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::EventHandler>(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view name, std::function<void(congelado_api_dto::EventHandler)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/event_handlers/{}", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::EventHandler>(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::event_handlers

export namespace congelado_api::metadata {

void get_health(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/metadata/health");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_tasks(std::function<void(std::vector<congelado_api_dto::TaskDef>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/metadata/tasks");
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::TaskDef>>(std::move(request), std::move(onResponse), std::move(onError));
}

void get_workflows(std::function<void(std::vector<congelado_api_dto::WorkflowDef>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/metadata/workflows");
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::WorkflowDef>>(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::metadata

export namespace congelado_api::query {

void post(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/query");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::query

export namespace congelado_api::queue {

void post_update(const congelado_api_dto::QueueUpdateBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/queue/update");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::queue

export namespace congelado_api::schedules {

void get_next_few_runs(std::string_view name, std::function<void(std::vector<congelado_api_dto::ScheduleNextRun>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/schedules/{}/next_few_runs", name));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::ScheduleNextRun>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_resume(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/schedules/{}/resume", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_pause(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/schedules/{}/pause", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::function<void(std::vector<congelado_api_dto::WorkflowSchedule>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/schedules");
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::WorkflowSchedule>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post(const congelado_api_dto::WorkflowSchedule &body, std::function<void(congelado_api_dto::WorkflowSchedule)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/schedules");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowSchedule>(std::move(request), std::move(onResponse), std::move(onError));
}

void delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/schedules/{}", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void put(std::string_view name, const congelado_api_dto::WorkflowSchedule &body, std::function<void(congelado_api_dto::WorkflowSchedule)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("PUT").with_path(std::format("/api/v1/schedules/{}", name));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowSchedule>(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view name, std::function<void(congelado_api_dto::WorkflowSchedule)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/schedules/{}", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowSchedule>(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::schedules

export namespace congelado_api::tasks {

void get_queue_polldata(std::function<void(std::vector<congelado_api_dto::PollData>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/tasks/queue_polldata");
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::PollData>>(std::move(request), std::move(onResponse), std::move(onError));
}

void get_queue_sizes(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/tasks/queue_sizes");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_queue(std::string_view type, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/tasks/queue/{}", type));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_search(const congelado_api_dto::SearchRequestBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/tasks/search");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_queue_domain(std::string_view type, std::string_view domain, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/tasks/queue/{}/domain/{}", type, domain));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void patch_heartbeat(std::string_view id, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("PATCH").with_path(std::format("/api/v1/tasks/{}/heartbeat", id));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/tasks/{}", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void put(std::string_view name, const congelado_api_dto::TaskDef &body, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("PUT").with_path(std::format("/api/v1/tasks/{}", name));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view name, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/tasks/{}", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_result(std::string_view id, const congelado_api_dto::TaskSubmitBody &body, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/tasks/{}/result", id));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void post(const congelado_api_dto::TaskDef &body, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/tasks");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_enqueue(std::string_view name, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/tasks/{}/enqueue", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_queue_requeue(std::string_view type, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/tasks/queue_requeue/{}", type));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::tasks

export namespace congelado_api::worker {

void delete_executions(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/worker/executions/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_executions(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/worker/executions/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_executions(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/worker/executions");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_ack(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/worker/ack/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_poll(std::string_view type, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/worker/poll/{}", type));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_health(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/worker/health");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_info(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/worker/info");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::worker

export namespace congelado_api::workflow {

void post_search(const congelado_api_dto::SearchRequestBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflow/search");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::workflow

export namespace congelado_api::workflows {

void delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/workflows/{}", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void put(std::string_view name, const congelado_api_dto::WorkflowDef &body, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("PUT").with_path(std::format("/api/v1/workflows/{}", name));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view name, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/workflows/{}", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void post(const congelado_api_dto::WorkflowDef &body, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_bulk_pause(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows/bulk/pause");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::BulkResult>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_exec_pause(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/exec/{}/pause", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void delete_exec(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/workflows/exec/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_exec(std::string_view id, std::function<void(congelado_api_dto::WorkflowExecution)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/workflows/exec/{}", id));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowExecution>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_bulk_retry(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows/bulk/retry");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::BulkResult>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_exec_retry(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/exec/{}/retry", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_exec_restart(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/exec/{}/restart", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_exec_rerun(std::string_view id, const congelado_api_dto::RerunBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/exec/{}/rerun", id));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_exec_signal(std::string_view id, const congelado_api_dto::SignalBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/exec/{}/signal", id));
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_bulk_resume(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows/bulk/resume");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::BulkResult>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_bulk_restart(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows/bulk/restart");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::BulkResult>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_bulk_terminate(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows/bulk/terminate");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::BulkResult>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_exec_resume(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/exec/{}/resume", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_bulk_remove(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows/bulk/remove");
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    congelado::client::ClientRuntime::send<std::vector<congelado_api_dto::BulkResult>>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_start(std::string_view name, std::function<void(congelado_api_dto::WorkflowExecution)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/workflows/{}/start", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowExecution>(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::workflows

