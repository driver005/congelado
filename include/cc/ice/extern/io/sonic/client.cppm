// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/client.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/client.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_io_sonic:client;

import std;
import :request;
import :response;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ClientOps : public ice::sonic::Runtime<::TF_ClientOps, ::TF_Client>
{
public:
    TF_ClientOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_ClientOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Client* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_ClientOps(const ::TF_ClientOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ClientOps(const ::TF_ClientOps* ops, ::TF_Client* handle) noexcept :
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

    void connect(int64_t timeout_ms, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->connect(get_handle(), timeout_ms, out_status.get_handle());
    }

    void connect_async(
        int64_t timeout_ms,
        TFClientConnectFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->connect_async(
            get_handle(),
            timeout_ms,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void disconnect(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->disconnect(get_handle(), out_status.get_handle());
    }

    void reconnect(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->reconnect(get_handle(), out_status.get_handle());
    }

    void on_disconnect(TFClientDisconnectFn handler, void* user_data) const noexcept
    {
        m_ops->on_disconnect(get_handle(), handler, user_data);
    }

    void is_connected(int* out_connected) const noexcept
    {
        m_ops->is_connected(get_handle(), out_connected);
    }

    void get_remote_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) const noexcept
    {
        m_ops->get_remote_endpoint(get_handle(), out_host.get_handle(), out_port);
    }

    void set_keep_alive(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_keep_alive(get_handle(), enabled, out_status.get_handle());
    }

    void is_keep_alive(int* out_keep_alive) const noexcept
    {
        m_ops->is_keep_alive(get_handle(), out_keep_alive);
    }

    void create_request(
        uint32_t stream_id,
        const ice::sonic::TF_RequestOps& out_request,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_request(
            get_handle(),
            stream_id,
            out_request.get_handle(),
            out_status.get_handle()
        );
    }

    void send(
        const ice::sonic::TF_RequestOps& request,
        const ice::sonic::TF_ResponseOps& out_response,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->send(
            get_handle(),
            request.get_handle(),
            out_response.get_handle(),
            out_status.get_handle()
        );
    }

    void send_async(
        const ice::sonic::TF_RequestOps& request,
        TFClientResponseFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->send_async(
            get_handle(),
            request.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void cancel_request(uint32_t stream_id, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->cancel_request(get_handle(), stream_id, out_status.get_handle());
    }

    void list_pending_requests(
        const ice::sonic::TF_VectorOps& out_stream_ids,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_pending_requests(
            get_handle(),
            out_stream_ids.get_handle(),
            out_status.get_handle()
        );
    }

    void get_pending_request_count(size_t* out_count) const noexcept
    {
        m_ops->get_pending_request_count(get_handle(), out_count);
    }

    void ping(
        TFClientConnectFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->ping(get_handle(), completion, user_data, out_status.get_handle());
    }

    void retry(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->retry(get_handle(), out_status.get_handle());
    }

    void set_max_retries(int max_retries, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_max_retries(get_handle(), max_retries, out_status.get_handle());
    }

    void set_retry_backoff(int64_t backoff_ms, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_retry_backoff(get_handle(), backoff_ms, out_status.get_handle());
    }

    void set_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_timeout(get_handle(), timeout_ms, out_status.get_handle());
    }

    void on_error(TFClientConnectFn handler, void* user_data) const noexcept
    {
        m_ops->on_error(get_handle(), handler, user_data);
    }

    void get_last_error(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_last_error(get_handle(), out_status.get_handle());
    }

    void get_stats(
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_stats(get_handle(), out_stats.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
