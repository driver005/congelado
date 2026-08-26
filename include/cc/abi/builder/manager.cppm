module;

#include "c/extern/manager.h"

export module cc_abi_builder:worker_manager;

import cc_abi_sonic_intern;

export namespace ice::builder {

class WorkerManagerBuilder
{
public:
    WorkerManagerBuilder() :
        m_handle{TP_WorkerManagerNew()}
    {
    }

    ~WorkerManagerBuilder()
    {
        TP_WorkerManagerDelete(m_handle);
    }

    WorkerManagerBuilder(const WorkerManagerBuilder&) = delete;
    WorkerManagerBuilder& operator=(const WorkerManagerBuilder&) = delete;

    WorkerManagerBuilder(WorkerManagerBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    WorkerManagerBuilder& operator=(WorkerManagerBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_WorkerManagerDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    WorkerManagerBuilder& set_required(TP_WorkerManager_RequiredFn callback)
    {

        TP_WorkerManager_SetRequiredCallback(m_handle, callback);
        return *this;
    }

    WorkerManagerBuilder& set_add_worker(TP_WorkerManager_AddWorkerFn callback)
    {

        TP_WorkerManager_SetAddWorkerCallback(m_handle, callback);
        return *this;
    }

    WorkerManagerBuilder& set_spawn(TP_WorkerManager_SpawnFn callback)
    {

        TP_WorkerManager_SetSpawnCallback(m_handle, callback);
        return *this;
    }

    WorkerManagerBuilder& set_start(TP_WorkerManager_StartFn callback)
    {

        TP_WorkerManager_SetStartCallback(m_handle, callback);
        return *this;
    }

    WorkerManagerBuilder& set_stop(TP_WorkerManager_StopFn callback)
    {

        TP_WorkerManager_SetStopCallback(m_handle, callback);
        return *this;
    }

    WorkerManagerBuilder& set_list(TP_WorkerManager_ListFn callback)
    {

        TP_WorkerManager_SetListCallback(m_handle, callback);
        return *this;
    }

    ice::sonic::StringRuntime get_name()
    {
        return ice::sonic::StringRuntime{&m_handle->backend_name};
    }

    // Underlying handle — pass directly to the C ABI
    TP_WorkerManager* get_handle()
    {
        return m_handle;
    }

    const TP_WorkerManager* get_handle() const
    {
        return m_handle;
    }

private:
    TP_WorkerManager* m_handle;
};

} // namespace ice::builder
