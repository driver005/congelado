// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/client/client.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/client/client.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_client;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Client
{
public:
    static Client* create(void* ctx) noexcept
    {
        return static_cast<Client*>(ctx);
    }

    template<typename HandleT>
    static Client* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Client*>(handle);
    }

    virtual ~Client() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_client(const ice::sonic::String& host, uint16_t port) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> destroy_client() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TF_Client_ConnectFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> disconnect() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> reconnect() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_disconnect(TF_Client_DisconnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_connected() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_remote_endpoint(TF_String* out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_keep_alive(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_keep_alive() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> create_request(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    send(const ice::sonic::Request& request, const ice::sonic::Response& out_response) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> send_async(
        const ice::sonic::Request& request,
        TF_Client_ResponseFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> cancel_request(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_pending_requests(const ice::sonic::Vector& out_stream_ids) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_pending_request_count() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    ping(TF_Client_ConnectFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> retry() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_max_retries(int max_retries) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_retry_backoff(int64_t backoff_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_error(TF_Client_ConnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_last_error() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_stats(const ice::sonic::Map& out_stats) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Client* get_generic_vtable()
    {
        static TF_Client vtable = {
            .struct_size = TF_CLIENT_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Client::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Client::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .new_client =
                [](void* plugin_context,
                   const TF_String_Handle* host,
                   uint16_t port,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(plugin_context);
                auto res = self->new_client(ice::sonic::String::wrap(host), port);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .destroy_client =
                [](TF_Client_Handle* client) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->destroy_client();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .connect =
                [](TF_Client_Handle* client, int64_t timeout_ms, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->connect(timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .connect_async =
                [](TF_Client_Handle* client,
                   int64_t timeout_ms,
                   TF_Client_ConnectFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->connect_async(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .disconnect =
                [](TF_Client_Handle* client, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->disconnect();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reconnect =
                [](TF_Client_Handle* client, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->reconnect();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_disconnect =
                [](TF_Client_Handle* client,
                   TF_Client_DisconnectFn handler,
                   void* user_data) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->on_disconnect(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_connected =
                [](TF_Client_Handle* client) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->is_connected();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_remote_endpoint =
                [](TF_Client_Handle* client, TF_String* out_host, uint16_t* out_port) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->get_remote_endpoint(out_host, out_port);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_keep_alive =
                [](TF_Client_Handle* client, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->set_keep_alive(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_keep_alive =
                [](TF_Client_Handle* client) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->is_keep_alive();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_request =
                [](TF_Client_Handle* client, uint32_t stream_id, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->create_request(stream_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send =
                [](TF_Client_Handle* client,
                   TF_Request_Handle* request,
                   TF_Response_Handle* out_response,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->send(
                    ice::sonic::Request::wrap(request),
                    ice::sonic::Response::wrap(out_response)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send_async =
                [](TF_Client_Handle* client,
                   TF_Request_Handle* request,
                   TF_Client_ResponseFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res =
                    self->send_async(ice::sonic::Request::wrap(request), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .cancel_request =
                [](TF_Client_Handle* client, uint32_t stream_id, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->cancel_request(stream_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_pending_requests =
                [](TF_Client_Handle* client,
                   TF_Vector_Handle* out_stream_ids,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->list_pending_requests(ice::sonic::Vector::wrap(out_stream_ids));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_pending_request_count =
                [](TF_Client_Handle* client) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->get_pending_request_count();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .ping =
                [](TF_Client_Handle* client,
                   TF_Client_ConnectFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->ping(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .retry =
                [](TF_Client_Handle* client, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->retry();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_max_retries =
                [](TF_Client_Handle* client, int max_retries, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->set_max_retries(max_retries);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_retry_backoff =
                [](TF_Client_Handle* client, int64_t backoff_ms, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->set_retry_backoff(backoff_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_timeout =
                [](TF_Client_Handle* client, int64_t timeout_ms, TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->set_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_error =
                [](TF_Client_Handle* client, TF_Client_ConnectFn handler, void* user_data) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->on_error(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_last_error =
                [](TF_Client_Handle* client, TF_Status_Handle* out_status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->get_last_error();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TF_Client_Handle* client,
                   TF_Map_Handle* out_stats,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Client::create(client);
                auto res = self->get_stats(ice::sonic::Map::wrap(out_stats));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
