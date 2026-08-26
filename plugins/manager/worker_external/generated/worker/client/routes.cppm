export module congelado_worker_api_routes;

import std;
import interfaces;
import serde;
import core_client;
import congelado_worker_api_dto;

export namespace congelado_worker_api {

class Client
{
public:
    Client() = default;

    void setRuntime(interfaces::IClient& client)
    {
        m_register.set_runtime(client);
    }

    void dispatch(interfaces::io::IRequest& request, interfaces::io::IResponse& response)
    {
        m_register.dispatch(request, response);
    }

    void ack_post(
        std::string_view id,
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request =
            core::client::Client::custom("POST", std::format("/api/v1/worker/ack/{}", id))
                .build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }

    void executions_delete_(
        std::string_view id,
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request =
            core::client::Client::custom("DELETE", std::format("/api/v1/worker/executions/{}", id))
                .build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }

    void executions_get(
        std::string_view id,
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request =
            core::client::Client::custom("GET", std::format("/api/v1/worker/executions/{}", id))
                .build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }

    void executions_get(
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request = core::client::Client::custom("GET", "/api/v1/worker/executions")
                           .build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }

    void health_get(
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request = core::client::Client::custom("GET", "/api/v1/worker/health")
                           .build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }

    void info_get(
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request =
            core::client::Client::custom("GET", "/api/v1/worker/info").build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }

    void poll_post(
        std::string_view type,
        std::function<void()> onResponse,
        std::function<void(std::string)> onError = [](std::string) {}
    )
    {
        if (!m_register.has_runtime()) {
            throw std::runtime_error("Please call setRuntime() first");
        }
        auto request =
            core::client::Client::custom("POST", std::format("/api/v1/worker/poll/{}", type))
                .build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_response = std::move(onResponse),
             on_error = std::move(onError)](interfaces::io::IResponse& response) mutable {
                if (response.is_success()) {
                    on_response();
                } else {
                    on_error(std::string{response.get_status_text()});
                }
            }
        );
    }


private:
    core::client::Register m_register;
};

} // namespace congelado_worker_api
