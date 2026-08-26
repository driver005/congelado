module;

#include "c/extern/orchestrator.h"

export module cc_abi_builder:worker_orchestrator;

import cc_abi_sonic_intern;

export namespace ice::builder {

class OrchestratorBuilder
{
public:
    OrchestratorBuilder() :
        m_handle{TP_OrchestratorNew()}
    {
    }

    ~OrchestratorBuilder()
    {
        TP_OrchestratorDelete(m_handle);
    }

    OrchestratorBuilder(const OrchestratorBuilder&) = delete;
    OrchestratorBuilder& operator=(const OrchestratorBuilder&) = delete;

    OrchestratorBuilder(OrchestratorBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    OrchestratorBuilder& operator=(OrchestratorBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_OrchestratorDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    OrchestratorBuilder& set_required(TP_Orchestrator_RequiredFn callback)
    {

        TP_Orchestrator_SetRequiredCallback(m_handle, callback);
        return *this;
    }

    OrchestratorBuilder& set_start_workflow(TP_Orchestrator_StartWorkflowFn callback)
    {

        TP_Orchestrator_SetStartWorkflowCallback(m_handle, callback);
        return *this;
    }

    OrchestratorBuilder& set_pause(TP_Orchestrator_PauseFn callback)
    {

        TP_Orchestrator_SetPauseCallback(m_handle, callback);
        return *this;
    }

    OrchestratorBuilder& set_resume(TP_Orchestrator_ResumeFn callback)
    {

        TP_Orchestrator_SetResumeCallback(m_handle, callback);
        return *this;
    }

    OrchestratorBuilder& set_terminate(TP_Orchestrator_TerminateFn callback)
    {

        TP_Orchestrator_SetTerminateCallback(m_handle, callback);
        return *this;
    }

    OrchestratorBuilder& set_complete_task(TP_Orchestrator_CompleteTaskFn callback)
    {

        TP_Orchestrator_SetCompleteTaskCallback(m_handle, callback);
        return *this;
    }

    ice::sonic::StringRuntime get_name()
    {
        return ice::sonic::StringRuntime{&m_handle->backend_name};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Orchestrator* get_handle()
    {
        return m_handle;
    }

    const TP_Orchestrator* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Orchestrator* m_handle;
};

} // namespace ice::builder
