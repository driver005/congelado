// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/event.h"

export module cc_ice_extern_stream_executor_sonic:event;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_EventOps : public ice::sonic::Runtime<::TF_EventOps, ::TF_Event>
{
public:
    template<typename Registry>
    TF_EventOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_EventOps(
        Registry& registry,
        ::TF_Event* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_EventOps(const ::TF_EventOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_EventOps(const ::TF_EventOps* ops, ::TF_Event* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void elapsed_time(
        const ice::sonic::TF_EventOps& end,
        float* out_milliseconds,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->elapsed_time(
            get_handle(),
            end.get_handle(),
            out_milliseconds,
            out_status.get_handle()
        );
    }

    void export_ipc(
        TF_IpcEventHandle* out_handle,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->export_ipc(get_handle(), out_handle, out_status.get_handle());
    }

    void get_native_handle(void** out_handle) const noexcept
    {
        m_ops->get_native_handle(get_handle(), out_handle);
    }
};

} // namespace ice::sonic
