export module congelado_api_routes;

import std;
import interfaces;
import serde;
import congelado_client;
import congelado_api_dto;

export namespace congelado_api::metadata {

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

void get_health(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/metadata/health");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::metadata

export namespace congelado_api::tasks {

void delete_executions(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/tasks/executions/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_executions(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/tasks/executions/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_ack(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/tasks/ack/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
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
    { auto encoded = serde::Json::encode(body);
      std::vector<std::byte> bytes(encoded.size());
      std::ranges::transform(encoded, bytes.begin(), [](char c) { return std::byte(c); });
      request->set_body(std::move(bytes)); }
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view name, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/tasks/{}", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void post_poll(std::string_view type, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/tasks/poll/{}", type));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void post_enqueue(std::string_view name, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/tasks/{}/enqueue", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void post(const congelado_api_dto::TaskDef &body, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/tasks");
    std::move(*request).with_content_type("application/json");
    { auto encoded = serde::Json::encode(body);
      std::vector<std::byte> bytes(encoded.size());
      std::ranges::transform(encoded, bytes.begin(), [](char c) { return std::byte(c); });
      request->set_body(std::move(bytes)); }
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void get_queue(std::string_view type, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/tasks/queue/{}", type));
    congelado::client::ClientRuntime::send<congelado_api_dto::TaskInstance>(std::move(request), std::move(onResponse), std::move(onError));
}

void get_executions(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/tasks/executions");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_info(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/tasks/info");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get_health(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/tasks/health");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_api::tasks

export namespace congelado_api::workflows {

void post(const congelado_api_dto::WorkflowDef &body, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path("/api/v1/workflows");
    std::move(*request).with_content_type("application/json");
    { auto encoded = serde::Json::encode(body);
      std::vector<std::byte> bytes(encoded.size());
      std::ranges::transform(encoded, bytes.begin(), [](char c) { return std::byte(c); });
      request->set_body(std::move(bytes)); }
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/workflows/{}", name));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void put(std::string_view name, const congelado_api_dto::WorkflowDef &body, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("PUT").with_path(std::format("/api/v1/workflows/{}", name));
    std::move(*request).with_content_type("application/json");
    { auto encoded = serde::Json::encode(body);
      std::vector<std::byte> bytes(encoded.size());
      std::ranges::transform(encoded, bytes.begin(), [](char c) { return std::byte(c); });
      request->set_body(std::move(bytes)); }
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowDef>(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view name, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/workflows/{}", name));
    congelado::client::ClientRuntime::send<congelado_api_dto::WorkflowDef>(std::move(request), std::move(onResponse), std::move(onError));
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

} // namespace congelado_api::workflows

