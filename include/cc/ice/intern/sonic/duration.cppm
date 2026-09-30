// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/duration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/duration.h"

export module cc_ice_intern_sonic:duration;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_DurationOps : public ice::sonic::Runtime<TF_DurationOps, TF_DurationOps>
{
public:
    explicit TF_DurationOps(TF_DurationOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_ticks(int64_t* out_ticks) noexcept
    {
        m_ops->get_ticks(get_handle(), out_ticks);
    }

    void get_ratio_num(int64_t* out_num) noexcept
    {
        m_ops->get_ratio_num(get_handle(), out_num);
    }

    void get_ratio_den(int64_t* out_den) noexcept
    {
        m_ops->get_ratio_den(get_handle(), out_den);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
