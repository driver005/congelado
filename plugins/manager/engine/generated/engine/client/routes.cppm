export module congelado_api_routes;

import std;
import interfaces;
import serde;
import core_client;
import congelado_api_dto;

export namespace congelado_api {

class Client {
  public:
    Client() = default;

    void setRuntime(interfaces::IClient &client) {
    m_register.set_runtime(client);
    }

    void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response) {
    m_register.dispatch(request, response);
    }

    void admin_get_config(std::function<void(congelado_api_dto::AdminConfig)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/admin/config").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::AdminConfig>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void admin_post_consistency(std::string_view exec_id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/admin/consistency/{}", exec_id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void event_handlers_get(std::function<void(std::vector<congelado_api_dto::EventHandler>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/event_handlers").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::EventHandler>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void event_handlers_post(const congelado_api_dto::EventHandler &body, std::function<void(congelado_api_dto::EventHandler)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/event_handlers").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::EventHandler>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void event_handlers_delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("DELETE", std::format("/api/v1/event_handlers/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void event_handlers_put(std::string_view name, const congelado_api_dto::EventHandler &body, std::function<void(congelado_api_dto::EventHandler)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("PUT", std::format("/api/v1/event_handlers/{}", name)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::EventHandler>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void event_handlers_get(std::string_view name, std::function<void(congelado_api_dto::EventHandler)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/event_handlers/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::EventHandler>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void metadata_get_health(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/metadata/health").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void metadata_get_tasks(std::function<void(std::vector<congelado_api_dto::TaskDef>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/metadata/tasks").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::TaskDef>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void metadata_get_workflows(std::function<void(std::vector<congelado_api_dto::WorkflowDef>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/metadata/workflows").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::WorkflowDef>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void query_post(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/query").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void queue_post_update(const congelado_api_dto::QueueUpdateBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/queue/update").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void schedules_get_next_few_runs(std::string_view name, std::function<void(std::vector<congelado_api_dto::ScheduleNextRun>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/schedules/{}/next_few_runs", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::ScheduleNextRun>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void schedules_post_resume(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/schedules/{}/resume", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void schedules_post_pause(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/schedules/{}/pause", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void schedules_get(std::function<void(std::vector<congelado_api_dto::WorkflowSchedule>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/schedules").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::WorkflowSchedule>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void schedules_post(const congelado_api_dto::WorkflowSchedule &body, std::function<void(congelado_api_dto::WorkflowSchedule)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/schedules").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowSchedule>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void schedules_delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("DELETE", std::format("/api/v1/schedules/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void schedules_put(std::string_view name, const congelado_api_dto::WorkflowSchedule &body, std::function<void(congelado_api_dto::WorkflowSchedule)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("PUT", std::format("/api/v1/schedules/{}", name)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowSchedule>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void schedules_get(std::string_view name, std::function<void(congelado_api_dto::WorkflowSchedule)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/schedules/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowSchedule>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_get_queue_polldata(std::function<void(std::vector<congelado_api_dto::PollData>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/tasks/queue_polldata").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::PollData>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_get_queue_sizes(std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", "/api/v1/tasks/queue_sizes").build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void tasks_get_queue(std::string_view type, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/tasks/queue/{}", type)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskInstance>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_post_search(const congelado_api_dto::SearchRequestBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/tasks/search").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void tasks_get_queue_domain(std::string_view type, std::string_view domain, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/tasks/queue/{}/domain/{}", type, domain)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskInstance>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_patch_heartbeat(std::string_view id, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("PATCH", std::format("/api/v1/tasks/{}/heartbeat", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskInstance>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("DELETE", std::format("/api/v1/tasks/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void tasks_put(std::string_view name, const congelado_api_dto::TaskDef &body, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("PUT", std::format("/api/v1/tasks/{}", name)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskDef>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_get(std::string_view name, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/tasks/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskDef>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_post_result(std::string_view id, const congelado_api_dto::TaskSubmitBody &body, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/tasks/{}/result", id)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskInstance>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_post(const congelado_api_dto::TaskDef &body, std::function<void(congelado_api_dto::TaskDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/tasks").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskDef>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_post_enqueue(std::string_view name, std::function<void(congelado_api_dto::TaskInstance)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/tasks/{}/enqueue", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::TaskInstance>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void tasks_post_queue_requeue(std::string_view type, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/tasks/queue_requeue/{}", type)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflow_post_search(const congelado_api_dto::SearchRequestBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflow/search").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_delete_(std::string_view name, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("DELETE", std::format("/api/v1/workflows/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_put(std::string_view name, const congelado_api_dto::WorkflowDef &body, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("PUT", std::format("/api/v1/workflows/{}", name)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowDef>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_get(std::string_view name, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/workflows/{}", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowDef>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post(const congelado_api_dto::WorkflowDef &body, std::function<void(congelado_api_dto::WorkflowDef)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowDef>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_bulk_pause(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows/bulk/pause").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::BulkResult>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_exec_pause(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/exec/{}/pause", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_delete_exec(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("DELETE", std::format("/api/v1/workflows/exec/{}", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_get_exec(std::string_view id, std::function<void(congelado_api_dto::WorkflowExecution)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("GET", std::format("/api/v1/workflows/exec/{}", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowExecution>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_bulk_retry(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows/bulk/retry").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::BulkResult>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_exec_retry(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/exec/{}/retry", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_post_exec_restart(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/exec/{}/restart", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_post_exec_rerun(std::string_view id, const congelado_api_dto::RerunBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/exec/{}/rerun", id)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_post_exec_signal(std::string_view id, const congelado_api_dto::SignalBody &body, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/exec/{}/signal", id)).build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_post_bulk_resume(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows/bulk/resume").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::BulkResult>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_bulk_restart(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows/bulk/restart").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::BulkResult>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_bulk_terminate(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows/bulk/terminate").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::BulkResult>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_exec_resume(std::string_view id, std::function<void()> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/exec/{}/resume", id)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        if (response.is_success()) { on_response(); }
        else { on_error(std::string{response.get_status_text()}); }
    });
    }

    void workflows_post_bulk_remove(const congelado_api_dto::BulkExecIdsBody &body, std::function<void(std::vector<congelado_api_dto::BulkResult>)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", "/api/v1/workflows/bulk/remove").build(m_register.runtime());
    std::move(*request).with_content_type("application/json");
    request->set_body(serde::Ser::serialize("application/json", body));
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<std::vector<congelado_api_dto::BulkResult>>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }

    void workflows_post_start(std::string_view name, std::function<void(congelado_api_dto::WorkflowExecution)> onResponse, std::function<void(std::string)> onError = [](std::string) {}) {
    if (!m_register.has_runtime()) { throw std::runtime_error("Please call setRuntime() first"); }
    auto request = core::client::Client::custom("POST", std::format("/api/v1/workflows/{}/start", name)).build(m_register.runtime());
    m_register.send(std::move(request), [on_response = std::move(onResponse), on_error = std::move(onError)](interfaces::io::IResponse &response) mutable {
        auto &body_view = response.get_body();
        std::string body;
        body.reserve(body_view.size());
        for (auto byte : body_view) { body.push_back(static_cast<char>(byte)); }
        auto result = serde::Ser::deserialize<congelado_api_dto::WorkflowExecution>(response.get_content_type(), body);
        if (result.has_value()) { on_response(std::move(*result)); }
        else { on_error(result.error()); }
    });
    }



  private:
    core::client::Register m_register;
};

} // namespace congelado_api

