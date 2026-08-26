module;

#include "c/extern/protocol.h"

export module cc_abi_sonic:protocol_server_runtime;

export namespace ice::sonic {

// Non-owning wrapper around a `TP_Protocol_Server*` returned by ProtocolRuntime's
// invoke_create_server(). Drives the started server's lifecycle from the mainframe side.
class ProtocolServerRuntime
{
public:
    ProtocolServerRuntime() :
        m_handle{nullptr}
    {
    }

    explicit ProtocolServerRuntime(TP_Protocol_Server* handle) :
        m_handle{handle}
    {
    }

    void invoke_start(TF_Status* status) const
    {

        if (m_handle && m_handle->start_cb) {
            m_handle->start_cb(m_handle->ext, status);
        }
    }

    void invoke_stop(TF_Status* status) const
    {

        if (m_handle && m_handle->stop_cb) {
            m_handle->stop_cb(m_handle->ext, status);
        }
    }

    TF_Bool invoke_is_running() const
    {

        return (m_handle && m_handle->is_running_cb) ? m_handle->is_running_cb(m_handle->ext) : 0;
    }

    // Underlying handle — pass directly to the C ABI
    TP_Protocol_Server* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Protocol_Server* m_handle;
};

} // namespace ice::sonic
