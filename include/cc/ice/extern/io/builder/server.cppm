// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/server.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/connection.h"
#include "include/c/extern/io/response.h"
#include "include/c/extern/io/server.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_io_builder:server;

import std;
import cc_ice_extern_io_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ServerOps
{
public:
    explicit TF_ServerOps(
        const ::TFServerConnectionOps* TFServerConnectionOps_ops,
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_ResponseOps* TF_ResponseOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFServerConnectionOps_ops = TFServerConnectionOps_ops;
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_TF_ResponseOps_ops = TF_ResponseOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
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
    virtual void start(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void stop(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void stop_accepting(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void resume_accepting(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void is_running(int* out_running) noexcept = 0;
    virtual void is_idle(int* out_idle) noexcept = 0;
    virtual void
    set_max_connections(size_t max_connections, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_max_connections(size_t* out_max_connections) noexcept = 0;
    virtual void find_connection(
        const ice::sonic::String& connection_id,
        const ice::sonic::TFServerConnectionOps& out_connection,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void broadcast(
        const ice::sonic::TF_ResponseOps& response,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void list_connections(
        const ice::sonic::TF_VectorOps& out_connections,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_connection_count(size_t* out_count) noexcept = 0;
    virtual void get_stats(
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void register_extension(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void unregister_extension(
        const ice::sonic::String& name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void list_extensions(
        const ice::sonic::TF_VectorOps& out_names,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void reload_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Server*)) noexcept
    {
        m_vtable = ::TF_ServerOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ServerOps, reload_certificate),

            .create = create,
            .destroy =
                [](TF_Server* handle) noexcept
            {
                auto& self = TF_ServerOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Server* server, TF_String* out_name) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .get_bind_host =
                [](TF_Server* server, TF_String* out_host) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_bind_host(self.wrap(std::type_identity<ice::sonic::String>{}, out_host));
            },
            .get_bind_port =
                [](TF_Server* server, uint16_t* out_port) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_bind_port(out_port);
            },
            .get_tls_cert =
                [](TF_Server* server, TF_String* out_cert) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_tls_cert(self.wrap(std::type_identity<ice::sonic::String>{}, out_cert));
            },
            .get_tls_key =
                [](TF_Server* server, TF_String* out_key) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_tls_key(self.wrap(std::type_identity<ice::sonic::String>{}, out_key));
            },
            .set_request_handler =
                [](TF_Server* server, TFServerRequestHandler handler, void* user_data) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.set_request_handler(handler, user_data);
            },
            .on_connect =
                [](TF_Server* server, TFServerConnectFn handler, void* user_data) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.on_connect(handler, user_data);
            },
            .on_disconnect =
                [](TF_Server* server, TFServerDisconnectFn handler, void* user_data) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.on_disconnect(handler, user_data);
            },
            .start =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.start(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .stop =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.stop(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .stop_accepting =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.stop_accepting(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .resume_accepting =
                [](TF_Server* server, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.resume_accepting(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .is_running =
                [](TF_Server* server, int* out_running) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.is_running(out_running);
            },
            .is_idle =
                [](TF_Server* server, int* out_idle) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.is_idle(out_idle);
            },
            .set_max_connections =
                [](TF_Server* server, size_t max_connections, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.set_max_connections(
                    max_connections,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_max_connections =
                [](TF_Server* server, size_t* out_max_connections) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_max_connections(out_max_connections);
            },
            .find_connection =
                [](TF_Server* server,
                   const TF_String* connection_id,
                   TFServerConnection* out_connection,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.find_connection(
                    self.wrap(std::type_identity<ice::sonic::String>{}, connection_id),
                    self.wrap(
                        std::type_identity<ice::sonic::TFServerConnectionOps>{},
                        out_connection
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .broadcast =
                [](TF_Server* server, TF_Response* response, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.broadcast(
                    self.wrap(std::type_identity<ice::sonic::TF_ResponseOps>{}, response),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_connections =
                [](TF_Server* server, TF_Vector* out_connections, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.list_connections(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_connections),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_connection_count =
                [](TF_Server* server, size_t* out_count) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_connection_count(out_count);
            },
            .get_stats =
                [](TF_Server* server, TF_Map* out_stats, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.get_stats(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_stats),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .register_extension =
                [](TF_Server* server,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.register_extension(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .unregister_extension =
                [](TF_Server* server, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.unregister_extension(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_extensions =
                [](TF_Server* server, TF_Vector* out_names, TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.list_extensions(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_names),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .reload_certificate =
                [](TF_Server* server,
                   const TF_String* cert_path,
                   const TF_String* key_path,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ServerOps::from_handle(server);
                self.reload_certificate(
                    self.wrap(std::type_identity<ice::sonic::String>{}, cert_path),
                    self.wrap(std::type_identity<ice::sonic::String>{}, key_path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFServerConnectionOps wrap(
        std::type_identity<ice::sonic::TFServerConnectionOps>,
        const ::TFServerConnection* handle
    ) const noexcept
    {
        return ice::sonic::TFServerConnectionOps{
            m_TFServerConnectionOps_ops,
            const_cast<::TFServerConnection*>(handle)
        };
    }

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
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

    const ::TF_ServerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Server& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_ServerOps*>(&m_vtable)
        );
    }

private:
    ::TF_ServerOps m_vtable;
    ::TF_Server m_handle;

    const ::TFServerConnectionOps* m_TFServerConnectionOps_ops{nullptr};

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_ResponseOps* m_TF_ResponseOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
