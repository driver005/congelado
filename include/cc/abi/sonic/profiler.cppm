module;

#include "c/extern/profiler.h"

export module cc_abi_sonic:profiler;

export namespace ice::sonic {

// Non-owning wrapper around a `TP_ProfilerFns*` handed to the mainframe. Unlike every other
// capability here, TP_ProfilerFns's callbacks take the `TP_Profiler*` instance directly as
// their first argument rather than the registration struct's own `ext` — so each invoke
// method here takes that profiler instance explicitly.
class ProfilerFnsRuntime
{
public:
    ProfilerFnsRuntime() :
        m_handle{nullptr}
    {
    }

    explicit ProfilerFnsRuntime(TP_ProfilerFns* handle) :
        m_handle{handle}
    {
    }

    void invoke_start(const TP_Profiler* profiler, TF_Status* status) const
    {

        if (m_handle && m_handle->start) {
            m_handle->start(profiler, status);
        }
    }

    void invoke_stop(const TP_Profiler* profiler, TF_Status* status) const
    {

        if (m_handle && m_handle->stop) {
            m_handle->stop(profiler, status);
        }
    }

    void invoke_collect_data_xspace(
        const TP_Profiler* profiler, uint8_t* buffer, size_t* size_in_bytes, TF_Status* status
    ) const
    {

        if (m_handle && m_handle->collect_data_xspace) {
            m_handle->collect_data_xspace(profiler, buffer, size_in_bytes, status);
        }
    }

    // Underlying handle — pass directly to the C ABI
    TP_ProfilerFns* get_handle() const
    {
        return m_handle;
    }

private:
    TP_ProfilerFns* m_handle;
};

} // namespace ice::sonic
