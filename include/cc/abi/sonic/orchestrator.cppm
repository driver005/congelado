module;

#include "c/extern/orchestrator.h"

export module cc_abi_sonic:worker_orchestrator;

import cc_abi_sonic_intern;

export namespace ice::sonic {

class OrchestratorRuntime
{
public:
    OrchestratorRuntime() :
        m_handle{nullptr}
    {
    }

    explicit OrchestratorRuntime(TP_Orchestrator* handle) :
        m_handle{handle}
    {
    }

    TF_Bool invoke_required() const
    {

        return (m_handle && m_handle->required_cb) ? m_handle->required_cb(m_handle->ext) : 0;
    }

    void invoke_start_workflow(
        const TF_TString* def_name,
        const TF_TString* variables_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->start_workflow_cb) {
            m_handle->start_workflow_cb(
                m_handle->ext, def_name, variables_json, completion, cb_user_data
            );
        }
    }

    void invoke_pause(
        const TF_TString* exec_id, TF_Worker_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->pause_cb) {
            m_handle->pause_cb(m_handle->ext, exec_id, completion, cb_user_data);
        }
    }

    void invoke_resume(
        const TF_TString* exec_id, TF_Worker_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->resume_cb) {
            m_handle->resume_cb(m_handle->ext, exec_id, completion, cb_user_data);
        }
    }

    void invoke_terminate(
        const TF_TString* exec_id, TF_Worker_CompletionFn completion, void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->terminate_cb) {
            m_handle->terminate_cb(m_handle->ext, exec_id, completion, cb_user_data);
        }
    }

    void invoke_complete_task(
        const TF_TString* exec_id,
        const TF_TString* node_ref,
        TF_Bool success,
        const TF_TString* output_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    ) const
    {

        if (m_handle && m_handle->complete_task_cb) {
            m_handle->complete_task_cb(
                m_handle->ext, exec_id, node_ref, success, output_json, completion, cb_user_data
            );
        }
    }

    StringRuntime get_name() const
    {
        return m_handle ? StringRuntime{&m_handle->backend_name} : StringRuntime{};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Orchestrator* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Orchestrator* m_handle;
};

} // namespace ice::sonic
