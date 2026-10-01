// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/connection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/connection.h"
#include "include/c/extern/io/response.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_io_builder:connection;

import std;
import cc_ice_extern_io_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFServerConnectionOps
{
public:
    explicit TFServerConnectionOps(
        const ::TF_ResponseOps* TF_ResponseOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_ResponseOps_ops = TF_ResponseOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFServerConnectionOps(const TFServerConnectionOps&) = delete;
    TFServerConnectionOps& operator=(const TFServerConnectionOps&) = delete;

    static TFServerConnectionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFServerConnectionOps*>(ctx);
    }

    template<typename HandleT>
    static TFServerConnectionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFServerConnectionOps*>(handle->plugin_data);
    }

    virtual ~TFServerConnectionOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_connection_id(const ice::sonic::String& out_connection_id) noexcept = 0;
    virtual void send_response(
        const ice::sonic::TF_ResponseOps& response,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void close_connection(const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFServerConnection*)) noexcept
    {
        m_vtable = ::TFServerConnectionOps{
            .struct_size = TF_OFFSET_OF_END(::TFServerConnectionOps, close_connection),

            .create = create,
            .destroy =
                [](TFServerConnection* handle) noexcept
            {
                auto& self = TFServerConnectionOps::from_handle(handle);
                self.destroy();
            },
            .get_connection_id =
                [](TFServerConnection* connection, TF_String* out_connection_id) noexcept
            {
                auto& self = TFServerConnectionOps::from_handle(connection);
                self.get_connection_id(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_connection_id)
                );
            },
            .send_response =
                [](TFServerConnection* connection,
                   TF_Response* response,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFServerConnectionOps::from_handle(connection);
                self.send_response(
                    self.wrap(std::type_identity<ice::sonic::TF_ResponseOps>{}, response),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .close_connection =
                [](TFServerConnection* connection, TF_Status* out_status) noexcept
            {
                auto& self = TFServerConnectionOps::from_handle(connection);
                self.close_connection(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
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

    const ::TFServerConnectionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFServerConnection& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TFServerConnectionOps*>(&m_vtable));
    }

private:
    ::TFServerConnectionOps m_vtable;
    ::TFServerConnection m_handle;

    const ::TF_ResponseOps* m_TF_ResponseOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
