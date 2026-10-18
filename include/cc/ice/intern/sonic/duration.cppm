// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/duration.h"
#include "include/c/intern/registration.h"

export module cc_ice_intern_sonic:duration;

import std;
import :registration;
import :runtime;
import :tstring;

export namespace ice::sonic {

class Duration : public ice::sonic::Runtime<::TF_DurationOps, ::TF_Duration>
{
public:
    Duration(
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

    Duration(
        const ice::sonic::Registration& registry,
        const ::TF_Duration& handle,
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

    explicit Duration(const ::TF_DurationOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Duration(const ::TF_DurationOps* ops, const ::TF_Duration* handle) noexcept :
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

    void get_ticks(std::int64_t& out_ticks) const noexcept
    {
        m_ops->get_ticks(get_handle(), &out_ticks);
    }

    void get_ratio_num(std::int64_t& out_num) const noexcept
    {
        m_ops->get_ratio_num(get_handle(), &out_num);
    }

    void get_ratio_den(std::int64_t& out_den) const noexcept
    {
        m_ops->get_ratio_den(get_handle(), &out_den);
    }
};

} // namespace ice::sonic
