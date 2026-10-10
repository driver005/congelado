// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/duration.h"

export module cc_ice_intern_sonic:duration;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_DurationOps : public ice::sonic::Runtime<::TF_DurationOps, ::TF_Duration>
{
public:
    TF_DurationOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_DurationOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Duration* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_DurationOps(const ::TF_DurationOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_DurationOps(const ::TF_DurationOps* ops, ::TF_Duration* handle) noexcept :
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

    void get_ticks(int64_t* out_ticks) const noexcept
    {
        m_ops->get_ticks(get_handle(), out_ticks);
    }

    void get_ratio_num(int64_t* out_num) const noexcept
    {
        m_ops->get_ratio_num(get_handle(), out_num);
    }

    void get_ratio_den(int64_t* out_den) const noexcept
    {
        m_ops->get_ratio_den(get_handle(), out_den);
    }
};

} // namespace ice::sonic
