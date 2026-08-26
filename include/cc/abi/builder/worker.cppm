module;

#include "c/extern/worker.h"

export module cc_abi_builder:worker;

import cc_abi_sonic_intern;

export namespace ice::builder {

class WorkerBuilder
{
public:
    WorkerBuilder() :
        m_handle{TP_WorkerNew()}
    {
    }

    ~WorkerBuilder()
    {
        TP_WorkerDelete(m_handle);
    }

    WorkerBuilder(const WorkerBuilder&) = delete;
    WorkerBuilder& operator=(const WorkerBuilder&) = delete;

    WorkerBuilder(WorkerBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    WorkerBuilder& operator=(WorkerBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_WorkerDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    WorkerBuilder& set_get_task_type(TP_Worker_GetTaskTypeFn callback)
    {

        TP_Worker_SetGetTaskTypeCallback(m_handle, callback);
        return *this;
    }

    WorkerBuilder& set_execute(TP_Worker_ExecuteFn callback)
    {

        TP_Worker_SetExecuteCallback(m_handle, callback);
        return *this;
    }

    WorkerBuilder& set_execute_async(TP_Worker_ExecuteAsyncFn callback)
    {

        TP_Worker_SetExecuteAsyncCallback(m_handle, callback);
        return *this;
    }

    WorkerBuilder& set_on_error(TP_Worker_OnErrorFn callback)
    {

        TP_Worker_SetOnErrorCallback(m_handle, callback);
        return *this;
    }

    WorkerBuilder& set_on_released(TP_Worker_OnReleasedFn callback)
    {

        TP_Worker_SetOnReleasedCallback(m_handle, callback);
        return *this;
    }

    ice::sonic::StringRuntime get_name()
    {
        return ice::sonic::StringRuntime{&m_handle->task_type};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Worker* get_handle()
    {
        return m_handle;
    }

    const TP_Worker* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Worker* m_handle;
};

} // namespace ice::builder
