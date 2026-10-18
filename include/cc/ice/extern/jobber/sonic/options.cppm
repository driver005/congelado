// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/options.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/options.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_jobber_sonic:options;

import std;
import :job;
import :job_options;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Options : public ice::sonic::Runtime<::TF_OptionsOps, ::TF_Options>
{
public:
    Options(
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

    Options(
        const ice::sonic::Registration& registry,
        const ::TF_Options& handle,
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

    explicit Options(const ::TF_OptionsOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Options(const ::TF_OptionsOps* ops, const ::TF_Options* handle) noexcept :
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
        const ice::sonic::Job& job,
        ice::sonic::JobOptions* out_options,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_options(
            get_handle(),
            job.get_handle(),
            out_options == nullptr ? nullptr : out_options->get_handle(),
            out_status.get_handle()
        );
    }

    void update_options(
        const ice::sonic::Job& job,
        const ice::sonic::JobOptions* new_options,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->update_options(
            get_handle(),
            job.get_handle(),
            new_options == nullptr ? nullptr : new_options->get_handle(),
            out_status.get_handle()
        );
    }

    void set_priority(
        const ice::sonic::Job& job,
        int priority,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_priority(get_handle(), job.get_handle(), priority, out_status.get_handle());
    }
};

} // namespace ice::sonic
