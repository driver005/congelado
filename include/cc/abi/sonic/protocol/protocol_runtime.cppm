module;

#include "c/extern/protocol.h"

export module cc_abi_sonic:protocol_runtime;

import cc_abi_sonic_intern;
import :protocol_server_runtime;

export namespace ice::sonic {

class ProtocolRuntime
{
public:
    ProtocolRuntime() :
        m_handle{nullptr}
    {
    }

    explicit ProtocolRuntime(TP_Protocol* handle) :
        m_handle{handle}
    {
    }

    ProtocolServerRuntime invoke_create_server(TF_Status* status) const
    {

        if (m_handle && m_handle->create_server_cb) {
            return ProtocolServerRuntime{m_handle->create_server_cb(m_handle->ext, status)};
        }
        return ProtocolServerRuntime{};
    }

    StringRuntime get_name() const
    {
        return m_handle ? StringRuntime{&m_handle->name} : StringRuntime{};
    }

    StringRuntime get_bind_host() const
    {
        return m_handle ? StringRuntime{&m_handle->bind_host} : StringRuntime{};
    }

    uint16_t get_bind_port() const
    {
        return m_handle ? m_handle->bind_port : 0;
    }

    StringRuntime get_tls_cert() const
    {
        return m_handle ? StringRuntime{&m_handle->tls_cert} : StringRuntime{};
    }

    StringRuntime get_tls_key() const
    {
        return m_handle ? StringRuntime{&m_handle->tls_key} : StringRuntime{};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Protocol* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Protocol* m_handle;
};

} // namespace ice::sonic
