// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/server.h"

export module cc_ice_extern_io_builder:server;

import std;

export namespace ice::builder {

class TF_ServerOps
{
public:
    TF_ServerOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ServerOps(const TF_ServerOps&) = delete;
    TF_ServerOps& operator=(const TF_ServerOps&) = delete;

    static TF_ServerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ServerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ServerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ServerOps*>(handle->plugin_data);
    }

    virtual ~TF_ServerOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_bind_host(const ice::sonic::String& out_host) noexcept = 0;
    virtual void get_bind_port(uint16_t* out_port) noexcept = 0;
    virtual void get_tls_cert(const ice::sonic::String& out_cert) noexcept = 0;
    virtual void get_tls_key(const ice::sonic::String& out_key) noexcept = 0;
    virtual void set_request_handler(TFServerRequestHandler handler, void* user_data) noexcept = 0;
    virtual void on_connect(TFServerConnectFn handler, void* user_data) noexcept = 0;
    virtual void on_disconnect(TFServerDisconnectFn handler, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> start() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> stop() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> stop_accepting() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> resume_accepting() noexcept = 0;
    virtual void is_running(int* out_running) noexcept = 0;
    virtual void is_idle(int* out_idle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_max_connections(size_t max_connections) noexcept = 0;
    virtual void get_max_connections(size_t* out_max_connections) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> find_connection(
        const ice::sonic::String& connection_id,
        TFServerConnection* out_connection
    ) noexcept = 0;
    virtual void get_connection_id(const ice::sonic::String& out_connection_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    send_response(const ice::sonic::TF_ResponseOps& response) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    broadcast(const ice::sonic::TF_ResponseOps& response) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> close_connection() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_connections(const ice::sonic::TF_VectorOps& out_connections) noexcept = 0;
    virtual void get_connection_count(size_t* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> register_extension(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    unregister_extension(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_extensions(const ice::sonic::TF_VectorOps& out_names) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> reload_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ServerOps{
            .struct_size = TF_SERVER_STRUCT_SIZE,
            .destroy =
                [](TF_Server* server) noexcept
            {
                TF_ServerOps::from_handle(server).destroy();
            },
            .get_name =
                [](TF_Server* server, TF_String* out_name) noexcept
            {
                TF_ServerOps::from_handle(server).get_name(ice::sonic::String::wrap(out_name));
            },
            .get_bind_host =
                [](TF_Server* server, TF_String* out_host) noexcept
            {
                TF_ServerOps::from_handle(server).get_bind_host(ice::sonic::String::wrap(out_host));
            },
            .get_bind_port =
                [](TF_Server* server, uint16_t* out_port) noexcept
            {
                TF_ServerOps::from_handle(server).get_bind_port(out_port);
            },
            .get_tls_cert =
                [](TF_Server* server, TF_String* out_cert) noexcept
            {
                TF_ServerOps::from_handle(server).get_tls_cert(ice::sonic::String::wrap(out_cert));
            },
            .get_tls_key =
                [](TF_Server* server, TF_String* out_key) noexcept
            {
                TF_ServerOps::from_handle(server).get_tls_key(ice::sonic::String::wrap(out_key));
            },
            .set_request_handler =
                [](TF_Server* server, TFServerRequestHandler handler, void* user_data) noexcept
            {
                TF_ServerOps::from_handle(server).set_request_handler(handler, user_data);
            },
            .on_connect =
                [](TF_Server* server, TFServerConnectFn handler, void* user_data) noexcept
            {
                TF_ServerOps::from_handle(server).on_connect(handler, user_data);
            },
            .on_disconnect =
                [](TF_Server* server, TFServerDisconnectFn handler, void* user_data) noexcept
            {
                TF_ServerOps::from_handle(server).on_disconnect(handler, user_data);
            },
            .start =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).start();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).stop();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .stop_accepting =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).stop_accepting();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .resume_accepting =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).resume_accepting();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_running =
                [](TF_Server* server, int* out_running) noexcept
            {
                TF_ServerOps::from_handle(server).is_running(out_running);
            },
            .is_idle =
                [](TF_Server* server, int* out_idle) noexcept
            {
                TF_ServerOps::from_handle(server).is_idle(out_idle);
            },
            .set_max_connections =
                [](TF_Server* server, size_t max_connections, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).set_max_connections(max_connections);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_max_connections =
                [](TF_Server* server, size_t* out_max_connections) noexcept
            {
                TF_ServerOps::from_handle(server).get_max_connections(out_max_connections);
            },
            .find_connection =
                [](TF_Server* server,
                   const TF_String* connection_id,
                   TFServerConnection* out_connection,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).find_connection(
                    ice::sonic::String::wrap(connection_id),
                    out_connection
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_connection_id =
                [](TFServerConnection* connection, TF_String* out_connection_id) noexcept
            {
                TF_ServerOps::from_handle(connection)
                    .get_connection_id(ice::sonic::String::wrap(out_connection_id));
            },
            .send_response =
                [](TFServerConnection* connection,
                   TF_Response* response,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(connection)
                               .send_response(ice::sonic::TF_ResponseOps::wrap(response));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .broadcast =
                [](TF_Server* server, TF_Response* response, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).broadcast(
                    ice::sonic::TF_ResponseOps::wrap(response)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .close_connection =
                [](TFServerConnection* connection, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(connection).close_connection();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_connections =
                [](TF_Server* server, TF_Vector* out_connections, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).list_connections(
                    ice::sonic::TF_VectorOps::wrap(out_connections)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_connection_count =
                [](TF_Server* server, size_t* out_count) noexcept
            {
                TF_ServerOps::from_handle(server).get_connection_count(out_count);
            },
            .get_stats =
                [](TF_Server* server, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).get_stats(
                    ice::sonic::TF_MapOps::wrap(out_stats)
                );
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
                auto res = TF_ServerOps::from_handle(server).register_extension(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TF_MapOps::wrap(config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .unregister_extension =
                [](TF_Server* server, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).unregister_extension(
                    ice::sonic::String::wrap(name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_extensions =
                [](TF_Server* server, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto res = TF_ServerOps::from_handle(server).list_extensions(
                    ice::sonic::TF_VectorOps::wrap(out_names)
                );
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
                auto res = TF_ServerOps::from_handle(server).reload_certificate(
                    ice::sonic::String::wrap(cert_path),
                    ice::sonic::String::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_ServerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Server& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ServerOps m_vtable;
    TF_Server m_handle;
};

} // namespace ice::builder
