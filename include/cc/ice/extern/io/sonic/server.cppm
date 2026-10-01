// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/server.h"

export module cc_ice_extern_io_sonic:server;

import std;
import :connection;
import :response;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ServerOps : public ice::sonic::Runtime<::TF_ServerOps, ::TF_Server>
{
public:
    template<typename Registry>
    TF_ServerOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ServerOps(
        Registry& registry,
        ::TF_Server* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ServerOps(const ::TF_ServerOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ServerOps(const ::TF_ServerOps* ops, ::TF_Server* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void get_bind_host(const ice::sonic::String& out_host) const noexcept
    {
        m_ops->get_bind_host(get_handle(), out_host.get_handle());
    }

    void get_bind_port(uint16_t* out_port) const noexcept
    {
        m_ops->get_bind_port(get_handle(), out_port);
    }

    void get_tls_cert(const ice::sonic::String& out_cert) const noexcept
    {
        m_ops->get_tls_cert(get_handle(), out_cert.get_handle());
    }

    void get_tls_key(const ice::sonic::String& out_key) const noexcept
    {
        m_ops->get_tls_key(get_handle(), out_key.get_handle());
    }

    void set_request_handler(TFServerRequestHandler handler, void* user_data) const noexcept
    {
        m_ops->set_request_handler(get_handle(), handler, user_data);
    }

    void on_connect(TFServerConnectFn handler, void* user_data) const noexcept
    {
        m_ops->on_connect(get_handle(), handler, user_data);
    }

    void on_disconnect(TFServerDisconnectFn handler, void* user_data) const noexcept
    {
        m_ops->on_disconnect(get_handle(), handler, user_data);
    }

    void start(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->start(get_handle(), out_status.get_handle());
    }

    void stop(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->stop(get_handle(), out_status.get_handle());
    }

    void stop_accepting(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->stop_accepting(get_handle(), out_status.get_handle());
    }

    void resume_accepting(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->resume_accepting(get_handle(), out_status.get_handle());
    }

    void is_running(int* out_running) const noexcept
    {
        m_ops->is_running(get_handle(), out_running);
    }

    void is_idle(int* out_idle) const noexcept
    {
        m_ops->is_idle(get_handle(), out_idle);
    }

    void set_max_connections(
        size_t max_connections,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_max_connections(get_handle(), max_connections, out_status.get_handle());
    }

    void get_max_connections(size_t* out_max_connections) const noexcept
    {
        m_ops->get_max_connections(get_handle(), out_max_connections);
    }

    void find_connection(
        const ice::sonic::String& connection_id,
        const ice::sonic::TFServerConnectionOps& out_connection,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->find_connection(
            get_handle(),
            connection_id.get_handle(),
            out_connection.get_handle(),
            out_status.get_handle()
        );
    }

    void broadcast(
        const ice::sonic::TF_ResponseOps& response,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->broadcast(get_handle(), response.get_handle(), out_status.get_handle());
    }

    void list_connections(
        const ice::sonic::TF_VectorOps& out_connections,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->list_connections(get_handle(), out_connections.get_handle(), out_status.get_handle());
    }

    void get_connection_count(size_t* out_count) const noexcept
    {
        m_ops->get_connection_count(get_handle(), out_count);
    }

    void get_stats(
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_stats(get_handle(), out_stats.get_handle(), out_status.get_handle());
    }

    void register_extension(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->register_extension(
            get_handle(),
            name.get_handle(),
            config.get_handle(),
            out_status.get_handle()
        );
    }

    void unregister_extension(
        const ice::sonic::String& name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->unregister_extension(get_handle(), name.get_handle(), out_status.get_handle());
    }

    void list_extensions(
        const ice::sonic::TF_VectorOps& out_names,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_extensions(get_handle(), out_names.get_handle(), out_status.get_handle());
    }

    void reload_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->reload_certificate(
            get_handle(),
            cert_path.get_handle(),
            key_path.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
