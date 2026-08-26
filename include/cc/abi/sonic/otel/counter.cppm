module;

#include "c/extern/otel/counter.h"

export module cc_abi_sonic_otel:counter;

export namespace ice::sonic {

// Non-owning wrapper around a `TP_Otel_Counter*` (e.g. returned by
// MeterRuntime::invoke_create_counter).
class CounterRuntime
{
public:
    CounterRuntime() :
        m_handle{nullptr}
    {
    }

    explicit CounterRuntime(TP_Otel_Counter* handle) :
        m_handle{handle}
    {
    }

    void invoke_add(double value) const
    {

        if (m_handle && m_handle->add_cb) {
            m_handle->add_cb(m_handle->ext, value);
        }
    }

    // Underlying handle — pass directly to the C ABI
    TP_Otel_Counter* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Otel_Counter* m_handle;
};

} // namespace ice::sonic
