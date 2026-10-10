// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/connection.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/connection.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_io_sonic:connection;

import std;
import :response;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFServerConnectionOps :
    public ice::sonic::Runtime<::TFServerConnectionOps, ::TFServerConnection>
{
public:
    TFServerConnectionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFServerConnectionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFServerConnection* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFServerConnectionOps(const ::TFServerConnectionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFServerConnectionOps(const ::TFServerConnectionOps* ops, ::TFServerConnection* handle) noexcept
        :
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

    void get_connection_id(const ice::sonic::String& out_connection_id) const noexcept
    {
        m_ops->get_connection_id(get_handle(), out_connection_id.get_handle());
    }

    void send_response(
        const ice::sonic::TF_ResponseOps& response,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->send_response(get_handle(), response.get_handle(), out_status.get_handle());
    }

    void close_connection(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->close_connection(get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
