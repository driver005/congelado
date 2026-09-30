// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/server.h"

export module cc_ice_extern_io_sonic:server;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ServerOps : public ice::sonic::Runtime<TF_ServerOps, TF_ServerOps>
{
public:
    explicit TF_ServerOps(TF_ServerOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "io";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void get_bind_host(const ice::sonic::String& out_host) noexcept
    {
        m_ops->get_bind_host(get_handle(), out_host.get_handle());
    }

    void get_bind_port(uint16_t* out_port) noexcept
    {
        m_ops->get_bind_port(get_handle(), out_port);
    }

    void get_tls_cert(const ice::sonic::String& out_cert) noexcept
    {
        m_ops->get_tls_cert(get_handle(), out_cert.get_handle());
    }

    void get_tls_key(const ice::sonic::String& out_key) noexcept
    {
        m_ops->get_tls_key(get_handle(), out_key.get_handle());
    }

    void set_request_handler(TFServerRequestHandler handler, void* user_data) noexcept
    {
        m_ops->set_request_handler(get_handle(), handler, user_data);
    }

    void on_connect(TFServerConnectFn handler, void* user_data) noexcept
    {
        m_ops->on_connect(get_handle(), handler, user_data);
    }

    void on_disconnect(TFServerDisconnectFn handler, void* user_data) noexcept
    {
        m_ops->on_disconnect(get_handle(), handler, user_data);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> start() noexcept
    {
        ice::sonic::Status status;
        m_ops->start(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> stop() noexcept
    {
        ice::sonic::Status status;
        m_ops->stop(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> stop_accepting() noexcept
    {
        ice::sonic::Status status;
        m_ops->stop_accepting(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> resume_accepting() noexcept
    {
        ice::sonic::Status status;
        m_ops->resume_accepting(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void is_running(int* out_running) noexcept
    {
        m_ops->is_running(get_handle(), out_running);
    }

    void is_idle(int* out_idle) noexcept
    {
        m_ops->is_idle(get_handle(), out_idle);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_max_connections(size_t max_connections) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_max_connections(get_handle(), max_connections, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_max_connections(size_t* out_max_connections) noexcept
    {
        m_ops->get_max_connections(get_handle(), out_max_connections);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> find_connection(
        const ice::sonic::String& connection_id,
        TFServerConnection* out_connection
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->find_connection(
            get_handle(),
            connection_id.get_handle(),
            out_connection,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_connection_id(const ice::sonic::String& out_connection_id) noexcept
    {
        m_ops->get_connection_id(get_handle(), out_connection_id.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    send_response(const ice::sonic::TF_ResponseOps& response) noexcept
    {
        ice::sonic::Status status;
        m_ops->send_response(get_handle(), response.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    broadcast(const ice::sonic::TF_ResponseOps& response) noexcept
    {
        ice::sonic::Status status;
        m_ops->broadcast(get_handle(), response.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> close_connection() noexcept
    {
        ice::sonic::Status status;
        m_ops->close_connection(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list_connections(const ice::sonic::TF_VectorOps& out_connections) noexcept
    {
        ice::sonic::Status status;
        m_ops->list_connections(get_handle(), out_connections.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_connection_count(size_t* out_count) noexcept
    {
        m_ops->get_connection_count(get_handle(), out_count);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_stats(get_handle(), out_stats.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    register_extension(const ice::sonic::String& name, const ice::sonic::TF_MapOps& config) noexcept
    {
        ice::sonic::Status status;
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

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    unregister_extension(const ice::sonic::String& name) noexcept
    {
        ice::sonic::Status status;
        m_ops->unregister_extension(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list_extensions(const ice::sonic::TF_VectorOps& out_names) noexcept
    {
        ice::sonic::Status status;
        m_ops->list_extensions(get_handle(), out_names.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> reload_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept
    {
        ice::sonic::Status status;
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
};

} // namespace ice::sonic
