// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/client.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/client.h"

export module cc_ice_extern_io_builder:client;

import std;

export namespace ice::builder {

class TF_ClientOps
{
public:
    TF_ClientOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TFClientConnectFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> disconnect() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> reconnect() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_disconnect(TFClientDisconnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_connected(int* out_connected) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_remote_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_keep_alive(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_keep_alive(int* out_keep_alive) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_request(uint32_t stream_id, const ice::sonic::TF_RequestOps& out_request) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> send(
        const ice::sonic::TF_RequestOps& request,
        const ice::sonic::TF_ResponseOps& out_response
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> send_async(
        const ice::sonic::TF_RequestOps& request,
        TFClientResponseFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    cancel_request(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_pending_requests(const ice::sonic::TF_VectorOps& out_stream_ids) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_pending_request_count(size_t* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    ping(TFClientConnectFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> retry() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_max_retries(int max_retries) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_retry_backoff(int64_t backoff_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_error(TFClientConnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_last_error() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ClientOps{
            .struct_size = TF_CLIENT_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_ClientOps>{&TF_ClientOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_ClientOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .connect =
                [](TF_Client* client, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).connect(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .connect_async =
                [](TF_Client* client,
                   int64_t timeout_ms,
                   TFClientConnectFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client)
                               .connect_async(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .disconnect =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).disconnect();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .reconnect =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).reconnect();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .on_disconnect =
                [](TF_Client* client, TFClientDisconnectFn handler, void* user_data) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).on_disconnect(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_connected =
                [](TF_Client* client, int* out_connected) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).is_connected(out_connected);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_remote_endpoint =
                [](TF_Client* client, TF_String* out_host, uint16_t* out_port) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).get_remote_endpoint(
                    ice::sonic::String::wrap(out_host),
                    out_port
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_keep_alive =
                [](TF_Client* client, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).set_keep_alive(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_keep_alive =
                [](TF_Client* client, int* out_keep_alive) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).is_keep_alive(out_keep_alive);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_request =
                [](TF_Client* client,
                   uint32_t stream_id,
                   TF_Request* out_request,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).create_request(
                    stream_id,
                    ice::sonic::TF_RequestOps::wrap(out_request)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .send =
                [](TF_Client* client,
                   TF_Request* request,
                   TF_Response* out_response,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).send(
                    ice::sonic::TF_RequestOps::wrap(request),
                    ice::sonic::TF_ResponseOps::wrap(out_response)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .send_async =
                [](TF_Client* client,
                   TF_Request* request,
                   TFClientResponseFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).send_async(
                    ice::sonic::TF_RequestOps::wrap(request),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .cancel_request =
                [](TF_Client* client, uint32_t stream_id, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).cancel_request(stream_id);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_pending_requests =
                [](TF_Client* client, TF_Vector* out_stream_ids, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).list_pending_requests(
                    ice::sonic::TF_VectorOps::wrap(out_stream_ids)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_pending_request_count =
                [](TF_Client* client, size_t* out_count) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).get_pending_request_count(out_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .ping =
                [](TF_Client* client,
                   TFClientConnectFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).ping(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .retry =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).retry();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_max_retries =
                [](TF_Client* client, int max_retries, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).set_max_retries(max_retries);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_retry_backoff =
                [](TF_Client* client, int64_t backoff_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).set_retry_backoff(backoff_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_timeout =
                [](TF_Client* client, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).set_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .on_error =
                [](TF_Client* client, TFClientConnectFn handler, void* user_data) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).on_error(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_last_error =
                [](TF_Client* client, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).get_last_error();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TF_Client* client, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto res = TF_ClientOps::from_handle(client).get_stats(
                    ice::sonic::TF_MapOps::wrap(out_stats)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_ClientOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Client& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ClientOps m_vtable;
    TF_Client m_handle;
};

} // namespace ice::builder
