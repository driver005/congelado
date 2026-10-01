// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/client.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/client.h"
#include "include/c/extern/io/request.h"
#include "include/c/extern/io/response.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_io_builder:client;

import std;
import cc_ice_extern_io_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ClientOps
{
public:
    explicit TF_ClientOps(
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_RequestOps* TF_RequestOps_ops,
        const ::TF_ResponseOps* TF_ResponseOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_TF_RequestOps_ops = TF_RequestOps_ops;
        m_TF_ResponseOps_ops = TF_ResponseOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
    }

    TF_ClientOps(const TF_ClientOps&) = delete;
    TF_ClientOps& operator=(const TF_ClientOps&) = delete;

    static TF_ClientOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ClientOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ClientOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ClientOps*>(handle->plugin_data);
    }

    virtual ~TF_ClientOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void connect(int64_t timeout_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void connect_async(
        int64_t timeout_ms,
        TFClientConnectFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void disconnect(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void reconnect(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void on_disconnect(TFClientDisconnectFn handler, void* user_data) noexcept = 0;
    virtual void is_connected(int* out_connected) noexcept = 0;
    virtual void
    get_remote_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) noexcept = 0;
    virtual void set_keep_alive(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void is_keep_alive(int* out_keep_alive) noexcept = 0;
    virtual void create_request(
        uint32_t stream_id,
        const ice::sonic::TF_RequestOps& out_request,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void send(
        const ice::sonic::TF_RequestOps& request,
        const ice::sonic::TF_ResponseOps& out_response,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void send_async(
        const ice::sonic::TF_RequestOps& request,
        TFClientResponseFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    cancel_request(uint32_t stream_id, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void list_pending_requests(
        const ice::sonic::TF_VectorOps& out_stream_ids,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_pending_request_count(size_t* out_count) noexcept = 0;
    virtual void ping(
        TFClientConnectFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void retry(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_max_retries(int max_retries, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_retry_backoff(int64_t backoff_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void on_error(TFClientConnectFn handler, void* user_data) noexcept = 0;
    virtual void get_last_error(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_stats(
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Client*)) noexcept
    {
        m_vtable = ::TF_ClientOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ClientOps, get_stats),

            .create = create,
            .destroy =
                [](TF_Client* handle) noexcept
            {
                auto& self = TF_ClientOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Client* client, TF_String* out_name) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .connect =
                [](TF_Client* client, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.connect(
                    timeout_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .connect_async =
                [](TF_Client* client,
                   int64_t timeout_ms,
                   TFClientConnectFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.connect_async(
                    timeout_ms,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .disconnect =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.disconnect(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .reconnect =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.reconnect(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .on_disconnect =
                [](TF_Client* client, TFClientDisconnectFn handler, void* user_data) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.on_disconnect(handler, user_data);
            },
            .is_connected =
                [](TF_Client* client, int* out_connected) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.is_connected(out_connected);
            },
            .get_remote_endpoint =
                [](TF_Client* client, TF_String* out_host, uint16_t* out_port) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.get_remote_endpoint(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_host),
                    out_port
                );
            },
            .set_keep_alive =
                [](TF_Client* client, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.set_keep_alive(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .is_keep_alive =
                [](TF_Client* client, int* out_keep_alive) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.is_keep_alive(out_keep_alive);
            },
            .create_request =
                [](TF_Client* client,
                   uint32_t stream_id,
                   TF_Request* out_request,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.create_request(
                    stream_id,
                    self.wrap(std::type_identity<ice::sonic::TF_RequestOps>{}, out_request),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .send =
                [](TF_Client* client,
                   TF_Request* request,
                   TF_Response* out_response,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.send(
                    self.wrap(std::type_identity<ice::sonic::TF_RequestOps>{}, request),
                    self.wrap(std::type_identity<ice::sonic::TF_ResponseOps>{}, out_response),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .send_async =
                [](TF_Client* client,
                   TF_Request* request,
                   TFClientResponseFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.send_async(
                    self.wrap(std::type_identity<ice::sonic::TF_RequestOps>{}, request),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .cancel_request =
                [](TF_Client* client, uint32_t stream_id, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.cancel_request(
                    stream_id,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_pending_requests =
                [](TF_Client* client, TF_Vector* out_stream_ids, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.list_pending_requests(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_stream_ids),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_pending_request_count =
                [](TF_Client* client, size_t* out_count) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.get_pending_request_count(out_count);
            },
            .ping =
                [](TF_Client* client,
                   TFClientConnectFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.ping(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .retry =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.retry(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .set_max_retries =
                [](TF_Client* client, int max_retries, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.set_max_retries(
                    max_retries,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_retry_backoff =
                [](TF_Client* client, int64_t backoff_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.set_retry_backoff(
                    backoff_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_timeout =
                [](TF_Client* client, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.set_timeout(
                    timeout_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .on_error =
                [](TF_Client* client, TFClientConnectFn handler, void* user_data) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.on_error(handler, user_data);
            },
            .get_last_error =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.get_last_error(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stats =
                [](TF_Client* client, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto& self = TF_ClientOps::from_handle(client);
                self.get_stats(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_stats),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
    }

    ice::sonic::TF_RequestOps
    wrap(std::type_identity<ice::sonic::TF_RequestOps>, const ::TF_Request* handle) const noexcept
    {
        return ice::sonic::TF_RequestOps{m_TF_RequestOps_ops, const_cast<::TF_Request*>(handle)};
    }

    ice::sonic::TF_ResponseOps
    wrap(std::type_identity<ice::sonic::TF_ResponseOps>, const ::TF_Response* handle) const noexcept
    {
        return ice::sonic::TF_ResponseOps{m_TF_ResponseOps_ops, const_cast<::TF_Response*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TF_ClientOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Client& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ClientOps*>(&m_vtable));
    }

private:
    ::TF_ClientOps m_vtable;
    ::TF_Client m_handle;

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_RequestOps* m_TF_RequestOps_ops{nullptr};

    const ::TF_ResponseOps* m_TF_ResponseOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
