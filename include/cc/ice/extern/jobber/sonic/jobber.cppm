// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/jobber.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/jobber.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_jobber_sonic:jobber;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Jobber : public ice::sonic::Runtime<::TF_JobberOps, ::TF_Jobber>
{
public:
    Jobber(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    Jobber(
        const ice::sonic::Registration& registry,
        const ::TF_Jobber& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit Jobber(const ::TF_JobberOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Jobber(const ::TF_JobberOps* ops, const ::TF_Jobber* handle) noexcept :
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
