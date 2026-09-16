// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/server/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/server/server.h"

export module cc_abi_sonic_server;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Server : public ice::sonic::Runtime<Server, TF_Server>
{
public:
    explicit Server(TF_Server* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "server";

    [[nodiscard]] std::expected<void, ice::Status> get_bind_host(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_bind_host(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_bind_port() noexcept
    {
        ice::Status status;
        m_ops->get_bind_port(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_tls_cert(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_tls_cert(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_tls_key(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_tls_key(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> new_server() noexcept
    {
        ice::Status status;
        m_ops->new_server(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_server() noexcept
    {
        ice::Status status;
        m_ops->destroy_server(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_request_handler(TF_Server_RequestHandler handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->set_request_handler(get_handle(), handler, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_connect(TF_Server_ConnectFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_connect(get_handle(), handler, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_disconnect(TF_Server_DisconnectFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_disconnect(get_handle(), handler, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> start() noexcept
    {
        ice::Status status;
        m_ops->start(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept
    {
        ice::Status status;
        m_ops->stop(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> stop_accepting() noexcept
    {
        ice::Status status;
        m_ops->stop_accepting(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> resume_accepting() noexcept
    {
        ice::Status status;
        m_ops->resume_accepting(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_running() noexcept
    {
        ice::Status status;
        m_ops->is_running(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_idle() noexcept
    {
        ice::Status status;
        m_ops->is_idle(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_max_connections(size_t max_connections) noexcept
    {
        ice::Status status;
        m_ops->set_max_connections(get_handle(), max_connections, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_max_connections() noexcept
    {
        ice::Status status;
        m_ops->get_max_connections(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    find_connection(const ice::sonic::String& connection_id) noexcept
    {
        ice::Status status;
        m_ops->find_connection(get_handle(), connection_id.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_connection_id(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_connection_id(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    send_response(const ice::sonic::Response& response) noexcept
    {
        ice::Status status;
        m_ops->send_response(get_handle(), response.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    broadcast(const ice::sonic::Response& response) noexcept
    {
        ice::Status status;
        m_ops->broadcast(get_handle(), response.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> close_connection() noexcept
    {
        ice::Status status;
        m_ops->close_connection(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_connections(const ice::sonic::Vector& out_connections) noexcept
    {
        ice::Status status;
        m_ops->list_connections(get_handle(), out_connections.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_connection_count() noexcept
    {
        ice::Status status;
        m_ops->get_connection_count(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_stats(const ice::sonic::Map& out_stats) noexcept
    {
        ice::Status status;
        m_ops->get_stats(get_handle(), out_stats.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    register_extension(const ice::sonic::String& name, const ice::sonic::Map& config) noexcept
    {
        ice::Status status;
        m_ops->register_extension(
            get_handle(),
            name.get_handle(),
            config.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    unregister_extension(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->unregister_extension(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_extensions(const ice::sonic::Vector& out_names) noexcept
    {
        ice::Status status;
        m_ops->list_extensions(get_handle(), out_names.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> reload_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept
    {
        ice::Status status;
        m_ops->reload_certificate(
            get_handle(),
            cert_path.get_handle(),
            key_path.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
