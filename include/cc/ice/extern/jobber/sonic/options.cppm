// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/options.h"

export module cc_ice_extern_jobber_sonic:options;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_OptionsOps : public ice::sonic::Runtime<::TF_OptionsOps, ::TF_Options>
{
public:
    template<typename Registry>
    TF_OptionsOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_OptionsOps(
        Registry& registry,
        ::TF_Options* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_OptionsOps(const ::TF_OptionsOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_OptionsOps(const ::TF_OptionsOps* ops, ::TF_Options* handle) noexcept :
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

    void get_options(
        const ice::sonic::TF_JobOps& job,
        TFJobOptions* out_options,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_options(get_handle(), job.get_handle(), out_options, out_status.get_handle());
    }

    void update_options(
        const ice::sonic::TF_JobOps& job,
        const TFJobOptions* new_options,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->update_options(get_handle(), job.get_handle(), new_options, out_status.get_handle());
    }

    void set_priority(
        const ice::sonic::TF_JobOps& job,
        int priority,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_priority(get_handle(), job.get_handle(), priority, out_status.get_handle());
    }
};

} // namespace ice::sonic
