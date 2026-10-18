// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/observe.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/observe.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_jobber_sonic:observe;

import std;
import :job;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Observe : public ice::sonic::Runtime<::TF_ObserveOps, ::TF_Observe>
{
public:
    Observe(
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

    Observe(
        const ice::sonic::Registration& registry,
        const ::TF_Observe& handle,
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

    explicit Observe(const ::TF_ObserveOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Observe(const ::TF_ObserveOps* ops, const ::TF_Observe* handle) noexcept :
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

    void get_status(
        const ice::sonic::Job& job,
        TFObserveStatusFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_status(
            get_handle(),
            job.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void get_result(
        const ice::sonic::Job& job,
        TFObserveResultFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_result(
            get_handle(),
            job.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void get_history(
        const ice::sonic::Job& job,
        const ice::sonic::Vector& out_transitions,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_history(
            get_handle(),
            job.get_handle(),
            out_transitions.get_handle(),
            out_status.get_handle()
        );
    }

    void get_metrics(
        const ice::sonic::Job& job,
        const ice::sonic::Map& out_metrics,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_metrics(
            get_handle(),
            job.get_handle(),
            out_metrics.get_handle(),
            out_status.get_handle()
        );
    }

    void get_logs(
        const ice::sonic::Job& job,
        const ice::sonic::Vector& out_lines,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_logs(
            get_handle(),
            job.get_handle(),
            out_lines.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
