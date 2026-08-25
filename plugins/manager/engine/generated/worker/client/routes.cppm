export module congelado_worker_api_routes;

import std;
import interfaces;
import serde;
import congelado_client;
import congelado_worker_api_dto;

export namespace congelado_worker_api::ack {

void post(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/worker/ack/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_worker_api::ack

export namespace congelado_worker_api::executions {

void delete_(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("DELETE").with_path(std::format("/api/v1/worker/executions/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path(std::format("/api/v1/worker/executions/{}", id));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

void get(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/worker/executions");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_worker_api::executions

export namespace congelado_worker_api::health {

void get(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/worker/health");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_worker_api::health

export namespace congelado_worker_api::info {

void get(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("GET").with_path("/api/v1/worker/info");
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_worker_api::info

export namespace congelado_worker_api::poll {

void post(std::string_view type, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    auto request = congelado::client::ClientRuntime::new_request();
    std::move(*request).with_method("POST").with_path(std::format("/api/v1/worker/poll/{}", type));
    congelado::client::ClientRuntime::send(std::move(request), std::move(onResponse), std::move(onError));
}

} // namespace congelado_worker_api::poll

