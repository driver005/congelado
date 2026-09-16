// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/server/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/server/server.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_server;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Server
{
public:
    static Server* create(void* ctx) noexcept
    {
        return static_cast<Server*>(ctx);
    }

    template<typename HandleT>
    static Server* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Server*>(handle);
    }

    virtual ~Server() = default;
    [[nodiscard]] std::expected<void, ice::Status> get_bind_host(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_bind_port() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_tls_cert(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_tls_key(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> new_server() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> destroy_server() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_request_handler(TF_Server_RequestHandler handler, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_connect(TF_Server_ConnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    on_disconnect(TF_Server_DisconnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> start() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> stop() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> stop_accepting() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> resume_accepting() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_running() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_idle() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_max_connections(size_t max_connections) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_max_connections() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    find_connection(const ice::sonic::String& connection_id) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_connection_id(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    send_response(const ice::sonic::Response& response) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    broadcast(const ice::sonic::Response& response) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> close_connection() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_connections(const ice::sonic::Vector& out_connections) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_connection_count() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_stats(const ice::sonic::Map& out_stats) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    register_extension(const ice::sonic::String& name, const ice::sonic::Map& config) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    unregister_extension(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_extensions(const ice::sonic::Vector& out_names) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> reload_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Server* get_generic_vtable()
    {
        static TF_Server vtable = {
            .struct_size = TF_SERVER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Server::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Server::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_bind_host =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Server::create(plugin_context);
                auto res = self->get_bind_host(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_bind_port =
                [](void* plugin_context) noexcept
            {
                auto* self = Server::create(plugin_context);
                auto res = self->get_bind_port();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_tls_cert =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Server::create(plugin_context);
                auto res = self->get_tls_cert(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_tls_key =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Server::create(plugin_context);
                auto res = self->get_tls_key(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .new_server =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(plugin_context);
                auto res = self->new_server();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .destroy_server =
                [](TF_Server_Handle* server) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->destroy_server();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_request_handler =
                [](TF_Server_Handle* server,
                   TF_Server_RequestHandler handler,
                   void* user_data) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->set_request_handler(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_connect =
                [](TF_Server_Handle* server, TF_Server_ConnectFn handler, void* user_data) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->on_connect(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .on_disconnect =
                [](TF_Server_Handle* server,
                   TF_Server_DisconnectFn handler,
                   void* user_data) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->on_disconnect(handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .start =
                [](TF_Server_Handle* server, TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stop =
                [](TF_Server_Handle* server, TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .stop_accepting =
                [](TF_Server_Handle* server, TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->stop_accepting();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .resume_accepting =
                [](TF_Server_Handle* server, TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->resume_accepting();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_running =
                [](TF_Server_Handle* server) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->is_running();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_idle =
                [](TF_Server_Handle* server) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->is_idle();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_max_connections =
                [](TF_Server_Handle* server,
                   size_t max_connections,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->set_max_connections(max_connections);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_max_connections =
                [](TF_Server_Handle* server) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->get_max_connections();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .find_connection =
                [](TF_Server_Handle* server, const TF_String_Handle* connection_id) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->find_connection(ice::sonic::String::wrap(connection_id));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_connection_id =
                [](TF_Server_Connection* connection, TF_String* out) noexcept
            {
                auto* self = Server::create(connection);
                auto res = self->get_connection_id(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send_response =
                [](TF_Server_Connection* connection,
                   TF_Response_Handle* response,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(connection);
                auto res = self->send_response(ice::sonic::Response::wrap(response));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .broadcast =
                [](TF_Server_Handle* server,
                   TF_Response_Handle* response,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->broadcast(ice::sonic::Response::wrap(response));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .close_connection =
                [](TF_Server_Connection* connection, TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(connection);
                auto res = self->close_connection();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_connections =
                [](TF_Server_Handle* server,
                   TF_Vector_Handle* out_connections,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->list_connections(ice::sonic::Vector::wrap(out_connections));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_connection_count =
                [](TF_Server_Handle* server) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->get_connection_count();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_stats =
                [](TF_Server_Handle* server,
                   TF_Map_Handle* out_stats,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->get_stats(ice::sonic::Map::wrap(out_stats));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .register_extension =
                [](TF_Server_Handle* server,
                   const TF_String_Handle* name,
                   const TF_Map_Handle* config,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->register_extension(
                    ice::sonic::String::wrap(name),
                    ice::sonic::Map::wrap(config)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .unregister_extension =
                [](TF_Server_Handle* server,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->unregister_extension(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_extensions =
                [](TF_Server_Handle* server,
                   TF_Vector_Handle* out_names,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->list_extensions(ice::sonic::Vector::wrap(out_names));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reload_certificate =
                [](TF_Server_Handle* server,
                   const TF_String_Handle* cert_path,
                   const TF_String_Handle* key_path,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Server::create(server);
                auto res = self->reload_certificate(
                    ice::sonic::String::wrap(cert_path),
                    ice::sonic::String::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
