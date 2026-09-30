// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/time_point.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/time_point.h"

export module cc_ice_intern_sonic:time_point;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TimePointOps : public ice::sonic::Runtime<TF_TimePointOps, TF_TimePointOps>
{
public:
    explicit TF_TimePointOps(TF_TimePointOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_duration_since_epoch(const ice::sonic::TF_DurationOps& out_duration) noexcept
    {
        m_ops->get_duration_since_epoch(get_handle(), out_duration.get_handle());
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
