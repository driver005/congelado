// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/jobber.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/jobber.h"

export module cc_ice_extern_jobber_sonic:jobber;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_JobberOps : public ice::sonic::Runtime<::TF_JobberOps, ::TF_Jobber>
{
public:
    template<typename Registry>
    TF_JobberOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_JobberOps(
        Registry& registry,
        ::TF_Jobber* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_JobberOps(const ::TF_JobberOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_JobberOps(const ::TF_JobberOps* ops, ::TF_Jobber* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }
};

} // namespace ice::sonic
