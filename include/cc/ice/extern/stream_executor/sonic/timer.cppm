// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/timer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/timer.h"

export module cc_ice_extern_stream_executor_sonic:timer;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_TimerOps : public ice::sonic::Runtime<::TF_TimerOps, ::TF_Timer>
{
public:
    TF_TimerOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_TimerOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Timer* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_TimerOps(const ::TF_TimerOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_TimerOps(const ::TF_TimerOps* ops, ::TF_Timer* handle) noexcept :
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

    void nanoseconds(uint64_t* out_nanoseconds) const noexcept
    {
        m_ops->nanoseconds(get_handle(), out_nanoseconds);
    }
};

} // namespace ice::sonic
