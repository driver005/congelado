// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/time_point.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/time_point.h"

export module cc_ice_intern_sonic:time_point;

import std;
import :duration;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_TimePointOps : public ice::sonic::Runtime<::TF_TimePointOps, ::TF_TimePoint>
{
public:
    TF_TimePointOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_TimePointOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_TimePoint* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_TimePointOps(const ::TF_TimePointOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_TimePointOps(const ::TF_TimePointOps* ops, ::TF_TimePoint* handle) noexcept :
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

    void get_duration_since_epoch(const ice::sonic::TF_DurationOps& out_duration) const noexcept
    {
        m_ops->get_duration_since_epoch(get_handle(), out_duration.get_handle());
    }
};

} // namespace ice::sonic
