module;

#include "c/extern/database.h"

export module cc_abi_sonic:database;

import cc_abi_sonic_intern;

export namespace ice::sonic {

class DatabaseRuntime
{
public:
    DatabaseRuntime() :
        m_handle{nullptr}
    {
    }

    explicit DatabaseRuntime(TP_Database* handle) :
        m_handle{handle}
    {
    }

    TF_Bool invoke_is_connected() const
    {

        return (m_handle && m_handle->is_connected_cb) ? m_handle->is_connected_cb(m_handle->ext)
                                                       : 0;
    }

    void invoke_query(
        const TF_TString* payload, TF_Database_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->query_cb) {
            m_handle->query_cb(m_handle->ext, payload, completion, cb_user_data);
        }
    }

    void invoke_insert(
        const TF_TString* payload, TF_Database_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->insert_cb) {
            m_handle->insert_cb(m_handle->ext, payload, completion, cb_user_data);
        }
    }

    void invoke_update(
        const TF_TString* payload, TF_Database_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->update_cb) {
            m_handle->update_cb(m_handle->ext, payload, completion, cb_user_data);
        }
    }

    void invoke_remove(
        const TF_TString* payload, TF_Database_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->remove_cb) {
            m_handle->remove_cb(m_handle->ext, payload, completion, cb_user_data);
        }
    }

    StringRuntime get_name() const
    {
        return m_handle ? StringRuntime{&m_handle->backend_name} : StringRuntime{};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Database* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Database* m_handle;
};

} // namespace ice::sonic
