// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/event.h"

export module cc_ice_extern_stream_executor_sonic:event_options;

import std;

export namespace ice::sonic {

class EventOptions
{
public:
    EventOptions() noexcept
    {
        m_value.struct_size = sizeof(::TF_EventOptions);
    }

    explicit EventOptions(const ::TF_EventOptions& value) noexcept :
        m_value{value}
    {
    }

    EventOptions(_Bool enable_timing, _Bool enable_ipc, _Bool reusable) noexcept
    {
        m_value.struct_size = sizeof(::TF_EventOptions);
        m_value.enable_timing = enable_timing;
        m_value.enable_ipc = enable_ipc;
        m_value.reusable = reusable;
    }

    _Bool getEnableTiming() const noexcept
    {
        return m_value.enable_timing;
    }

    void setEnableTiming(_Bool value) noexcept
    {
        m_value.enable_timing = value;
    }

    _Bool getEnableIpc() const noexcept
    {
        return m_value.enable_ipc;
    }

    void setEnableIpc(_Bool value) noexcept
    {
        m_value.enable_ipc = value;
    }

    _Bool getReusable() const noexcept
    {
        return m_value.reusable;
    }

    void setReusable(_Bool value) noexcept
    {
        m_value.reusable = value;
    }

    ::TF_EventOptions* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_EventOptions* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_EventOptions m_value{};
};

} // namespace ice::sonic
