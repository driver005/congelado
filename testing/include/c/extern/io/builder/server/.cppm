// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/server.h"

export module cc_abi_builder_io;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_ServerOps
{
public:
    static TF_ServerOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ServerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ServerOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ServerOps*>(handle->plugin_data);
    }

    virtual ~TF_ServerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_bind_host(const ice::sonic::TF_StringOps& out_host) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_bind_port(uint16_t* out_port) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_tls_cert(const ice::sonic::TF_StringOps& out_cert) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_tls_key(const ice::sonic::TF_StringOps& out_key) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_request_handler(TFServerRequestHandler handler, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_connect(TFServerConnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    on_disconnect(TFServerDisconnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> start() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stop() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> stop_accepting() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> resume_accepting() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_running(int* out_running) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> is_idle(int* out_idle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_max_connections(size_t max_connections) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_max_connections(size_t* out_max_connections) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> find_connection(
        const ice::sonic::TF_StringOps& connection_id,
        TFServerConnection* out_connection
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_connection_id(const ice::sonic::TF_StringOps& out_connection_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    send_response(const ice::sonic::TF_ResponseOps& response) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    broadcast(const ice::sonic::TF_ResponseOps& response) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> close_connection() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_connections(const ice::sonic::TF_VectorOps& out_connections) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_connection_count(size_t* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_extension(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_MapOps& config
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    unregister_extension(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_extensions(const ice::sonic::TF_VectorOps& out_names) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> reload_certificate(
        const ice::sonic::TF_StringOps& cert_path,
        const ice::sonic::TF_StringOps& key_path
    ) noexcept = 0;

    static TF_ServerOps* get_generic_vtable()
    {
        static TF_ServerOps vtable = {
            .struct_size = TF_SERVER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ServerOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_ServerOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_bind_host =
                [](TF_Server* server, TF_String* out_host) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_bind_host(ice::sonic::TF_StringOps::wrap(out_host));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_bind_port =
                [](TF_Server* server, uint16_t* out_port) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_bind_port(out_port);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_tls_cert =
                [](TF_Server* server, TF_String* out_cert) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_tls_cert(ice::sonic::TF_StringOps::wrap(out_cert));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_tls_key =
                [](TF_Server* server, TF_String* out_key) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_tls_key(ice::sonic::TF_StringOps::wrap(out_key));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_request_handler =
                [](TF_Server* server, TFServerRequestHandler handler, void* user_data) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->set_request_handler(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_connect =
                [](TF_Server* server, TFServerConnectFn handler, void* user_data) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->on_connect(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_disconnect =
                [](TF_Server* server, TFServerDisconnectFn handler, void* user_data) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->on_disconnect(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop_accepting =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->stop_accepting();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .resume_accepting =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->resume_accepting();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_running =
                [](TF_Server* server, int* out_running) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->is_running(out_running);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_idle =
                [](TF_Server* server, int* out_idle) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->is_idle(out_idle);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_max_connections =
                [](TF_Server* server, size_t max_connections, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->set_max_connections(max_connections);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_max_connections =
                [](TF_Server* server, size_t* out_max_connections) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_max_connections(out_max_connections);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .find_connection =
                [](TF_Server* server,
                   const TF_String* connection_id,
                   TFServerConnection* out_connection,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->find_connection(
                    ice::sonic::TF_StringOps::wrap(connection_id),
                    out_connection
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_connection_id =
                [](TFServerConnection* connection, TF_String* out_connection_id) noexcept
            {
                auto* self = TF_ServerOps::create(connection);
                auto res =
                    self->get_connection_id(ice::sonic::TF_StringOps::wrap(out_connection_id));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send_response =
                [](TFServerConnection* connection,
                   TF_Response* response,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(connection);
                auto res = self->send_response(ice::sonic::TF_ResponseOps::wrap(response));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .broadcast =
                [](TF_Server* server, TF_Response* response, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->broadcast(ice::sonic::TF_ResponseOps::wrap(response));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .close_connection =
                [](TFServerConnection* connection, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(connection);
                auto res = self->close_connection();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_connections =
                [](TF_Server* server, TF_Vector* out_connections, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->list_connections(ice::sonic::TF_VectorOps::wrap(out_connections));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_connection_count =
                [](TF_Server* server, size_t* out_count) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_connection_count(out_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_stats =
                [](TF_Server* server, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->get_stats(ice::sonic::TF_MapOps::wrap(out_stats));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .register_extension =
                [](TF_Server* server,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->register_extension(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_MapOps::wrap(config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .unregister_extension =
                [](TF_Server* server, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->unregister_extension(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_extensions =
                [](TF_Server* server, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->list_extensions(ice::sonic::TF_VectorOps::wrap(out_names));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .reload_certificate =
                [](TF_Server* server,
                   const TF_String* cert_path,
                   const TF_String* key_path,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ServerOps::create(server);
                auto res = self->reload_certificate(
                    ice::sonic::TF_StringOps::wrap(cert_path),
                    ice::sonic::TF_StringOps::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
