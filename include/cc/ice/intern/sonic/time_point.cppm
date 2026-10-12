// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/time_point.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/registration.h"
#include "include/c/intern/time_point.h"

export module cc_ice_intern_sonic:time_point;

import std;
import :duration;
import :registration;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TimePoint : public ice::sonic::Runtime<::TF_TimePointOps, ::TF_TimePoint>
{
public:
    TimePoint(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    TimePoint(
        const ice::sonic::Registration& registry,
        ::TF_TimePoint* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit TimePoint(const ::TF_TimePointOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TimePoint(const ::TF_TimePointOps* ops, ::TF_TimePoint* handle) noexcept :
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

    void get_duration_since_epoch(const ice::sonic::Duration& out_duration) const noexcept
    {
        m_ops->get_duration_since_epoch(get_handle(), out_duration.get_handle());
    }
};

} // namespace ice::sonic
