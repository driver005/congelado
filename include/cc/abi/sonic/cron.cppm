module;

#include "c/extern/cron.h"

export module cc_abi_sonic:cron;

import cc_abi_sonic_intern;

export namespace ice::sonic {

class CronRuntime
{
public:
    CronRuntime() :
        m_handle{nullptr}
    {
    }

    explicit CronRuntime(TP_Cron* handle) :
        m_handle{handle}
    {
    }

    TF_Bool invoke_required() const
    {

        return (m_handle && m_handle->required_cb) ? m_handle->required_cb(m_handle->ext) : 0;
    }

    TF_Bool invoke_validate(const TF_TString* expression) const
    {

        return (m_handle && m_handle->validate_cb)
                   ? m_handle->validate_cb(m_handle->ext, expression)
                   : 0;
    }

    TF_Bool invoke_next_after(
        const TF_TString* expression, int64_t base_time_ms, int64_t* out_time_ms
    ) const
    {

        return (m_handle && m_handle->next_after_cb)
                   ? m_handle->next_after_cb(m_handle->ext, expression, base_time_ms, out_time_ms)
                   : 0;
    }

    void invoke_upsert_job(const TF_TString* name, const TF_TString* expression) const
    {

        if (m_handle && m_handle->upsert_job_cb) {
            m_handle->upsert_job_cb(m_handle->ext, name, expression);
        }
    }

    void invoke_remove_job(const TF_TString* name) const
    {

        if (m_handle && m_handle->remove_job_cb) {
            m_handle->remove_job_cb(m_handle->ext, name);
        }
    }

    StringRuntime get_name() const
    {
        return m_handle ? StringRuntime{&m_handle->backend_name} : StringRuntime{};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Cron* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Cron* m_handle;
};

} // namespace ice::sonic
